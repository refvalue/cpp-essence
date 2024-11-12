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

#include "abi/string.hpp"
#include "abi/vector.hpp"
#include "compat.hpp"

#include <cstdint>
#include <span>
#include <string_view>

namespace essence {
#ifdef _WIN32
    using nstring      = std::wstring;
    using nstring_view = std::wstring_view;
#else
    using nstring      = std::string;
    using nstring_view = std::string_view;
#endif
} // namespace essence

namespace essence::internal {
    std::string to_utf8_string(nstring_view str);
    std::string to_utf8_string(std::u16string_view str);
    std::string to_utf8_string(std::span<const std::uint16_t> literal);
    nstring to_native_string(std::string_view utf8_str);
    std::u16string to_utf16_string(std::string_view utf8_str);
    std::vector<std::uint16_t> to_uint16_t_literal(std::string_view utf8_str);
} // namespace essence::internal

namespace essence {
    /**
     * @brief Converts a native string to a UTF-8 string.
     * @param str The native string.
     * @return The UTF-8 string.
     */
    ES_API(CPPESSENCE) abi::string to_utf8_string(nstring_view str);

    /**
     * @brief Converts a UTF-16 string to a UTF-8 string.
     * @param str The UTF-16 string.
     * @return The UTF-8 string.
     */
    ES_API(CPPESSENCE) abi::string to_utf8_string(std::u16string_view str);

    /**
     * @brief Converts a UTF-16 string literal (aka. a range of std::uint16_t-s) to a UTF-8 string.
     * @param literal The UTF-16 string literal in a range of std::uint16_t-s.
     * @return The UTF-8 string.
     */
    ES_API(CPPESSENCE) abi::string to_utf8_string(std::span<const std::uint16_t> literal);

    /**
     * @brief Converts a UTF-8 string to a native string.
     * @param utf8_str The UTF-8 string.
     * @return The native string.
     */
    ES_API(CPPESSENCE) abi::nstring to_native_string(std::string_view utf8_str);

    /**
     * @brief Converts a UTF-8 string to a UTF-16 string.
     * @param utf8_str The UTF-8 string.
     * @return The UTF-16 string.
     */
    ES_API(CPPESSENCE) abi::u16string to_utf16_string(std::string_view utf8_str);

    /**
     * @brief Converts a UTF-8 string to a UTF-16 string literal (aka. a range of std::uint16_t-s).
     * @param utf8_str The UTF-8 string.
     * @return The UTF-16 string literal in a range of std::uint16_t-s.
     */
    ES_API(CPPESSENCE) abi::vector<std::uint16_t> to_uint16_t_literal(std::string_view utf8_str);
} // namespace essence
