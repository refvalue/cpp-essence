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
#include <type_traits>
#include <utility>

namespace essence {
    template <std::size_t N>
    struct rank : rank<N - 1> {};

    template <>
    struct rank<0> {};

    template <typename... Ts>
    struct overloaded : Ts... {
        using Ts::operator()...;
    };

    template <typename... Ts>
    overloaded(Ts&&...) -> overloaded<Ts...>;

    template <auto Enum>
        requires std::is_enum_v<decltype(Enum)>
    struct enum_constant : std::integral_constant<decltype(Enum), Enum> {};

    template <auto... Enums, typename T, typename Callable>
        requires(std::is_enum_v<T> && (std::is_enum_v<decltype(Enums)> && ...))
    constexpr void select_enums(T value, Callable&& handler) noexcept(
        (noexcept(std::forward<Callable>(handler)(enum_constant<Enums>{})) && ...)) {
        auto tuple       = std::forward_as_tuple(std::forward<Callable>(handler));
        auto check_value = [&, found = false]<auto Enum>(enum_constant<Enum>) mutable {
            if (!found && value == Enum) {
                std::apply(
                    []<typename InnerCallable>(
                        InnerCallable&& handler) { std::forward<InnerCallable>(handler)(enum_constant<Enum>{}); },
                    std::move(tuple));

                found = true;
            }
        };

        (check_value(enum_constant<Enums>{}), ...);
    }
} // namespace essence
