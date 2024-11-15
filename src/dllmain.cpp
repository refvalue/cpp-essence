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

#include "char8_t_remediation.hpp"
#include "compat.hpp"

#include <locale>

#include <spdlog/spdlog.h>

#ifdef __UCLIBC__
extern "C" {
ES_API(CPPESSENCE) unsigned long getauxval(unsigned long type) {
    return 0;
}
}
#endif

namespace {
    [[maybe_unused]] ES_KEEP_ALIVE struct init {
        init() {
            try {
#ifdef _WIN32
                std::locale::global(std::locale{"en_US.UTF-8"});
#else
                std::locale::global(std::locale{"C.UTF-8"});
#endif
            } catch (...) {
                spdlog::warn(U8("Failed to initialize the locale, then keeping the default."));
            }
        }
    } force_init;
} // namespace
