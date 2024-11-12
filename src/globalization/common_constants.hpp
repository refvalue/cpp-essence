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

#include <array>
#include <cstdint>
#include <string_view>

namespace essence::globalization {
    struct common_constants {
        static constexpr std::string_view default_working_folder{U8("lang")};
        static constexpr std::string_view default_language{U8("en-US")};
        static constexpr std::string_view language_file_extension{U8(".lang")};

        static constexpr char language_key_value_delimiter{U8('\x1E')};
        static constexpr char language_key_value_terminator{U8('\x1F')};

        static constexpr std::array language_file_magic_flag{
            U8('M'), U8('I'), U8('S'), U8('C'), U8(' '), U8('L'), U8('A'), U8('N'), U8('G')};

        static constexpr std::array<std::uint8_t, 2> language_file_version{0x1, 0x0};

        static constexpr auto language_file_version_number =
            (static_cast<std::uint32_t>(language_file_version.front()) << 16) + language_file_version.back();

        // Ensures that the size of the magic flag is always greater than the size of the version.
        static_assert(language_file_version.size() <= language_file_magic_flag.size());
    };
} // namespace essence::globalization
