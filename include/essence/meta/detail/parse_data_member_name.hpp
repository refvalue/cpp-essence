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

#include "extract_keyword.hpp"
#include "language_tokens.hpp"

#include <string_view>

namespace essence::meta::detail {
    /**
     * @brief Parses the raw name of a data member into a pure name.
     * @param raw_name The raw name of the data member.
     * @return The name of the data member.
     */
    consteval std::string_view parse_data_member_name(std::string_view raw_name) noexcept {
#ifdef _MSC_VER
        return extract_keyword<find_mode_type::full_match_reverse>(raw_name, language_tokens::arrow);
#elif defined(__llvm__) && defined(__clang__)
        return extract_keyword<find_mode_type::full_match_reverse>(raw_name, language_tokens::dot,
            extraction_param{
                .suffix_size = 1U,
            });
#elif defined(__GNUC__)
        return extract_keyword<find_mode_type::full_match_reverse>(raw_name, language_tokens::scope,
            extraction_param{
                .suffix_size = 1U,
            });
#endif
    }
} // namespace essence::meta::detail
