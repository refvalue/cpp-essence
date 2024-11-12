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

#include "../basic_vector.hpp"
#include "../char8_t_remediation.hpp"
#include "friendly_name.hpp"

#include <algorithm>
#include <array>
#include <string_view>

namespace essence::meta {
    template <std_vector T>
    struct friendly_name<T> {
        static constexpr auto&& nested_value = friendly_name_v<typename T::value_type>;
        static constexpr auto buffer{[] {
            std::array<char, nested_value.size() + 9> result{};
            auto iter = result.begin();

            iter = std::ranges::copy(std::string_view{U8("vector<")}, iter).out;
            iter = std::ranges::copy(nested_value, iter).out;
            iter = std::ranges::copy(std::string_view{U8(">")}, iter).out;

            return result;
        }()};

        static constexpr std::string_view value{buffer.data()};
    }; // namespace essence::meta
} // namespace essence::meta
