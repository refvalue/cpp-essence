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

#include "detail/identifier.hpp"

#include <type_traits>

namespace essence::meta {
    /**
     * @brief Gets the literal string of a type.
     * @tparam T The type.
     * @tparam Param The parameter for parsing the identifier.
     * @return The literal string.
     */
    template <typename T, identifier_param Param = {}>
    consteval auto get_literal_string_t() noexcept {
        if constexpr (Param.shortened) {
            return detail::get_short_literal_string<T, Param>();
        } else {
            return detail::get_literal_string<T, Param>();
        }
    }

    /**
     * @brief Gets the literal string of a literal value attached to a concrete type.
     * @tparam T The type, to which the literal value is attached.
     * @tparam Value The literal value, e.g. an enumeration or a function pointer.
     * @tparam Param The parameter for parsing the identifier.
     * @return The literal string.
     */
    template <typename T, auto Value, identifier_param Param = {}>
    consteval auto get_literal_string_v() noexcept {
        if constexpr (Param.shortened) {
            return detail::get_short_literal_string<Value, T, Param>();
        } else {
            return detail::get_literal_string<Value, T, Param>();
        }
    };

    /**
     * @brief Gets the friendly name of a function without arguments.
     * @tparam Value The function pointer.
     * @tparam Param The parameter for parsing the identifier.
     * @return The friendly name.
     */
    template <auto Value, identifier_param Param = {}>
        requires(std::is_pointer_v<decltype(Value)> && std::is_function_v<std::remove_pointer_t<decltype(Value)>>)
    consteval auto get_function_name() noexcept {
        if constexpr (Param.shortened) {
            return detail::get_short_function_name<Value, Param>();
        } else {
            return detail::get_function_name<Value, Param>();
        }
    }
} // namespace essence::meta
