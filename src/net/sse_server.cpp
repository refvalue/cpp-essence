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

#include "net/sse_server.hpp"

#include "delegate.hpp"
#include "encoding.hpp"
#include "net/cors.hpp"
#include "sse_types.hpp"
#include "util.hpp"

#include <cstdint>
#include <functional>
#include <optional>
#include <stop_token>
#include <utility>

#include <cpprest/http_listener.h>
#include <cpprest/producerconsumerstream.h>

namespace essence::net {
    namespace {
        class sse_connection_impl {
        public:
            sse_connection_impl(
                const web::http::http_request& request, const error_handler& on_error, const std::stop_token& token)
                : fields_{std::make_unique<private_fields>(request, token, on_error)} {
                web::http::http_response response{web::http::status_codes::OK};

                response.headers().set_content_type(_XPLATSTR("text/event-stream"));
                response.headers().set_cache_control(_XPLATSTR("no-store"));
                response.set_body(concurrency::streams::basic_istream<std::uint8_t>{
                    static_cast<Concurrency::streams::streambuf<char>>(fields_->rw_buffer)});
                make_cors_any(response);

                // Avoids binding "this" here to keep fields valid after moving.
                fields_->response_task = fields_->request.reply(response).then(
                    std::bind_front(&sse_connection_impl::raise_error, fields_.get()));
            }

            sse_connection_impl(sse_connection_impl&&) noexcept = default;

            ~sse_connection_impl() {
                close();
            }

            sse_connection_impl& operator=(sse_connection_impl&&) noexcept = default;

            [[nodiscard]] [[maybe_unused]] bool canceled() const noexcept {
                return fields_->token.stop_requested() || fields_->close_flag.stop_requested();
            }

            [[nodiscard]] [[maybe_unused]] uri request_uri() const {
                return make_uri(fields_->request.request_uri());
            }

            [[nodiscard]] [[maybe_unused]] abi::string remote_address() const {
                return to_utf8_string(fields_->request.remote_address());
            }

            [[maybe_unused]] void send_message(const sse_message& message) const {
                auto&& rw_buffer     = fields_->rw_buffer;
                auto put_data_prefix = [&] {
                    static_cast<void>(
                        rw_buffer.putn_nocopy(sse_field_prefixes::data.data(), sse_field_prefixes::data.size()).wait());
                };

                // Puts the data collection and joins them with the ASCII character LF.
                for (auto&& item : message.data) {
                    put_data_prefix();
                    static_cast<void>(rw_buffer.putn_nocopy(item.c_str(), item.size()).wait());
                    put_new_line();
                }

                // An event with empty data.
                if (message.data.empty()) {
                    put_data_prefix();
                    put_new_line();
                }

                // Puts the last event ID if not empty.
                if (!message.last_event_id.empty()) {
                    static_cast<void>(rw_buffer
                                          .putn_nocopy(sse_field_prefixes::last_event_id.data(),
                                              sse_field_prefixes::last_event_id.size())
                                          .wait());
                    static_cast<void>(
                        rw_buffer.putn_nocopy(message.last_event_id.c_str(), message.last_event_id.size()).wait());
                    put_new_line();
                }

                // A blank line indicates the end of the event.
                put_new_line();
                static_cast<void>(rw_buffer.sync().wait());
            }

            [[maybe_unused]] void tick() const {
                // A comment to keep alive.
                fields_->rw_buffer.putn_nocopy(sse_field_prefixes::comment.data(), sse_field_prefixes::comment.size());
                put_new_line();
                put_new_line();
                static_cast<void>(fields_->rw_buffer.sync().wait());
            }

            void close() const {
                if (fields_) {
                    static_cast<void>(fields_->rw_buffer.close(std::ios_base::out).wait());
                    static_cast<void>(fields_->close_flag.request_stop());

                    if (fields_->response_task) {
                        static_cast<void>(fields_->response_task->wait());
                        fields_->response_task.reset();
                    }
                }
            }

        private:
            void put_new_line() const {
                static_cast<void>(fields_->rw_buffer.putc(U8('\n')).wait());
            }

            struct private_fields {
                web::http::http_request request;
                std::stop_token token;
                error_handler on_error;
                std::stop_source close_flag;
                std::optional<pplx::task<void>> response_task;
                concurrency::streams::producer_consumer_buffer<char> rw_buffer;
            };

            static void raise_error(private_fields* fields, const pplx::task<void>& task) {
                try {
                    static_cast<void>(task.wait());
                } catch (const std::exception& ex) {
                    static_cast<void>(fields->close_flag.request_stop());
                    fields->on_error(ex.what());
                }
            }

            std::unique_ptr<private_fields> fields_;
        };
    } // namespace

    class sse_server::impl {
    public:
        explicit impl(const uri& listen_uri) : impl{listen_uri, http_listener_config::get_default_no_timeout()} {}

        impl(const uri& listen_uri, const http_listener_config& config)
            : listener_{make_cpprest_uri(listen_uri), make_cpprest_http_listener_config(config, true)} {
            listener_.support(web::http::methods::GET, std::bind_front(&impl::handle_request, this));
            listener_.support(web::http::methods::OPTIONS, [](const web::http::http_request& request) {
                web::http::http_response response{web::http::status_codes::OK};

                // Supports CROS from any origin.
                make_cors_any(response);
                response.set_status_code(web::http::status_codes::OK);
                static_cast<void>(request.reply(response));
            });
        }

        ~impl() {
            close();
        }

        void open() {
            close();
            static_cast<void>(listener_.open().wait());
        }

        void close() {
            static_cast<void>(stop_source_.request_stop());

            // As a safeguard, closes the listener if not already done.
            // Users are required to call close, but this is just a safeguard.
            try {
                static_cast<void>(listener_.close().wait());
            } catch (...) {
            }

            std::stop_source{}.swap(stop_source_);
        }

        void on_connection(const sse_connection_handler& handler) {
            on_connection_ += handler;
        }

        void on_error(const error_handler& handler) {
            on_error_ += handler;
        }

    private:
        void handle_request(const web::http::http_request& request) const {
            on_connection_.try_invoke(abstract::sse_connection{
                sse_connection_impl{request, on_error_.to_nothrow_function(), stop_source_.get_token()}});
        }

        std::stop_source stop_source_;
        delegate<error_handler> on_error_;
        delegate<sse_connection_handler> on_connection_;
        web::http::experimental::listener::http_listener listener_;
    };

    sse_server::sse_server(const uri& listen_uri) : impl_{std::make_unique<impl>(listen_uri)} {}

    sse_server::sse_server(const uri& listen_uri, const http_listener_config& config)
        : impl_{std::make_unique<impl>(listen_uri, config)} {}

    sse_server::sse_server(sse_server&&) noexcept = default;

    sse_server::~sse_server() = default;

    sse_server& sse_server::operator=(sse_server&&) noexcept = default;

    void sse_server::open() const {
        impl_->open();
    }

    void sse_server::close() const {
        impl_->close();
    }

    void sse_server::on_connection(const sse_connection_handler& handler) const {
        impl_->on_connection(handler);
    }

    void sse_server::on_error(const error_handler& handler) const {
        impl_->on_error(handler);
    }
} // namespace essence::net
