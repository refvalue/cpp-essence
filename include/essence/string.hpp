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

#if !defined(CPP_ESSENCE_HEADER_ONLY) || !CPP_ESSENCE_HEADER_ONLY
#include "abi/string.hpp"
#endif

#include "char.hpp"
#include "char8_t_remediation.hpp"
#include "hashing.hpp"

#include <algorithm>
#include <cstddef>
#include <string_view>

namespace essence {
    inline constexpr std::string_view ascii_blank_chars{U8(" \t")};

    /**
     * @brief A hash function for const char*, std::string_view and std::string.
     */
    struct string_hash {
        using hash_type      = std::hash<std::string_view>;
        using is_transparent = void;

        std::size_t operator()(const char* str) const {
            return hash_type{}(str);
        }

        std::size_t operator()(std::string_view str) const {
            return hash_type{}(str);
        }

        std::size_t operator()(const std::string& str) const {
            return hash_type{}(str);
        }
    };

    /**
     * @brief A case-insensitive hash function for const char*, std::string_view and std::string.
     */
    struct icase_string_hash {
        using is_transparent = void;

        std::size_t operator()(const char* str) const {
            return (*this)(std::string_view{str});
        }

        std::size_t operator()(const std::string& str) const {
            return (*this)(std::string_view{str});
        }

        std::size_t operator()(std::string_view value) const {
            std::size_t result{};

            std::ranges::for_each(value, [&](char c) { hash_combine(result, to_lower(c)); });

            return result;
        }
    };

    /**
     * @brief A case-insensitive equality comparer for two strings.
     */
    struct icase_string_comparer {
        constexpr bool operator()(std::string_view left, std::string_view right) const {
            return left.size() == right.size() && std::ranges::equal(left, right, [](char c1, char c2) {
                return c1 == c2 || to_lower(c1) == to_lower(c2);
            });
        }
    };

    /**
     * @brief Checks whether the first string is lexicographically less than the second string,
              in a case-insensitive way.
     */
    struct icase_lexicographical_compare {
        constexpr bool operator()(std::string_view left, std::string_view right) const {
            return std::ranges::lexicographical_compare(
                left, right, [](char c1, char c2) { return to_lower(c1) < to_lower(c2); });
        }
    };

    /**
     * @brief Removes a group of characters from the beginning of a string.
     * @param str The string.
     * @param group The group of characters.
     * @return The result.
     */
    constexpr std::string_view trim_left(std::string_view str, std::string_view group = ascii_blank_chars) noexcept {
        if (const auto index = str.find_first_not_of(group); index != std::string_view::npos) {
            return str.substr(index);
        }

        return str;
    }

    /**
     * @brief Removes a group of characters from the end of a string.
     * @param str The string.
     * @param group The group of characters.
     * @return The result.
     */
    constexpr std::string_view trim_right(std::string_view str, std::string_view group = ascii_blank_chars) noexcept {
        return str.substr(0, str.find_last_not_of(group) + 1);
    }

    /**
     * @brief Removes a group of characters from the beginning and the end of a string.
     * @param str The string.
     * @param group The group of characters.
     * @return The result.
     */
    constexpr std::string_view trim(std::string_view str, std::string_view group = ascii_blank_chars) noexcept {
        return trim_right(trim_left(str, group), group);
    }

#if !defined(CPP_ESSENCE_HEADER_ONLY) || !CPP_ESSENCE_HEADER_ONLY
    /**
     * @brief Lowercases a string.
     * @param str The string.
     * @return The result.
     */
    ES_API(CPPESSENCE) abi::string to_lower(std::string_view str);

    /**
     * @brief Capitalizes a string.
     * @param str The string.
     * @return The result.
     *
     */
    ES_API(CPPESSENCE) abi::string to_upper(std::string_view str);
#endif

} // namespace essence
