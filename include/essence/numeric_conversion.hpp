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

#include "basic_string.hpp"

#include <array>
#include <charconv>
#include <concepts>
#include <cstdint>
#include <optional>
#include <string>
#include <string_view>
#include <system_error>
#include <type_traits>
#include <utility>

namespace essence::detail {
    template <typename T, typename... Args>
        requires std::is_arithmetic_v<T>
    std::optional<T> from_string(std::string_view str, Args&&... args)
        requires requires(
            T number) { std::from_chars(str.data(), str.data() + str.size(), number, std::forward<Args>(args)...); }
    {
        if (str.empty()) {
            return std::nullopt;
        }

        if (T result{}; std::from_chars(str.data(), str.data() + str.size(), result, std::forward<Args>(args)...).ec
                        == std::errc{}) {
            return result;
        }

        return std::nullopt;
    }

    template <std_basic_string S, typename T, typename... Args>
        requires std::is_arithmetic_v<T>
    S to_string(T number, Args&&... args)
        requires requires(char* str) { std::to_chars(str, str, number, std::forward<Args>(args)...); }
    {
        thread_local std::array<char, 64> buffer{};

        if (auto&& [ptr, ec] =
                std::to_chars(buffer.data(), buffer.data() + buffer.size(), number, std::forward<Args>(args)...);
            ec == std::errc{}) {
            return {buffer.data(), ptr};
        }

        return {};
    }
} // namespace essence::detail

namespace essence {
    /**
     * @brief Converts a string to an integer.
     * @tparam T The type of the integer.
     * @param str The string.
     * @param base The base of the integer.
     * @return The number if succeeded; otherwise std::nullopt.
     */
    template <std::integral T>
    std::optional<T> from_string(std::string_view str, std::int32_t base = 10) {
        return detail::from_string<T>(str, base);
    }

    /**
     * @brief Converts a string to an integer.
     * @tparam T The type of the floating-point number.
     * @param str The string.
     * @param format The format of the floating-point number.
     * @return The number if succeeded; otherwise std::nullopt.
     */
    template <std::floating_point T>
    std::optional<T> from_string(std::string_view str, std::chars_format format = std::chars_format::general) {
        return detail::from_string<T>(str, format);
    }

    /**
     * @brief Converts an integer to a string.
     * @tparam S The return type based on std::basic_string<>.
     * @tparam T The type of the integer.
     * @param number The integer.
     * @param base The base of the integer.
     * @return The string if succeeded; otherwise an empty string.
     */
    template <std_basic_string S = std::string, std::integral T>
    S to_string(T number, std::int32_t base = 10) {
        return detail::to_string<S>(number, base);
    }

    /**
     * @brief Converts a floating-point number to a string.
     * @tparam S The return type based on std::basic_string<>.
     * @tparam T The type of the floating-point number.
     * @param number The floating-point number.
     * @return The string if succeeded; otherwise an empty string.
     */
    template <std_basic_string S = std::string, std::floating_point T>
    S to_string(T number) {
        return detail::to_string<S>(number);
    }

    /**
     * @brief Converts a floating-point number to a string.
     * @tparam S The return type based on std::basic_string<>.
     * @tparam T The type of the floating-point number.
     * @param number The floating-point number.
     * @param format The format of the floating-point number.
     * @return The string if succeeded; otherwise an empty string.
     */
    template <std_basic_string S = std::string, std::floating_point T>
    S to_string(T number, std::chars_format format) {
        return detail::to_string<S>(number, format);
    }

    /**
     * @brief Converts a floating-point number to a string.
     * @tparam S The return type based on std::basic_string<>.
     * @tparam T The type of the floating-point number.
     * @param number The floating-point number.
     * @param format The format of the floating-point number.
     * @param precision The precision of the floating-point number.
     * @return The string if succeeded; otherwise an empty string.
     */
    template <std_basic_string S = std::string, std::floating_point T>
    S to_string(T number, std::chars_format format, std::int32_t precision) {
        return detail::to_string<S>(number, format, precision);
    }
} // namespace essence
