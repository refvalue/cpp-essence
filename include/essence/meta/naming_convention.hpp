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

#include "../basic_string.hpp"
#include "common_types.hpp"
#include "detail/naming_convention.hpp"

#include <utility>

namespace essence::meta {
    consteval auto get_enum_searching_range(naming_convention) noexcept {
        return std::pair{0, 4};
    }

    template <std_basic_string T = std::string>
    T convert_naming_convention(std::string_view name, naming_convention convention) {
        switch (convention) {
        case naming_convention::camel_case:
            return detail::camelize_or_pascalize<T>(name, true);
        case naming_convention::pascal_case:
            return detail::camelize_or_pascalize<T>(name, false);
        case naming_convention::snake_case:
            return detail::make_snake_case<T>(name);
        default:
            return T{name};
        }
    }
} // namespace essence::meta
