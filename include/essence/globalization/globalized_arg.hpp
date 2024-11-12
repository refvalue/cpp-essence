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

#include "../abi/string.hpp"
#include "simple_messages.hpp"

#include <concepts>
#include <locale>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::globalization {
    template <typename T>
    using globalized_arg_t = std::conditional_t<std::convertible_to<T, std::string_view>, abi::string, T>;

    template <typename T>
    decltype(auto) make_globalized_arg(const std::locale& locale, T&& arg) {
        if constexpr (std::convertible_to<T, std::string_view>) {
            if (std::has_facet<simple_messages>(locale)) {
                return std::use_facet<simple_messages>(locale).get(std::forward<T>(arg));
            }

            return abi::string{std::string_view{std::forward<T>(arg)}};
        } else {
            return std::forward<T>(arg);
        }
    }
} // namespace essence::globalization
