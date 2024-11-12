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

#include "../../uniform_string_finder.hpp"
#include "../literal_string.hpp"
#include "language_tokens.hpp"

#include <algorithm>
#include <cstddef>
#include <string_view>
#include <utility>

namespace essence::meta::detail {
    template <find_mode_type = find_mode_type::reserved>
    struct depth_level_finder_traits {
        template <bool Reverse, bool AnyOf, typename Iter, typename End>
        static constexpr std::size_t find_impl(
            Iter first, End last, std::string_view keyword, std::size_t index, std::size_t depth = 0) noexcept {
            char last_char{};
            std::size_t current_depth{};

            constexpr auto next_iter = [](auto& iter) {
                if constexpr (Reverse) {
                    --iter;
                } else {
                    ++iter;
                }
            };

            // Matches the keyword at the current depth.
            auto match_string = [&](auto& iter) {
                if constexpr (AnyOf) {
                    return current_depth == depth && keyword.find_first_of(*iter) != std::string_view::npos;
                } else {
                    return current_depth == depth
                        && std::equal(iter,
                            iter + std::min(std::distance(iter, last), static_cast<std::ptrdiff_t>(keyword.size())),
                            keyword.begin());
                }
            };

            // Acquires the actual boundary iterators.
            auto&& [real_first, real_last] = [&] {
                if constexpr (Reverse) {
                    return std::pair{std::prev(last),
                        std::prev(std::next(
                            first, index == std::string_view::npos ? 0 : static_cast<std::ptrdiff_t>(index)))};
                } else {
                    return std::pair{std::next(first, static_cast<std::ptrdiff_t>(index)), last};
                }
            }();

            for (auto iter = real_first; iter != real_last; next_iter(iter)) {
                if (language_tokens::left_enclosing_tokens.contains(*iter)) {
                    ++current_depth;
                }

                // Except the pointer operator ->.
                if (current_depth != 0 && language_tokens::right_enclosing_tokens.contains(*iter)
                    && last_char != language_tokens::arrow.front()) {
                    --current_depth;
                }

                last_char = *iter;

                // Matches the keyword at this depth.
                if (match_string(iter)) {
                    // Calculates the distance to the user-input first iterator.
                    return std::distance(first, iter) + (AnyOf ? 1 : keyword.size());
                }
            }

            return std::string_view::npos;
        }
    };

    template <>
    struct depth_level_finder_traits<find_mode_type::any_of> {
        static constexpr std::size_t find(std::string_view str, std::string_view keyword, std::size_t depth = 0,
            std::size_t index = string_finder_traits<find_mode_type::any_of>::default_index) noexcept {
            return depth_level_finder_traits<>::find_impl<false, true>(str.begin(), str.end(), keyword, index, depth);
        }
    };

    template <>
    struct depth_level_finder_traits<find_mode_type::full_match> {
        static constexpr std::size_t find(std::string_view str, std::string_view keyword, std::size_t depth = 0,
            std::size_t index = string_finder_traits<find_mode_type::full_match>::default_index) noexcept {
            return depth_level_finder_traits<>::find_impl<false, false>(str.begin(), str.end(), keyword, index, depth);
        }
    };

    template <>
    struct depth_level_finder_traits<find_mode_type::any_of_reverse> {
        static constexpr std::size_t find(std::string_view str, std::string_view keyword, std::size_t depth = 0,
            std::size_t index = string_finder_traits<find_mode_type::any_of_reverse>::default_index) noexcept {
            return depth_level_finder_traits<>::find_impl<true, true>(str.begin(), str.end(), keyword, index, depth);
        }
    };

    template <>
    struct depth_level_finder_traits<find_mode_type::full_match_reverse> {
        static constexpr std::size_t find(std::string_view str, std::string_view keyword, std::size_t depth = 0,
            std::size_t index = string_finder_traits<find_mode_type::full_match_reverse>::default_index) noexcept {
            return depth_level_finder_traits<>::find_impl<true, false>(str.begin(), str.end(), keyword, index, depth);
        }
    };

    template <find_mode_type Mode>
    constexpr std::size_t find_at_depth(std::string_view str, std::string_view keyword, std::size_t depth = 0,
        std::size_t index = string_finder_traits<Mode>::default_index) noexcept {
        return depth_level_finder_traits<Mode>::find(str, keyword, depth, index);
    }
} // namespace essence::meta::detail
