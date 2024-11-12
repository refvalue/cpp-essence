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

#if _MSC_VER
#pragma comment(linker, \
    "/export:??0log_msg@details@spdlog@@QEAA@Usource_loc@2@V?$basic_string_view@D@v10@fmt@@W4level_enum@level@2@1@Z")
#pragma comment(linker, "/export:?enabled@backtracer@details@spdlog@@QEBA_NXZ")
#pragma comment(linker, "/export:?default_logger_raw@spdlog@@YAPEAVlogger@1@XZ")
#pragma comment(linker, \
    "/export:?err_handler_@logger@spdlog@@IEAAXAEBV?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@Z")
#pragma comment(linker, "/export:?log_it_@logger@spdlog@@IEAAXAEBUlog_msg@details@2@_N1@Z")
#endif

#ifdef __UCLIBC__
extern "C" {
ES_API(CPPESSENCE) unsigned long getauxval(unsigned long type) {
    return 0;
}
}
#endif

namespace {SPDLOG_INLINE
    ES_KEEP_ALIVE [[maybe_unused]] struct init {
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

            spdlog::info(U8("C++ Essence Library Version {} | Build {}"), CPP_ESSENCE_VERSION, CPP_ESSENCE_DATETIME);
        }
    } force_init;
} // namespace
