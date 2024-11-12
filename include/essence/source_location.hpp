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

#include <cstdint>

#if __has_include(<version>)
#include <version>
#endif

#if __cpp_lib_source_location >= 201907L
#include <source_location>

namespace essence {
    using source_location = std::source_location;
}
#else
namespace essence {
    struct source_location {
        constexpr source_location() noexcept : file_name_{"unknown"}, function_name_{file_name_}, line_{}, column_{} {}

        constexpr uint_least32_t line() const noexcept {
            return line_;
        }

        constexpr uint_least32_t column() const noexcept {
            return column_;
        }

        constexpr const char* file_name() const noexcept {
            return file_name_;
        }

        constexpr const char* function_name() const noexcept {
            return function_name_;
        }

        static constexpr source_location current(const char* file_name = __builtin_FILE(),
            const char* function_name = __builtin_FUNCTION(), std::int32_t line = __builtin_LINE(),
            std::int32_t column = __builtin_COLUMN()) noexcept {
            source_location result;

            result.file_name_     = file_name;
            result.function_name_ = function_name;
            result.line_          = line;
            result.column_        = column;

            return result;
        }

    private:
        const char* file_name_;
        const char* function_name_;
        std::uint_least32_t line_;
        std::uint_least32_t column_;
    };
} // namespace essence
#endif
