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

#include <cstddef>
#include <string_view>

namespace essence {
    enum class find_mode_type {
        any_of = 0,
        full_match,
        any_of_reverse,
        full_match_reverse,
        reserved,
    };

    template <find_mode_type>
    struct string_finder_traits {};

    template <>
    struct string_finder_traits<find_mode_type::any_of> {
        static constexpr std::size_t default_index{};
        static constexpr bool forward_direction = true;
        static constexpr auto find              = [](std::string_view source, std::string_view keyword,
                                         std::size_t index = default_index) {
            return source.find_first_of(keyword, index);
        };

        static constexpr auto keyword_size = [](std::string_view keyword) -> std::size_t { return 1; };
    };

    template <>
    struct string_finder_traits<find_mode_type::full_match> {
        static constexpr std::size_t default_index{};
        static constexpr bool forward_direction = true;
        static constexpr auto find              = [](std::string_view source, std::string_view keyword,
                                         std::size_t index = default_index) { return source.find(keyword, index); };

        static constexpr auto keyword_size = [](std::string_view keyword) { return keyword.size(); };
    };

    template <>
    struct string_finder_traits<find_mode_type::any_of_reverse> {
        static constexpr auto default_index = std::string_view::npos;
        static constexpr bool forward_direction{};
        static constexpr auto find = [](std::string_view source, std::string_view keyword,
                                         std::size_t index = default_index) {
            return source.find_last_of(keyword, index);
        };

        static constexpr auto keyword_size = [](std::string_view keyword) -> std::size_t { return 1; };
    };

    template <>
    struct string_finder_traits<find_mode_type::full_match_reverse> {
        static constexpr auto default_index = std::string_view::npos;
        static constexpr bool forward_direction{};
        static constexpr auto find = [](std::string_view source, std::string_view keyword,
                                         std::size_t index = default_index) { return source.rfind(keyword, index); };

        static constexpr auto keyword_size = [](std::string_view keyword) { return keyword.size(); };
    };

    /**
     * @brief Finds a substring within a given string in the specified searching mode.
     * @tparam Mode The searching mode.
     * @param source The source string.
     * @param keyword The keyword to search in the source.
     * @param index The start index.
     * @param plus_keyword_size Whether to plus the size of the keyword to the found index.
     * @return The start index of the substring, or std::string_view::npos if not found.
     */
    template <find_mode_type Mode>
    constexpr auto uniform_find_string(std::string_view source, std::string_view keyword,
        std::size_t index      = string_finder_traits<Mode>::default_index,
        bool plus_keyword_size = false) noexcept(noexcept(string_finder_traits<Mode>::find(source, keyword, index))) {
        auto found_index = string_finder_traits<Mode>::find(source, keyword, index);

        if (plus_keyword_size && found_index != std::string_view::npos) {
            found_index += string_finder_traits<Mode>::keyword_size(keyword);
        }

        return found_index;
    }

    /**
     * @brief Advances the index by analyzing the keyword in the specified searching mode.
     * @tparam Mode The searching mode.
     * @param index The source index.
     * @param keyword The keyword to be analyzed.
     * @return The advanced index.
     */
    template <find_mode_type Mode>
    constexpr auto skip_keyword(std::size_t index, std::string_view keyword) noexcept {
        if constexpr (string_finder_traits<Mode>::forward_direction) {
            return index + string_finder_traits<Mode>::keyword_size(keyword);
        } else {
            return index - string_finder_traits<Mode>::keyword_size(keyword);
        }
    }
} // namespace essence
