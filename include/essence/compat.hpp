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

// Directives for imports and exports are platform-specific not compiler-specific,
// so use WIN32 instead of _MSC_VER here to make it behave correctly and expectedly.
#ifdef _WIN32
#define ES_IMPORT __declspec(dllimport)
#define ES_EXPORT __declspec(dllexport)
#else
#define ES_IMPORT
#define ES_EXPORT __attribute__((visibility("default")))
#endif

#define ES_CONDITIONAL_COMMA_1       ,
#define ES_CONDITIONAL_COMMA_IMPL(x) ES_CONDITIONAL_COMMA_##x
#define ES_CONDITIONAL_COMMA(x)      ES_CONDITIONAL_COMMA_IMPL(x)

#define ES_SELECT_THIRD_ARGUMENT(a, b, c, ...) c
#define ES_CONDITIONAL(condition, consequent, alternate) \
    ES_SELECT_THIRD_ARGUMENT(ES_CONDITIONAL_COMMA(condition), consequent, alternate)
#define ES_API(x) ES_CONDITIONAL(ES_IS_##x##_IMPL, ES_IMPORT, ES_EXPORT)

#if __cplusplus >= 202002L
#define ES_HAS_CXX20 1
#else
#define ES_HAS_CXX20 0
#endif

#if defined(__GUNC__) || defined(__clang__)
#define ES_KEEP_ALIVE __attribute__((used))
#else
#define ES_KEEP_ALIVE
#endif
