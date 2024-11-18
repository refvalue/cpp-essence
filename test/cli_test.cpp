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
#include <span>
#include <string>
#include <string_view>

#include <essence/abi/string.hpp>
#include <essence/cli/arg_parser.hpp>
#include <essence/cli/option.hpp>

#include <gtest/gtest.h>

using namespace essence;

#define MAKE_TEST(name) TEST(cli_test, name)

MAKE_TEST(option) {
    enum class animal_type {
        cat,
        dog,
        mouse,
    };

    const std::array options{
        cli::option<bool>{}
            .set_bound_name(U8("boolean"))
            .set_description(U8("test"))
            .add_aliases(U8("b"))
            .as_abstract(),

        cli::option<std::int32_t>{}
            .set_bound_name(U8("int32"))
            .set_description(U8("test"))
            .add_aliases(U8("i"))
            .set_valid_values(1, 2, 3)
            .as_abstract(),

        cli::option<float>{}
            .set_bound_name(U8("float32"))
            .set_description(U8("test"))
            .add_aliases(U8("f"))
            .as_abstract(),

        cli::option<std::string>{}
            .set_bound_name(U8("string"))
            .set_description(U8("test"))
            .add_aliases(U8("s"))
            .as_abstract(),

        cli::option<animal_type>{}
            .set_bound_name(U8("animal"))
            .set_description(U8("test"))
            .add_aliases(U8("a"))
            .as_abstract(),

        cli::option<std::vector<std::string>>{}
            .set_bound_name(U8("lines"))
            .set_description(U8("test"))
            .add_aliases(U8("l"))
            .set_valid_values(U8("abc"), U8("123"))
            .as_abstract(),

        cli::option<std::vector<std::int32_t>>{}
            .set_bound_name(U8("numbers"))
            .set_description(U8("test"))
            .add_aliases(U8("n"))
            .set_valid_values(1, 2, 3)
            .as_abstract(),

        cli::option<std::vector<animal_type>>{}
            .set_bound_name(U8("animals"))
            .set_description(U8("test"))
            .add_aliases(U8("z"))
            .as_abstract(),
    };

    const cli::arg_parser parser;

    for (auto&& item : options) {
        parser.add_option(item);
    }

    parser.on_error([](std::string_view message) {});
    parser.on_output([](std::string_view message) {});

    if (parser.parse(std::vector<essence::abi::string>{U8("-b"), U8("-i=2"), U8("--float32=3.14"), U8("--string"),
            U8("hello"), U8("-a=dog"), U8("--lines"), U8("123,abc"), U8("--numbers=2,2,2,3,1,1"), U8("-z"),
            U8("cat,mouse,dog"), U8("other"), U8("lol")});
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
        ASSERT_EQ(parser.unmatched_args()[0], U8("other"));
        ASSERT_EQ(parser.unmatched_args()[1], U8("lol"));

        const auto model = parser.to_model<foo>();

        ASSERT_TRUE(model);
        ASSERT_EQ(model->boolean, true);
        ASSERT_EQ(model->int32, 2);
        ASSERT_EQ(model->float32, 3.14f);
        ASSERT_EQ(model->string, U8("hello"));
        ASSERT_EQ(model->animal, animal_type::dog);
        ASSERT_EQ(model->lines, (std::vector<std::string>{U8("123"), U8("abc")}));
        ASSERT_EQ(model->numbers, (std::vector{2, 2, 2, 3, 1, 1}));
        ASSERT_EQ(model->animals, (std::vector{animal_type::cat, animal_type::mouse, animal_type::dog}));
    }
}
