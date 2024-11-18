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

#include "abi/string.hpp"
#include "error.hpp"

#include <utility>

namespace essence::crypto {
    struct cipher_error_builder {
        abi::string cipher_name;
        abi::string routine_name;

        template <typename... Args>
        [[noreturn]] void raise_error(logging_string_view hint, Args&&... args) const {
            if (sizeof...(Args) == 0) {
                throw crypto_error{
                    hint.location, U8("Cipher"), cipher_name, U8("Routine"), routine_name, U8("Message"), hint.str};
            }

            throw crypto_error{hint.location, U8("Cipher"), cipher_name, U8("Routine"), routine_name, hint.str,
                std::forward<Args>(args)...};
        }

        template <std::int32_t SpecialUpperBound = 0, typename... Args>
        void check_error(std::int32_t code, logging_string_view hint, Args&&... args) const {
            if (code <= SpecialUpperBound) {
                raise_error(hint, std::forward<Args>(args)...);
            }
        }
    };
} // namespace essence::crypto
