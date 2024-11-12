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

#include "compression_routines.hpp"

#include "error_extensions.hpp"

#ifdef EMSCRIPTEN
#include "compat.hpp"
#endif

#include <unordered_map>

namespace essence::io {
    namespace {
        auto& get_routines_map() {
            static std::unordered_map<compression_mode, compression_routines> routines;

            return routines;
        }
    } // namespace

    compression_routines get_compression_routines(compression_mode mode) {
        if (const auto iter = get_routines_map().find(mode); iter != get_routines_map().end()) {
            return iter->second;
        }

        throw source_code_aware_runtime_error{U8("Invalid compression mode.")};
    }

    void add_compression_routines(compression_mode mode, const compression_routines& routines) {
        get_routines_map().insert_or_assign(mode, routines);
    }

#ifdef EMSCRIPTEN
    extern void (*compression_zstd_keep_alive)();
    extern void (*compression_zlibng_keep_alive)();

    ES_KEEP_ALIVE void keep_alive_for_emscripten() {
        compression_zstd_keep_alive();
        compression_zlibng_keep_alive();
    }
#endif
} // namespace essence::io
