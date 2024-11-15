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

#include "../generator.hpp"
#include "detail/language_tokens.hpp"
#include "identifier.hpp"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::meta {
    /**
     * @brief Gets the numeric range of an enumeration for searching its valid values.
     * @tparam T The type of the enumeration.
     * @return The searching range.
     */
    template <typename T>
    consteval auto get_enum_searching_range() noexcept {
        return std::pair<std::int64_t, std::int64_t>{-64, 64};
    };

    /**
     * @brief Gets the pairs of names and values of an enumeration.
     * @tparam T The type of the enumeration.
     * @tparam Short Whether to get a short name (without a namespace).
     * @return A coroutine generator to yield the values.
     */
    template <typename T, bool Short = true>
        requires std::is_enum_v<T>
    generator<std::pair<std::string_view, T>> probe_enum_names() {
        const auto origin = []<std::size_t... Is>(
                                std::index_sequence<Is...>) -> generator<std::pair<std::string_view, T>> {
            constexpr auto range = get_enum_searching_range<T>();
            constexpr auto min   = range.first;
            constexpr auto max   = range.second;

            (co_yield std::pair{get_literal_string_v<T, static_cast<T>(min + Is),
                                    identifier_param{
                                        .shortened          = Short,
                                        .ensure_correctness = false,
                                    }>(),
                 static_cast<T>(min + Is)},
                ...);
        }(std::make_index_sequence<max - min + 1>{});

        for (auto&& item : origin) {
            if (item.first.find(detail::language_tokens::right_parentheses.front()) == std::string_view::npos) {
                co_yield item;
            }
        }
    };
} // namespace essence::meta
