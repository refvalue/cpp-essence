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

module;

#include <essence/compat.hpp>

export module essence.meta:detail.language_tokens;

import std;
import :literal_string;
import essence.basic;

export namespace essence::meta::detail {
    /**
     * Some tokens of the C++ programming language.
     */
    struct language_tokens {
        ES_API(CPPESSENCE) static constexpr literal_string reference { "&" };
        ES_API(CPPESSENCE) static constexpr literal_string dot { "." };
        ES_API(CPPESSENCE) static constexpr literal_string scope { "::" };
        ES_API(CPPESSENCE) static constexpr literal_string arrow { "->" };
        ES_API(CPPESSENCE) static constexpr literal_string comma { "," };
        ES_API(CPPESSENCE) static constexpr literal_string semicolon { ";" };

        ES_API(CPPESSENCE) static constexpr literal_string enum_prefix { "enum " };
        ES_API(CPPESSENCE) static constexpr literal_string class_prefix { "class " };
        ES_API(CPPESSENCE) static constexpr literal_string struct_prefix { "struct " };

        ES_API(CPPESSENCE) static constexpr literal_string left_parentheses { "(" };
        ES_API(CPPESSENCE) static constexpr literal_string left_angle_bracket { "<" };
        ES_API(CPPESSENCE) static constexpr literal_string left_square_bracket { "[" };

        ES_API(CPPESSENCE) static constexpr literal_string right_parentheses { ")" };
        ES_API(CPPESSENCE) static constexpr literal_string right_angle_bracket { ">" };
        ES_API(CPPESSENCE) static constexpr literal_string right_square_bracket { "]" };

        ES_API(CPPESSENCE)
        static constexpr literal_string left_enclosing_tokens{
            left_parentheses, left_angle_bracket, left_square_bracket};

        ES_API(CPPESSENCE)
        static constexpr literal_string right_enclosing_tokens {
            right_parentheses,
            right_angle_bracket,
            right_square_bracket
        };

        ES_API(CPPESSENCE)
        static constexpr auto type_prefixes = std::forward_as_tuple(enum_prefix, class_prefix, struct_prefix);
    };
} // namespace essence::meta::detail
