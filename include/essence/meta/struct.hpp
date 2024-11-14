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
#include "detail/data_member_binding.hpp"
#include "detail/fake_object_wrapper.hpp"
#include "detail/parse_data_member_name.hpp"
#include "identifier.hpp"

#include <cstddef>
#include <tuple>
#include <type_traits>
#include <utility>

namespace essence::meta {
    /**
     * @brief Gets the names of the direct data members of a class.
     * @tparam T The type of the class.
     * @return A coroutine generator to yield the values.
     */
    template <typename T>
        requires std::is_class_v<T>
    generator<std::string_view> probe_data_member_names() {

        const auto names = []<std::size_t... Is>(
                               std::index_sequence<Is...>) -> generator<std::string_view> {
#if defined(__llvm__) && defined(__clang__)
            (co_yield detail::parse_data_member_name(get_literal_string_v<T,
                 detail::make_fake_object_wrapper(
                     std::get<Is>(detail::make_data_member_pointers(detail::make_fake_object_wrapper<T>())))>()),
                ...);
#else
            (co_yield detail::parse_data_member_name(get_literal_string_v<T,
                 std::get<Is>(detail::make_data_member_pointers(detail::make_fake_object_wrapper<T>()))>()),
                ...);
#endif
        }(std::make_index_sequence<std::tuple_size_v<decltype(detail::make_data_member_pointers(
                                                                   detail::make_fake_object_wrapper<T>()))>>{});

        for (auto&& item : names) {
            co_yield item;
        }
    };
} // namespace essence::meta
