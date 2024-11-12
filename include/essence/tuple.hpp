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
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace essence {
    /**
     * @brief Filters a tuple given a predicate.
     * @tparam GetTuple A constant callable to return the tuple.
     * @tparam Predicate The predicate.
     */
    template <auto GetTuple, auto Predicate>
    struct filter_tuple {
        static constexpr auto tuple = GetTuple();

        template <std::size_t I>
        static consteval auto do_single() noexcept {
            constexpr auto value = std::get<I>(tuple);

            if constexpr (Predicate(value)) {
                return std::tuple{value};
            } else {
                return std::tuple<>{};
            }
        }

        static consteval auto do_filter() noexcept {
            return []<std::size_t... Is>(std::index_sequence<Is...>) { return std::tuple_cat(do_single<Is>()...); }(
                       std::make_index_sequence<std::tuple_size_v<decltype(tuple)>>{});
        }

        static constexpr auto value = do_filter();
    };

    /**
     * @brief Filters a tuple given a predicate.
     * @tparam GetTuple A constant callable to return the tuple.
     * @tparam Predicate The predicate.
     */
    template <auto GetTuple, auto Predicate>
    inline constexpr auto filter_tuple_v = filter_tuple<GetTuple, Predicate>::value;

    /**
     * @brief Tests whether a tuple element satisfies the given conditions and returns the first index.
     * @tparam Ts The types of elements.
     * @tparam Callable The type of the predicate.
     * @param tuple The tuple.
     * @param predicate The predicate.
     * @return The index if found; otherwise static_cast<std::size_t>(-1).
     */
    template <typename... Ts, std::predicate<Ts...> Callable>
    constexpr std::size_t get_index_if(const std::tuple<Ts...>& tuple, Callable&& predicate) noexcept(
        (noexcept(std::declval<Callable>()(std::declval<Ts>())) && ...)) {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            auto index = std::numeric_limits<std::size_t>::max();

            auto handler = [&]<std::size_t I>(std::integral_constant<std::size_t, I>) {
                if (index == std::numeric_limits<std::size_t>::max()
                    && std::forward<Callable>(predicate)(std::get<I>(tuple))) {
                    index = I;
                }
            };

            (handler(std::integral_constant<std::size_t, Is>{}), ...);

            return index;
        }(std::make_index_sequence<sizeof...(Ts)>{});
    }
} // namespace essence
