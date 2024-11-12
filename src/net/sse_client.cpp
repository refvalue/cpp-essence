/*
 * Copyright (c) 2024 The RefValue Project
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "net/sse_client.hpp"

#include "char8_t_remediation.hpp"
#include "delegate.hpp"
#include "format_remediation.hpp"
#include "sse_types.hpp"
#include "string.hpp"
#include "util.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <exception>
#include <functional>
#include <mutex>
#include <optional>
#include <string_view>
#include <thread>

#include <cpprest/http_client.h>

namespace essence::net {
    namespace {
        using string_rw_buffer = concurrency::streams::container_buffer<std::string>;

        enum class sse_client_stage {
            idle,
            connecting,
            connected,
        };

        void parse_line(std::string_view content, sse_message& message) {
            static constexpr std::array prefixes{sse_field_prefixes::data, sse_field_prefixes::last_event_id};

            if (content.starts_with(sse_field_prefixes::comment)) {
                return;
            }

            // Matches one prefix and trims the leading whitespaces of the content.
            auto match_prefix = [&]() -> std::optional<std::pair<std::string_view, std::string_view>> {
                if (const auto iter = std::ranges::find_if(
                        prefixes, [&](std::string_view inner) { return content.starts_with(inner); });
                    iter != prefixes.end()) {
                    content.remove_prefix(iter->size());

                    return std::pair{*iter, trim_left(content)};
                }

                return std::nullopt;
            };

            if (const auto pair = match_prefix()) {
                if (pair->first == sse_field_prefixes::data) {
                    message.data.emplace_back(content);
                } else if (pair->first == sse_field_prefixes::last_event_id) {
                    message.last_event_id = content;
                }
            }
        }
    } // namespace

    class sse_client::impl {
    public:
        explicit impl(const uri& base_uri) : impl{base_uri, http_client_config::get_default_no_timeout()} {}

        impl(const uri& base_uri, const http_client_config& config)
            : timeout_{config.timeout}, stage_{sse_client_stage::idle},
              client_{make_cpprest_uri(base_uri), make_cpprest_http_client_config(config, true)} {}

        ~impl() {
            close();
        }

        void connect(const uri& relative_uri) {
            close();

            web::http::http_request request{web::http::methods::GET};

            request.set_request_uri(make_cpprest_uri(relative_uri));
            request.headers().add(web::http::header_names::accept, U("text/event-stream"));
            request.headers().add(web::http::header_names::cache_control, U("no-store"));
            request.headers().add(web::http::header_names::connection, U("keep-alive"));

            auto token = cancellation_token_source_.get_token();

            request_task_ = client_.request(request, token)
                                .then(std::bind_front(&impl::parse_stream, this, token))
                                .then(std::bind_front(&impl::raise_error, this));

            stage_.store(sse_client_stage::connecting, std::memory_order::release);

            if (timeout_ != http_client_config::no_timeout) {
                deadline_task_ = pplx::create_task(std::bind_front(&impl::deadline_control, this, token));
            }
        }

        void close() {
            cancellation_token_source_.cancel();
            cv_deadline_.notify_all();

            if (request_task_) {
                static_cast<void>(request_task_->wait());
                request_task_.reset();
            }

            if (deadline_task_) {
                static_cast<void>(deadline_task_->wait());
                request_task_.reset();
            }

            cancellation_token_source_ = {};
        }

        void on_message(const sse_message_handler& handler) {
            on_message_ += handler;
        }

        void on_error(const error_handler& handler) {
            on_error_ += handler;
        }

    private:
        /**
         * @brief Implements the timeout mechanism.
         */
        void deadline_control(const pplx::cancellation_token& token) {
            std::unique_lock lock{mutex_};
            const auto stop_waiting = cv_deadline_.wait_for(lock, std::chrono::seconds{timeout_}, [&] {
                return stage_.load(std::memory_order::acquire) != sse_client_stage::connecting || token.is_canceled();
            });

            // Time expires.
            if (!stop_waiting) {
                on_error_.try_invoke(format(U8("SSE timeout: {} second(s)."), timeout_));
                cancellation_token_source_.cancel();
            }
        }

        void raise_error(const pplx::task<void>& task) {
            try {
                static_cast<void>(task.wait());
            } catch (const std::exception& ex) {
                on_error_(ex.what());
            }

            stage_.store(sse_client_stage::idle, std::memory_order::release);
            cv_deadline_.notify_all();
        }

        void parse_stream(const pplx::cancellation_token& token, const web::http::http_response& response) {
            // https://html.spec.whatwg.org/multipage/server-sent-events.html#parsing-an-event-stream
            // All streams and streambufs are reference-counted and can be copied fast.
            ensure_status_code(response);
            stage_.store(sse_client_stage::connected, std::memory_order::release);

            string_rw_buffer rw_buffer;
            const auto net_stream    = response.body();
            auto wait_data_available = [=] {
                while (net_stream.streambuf().getc().get() == EOF && !token.is_canceled()) {
                    std::this_thread::sleep_for(std::chrono::milliseconds{500});
                }

                return !token.is_canceled();
            };

            sse_message message;

            while (!token.is_canceled()) {
                const auto size_read = net_stream.read_line(rw_buffer).get();

                // Touching the end of the stream, starts waiting for more data available.
                if (net_stream.is_eof()) {
                    if (!wait_data_available()) {
                        return;
                    }
                }

                // An empty line indicates the end of the current event.
                if (size_read == 0) {
                    on_message_.try_invoke(message);
                    message.data.clear();
                    message.last_event_id.clear();
                } else {
                    parse_line(rw_buffer.collection(), message);
                }

                rw_buffer.collection().clear();
                rw_buffer.seekpos(0, std::ios_base::in);
                rw_buffer.seekpos(0, std::ios_base::out);
                rw_buffer.set_buffer_size(0, std::ios_base::in);
                rw_buffer.set_buffer_size(0, std::ios_base::out);
            }
        }

        delegate<error_handler> on_error_;
        delegate<sse_message_handler> on_message_;

        std::uint32_t timeout_;
        mutable std::mutex mutex_;
        std::condition_variable cv_deadline_;
        std::atomic<sse_client_stage> stage_;
        std::optional<pplx::task<void>> request_task_;
        std::optional<pplx::task<void>> deadline_task_;
        pplx::cancellation_token_source cancellation_token_source_;
        web::http::client::http_client client_;
    };

    sse_client::sse_client(const uri& base_uri) : impl_{std::make_unique<impl>(base_uri)} {}

    sse_client::sse_client(const uri& base_uri, const http_client_config& config)
        : impl_{std::make_unique<impl>(base_uri, config)} {}

    sse_client::sse_client(sse_client&&) noexcept = default;

    sse_client::~sse_client() = default;

    sse_client& sse_client::operator=(sse_client&&) noexcept = default;

    void sse_client::connect(const uri& relative_uri) const {
        impl_->connect(relative_uri);
    }

    void sse_client::close() const {
        impl_->close();
    }

    void sse_client::on_message(const sse_message_handler& handler) const {
        impl_->on_message(handler);
    }

    void sse_client::on_error(const error_handler& handler) const {
        impl_->on_error(handler);
    }
} // namespace essence::net
