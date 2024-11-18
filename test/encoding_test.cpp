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

#include <string>
#include <string_view>

#include <essence/abi/vector.hpp>
#include <essence/char8_t_remediation.hpp>
#include <essence/encoding.hpp>

#include <gtest/gtest.h>

using namespace essence;

#define MAKE_TEST(name) TEST(encoding_test, name)

MAKE_TEST(conversion) {
    static constexpr std::string_view str{U8("Hello world!")};
    static constexpr std::string_view cjk_str{U8("中日韩汉字")};

    static constexpr std::wstring_view wide_str{L"Hello world!"};
    static constexpr std::wstring_view wide_cjk_str{L"中日韩汉字"};

    static constexpr std::u16string_view u16_str{u"Hello world!"};
    static constexpr std::u16string_view u16_cjk_str{u"中日韩汉字"};

    const essence::abi::vector<std::uint16_t> u16_vec{
        u'H', u'e', u'l', u'l', u'o', u' ', u'w', u'o', u'r', u'l', u'd', u'!'};
    const essence::abi::vector<std::uint16_t> u16_cjk_vec{u'中', u'日', u'韩', u'汉', u'字'};

#ifdef _WIN32
    ASSERT_EQ(to_native_string(str), wide_str);
    ASSERT_EQ(to_native_string(cjk_str), wide_cjk_str);
#else
    ASSERT_EQ(to_native_string(str), str);
    ASSERT_EQ(to_native_string(cjk_str), cjk_str);
#endif

    ASSERT_EQ(to_utf16_string(str), u16_str);
    ASSERT_EQ(to_utf16_string(cjk_str), u16_cjk_str);
    ASSERT_EQ(to_uint16_t_literal(str), u16_vec);
    ASSERT_EQ(to_uint16_t_literal(cjk_str), u16_cjk_vec);

    ASSERT_EQ(to_utf8_string(u16_str), str);
    ASSERT_EQ(to_utf8_string(u16_vec), str);
    ASSERT_EQ(to_utf8_string(u16_cjk_str), cjk_str);
    ASSERT_EQ(to_utf8_string(u16_cjk_vec), cjk_str);

#ifdef _WIN32
    ASSERT_EQ(to_utf8_string(wide_str), str);
    ASSERT_EQ(to_utf8_string(wide_cjk_str), cjk_str);
#else
    ASSERT_EQ(to_utf8_string(str), str);
    ASSERT_EQ(to_utf8_string(cjk_str), cjk_str);
#endif
}
