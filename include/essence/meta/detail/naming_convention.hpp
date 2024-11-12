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

#include "../../basic_string.hpp"
#include "../../char8_t_remediation.hpp"

#include <cctype>
#include <string>
#include <string_view>

namespace essence::meta::detail {
    inline constexpr auto underscore = U8('_');

    template <std_basic_string T>
    T camelize_or_pascalize(std::string_view name, bool camel) {
        T result{name};

        if (!result.empty()) {
            result.front() = camel ? std::tolower(result.front()) : std::toupper(result.front());
        }

        for (auto iter = result.begin(); iter != result.end(); ++iter) {
            if (*iter == underscore && iter + 1 != result.end()) {
                *(iter + 1) = std::toupper(*(iter + 1));
            }
        }

        std::erase_if(result, [](char c) { return c == underscore; });

        return result;
    }

    template <std_basic_string T>
    T make_snake_case(std::string_view name) {
        if (name.empty()) {
            return {};
        }

        T result(name.size() * 2, U8('\0'));
        auto iter_result = result.begin();

        *iter_result++ = std::tolower(*name.begin());

        for (auto iter = name.begin() + 1; iter != name.end(); ++iter, ++iter_result) {
            if (std::isupper(*iter)) {
                *iter_result     = underscore;
                *(++iter_result) = std::tolower(*iter);
                continue;
            }

            *iter_result = *iter;
        }

        result.resize(iter_result - result.begin());
        result.shrink_to_fit();

        return result;
    }
} // namespace essence::meta::detail
