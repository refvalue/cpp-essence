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

namespace essence::meta::detail {
    struct fake_any_caster {
        template <typename T>
        // ReSharper disable once CppFunctionIsNotImplemented
        // ReSharper disable once CppNonExplicitConversionOperator
        constexpr operator T() const noexcept; // NOLINT(*-explicit-constructor)
    };

    template <typename T, typename Sequence, typename = void>
    struct is_constructible_with_n_data_members_impl : std::false_type {};

    template <typename T, std::size_t... Is>
    struct is_constructible_with_n_data_members_impl<T, std::index_sequence<Is...>,
        std::void_t<decltype(T{(Is, fake_any_caster{})...})>> : std::true_type {};

    template <typename T, std::size_t N>
    concept constructible_with_n_data_members =
        is_constructible_with_n_data_members_impl<T, std::make_index_sequence<N>>::value
        && !is_constructible_with_n_data_members_impl<T, std::make_index_sequence<N + 1>>::value;
} // namespace essence::meta::detail
