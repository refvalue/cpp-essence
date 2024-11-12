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

#pragma once

#include "../abi/json.hpp"
#include "../error_extensions.hpp"
#include "../exception.hpp"
#include "../meta/identifier.hpp"
#include "../range.hpp"
#include "common_types.hpp"
#include "http_headers_proxy.hpp"
#include "rest_api.hpp"
#include "uri.hpp"

#include <concepts>
#include <exception>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace essence::net {
    template <typename Impl>
    class http_client_abstract {
    public:
        friend Impl;

        [[nodiscard]] const uri& base_uri() const {
            return self().base_uri();
        }

        [[nodiscard]] abi::json commit_json_nop(
            http_method method, const uri& relative_uri, const http_header_handler& header_handler = {}) const {
            return self().commit_json_nop(method, relative_uri, header_handler);
        }

        [[nodiscard]] abi::json commit_json(http_method method, const uri& relative_uri, const abi::json& params,
            const http_header_handler& header_handler = {}) const {
            return self().commit_json(method, relative_uri, params, header_handler);
        }

        [[nodiscard]] abi::json commit_bytes(http_method method, const uri& relative_uri, std::string_view content_type,
            std::span<const std::byte> bytes, const http_header_handler& header_handler = {}) const {
            return self().commit_bytes(method, relative_uri, content_type, bytes, header_handler);
        }

        template <byte_like_contiguous_range Range>
        abi::json commit_bytes(http_method method, const uri& relative_uri, std::string_view content_type,
            Range&& bytes, const http_header_handler& header_handler = {}) const {
            return self().commit_bytes(method, relative_uri, content_type, as_const_byte_span(bytes), header_handler);
        }

        void on_progress(const http_progress_handler& progress_handler) const {
            return self().on_progress(progress_handler);
        }

        template <typename T>
            requires rest_message<std::decay_t<T>>
        auto commit_message(T&& message, const http_header_handler& header_handler = {}) const {
            using decayed_type = std::decay_t<T>;
            using traits_type  = rest_message_traits<decayed_type>;

            static constexpr bool is_dummy_message = std::derived_from<decayed_type, dummy_body_tag>;

            auto handler = [header_handler, this]<typename U>(U&& message) {
                static_assert(
                    has_rest_message_transformer<decayed_type> || !has_rest_message_transformer_only_name<decayed_type>,
                    "An invalid \"message_transformer\" found.");

                auto params = [&] {
                    if constexpr (is_dummy_message) {
                        return abi::json{};
                    } else if constexpr (has_rest_message_transformer<decayed_type>) {
                        return abi::to_abi_json(rest_message_transformer_v<decayed_type>(std::forward<U>(message)));
                    } else {
                        return abi::to_abi_json(json(std::forward<U>(message)));
                    }
                }();

                auto response = [&] {
                    auto raw_response = [&] {
                        if constexpr (is_dummy_message) {
                            return abi::from_abi_json(
                                commit_json_nop(traits_type::method, traits_type::relative_uri, header_handler));
                        } else {
                            return abi::from_abi_json(
                                commit_json(traits_type::method, traits_type::relative_uri, params, header_handler));
                        }
                    }();

                    try {
                        return raw_response.template get<typename traits_type::response_type>();
                    } catch (...) {
                        std::throw_with_nested(source_code_aware_runtime_error{
                            U8("Raw Response"), raw_response.dump(default_nested_exception_indent)});
                    }
                }();

                static_assert(has_rest_response_transformer<decayed_type>
                                  || !has_rest_response_transformer_only_name<decayed_type>,
                    "An invalid \"response_transformer\" found.");

                if constexpr (has_rest_response_transformer<decayed_type>) {
                    return rest_response_transformer_v<decayed_type>(response);
                } else {
                    return response;
                }
            };

            auto message_content = [&] {
                if constexpr (is_dummy_message) {
                    return U8("");
                } else {
                    return json(message).dump(default_nested_exception_indent);
                }
            }();

            auto tuple = std::forward_as_tuple(std::forward<T>(message));

            return throw_nested_and_flatten(
                source_code_aware_runtime_error{U8("Error"), U8("Failed to commit the message."), U8("Entity"),
                    meta::get_literal_string_t<decayed_type,
                        meta::identifier_param{
                            .shortened = true,
                        }>(),
                    U8("Raw Message"), message_content},
                [&] { return std::apply(handler, std::move(tuple)); });
        }

    private:
        http_client_abstract() = default;

        Impl& self() noexcept {
            return static_cast<Impl&>(*this);
        }

        const Impl& self() const noexcept {
            return static_cast<const Impl&>(*this);
        }
    };
} // namespace essence::net
