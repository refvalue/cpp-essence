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

#include "char8_t_remediation.hpp"

#include <string_view>

namespace essence::cli {
    struct common_tokens {
        static constexpr std::string_view underscore{U8("_")};
        static constexpr std::string_view option_name_prefix{U8("--")};
        static constexpr std::string_view option_abbreviation_prefix{U8("-")};
        static constexpr std::string_view equal{U8("=")};
        static constexpr std::string_view fixed_help_content{U8(R"(Passing parameters:

--opt1[=]<value>
--opt2[=]<value1>,<value2>,<value>,...
--switch3[[=]true|false]
-o1[=]<value>
-o2[=]<value1>,<value2>,<value>,...
-s3[[=]true|false]

Available options:
)")};
        static constexpr std::string_view help_option_header_pattern{U8("{:<32} {:<28} {:<28} {:<32}")};
    };
} // namespace essence::cli
