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

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>

namespace essence {
    template <typename T>
    struct is_std_array : std::false_type {};

    template <typename T, std::size_t N>
    struct is_std_array<std::array<T, N>> : std::true_type {};

    template <typename T>
    inline constexpr bool is_std_array_v = is_std_array<T>::value;

    template <typename T>
    concept std_array = is_std_array_v<T>;

    template <auto GetArray, auto Predicate>
        requires std::predicate<decltype(Predicate), typename decltype(GetArray())::value_type>
    struct filter_array {
        using value_type = typename decltype(GetArray())::value_type;

        static constexpr auto value = [] {
            constexpr auto array       = GetArray();
            constexpr auto actual_size = std::count_if(array.begin(), array.end(), Predicate);

            std::size_t index{};
            std::array<value_type, actual_size> result{};

            for (auto&& item : array) {
                if (Predicate(item)) {
                    result[index++] = item;
                }
            }

            return result;
        }();
    };

    template <auto GetArray, auto Predicate>
        requires std::predicate<decltype(Predicate), typename decltype(GetArray())::value_type>
    inline constexpr auto filter_array_v = filter_array<GetArray, Predicate>::value;
} // namespace essence
