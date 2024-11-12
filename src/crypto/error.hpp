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

#pragma once

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "source_location.hpp"

#include <cstdint>
#include <string>
#include <utility>

namespace essence::crypto {
    std::uint64_t get_last_error_code();
    std::string get_last_error_string();
    std::string get_error_string(std::uint64_t code);

    struct crypto_error : source_code_aware_runtime_error {
        template <typename... Args>
        explicit crypto_error(logging_string_view hint, Args&&... args)
            : crypto_error{hint.location, hint.str, std::forward<Args>(args)...} {}

        template <typename Arg>
        crypto_error(const source_location& location, Arg&& arg)
            : source_code_aware_runtime_error{
                location, U8("Message"), std::forward<Arg>(arg), U8("Internal"), get_last_error_string()} {}

        template <typename... Args>
        explicit crypto_error(const source_location& location, Args&&... args)
            : source_code_aware_runtime_error{
                location, std::forward<Args>(args)..., U8("Internal"), get_last_error_string()} {}
    };
} // namespace essence::crypto
