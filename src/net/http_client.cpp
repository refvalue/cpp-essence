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

#include "net/http_client.hpp"

#include "char8_t_remediation.hpp"
#include "delegate.hpp"
#include "encoding.hpp"
#include "error_extensions.hpp"
#include "exception.hpp"
#include "functional.hpp"
#include "net/download.hpp"
#include "net_error_impl.hpp"
#include "range.hpp"
#include "util.hpp"

#include <atomic>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <cpprest/http_client.h>

namespace essence::net {
    namespace {
        template <typename T>
        using atomic_optional = std::atomic<std::optional<T>>;

        constexpr std::string_view json_content_type{U8("application/json; charset=utf-8")};

        web::http::method map_http_method(http_method method) {
            switch (method) {
            case http_method::put:
                return web::http::methods::PUT;
            case http_method::post:
                return web::http::methods::POST;
            case http_method::get:
                return web::http::methods::GET;
            }

            return web::http::methods::POST;
        }

        pplx::task<abi::vector<std::byte>> extract_body(
            atomic_optional<std::uint64_t>& content_size, const pplx::task<web::http::http_response>& response) {
            return response
                .then([&content_size](const web::http::http_response& response) {
                    ensure_status_code(response);

                    // Assigns the detected content length of the HTTP request to the background.
                    if (auto content_length = response.headers().content_length();
                        response.headers().content_length() != 0) {
                        content_size.store(content_length, std::memory_order::release);
                    }

                    return response.content_ready();
                })
                .then([](const web::http::http_response& response) {
                    auto buffer = response.body().streambuf();
                    abi::vector<std::byte> result(buffer.in_avail());
                    auto size_read = buffer.getn(reinterpret_cast<std::uint8_t*>(result.data()), result.size()).get();

                    if (result.size() != size_read) {
                        throw source_code_aware_runtime_error{U8("Expected Size"), result.size(), U8("Size Read"),
                            size_read, U8("HTTP Download Error"),
                            U8("The expected size is unequal to the size read (in bytes).")};
                    }

                    return result;
                });
        }

        pplx::task<abi::json> extract_json(
            atomic_optional<std::uint64_t>& content_size, const pplx::task<web::http::http_response>& response) {
            return extract_body(content_size, response).then([](std::span<const std::byte> bytes) {
                auto json = abi::json::parse(bytes, nullptr, false, true);

                if (json.is_discarded()) {
                    throw source_code_aware_runtime_error{
                        U8("HTTP Content Error"), U8("The server returns an invalid JSON.")};
                }

                return json;
            });
        }

        void set_progress_handlers(web::http::http_request& request, const http_progress_handlers& handlers,
            const atomic_optional<std::uint64_t>& upload_content_size,
            const atomic_optional<std::uint64_t>& download_content_size) {
            // TODO: calculates the upload content size.
            if (handlers.on_progress || handlers.on_percentage) {
                request.set_progress_handler(
                    [&, handlers](web::http::message_direction::direction direction, utility::size64_t bytes_read) {
                        auto inner_direction = static_cast<message_direction>(direction);
                        auto content_size    = direction == web::http::message_direction::upload
                                                 ? upload_content_size.load(std::memory_order::acquire)
                                                 : download_content_size.load(std::memory_order::acquire);
                        invoke_optional(handlers.on_progress, inner_direction, bytes_read, content_size);
                        invoke_optional(handlers.on_percentage, inner_direction,
                            content_size ? static_cast<float>(static_cast<double>(bytes_read) * 100.0
                                                              / static_cast<double>(*content_size))
                                         : 0.0f);
                    });
            }
        }
    } // namespace

    class http_client::impl {
    public:
        explicit impl(const uri& base_uri) : impl{base_uri, http_client_config::get_default()} {}

        impl(const uri& base_uri, const http_client_config& config)
            : base_uri_{base_uri}, client_{make_cpprest_uri(base_uri), make_cpprest_http_client_config(config, false)} {
        }

        const uri& base_uri() const {
            return base_uri_;
        }

        abi::json commit_json_nop(
            http_method method, const uri& relative_uri, const http_header_handler& header_handler) {
            return commit_bytes_impl(method, relative_uri, std::nullopt, header_handler);
        }

        abi::json commit_json(http_method method, const uri& relative_uri, const abi::json& params,
            const http_header_handler& header_handler) {
            return commit_bytes_impl(
                method, relative_uri, std::pair{json_content_type, as_const_byte_span(params.dump())}, header_handler);
        }

        abi::json commit_bytes(http_method method, const uri& relative_uri, std::string_view content_type,
            std::span<const std::byte> bytes, const http_header_handler& header_handler) {
            return commit_bytes_impl(method, relative_uri, std::pair{content_type, bytes}, header_handler);
        }

        void on_progress(const http_progress_handler& handler) {
            on_progress_ += handler;
        }

        void on_percentage(const http_percentage_handler& handler) {
            on_percentage_ += handler;
        }

    private:
        abi::json commit_bytes_impl(http_method method, const uri& relative_uri,
            std::optional<std::pair<std::string_view, std::span<const std::byte>>> content_type_and_bytes,
            const http_header_handler& header_handler) {
            return throw_nested_and_flatten(
                net_error_impl{U8("HTTP Method"), to_string(method), U8("Base URI"), base_uri_.str(),
                    U8("Relative URI"), relative_uri.str(), U8("Content Type"),
                    content_type_and_bytes ? content_type_and_bytes->first : U8("Unknown")},
                [&] {
                    web::http::http_request request{map_http_method(method)};
                    http_headers_proxy headers{request.headers()};
                    uri absolute_uri{internal::to_utf8_string(web::uri_builder{make_cpprest_uri(base_uri_)}
                                                                  .append(make_cpprest_uri(relative_uri))
                                                                  .to_string())};

                    request.set_request_uri(make_cpprest_uri(relative_uri));

                    if (const auto content_type_and_input_stream =
                            [&]() -> std::optional<std::pair<std::string_view, concurrency::streams::istream>> {
                            if (!content_type_and_bytes) {
                                return std::nullopt;
                            }

                            auto&& [content_type, bytes] = *content_type_and_bytes;
                            auto input_stream            = concurrency::streams::bytestream::open_istream(
                                std::vector<std::uint8_t>{reinterpret_cast<const std::uint8_t*>(bytes.data()),
                                               reinterpret_cast<const std::uint8_t*>(bytes.data()) + bytes.size()});

                            return std::pair{content_type, input_stream};
                        }()) {
                        request.set_body(content_type_and_input_stream->second,
                            internal::to_native_string(content_type_and_input_stream->first));
                    }

                    invoke_optional(header_handler, absolute_uri, headers);
                    set_progress_handlers(request,
                        {
                            .on_progress   = on_progress_.to_nothrow_function(),
                            .on_percentage = on_percentage_.to_nothrow_function(),
                        },
                        upload_content_size_, download_content_size_);

                    return extract_json(download_content_size_, client_.request(request)).get();
                });
        }

        uri base_uri_;
        web::http::client::http_client client_;
        delegate<http_progress_handler> on_progress_;
        delegate<http_percentage_handler> on_percentage_;
        atomic_optional<std::uint64_t> upload_content_size_;
        atomic_optional<std::uint64_t> download_content_size_;
    };

    http_client::http_client(const uri& base_uri) : impl_{std::make_unique<impl>(base_uri)} {}

    http_client::http_client(const uri& base_uri, const http_client_config& config)
        : impl_{std::make_unique<impl>(base_uri, config)} {}

    http_client::http_client(http_client&&) noexcept = default;

    http_client::~http_client() = default;

    http_client& http_client::operator=(http_client&&) noexcept = default;

    const uri& http_client::base_uri() const {
        return impl_->base_uri();
    }

    abi::json http_client::commit_json_nop(
        http_method method, const uri& relative_uri, const http_header_handler& header_handler) const {
        return impl_->commit_json_nop(method, relative_uri, header_handler);
    }

    abi::json http_client::commit_json(http_method method, const uri& relative_uri, const abi::json& params,
        const http_header_handler& header_handler) const {
        return impl_->commit_json(method, relative_uri, params, header_handler);
    }

    abi::json http_client::commit_bytes(http_method method, const uri& relative_uri, std::string_view content_type,
        std::span<const std::byte> bytes, const http_header_handler& header_handler) const {
        return impl_->commit_bytes(method, relative_uri, content_type, bytes, header_handler);
    }

    void http_client::on_progress(const http_progress_handler& handler) const {
        impl_->on_progress(handler);
    }

    void http_client::on_percentage(const http_percentage_handler& handler) const {
        impl_->on_percentage(handler);
    }

    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_header_handler& header_handler) {
        return download_file(absolute_uri, http_client_config::get_default(), header_handler);
    }

    abi::vector<std::byte> download_file(
        const uri& absolute_uri, const http_client_config& config, const http_header_handler& header_handler) {
        return download_file(absolute_uri, config, {}, header_handler);
    }

    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_progress_handlers& progress_handlers,
        const http_header_handler& header_handler) {
        return download_file(absolute_uri, http_client_config::get_default(), progress_handlers, header_handler);
    }

    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_client_config& config,
        const http_progress_handlers& progress_handlers, const http_header_handler& header_handler) {
        return throw_nested_and_flatten(net_error_impl{U8("Absolute URI"), absolute_uri.str()}, [&] {
            web::http::client::http_client_config native_config;
            web::http::client::http_client client{make_cpprest_uri(absolute_uri), config.assign_to(native_config)};

            web::http::http_request request{web::http::methods::GET};
            http_headers_proxy headers{request.headers()};
            const atomic_optional<std::uint64_t> upload_content_size;
            atomic_optional<std::uint64_t> download_content_size;

            invoke_optional(header_handler, absolute_uri, headers);
            set_progress_handlers(request, progress_handlers, upload_content_size, download_content_size);

            return extract_body(download_content_size, client.request(request)).get();
        });
    }
} // namespace essence::net
