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

namespace essence::meta {
    /**
     * @brief The available naming conventions.
     */
    enum class naming_convention {
        /**
         * @brief Leaves unchanged.
         */
        none = 0,

        /**
         * @brief The camel case.
         */
        camel_case,

        /**
         * @brief The pascal case.
         */
        pascal_case,

        /**
         * @brief The snake case.
         */
        snake_case,

        /**
         * @see snake_case
         */
        lower_case_with_underscores = snake_case,

        /**
         * @see camel_case
         */
        camel = camel_case,

        /**
         * @see pascal_case
         */
        pascal_ = pascal_case,

        /**
         * @see snake_case
         */
        snake = snake_case,
    };

    struct all_naming_conventions_tag {};
    struct original_name_cache_tag {};
} // namespace essence::meta
