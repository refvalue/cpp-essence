// Copyright (c) 2024 The RefValue Project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <iostream>
#include <string>
#include <string_view>
#include <thread>

#include <essence/char8_t_remediation.hpp>
#include <essence/io/stdio_watcher.hpp>

#include <gtest/gtest.h>

using namespace essence;
using namespace essence::io;

#define MAKE_TEST(name) TEST(io_test, name)

MAKE_TEST(stdio_watcher) {
    const stdio_watcher watcher{stdio_watcher_mode::error};
    std::string lines;

    watcher.on_message([&](std::string_view message) { lines.append(message); });
    watcher.start();

    std::cerr << U8("一些测试内容，Some Tests Included") << '\n';
    std::cerr << U8("一旦发生错误所有的信息推荐使用 stderr 输出，以和 stdout 区分。") << '\n';
    std::cerr << U8("Any error that occurrs in the context should be printed via stderr.") << '\n';
    std::cerr.flush();

    std::this_thread::sleep_for(std::chrono::seconds{3});
    watcher.stop();

    ASSERT_STREQ(
        lines.c_str(), U8("一些测试内容，Some Tests Included\n一旦发生错误所有的信息推荐使用 stderr 输出，以和 stdout "
                          "区分。\nAny error that occurrs in the context should be printed via stderr.\n"));
}
