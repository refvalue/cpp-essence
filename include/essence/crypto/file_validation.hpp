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

#include "../compat.hpp"
#include "common_types.hpp"

#include <string_view>

namespace essence::crypto {
    /**
     * @brief Creates a validation file with the specified digest mode.
     * @param mode The digest mode.
     * @param path The path of the file.
     * @remark The validation file is something like a.txt.sha512.
     */
    ES_API(CPPESSENCE) void make_validation_file(digest_mode mode, std::string_view path);

    /**
     * @brief Validates a file with the specified digest mode and the existing validation file on the disk.
     * @param mode The digest mode.
     * @param path The path of the file.
     * @return true if the file is valid; otherwise false.
     */
    ES_API(CPPESSENCE) bool validate_file(digest_mode mode, std::string_view path);
} // namespace essence::crypto
