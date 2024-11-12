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

#include "literal_string.hpp"

#include <algorithm>
#include <array>
#include <cstddef>
#include <span>
#include <string_view>
#include <utility>

namespace essence::meta {
    /**
     * @brief Splits a literal string with a set of delimiters.
     * @tparam S The literal string.
     * @tparam Delimiter The delimiters.
     */
    template <literal_string S, literal_string Delimiter>
    struct split_of {
        static constexpr auto value = [] {
            constexpr std::string_view view{S};
            constexpr auto group_count =
                std::count_if(S.begin(), S.end(), [](char c) { return Delimiter.contains(c); }) + 1;
            std::array<std::string_view, group_count> result{};

            auto iter = result.begin();

            for (std::size_t start_index = 0, end_index = view.find_first_of(Delimiter);;
                 start_index = end_index + 1, end_index = view.find_first_of(Delimiter, start_index)) {
                *(iter++) = view.substr(start_index, end_index - start_index);

                if (end_index == std::string_view::npos) {
                    break;
                }
            }

            return result;
        }();
    };

    /**
     * @brief Splits a literal string with a set of delimiters.
     * @tparam S The literal string.
     * @tparam Delimiter The delimiters.
     */
    template <literal_string S, literal_string Delimiter>
    inline constexpr auto&& split_of_v = split_of<S, Delimiter>::value;

    /**
     * @brief Splits a literal string with a delimiter.
     * @tparam S The literal string.
     * @tparam Delimiter The delimiter.
     */
    template <literal_string S, literal_string Delimiter>
    struct split {
        static constexpr std::string_view view{S};
        static constexpr auto value = [] {
            constexpr auto group_count = [] {
                std::size_t count{};
                std::size_t index{};

                while ((index = view.find(Delimiter, index)) != std::string_view::npos) {
                    count++;
                    index += Delimiter.size();
                }

                return count + 1;
            }();
            std::array<std::string_view, group_count> result{};

            auto iter = result.begin();

            for (std::size_t start_index = 0, end_index = view.find(Delimiter);;
                 start_index = end_index + Delimiter.size(), end_index = view.find(Delimiter, start_index)) {
                *(iter++) = view.substr(start_index, end_index - start_index);

                if (end_index == std::string_view::npos) {
                    break;
                }
            }

            return result;
        }();
    };

    /**
     * @brief Splits a literal string with a delimiter.
     * @tparam S The literal string.
     * @tparam Delimiter The delimiter.
     */
    template <literal_string S, literal_string Delimiter>
    inline constexpr auto&& split_v = split<S, Delimiter>::value;

    /**
     * @brief Removes a character from a literal string.
     * @tparam S The literal string.
     * @tparam C The character.
     */
    template <literal_string S, char C>
    struct remove_char {
        static constexpr auto value = [] {
            struct removal_metadata {
                decltype(S) result;
                std::size_t actual_size;
            };

            constexpr auto metadata = [] {
                auto result      = S;
                auto removal_end = std::remove(result.begin(), result.end(), C);

                return removal_metadata{
                    .result      = std::move(result),
                    .actual_size = static_cast<std::size_t>(removal_end - result.begin()),
                };
            }();

            literal_string<metadata.actual_size> result;

            std::copy(metadata.result.begin(), metadata.result.begin() + metadata.actual_size, result.begin());

            return result;
        }();
    };

    /**
     * @brief Removes a character from a literal string.
     * @tparam S The literal string.
     * @tparam C The character.
     */
    template <literal_string S, char C>
    inline constexpr auto&& remove_char_v = remove_char<S, C>::value;

    /**
     * @brief Removes a substring from a literal string.
     * @tparam S The literal string.
     * @tparam Removal The substring.
     */
    template <literal_string S, literal_string Removal>
    struct remove {
        static constexpr auto groups = split_v<S, Removal>;
        static constexpr auto value  = [] {
            return []<std::size_t... Is>(std::index_sequence<Is...>) {
                return literal_string{
                    std::span<const char, groups[Is].size()>{groups[Is].data(), groups[Is].size()}...};
            }(std::make_index_sequence<groups.size()>{});
        }();
    };

    /**
     * @brief Removes a substring from a literal string.
     * @tparam S The literal string.
     * @tparam Removal The substring.
     */
    template <literal_string S, literal_string Removal>
    inline constexpr auto&& remove_v = remove<S, Removal>::value;
} // namespace essence::meta
