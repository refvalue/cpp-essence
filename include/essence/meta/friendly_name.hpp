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

#include "../basic_string.hpp"
#include "../char8_t_remediation.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <string_view>

namespace essence::meta {
    template <typename T>
    struct friendly_name {
        static constexpr std::string_view value{};
    };

    template <typename T>
    inline constexpr auto friendly_name_v = friendly_name<T>::value;

    template <>
    struct friendly_name<std::int8_t> {
        static constexpr std::string_view value{U8("int8")};
    };

    template <>
    struct friendly_name<std::int16_t> {
        static constexpr std::string_view value{U8("int16")};
    };

    template <>
    struct friendly_name<std::int32_t> {
        static constexpr std::string_view value{U8("int32")};
    };

    template <>
    struct friendly_name<std::int64_t> {
        static constexpr std::string_view value{U8("int64")};
    };

    template <>
    struct friendly_name<std::uint8_t> {
        static constexpr std::string_view value{U8("uint8")};
    };

    template <>
    struct friendly_name<std::uint16_t> {
        static constexpr std::string_view value{U8("uint16")};
    };

    template <>
    struct friendly_name<std::uint32_t> {
        static constexpr std::string_view value{U8("uint32")};
    };

    template <>
    struct friendly_name<std::uint64_t> {
        static constexpr std::string_view value{U8("uint64")};
    };

    template <>
    struct friendly_name<float> {
        static constexpr std::string_view value{U8("float")};
    };

    template <>
    struct friendly_name<double> {
        static constexpr std::string_view value{U8("double")};
    };

    template <>
    struct friendly_name<long double> {
        static constexpr std::string_view value{U8("long double")};
    };

    template <>
    struct friendly_name<bool> {
        static constexpr std::string_view value{U8("boolean")};
    };

    template <>
    struct friendly_name<std::byte> {
        static constexpr std::string_view value{U8("byte")};
    };

    template <>
    struct friendly_name<char> {
        static constexpr std::string_view value{U8("char")};
    };

    template <>
    struct friendly_name<wchar_t> {
        static constexpr std::string_view value{U8("wchar")};
    };

    template <>
    struct friendly_name<char8_t> {
        static constexpr std::string_view value{U8("u8char")};
    };

    template <>
    struct friendly_name<char16_t> {
        static constexpr std::string_view value{U8("u16char")};
    };

    template <>
    struct friendly_name<char32_t> {
        static constexpr std::string_view value{U8("u32char")};
    };

    template <std_basic_string T>
        requires std::same_as<typename T::value_type, char>
    struct friendly_name<T> {
        static constexpr std::string_view value{U8("string")};
    };

    template <std_basic_string T>
        requires std::same_as<typename T::value_type, wchar_t>
    struct friendly_name<T> {
        static constexpr std::string_view value{U8("wstring")};
    };

    template <std_basic_string T>
        requires std::same_as<typename T::value_type, char8_t>
    struct friendly_name<T> {
        static constexpr std::string_view value{U8("u8string")};
    };

    template <std_basic_string T>
        requires std::same_as<typename T::value_type, char16_t>
    struct friendly_name<T> {
        static constexpr std::string_view value{U8("u16string")};
    };

    template <std_basic_string T>
        requires std::same_as<typename T::value_type, char32_t>
    struct friendly_name<T> {
        static constexpr std::string_view value{U8("u32string")};
    };
} // namespace essence::meta
