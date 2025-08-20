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

import essence.basic;
import essence.cli;
import essence.unit_test.lang;
import std;

using namespace essence;

#define MAKE_TEST(name) TEST(cli_test, name)

MAKE_TEST(option) {
    enum class animal_type {
        cat,
        dog,
        mouse,
    };

    const std::array options{
        cli::option<bool>{}.set_bound_name("boolean").set_description("test").add_aliases("b").as_abstract(),

        cli::option<std::int32_t>{}
            .set_bound_name("int32")
            .set_description("test")
            .add_aliases("i")
            .set_valid_values(1, 2, 3)
            .as_abstract(),

        cli::option<float>{}.set_bound_name("float32").set_description("test").add_aliases("f").as_abstract(),

        cli::option<std::string>{}.set_bound_name("string").set_description("test").add_aliases("s").as_abstract(),

        cli::option<animal_type>{}.set_bound_name("animal").set_description("test").add_aliases("a").as_abstract(),

        cli::option<std::vector<std::string>>{}
            .set_bound_name("lines")
            .set_description("test")
            .add_aliases("l")
            .set_valid_values("abc", "123")
            .as_abstract(),

        cli::option<std::vector<std::int32_t>>{}
            .set_bound_name("numbers")
            .set_description("test")
            .add_aliases("n")
            .set_valid_values(1, 2, 3)
            .as_abstract(),

        cli::option<std::vector<animal_type>>{}
            .set_bound_name("animals")
            .set_description("test")
            .add_aliases("z")
            .as_abstract(),
    };

    const cli::arg_parser parser;

    for (auto&& item : options) {
        parser.add_option(item);
    }

    parser.on_error([](std::string_view message) {});
    parser.on_output([](std::string_view message) {});

    if (parser.parse(std::vector<essence::abi::string>{"-b", "-i=2", "--float32=3.14", "--string", "hello", "-a=dog",
            "--lines", "123,abc", "--numbers=2,2,2,3,1,1", "-z", "cat,mouse,dog", "other", "lol"});
        parser) {
        struct foo {
            bool boolean{};
            std::int32_t int32{};
            float float32{};
            std::string string;
            animal_type animal{};
            std::vector<std::string> lines;
            std::vector<std::int32_t> numbers;
            std::vector<animal_type> animals;
        };

        ASSERT_EQ(parser.unmatched_args().size(), 2);
        ASSERT_EQ(parser.unmatched_args()[0], "other");
        ASSERT_EQ(parser.unmatched_args()[1], "lol");

        const auto model = parser.to_model<foo>();

        ASSERT_TRUE(model);
        ASSERT_EQ(model->boolean, true);
        ASSERT_EQ(model->int32, 2);
        ASSERT_EQ(model->float32, 3.14f);
        ASSERT_EQ(model->string, "hello");
        ASSERT_EQ(model->animal, animal_type::dog);
        ASSERT_EQ(model->lines, (std::vector<std::string>{"123", "abc"}));
        ASSERT_EQ(model->numbers, (std::vector{2, 2, 2, 3, 1, 1}));
        ASSERT_EQ(model->animals, (std::vector{animal_type::cat, animal_type::mouse, animal_type::dog}));
    }
}
