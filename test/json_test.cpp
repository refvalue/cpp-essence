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

#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <essence/char8_t_remediation.hpp>
#include <essence/io/spanstream.hpp>
#include <essence/json_compat.hpp>

#include <gtest/gtest.h>

using namespace essence;

#define MAKE_TEST(name) TEST(json_test, name)

namespace {
    template <typename T>
    void test_json_io(T&& input) {
        const auto json = json::parse(std::forward<T>(input), nullptr, false);

        EXPECT_FALSE(json.is_discarded());

        EXPECT_TRUE(json.contains(U8("name")));
        EXPECT_TRUE(json.contains(U8("sex")));
        EXPECT_TRUE(json.contains(U8("age")));
        EXPECT_TRUE(json.contains(U8("data")));
        EXPECT_EQ(json[U8("data")].size(), 2);
        EXPECT_TRUE(json[U8("data")][0].contains(U8("friend")));
        EXPECT_TRUE(json[U8("data")][1].contains(U8("friend")));

        EXPECT_EQ(json[U8("name")], U8("testing"));
        EXPECT_EQ(json[U8("sex")], U8("female"));
        EXPECT_EQ(json[U8("age")], 18);
        EXPECT_EQ(json[U8("data")][0][U8("friend")], U8("Sam"));
        EXPECT_EQ(json[U8("data")][1][U8("friend")], U8("John"));
    }
} // namespace

MAKE_TEST(fundamental_io) {
    test_json_io(R"({
  "name": "testing",
  "sex": "female",
  "age": 18,
  "data" : [
    { "friend": "Sam" },
    { "friend": "John" }
  ]
})");
}

MAKE_TEST(from_spanstream) {
    constexpr std::string_view str{R"({
  "name": "testing",
  "sex": "female",
  "age": 18,
  "data" : [
    { "friend": "Sam" },
    { "friend": "John" }
  ]
})"};

    test_json_io(io::ispanstream{std::span{str}});
}

MAKE_TEST(serialization) {
    enum class location_type {
        home,
        work,
        travel,
    };

    struct foo {
        struct bar {
            enum class json_serialization { camel_case };

            std::string name;
            std::optional<std::string> nullable;
        };

        enum class json_serialization { camel_case };

        location_type location{};
        std::vector<bar> items;
    };

    static_assert(json_serializable<foo>);
    static_assert(json_serializable<foo::bar>);

    const json json(foo{
        .location = location_type::travel,
        .items =
            {
                {.name = U8("bob"), .nullable = std::nullopt},
                {.name = U8("alice"), .nullable = U8("tag = remote")},
                {.name = U8("john"), .nullable = U8("tag = hybrid")},
            },
    });

    const auto obj = json.get<foo>();

    EXPECT_EQ(obj.location, location_type::travel);
    EXPECT_EQ(obj.items.size(), 3);
    EXPECT_EQ(obj.items[0].name, U8("bob"));
    EXPECT_EQ(obj.items[1].name, U8("alice"));
    EXPECT_EQ(obj.items[2].name, U8("john"));

    EXPECT_FALSE(obj.items[0].nullable);
    EXPECT_TRUE(obj.items[1].nullable);
    EXPECT_TRUE(obj.items[2].nullable);

    EXPECT_EQ(*obj.items[1].nullable, U8("tag = remote"));
    EXPECT_EQ(*obj.items[2].nullable, U8("tag = hybrid"));
}

MAKE_TEST(naming_convention) {
    struct foo {
        enum class json_serialization { camel_case };

        std::string game_over;
        std::int32_t geared_up{};
        bool standing_by{};
    };

    struct bar {
        enum class json_serialization { pascal_case };

        std::string hello_world;
        std::int32_t rust_and_cpp{};
        bool java_and_csharp{};
    };

    struct qux {
        enum class json_serialization { snake_case };

        std::string atlantic_ocean;
        std::int32_t pacific_ocean{};
        bool indian_ocean{};
    };

    static_assert(json_serializable<foo>);
    static_assert(json_serializable<bar>);
    static_assert(json_serializable<qux>);

    const json json_foo(foo{
        .game_over   = U8("Weather"),
        .geared_up   = 1,
        .standing_by = true,
    });

    const json json_bar(bar{
        .hello_world     = U8("Meteorology"),
        .rust_and_cpp    = 2,
        .java_and_csharp = true,
    });

    const json json_qux(qux{
        .atlantic_ocean = U8("West"),
        .pacific_ocean  = 3,
        .indian_ocean   = true,
    });

    ASSERT_TRUE(json_foo.contains(U8("gameOver")));
    ASSERT_TRUE(json_foo.contains(U8("gearedUp")));
    ASSERT_TRUE(json_foo.contains(U8("standingBy")));

    ASSERT_TRUE(json_bar.contains(U8("HelloWorld")));
    ASSERT_TRUE(json_bar.contains(U8("RustAndCpp")));
    ASSERT_TRUE(json_bar.contains(U8("JavaAndCsharp")));

    ASSERT_TRUE(json_qux.contains(U8("atlantic_ocean")));
    ASSERT_TRUE(json_qux.contains(U8("pacific_ocean")));
    ASSERT_TRUE(json_qux.contains(U8("indian_ocean")));

    const auto obj_foo = json_foo.get<foo>();
    const auto obj_bar = json_bar.get<bar>();
    const auto obj_qux = json_qux.get<qux>();

    EXPECT_EQ(obj_foo.game_over, U8("Weather"));
    EXPECT_EQ(obj_foo.geared_up, 1);
    EXPECT_EQ(obj_foo.standing_by, true);

    EXPECT_EQ(obj_bar.hello_world, U8("Meteorology"));
    EXPECT_EQ(obj_bar.rust_and_cpp, 2);
    EXPECT_EQ(obj_bar.java_and_csharp, true);

    EXPECT_EQ(obj_qux.atlantic_ocean, U8("West"));
    EXPECT_EQ(obj_qux.pacific_ocean, 3);
    EXPECT_EQ(obj_qux.indian_ocean, true);
}

MAKE_TEST(enum_to_string) {
    struct foo {
        enum class catalog {
            first_item,
            second_item,
            third_item,
        };

        enum class json_serialization {
            pascal_case,
            enum_to_string,
        };

        catalog root{catalog::second_item};

        std::vector<catalog> catalogs{
            catalog::first_item,
            catalog::second_item,
            catalog::third_item,
        };
    };

    static_assert(json_serializable<foo>);

    const json json(foo{});

    EXPECT_EQ(json[U8("Root")], U8("SecondItem"));

    EXPECT_EQ(json[U8("Catalogs")].size(), 3);
    EXPECT_EQ(json[U8("Catalogs")][0], U8("FirstItem"));
    EXPECT_EQ(json[U8("Catalogs")][1], U8("SecondItem"));
    EXPECT_EQ(json[U8("Catalogs")][2], U8("ThirdItem"));

    const auto obj = json.get<foo>();

    EXPECT_EQ(obj.root, foo::catalog::second_item);

    EXPECT_EQ(obj.catalogs.size(), 3);
    EXPECT_EQ(obj.catalogs[0], foo::catalog::first_item);
    EXPECT_EQ(obj.catalogs[1], foo::catalog::second_item);
    EXPECT_EQ(obj.catalogs[2], foo::catalog::third_item);
}

MAKE_TEST(exceptions) {
    struct foo {
        enum class json_serialization { camel_case };
        enum class catalog { here };

        catalog value{};
    };

    try {
        [[maybe_unused]] const auto obj = json{{U8("value"), U8("non-existance")}}.get<foo>();
    } catch (const std::exception& ex) {
        EXPECT_TRUE(
            std::string_view{ex.what()}.find(meta::fingerprint{std::type_identity<foo::catalog>{}}.friendly_name())
            != std::string_view::npos);
    }

    try {
        [[maybe_unused]] const auto obj = json{{U8("non_existance"), U8("whatever")}}.get<foo>();
    } catch (const std::exception& ex) {
        EXPECT_TRUE(std::string_view{ex.what()}.find(U8("Failed to deserialize the JSON value to the data member."))
                    != std::string_view::npos);
    }
}
