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

#include <concepts>
#include <cstddef>
#include <type_traits>

namespace essence {
    template <typename T>
    concept builtin_char_type =
        std::same_as<T, char> || std::same_as<T, char8_t> || std::same_as<T, char16_t> || std::same_as<T, char32_t>
        || std::same_as<T, signed char> || std::same_as<T, unsigned char>;

    template <typename T>
    concept byte_like = std::same_as<std::decay_t<T>, char> || std::same_as<std::decay_t<T>, unsigned char>
                     || std::same_as<std::decay_t<T>, signed char> || std::same_as<std::decay_t<T>, std::byte>;

    template <builtin_char_type T>
    constexpr bool is_lower(T c) noexcept {
        return c >= T{'a'} && c <= T{'z'};
    }

    template <builtin_char_type T>
    constexpr bool is_upper(T c) noexcept {
        return c >= T{'A'} && c <= T{'Z'};
    }

    template <builtin_char_type T>
    constexpr T to_lower(T c) noexcept {
        return is_upper(c) ? static_cast<T>(c - T{'A'} + T{'a'}) : c;
    }

    template <builtin_char_type T>
    constexpr T to_upper(T c) noexcept {
        return is_lower(c) ? static_cast<T>(c - T{'a'} + T{'A'}) : c;
    }
} // namespace essence
