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

#include "memory.hpp"

#include <concepts>
#include <string>

namespace essence::abi {
    template <typename T>
    using basic_string = std::basic_string<T, std::char_traits<T>, uniform_allocator<T>>;

    using string  = basic_string<char>;
    using wstring = basic_string<wchar_t>;

#if __cpp_char8_t >= 201811L
    using u8string = basic_string<char8_t>;
#endif

    using u16string = basic_string<char16_t>;
    using u32string = basic_string<char32_t>;

#ifdef _WIN32
    using nstring = abi::wstring;
#else
    using nstring = abi::string;
#endif

    template <typename T>
    std::basic_string<T> to_local_string(const abi::basic_string<T>& value) {
        return std::basic_string<T>{value.begin(), value.end()};
    }

    template <typename T>
    std::basic_string<T> from_abi_string(const abi::basic_string<T>& value) {
        return std::basic_string<T>{value.begin(), value.end()};
    }

    template <typename T, typename CharTraits, typename Allocator>
        requires(!std::same_as<T, char8_t>)
    abi::basic_string<T> to_abi_string(const std::basic_string<T, CharTraits, Allocator>& value) {
        return abi::basic_string<T>{value.begin(), value.end()};
    }

#if __cpp_char8_t >= 201811L
    template <typename CharTraits, typename Allocator>
    abi::basic_string<char> to_abi_string(const std::basic_string<char8_t, CharTraits, Allocator>& value) {
        return abi::basic_string<char>{value.begin(), value.end()};
    }
#endif
} // namespace essence::abi
