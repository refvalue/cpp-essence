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

#include <essence/char8_t_remediation.hpp>

#include <gtest/gtest.h>

import std;
import essence.basic;
import essence.io;
import essence.meta;
import essence.serialization;

using namespace essence;

#define MAKE_TEST(name) TEST(json_test, name)

namespace {
    template <typename T>
    void test_json_io(T&& input) {
        const auto json = json::parse(std::forward<T>(input), nullptr, false);

        EXPECT_FALSE(json.is_discarded());

        EXPECT_TRUE(json.contains("name"));
        EXPECT_TRUE(json.contains("sex"));
        EXPECT_TRUE(json.contains("age"));
        EXPECT_TRUE(json.contains("data"));
        EXPECT_EQ(json["data"].size(), 2);
        EXPECT_TRUE(json["data"][0].contains("friend"));
        EXPECT_TRUE(json["data"][1].contains("friend"));

        EXPECT_EQ(json["name"], "testing");
        EXPECT_EQ(json["sex"], "female");
        EXPECT_EQ(json["age"], 18);
        EXPECT_EQ(json["data"][0]["friend"], "Sam");
        EXPECT_EQ(json["data"][1]["friend"], "John");
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

    test_json_io(std::ispanstream{std::span{str}});
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
                {.name = "bob", .nullable = std::nullopt},
                {.name = "alice", .nullable = "tag = remote"},
                {.name = "john", .nullable = "tag = hybrid"},
            },
    });

    const auto obj = json.get<foo>();

    EXPECT_EQ(obj.location, location_type::travel);
    EXPECT_EQ(obj.items.size(), 3);
    EXPECT_EQ(obj.items[0].name, "bob");
    EXPECT_EQ(obj.items[1].name, "alice");
    EXPECT_EQ(obj.items[2].name, "john");

    EXPECT_FALSE(obj.items[0].nullable);
    EXPECT_TRUE(obj.items[1].nullable);
    EXPECT_TRUE(obj.items[2].nullable);

    EXPECT_EQ(*obj.items[1].nullable, "tag = remote");
    EXPECT_EQ(*obj.items[2].nullable, "tag = hybrid");
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
        .game_over   = "Weather",
        .geared_up   = 1,
        .standing_by = true,
    });

    const json json_bar(bar{
        .hello_world     = "Meteorology",
        .rust_and_cpp    = 2,
        .java_and_csharp = true,
    });

    const json json_qux(qux{
        .atlantic_ocean = "West",
        .pacific_ocean  = 3,
        .indian_ocean   = true,
    });

    ASSERT_TRUE(json_foo.contains("gameOver"));
    ASSERT_TRUE(json_foo.contains("gearedUp"));
    ASSERT_TRUE(json_foo.contains("standingBy"));

    ASSERT_TRUE(json_bar.contains("HelloWorld"));
    ASSERT_TRUE(json_bar.contains("RustAndCpp"));
    ASSERT_TRUE(json_bar.contains("JavaAndCsharp"));

    ASSERT_TRUE(json_qux.contains("atlantic_ocean"));
    ASSERT_TRUE(json_qux.contains("pacific_ocean"));
    ASSERT_TRUE(json_qux.contains("indian_ocean"));

    const auto obj_foo = json_foo.get<foo>();
    const auto obj_bar = json_bar.get<bar>();
    const auto obj_qux = json_qux.get<qux>();

    EXPECT_EQ(obj_foo.game_over, "Weather");
    EXPECT_EQ(obj_foo.geared_up, 1);
    EXPECT_EQ(obj_foo.standing_by, true);

    EXPECT_EQ(obj_bar.hello_world, "Meteorology");
    EXPECT_EQ(obj_bar.rust_and_cpp, 2);
    EXPECT_EQ(obj_bar.java_and_csharp, true);

    EXPECT_EQ(obj_qux.atlantic_ocean, "West");
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

    EXPECT_EQ(json["Root"], "SecondItem");

    EXPECT_EQ(json["Catalogs"].size(), 3);
    EXPECT_EQ(json["Catalogs"][0], "FirstItem");
    EXPECT_EQ(json["Catalogs"][1], "SecondItem");
    EXPECT_EQ(json["Catalogs"][2], "ThirdItem");

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
        [[maybe_unused]] const auto obj = json{{"value", "non-existance"}}.get<foo>();
    } catch (const std::exception& ex) {
        EXPECT_NE(
            std::string_view{ex.what()}.find(meta::fingerprint{std::type_identity<foo::catalog>{}}.friendly_name()),
            std::string_view::npos);
    }

    try {
        [[maybe_unused]] const auto obj = json{{"non_existance", "whatever"}}.get<foo>();
    } catch (const std::exception& ex) {
        EXPECT_NE(std::string_view{ex.what()}.find("Failed to deserialize the JSON value to the data member."),
            std::string_view::npos);
    }
}
