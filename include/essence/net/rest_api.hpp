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

#include "../json_compat.hpp"
#include "../meta/literal_string.hpp"
#include "common_types.hpp"

#include <concepts>
#include <functional>
#include <string_view>
#include <type_traits>

namespace essence::net {
    enum class http_method {
        put,
        post,
        get,
    };

    template <typename T>
    struct rest_message_traits {};

    template <typename T>
    concept rest_message = requires {
        { rest_message_traits<T>::method } -> std::convertible_to<http_method>;
        { rest_message_traits<T>::relative_uri } -> std::convertible_to<std::string_view>;
        requires std::default_initializable<T>;
        requires json_serializable<T>;
        requires std::derived_from<T, dummy_body_tag>;
        typename rest_message_traits<T>::response_type;
    };

    template <typename T>
    concept has_rest_message_transformer_only_name = requires { rest_message_traits<T>::message_transformer; };

    template <typename T>
    concept has_rest_message_transformer =
        std::is_invocable_r_v<json, decltype(rest_message_traits<T>::message_transformer), const T&>;

    template <typename T>
    concept has_rest_response_transformer_only_name = requires { rest_message_traits<T>::response_transformer; };

    template <typename T>
    concept has_rest_response_transformer = requires(const typename rest_message_traits<T>::response_type& response) {
        { std::invoke(rest_message_traits<T>::response_transformer, response) } -> json_serializable;
    };

    template <has_rest_message_transformer T>
    struct rest_message_transformer {
        static constexpr auto&& value = rest_message_traits<T>::message_transformer;
    };

    template <has_rest_message_transformer T>
    inline constexpr auto&& rest_message_transformer_v = rest_message_transformer<T>::value;

    template <has_rest_response_transformer T>
    struct rest_response_transformer {
        static constexpr auto&& value = rest_message_traits<T>::response_transformer;
    };

    template <has_rest_response_transformer T>
    inline constexpr auto&& rest_response_transformer_v = rest_response_transformer<T>::value;

    template <http_method Method, meta::literal_string RelativeUri, typename ResponseType>
    struct make_rest_api {
        using response_type = ResponseType;

        static constexpr http_method method{Method};
        static constexpr std::string_view relative_uri{RelativeUri};
    };

    constexpr std::string_view to_string(http_method method) noexcept {
        switch (method) {
        case http_method::put:
            return U8("PUT");
        case http_method::post:
            return U8("POST");
        case http_method::get:
            return U8("GET");
        }

        return U8("POST");
    }
} // namespace essence::net
