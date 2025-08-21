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

#include <gtest/gtest.h>

import std;
import essence.basic;
import essence.meta;

#define MAKE_TEST(name) TEST(meta_test, name)

namespace essence::testing {
    struct foo {};

    enum class problem {
        nuts,
        playing,
        tooling,
    };

    enum class face_action {
        singing  = 1000,
        laughing = 1005,
        crying   = 1010,
    };

    /**
     * @brief Uses ADL to provide a customized range.
     * @return The range of the enumeration.
     */
    consteval auto get_enum_searching_range(face_action) noexcept {
        return std::pair<std::int64_t, std::int64_t>{1000, 1010};
    };
} // namespace essence::testing

using namespace essence;
using namespace essence::testing;

MAKE_TEST(fingerprint_for_nontemplate_types) {
    static constexpr std::array fingerprints{
        std::pair{meta::fingerprint{std::type_identity<std::int8_t>{}}, "int8"},
        std::pair{meta::fingerprint{std::type_identity<std::int16_t>{}}, "int16"},
        std::pair{meta::fingerprint{std::type_identity<std::int32_t>{}}, "int32"},
        std::pair{meta::fingerprint{std::type_identity<std::int64_t>{}}, "int64"},
        std::pair{meta::fingerprint{std::type_identity<float>{}}, "float"},
        std::pair{meta::fingerprint{std::type_identity<double>{}}, "double"},
        std::pair{meta::fingerprint{std::type_identity<char>{}}, "char"},
        std::pair{meta::fingerprint{std::type_identity<char8_t>{}}, "u8char"},
        std::pair{meta::fingerprint{std::type_identity<char16_t>{}}, "u16char"},
        std::pair{meta::fingerprint{std::type_identity<char32_t>{}}, "u32char"},
        std::pair{meta::fingerprint{std::type_identity<bool>{}}, "boolean"},
        std::pair{meta::fingerprint{std::type_identity<foo>{}}, "essence::testing::foo"},
        std::pair{meta::fingerprint{std::type_identity<problem>{}}, "essence::testing::problem"},
        std::pair{meta::fingerprint{std::type_identity<std::string>{}}, "string"},
        std::pair{meta::fingerprint{std::type_identity<std::vector<std::string>>{}}, "vector<string>"},
        std::pair{meta::fingerprint{std::type_identity<std::vector<foo>>{}}, "vector<essence::testing::foo>"},
    };

    for (auto&& [item, name] : fingerprints) {
        EXPECT_STREQ(item.friendly_name(), name);
    }
}

MAKE_TEST(enumerations) {
    EXPECT_EQ(meta::runtime::to_string(problem::nuts), "nuts");
    EXPECT_TRUE(meta::runtime::from_string<problem>("playing"));
    EXPECT_FALSE(meta::runtime::from_string<problem>("none"));
    EXPECT_EQ(*meta::runtime::from_string<problem>("tooling"), problem::tooling);

    const auto names      = meta::runtime::get_enum_names<problem>();
    const auto names_only = meta::runtime::get_enum_names_only<problem>();

    const auto full_names      = meta::runtime::get_enum_names<problem>(false);
    const auto full_names_only = meta::runtime::get_enum_names_only<problem>(false);

    EXPECT_EQ(names.size(), 3);
    EXPECT_EQ(names_only.size(), 3);

    EXPECT_EQ(full_names.size(), 3);
    EXPECT_EQ(full_names_only.size(), 3);

    EXPECT_EQ(names[0], (std::pair<std::string, problem>{"nuts", problem::nuts}));
    EXPECT_EQ(names[1], (std::pair<std::string, problem>{"playing", problem::playing}));
    EXPECT_EQ(names[2], (std::pair<std::string, problem>{"tooling", problem::tooling}));

    EXPECT_EQ(names_only[0], "nuts");
    EXPECT_EQ(names_only[1], "playing");
    EXPECT_EQ(names_only[2], "tooling");

    EXPECT_EQ(full_names[0], (std::pair<std::string, problem>{"essence::testing::problem::nuts", problem::nuts}));
    EXPECT_EQ(full_names[1], (std::pair<std::string, problem>{"essence::testing::problem::playing", problem::playing}));
    EXPECT_EQ(full_names[2], (std::pair<std::string, problem>{"essence::testing::problem::tooling", problem::tooling}));

    EXPECT_EQ(full_names_only[0], "essence::testing::problem::nuts");
    EXPECT_EQ(full_names_only[1], "essence::testing::problem::playing");
    EXPECT_EQ(full_names_only[2], "essence::testing::problem::tooling");

    const auto customized_range_enum_names = meta::runtime::get_enum_names<face_action>();

    EXPECT_EQ(customized_range_enum_names.size(), 3);

    EXPECT_EQ(customized_range_enum_names[0], (std::pair<std::string, face_action>{"singing", face_action::singing}));
    EXPECT_EQ(customized_range_enum_names[1], (std::pair<std::string, face_action>{"laughing", face_action::laughing}));
    EXPECT_EQ(customized_range_enum_names[2], (std::pair<std::string, face_action>{"crying", face_action::crying}));
}

MAKE_TEST(boolean) {
    EXPECT_EQ(meta::true_string, "true");
    EXPECT_EQ(meta::false_string, "false");

    EXPECT_TRUE(meta::runtime::from_string<bool>("true"));
    EXPECT_TRUE(meta::runtime::from_string<bool>("false"));
    EXPECT_EQ(*meta::runtime::from_string<bool>("true"), true);
    EXPECT_EQ(*meta::runtime::from_string<bool>("false"), false);

    EXPECT_EQ(meta::runtime::to_string(true), "true");
    EXPECT_EQ(meta::runtime::to_string(false), "false");
}

MAKE_TEST(literal_string) {
    static constexpr meta::literal_string str1{"Hello"};
    static constexpr meta::literal_string str2{"World"};

    static constexpr std::string_view strv1{str1};
    static constexpr std::string_view strv2{str2};

    static constexpr auto span1 = str1.to_span();
    static constexpr auto span2 = str2.to_span();

    EXPECT_EQ(str1.size(), 5);
    EXPECT_EQ(str2.size(), 5);

    EXPECT_EQ(strv1.size(), 5);
    EXPECT_EQ(strv2.size(), 5);

    EXPECT_EQ(span1.size(), 5);
    EXPECT_EQ(span2.size(), 5);

    EXPECT_NE(str1, str2);
    EXPECT_EQ(str1, str1);
    EXPECT_EQ(str2, str2);

    EXPECT_STREQ(str1.c_str(), strv1.data());
    EXPECT_STREQ(str2.c_str(), strv2.data());
    EXPECT_TRUE(std::ranges::equal(str1, strv1));
    EXPECT_TRUE(std::ranges::equal(str2, strv2));
    EXPECT_TRUE(std::ranges::equal(str1, span1));
    EXPECT_TRUE(std::ranges::equal(str2, span2));

    EXPECT_TRUE(str1.contains('e'));
    EXPECT_TRUE(str2.contains('o'));
    EXPECT_EQ(str1.front(), 'H');
    EXPECT_EQ(str2.front(), 'W');
    EXPECT_EQ(str1.back(), 'o');
    EXPECT_EQ(str2.back(), 'd');

    static constexpr meta::literal_string str12 = str1 + str2;
    static constexpr meta::literal_string str12_2{str1, str2};
    static constexpr meta::literal_string str12_3{span1, span2};
    static constexpr meta::literal_string str12_4{'H', 'e', 'l', 'l', 'o', 'W', 'o', 'r', 'l', 'd'};

    EXPECT_EQ(str12, str12_2);
    EXPECT_EQ(str12_2, str12_3);
    EXPECT_EQ(str12_3, str12_4);
    EXPECT_EQ(str12, str12_4);
}
