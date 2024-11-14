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

#include <type_traits>

#include <essence/char8_t_remediation.hpp>
#include <essence/meta/fingerprint.hpp>
#include <essence/meta/literal_string.hpp>
#include <essence/meta/runtime/enum.hpp>

#include <gtest/gtest.h>

namespace essence::testing {
    struct foo {};

    enum class bazflags{a};
} // namespace essence::testing

using namespace essence;
using namespace essence::testing;

TEST(meta_test, identifiers) {
    static constexpr meta::fingerprint fp_foo{std::type_identity<foo>{}};

    EXPECT_STREQ(fp_foo.friendly_name(), U8("essence::testing::foo"));
    EXPECT_EQ(meta::runtime::to_string(bazflags::a), U8("a"));
}
