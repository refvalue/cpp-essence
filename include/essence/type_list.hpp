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
#include <limits>
#include <tuple>
#include <type_traits>
#include <utility>

namespace essence {
    template <typename... Ts>
    struct type_list {
        static constexpr std::size_t size = sizeof...(Ts);
    };

    template <typename T>
    struct is_type_list_like : std::false_type {};

    template <template <typename...> typename List, typename... Ts>
    struct is_type_list_like<List<Ts...>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_type_list_like_v = is_type_list_like<T>::value;

    template <typename T>
    concept type_list_like = is_type_list_like_v<T>;

    template <typename List>
    struct type_list_size {};

    template <template <typename...> typename List, typename... Ts>
    struct type_list_size<List<Ts...>> {
        static constexpr std::size_t value = sizeof...(Ts);
    };

    template <typename List>
    inline constexpr std::size_t type_list_size_v = type_list_size<List>::value;

    template <typename List, std::size_t I>
    struct at {};

    template <template <typename...> typename List, typename... Ts, std::size_t I>
    struct at<List<Ts...>, I> {
        using type = std::tuple_element_t<I, std::tuple<Ts...>>;
    };

    template <typename List, std::size_t I>
    using at_t = typename at<List, I>::type;

    template <typename List, typename T>
    struct append {};

    template <template <typename...> typename List, typename... Ts, typename T>
    struct append<List<Ts...>, T> {
        using type = List<Ts..., T>;
    };

    template <typename List, typename T>
    using append_t = typename append<List, T>::type;

    template <type_list_like List, typename Callable>
    constexpr auto for_each_as_values(Callable&& handler) {
        auto tuple = std::forward_as_tuple(std::forward<Callable>(handler));

        return
            [&]<template <typename...> typename ConcreteList, typename... Ts>(std::type_identity<ConcreteList<Ts...>>) {
                return std::get<0>(std::move(tuple))(std::type_identity<Ts>{}...);
            }(std::type_identity<List>{});
    }

    template <type_list_like List, auto Predicate>
    struct index_if {
        static constexpr auto value = for_each_as_values<List>([]<typename... Ts>(std::type_identity<Ts>...) {
            bool found{};
            std::size_t index{std::numeric_limits<std::size_t>::max()};
            auto handler = [&, counter = static_cast<std::size_t>(0)]<typename U>(std::type_identity<U>) mutable {
                if (!found && Predicate(std::type_identity<U>{})) {
                    found = true;
                    index = counter;
                }

                counter++;
            };

            return (handler(std::type_identity<Ts>{}), ..., index);
        });
    };

    template <type_list_like List, auto Predicate>
    inline constexpr auto index_if_v = index_if<List, Predicate>::value;

    template <type_list_like List, auto Predicate>
    struct at_if {
        using type = at_t<List, index_if_v<List, Predicate>>;
    };

    template <type_list_like List, auto Predicate>
    using at_if_t = typename at_if<List, Predicate>::type;
} // namespace essence
