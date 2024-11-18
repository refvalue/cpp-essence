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

#include <array>
#include <cstdint>
#include <iostream>
#include <ranges>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/meta/fingerprint.hpp>
#include <essence/meta/literal_string.hpp>
#include <essence/meta/runtime/boolean.hpp>
#include <essence/meta/runtime/enum.hpp>
#include <essence/meta/runtime/json_serializer.hpp>

#include <gtest/gtest.h>

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
        std::pair{meta::fingerprint{std::type_identity<std::int8_t>{}}, U8("int8")},
        std::pair{meta::fingerprint{std::type_identity<std::int16_t>{}}, U8("int16")},
        std::pair{meta::fingerprint{std::type_identity<std::int32_t>{}}, U8("int32")},
        std::pair{meta::fingerprint{std::type_identity<std::int64_t>{}}, U8("int64")},
        std::pair{meta::fingerprint{std::type_identity<float>{}}, U8("float")},
        std::pair{meta::fingerprint{std::type_identity<double>{}}, U8("double")},
        std::pair{meta::fingerprint{std::type_identity<char>{}}, U8("char")},
        std::pair{meta::fingerprint{std::type_identity<char8_t>{}}, U8("u8char")},
        std::pair{meta::fingerprint{std::type_identity<char16_t>{}}, U8("u16char")},
        std::pair{meta::fingerprint{std::type_identity<char32_t>{}}, U8("u32char")},
        std::pair{meta::fingerprint{std::type_identity<bool>{}}, U8("boolean")},
        std::pair{meta::fingerprint{std::type_identity<foo>{}}, U8("essence::testing::foo")},
        std::pair{meta::fingerprint{std::type_identity<problem>{}}, U8("essence::testing::problem")},
        std::pair{meta::fingerprint{std::type_identity<std::string>{}}, U8("string")},
        std::pair{meta::fingerprint{std::type_identity<std::vector<std::string>>{}}, U8("vector<string>")},
        std::pair{meta::fingerprint{std::type_identity<std::vector<foo>>{}}, U8("vector<essence::testing::foo>")},
    };

    for (auto&& [item, name] : fingerprints) {
        EXPECT_STREQ(item.friendly_name(), name);
    }
}

MAKE_TEST(enumerations) {
    EXPECT_EQ(meta::runtime::to_string(problem::nuts), U8("nuts"));
    EXPECT_TRUE(meta::runtime::from_string<problem>(U8("playing")));
    EXPECT_FALSE(meta::runtime::from_string<problem>(U8("none")));
    EXPECT_EQ(*meta::runtime::from_string<problem>(U8("tooling")), problem::tooling);

    const auto names      = meta::runtime::get_enum_names<problem>();
    const auto names_only = meta::runtime::get_enum_names_only<problem>();

    const auto full_names      = meta::runtime::get_enum_names<problem>(false);
    const auto full_names_only = meta::runtime::get_enum_names_only<problem>(false);

    EXPECT_EQ(names.size(), 3);
    EXPECT_EQ(names_only.size(), 3);

    EXPECT_EQ(full_names.size(), 3);
    EXPECT_EQ(full_names_only.size(), 3);

    EXPECT_EQ(names[0], (std::pair<std::string, problem>{U8("nuts"), problem::nuts}));
    EXPECT_EQ(names[1], (std::pair<std::string, problem>{U8("playing"), problem::playing}));
    EXPECT_EQ(names[2], (std::pair<std::string, problem>{U8("tooling"), problem::tooling}));

    EXPECT_EQ(names_only[0], U8("nuts"));
    EXPECT_EQ(names_only[1], U8("playing"));
    EXPECT_EQ(names_only[2], U8("tooling"));

    EXPECT_EQ(full_names[0], (std::pair<std::string, problem>{U8("essence::testing::problem::nuts"), problem::nuts}));
    EXPECT_EQ(
        full_names[1], (std::pair<std::string, problem>{U8("essence::testing::problem::playing"), problem::playing}));
    EXPECT_EQ(
        full_names[2], (std::pair<std::string, problem>{U8("essence::testing::problem::tooling"), problem::tooling}));

    EXPECT_EQ(full_names_only[0], U8("essence::testing::problem::nuts"));
    EXPECT_EQ(full_names_only[1], U8("essence::testing::problem::playing"));
    EXPECT_EQ(full_names_only[2], U8("essence::testing::problem::tooling"));

    const auto customized_range_enum_names = meta::runtime::get_enum_names<face_action>();

    EXPECT_EQ(customized_range_enum_names.size(), 3);

    EXPECT_EQ(
        customized_range_enum_names[0], (std::pair<std::string, face_action>{U8("singing"), face_action::singing}));
    EXPECT_EQ(
        customized_range_enum_names[1], (std::pair<std::string, face_action>{U8("laughing"), face_action::laughing}));
    EXPECT_EQ(customized_range_enum_names[2], (std::pair<std::string, face_action>{U8("crying"), face_action::crying}));
}

MAKE_TEST(boolean) {
    EXPECT_EQ(meta::true_string, U8("true"));
    EXPECT_EQ(meta::false_string, U8("false"));

    EXPECT_TRUE(meta::runtime::from_string<bool>(U8("true")));
    EXPECT_TRUE(meta::runtime::from_string<bool>(U8("false")));
    EXPECT_EQ(*meta::runtime::from_string<bool>(U8("true")), true);
    EXPECT_EQ(meta::runtime::from_string<bool>(U8("false")), false);

    EXPECT_EQ(meta::runtime::to_string(true), U8("true"));
    EXPECT_EQ(meta::runtime::to_string(false), U8("false"));
}

MAKE_TEST(literal_string) {
    static constexpr meta::literal_string str1{U8("Hello")};
    static constexpr meta::literal_string str2{U8("World")};

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

    EXPECT_TRUE(str1.contains(U8('e')));
    EXPECT_TRUE(str2.contains(U8('o')));
    EXPECT_EQ(str1.front(), U8('H'));
    EXPECT_EQ(str2.front(), U8('W'));
    EXPECT_EQ(str1.back(), U8('o'));
    EXPECT_EQ(str2.back(), U8('d'));

    static constexpr meta::literal_string str12 = str1 + str2;
    static constexpr meta::literal_string str12_2{str1, str2};
    static constexpr meta::literal_string str12_3{span1, span2};
    static constexpr meta::literal_string str12_4{
        U8('H'), U8('e'), U8('l'), U8('l'), U8('o'), U8('W'), U8('o'), U8('r'), U8('l'), U8('d')};

    EXPECT_EQ(str12, str12_2);
    EXPECT_EQ(str12_2, str12_3);
    EXPECT_EQ(str12_3, str12_4);
    EXPECT_EQ(str12, str12_4);
}
