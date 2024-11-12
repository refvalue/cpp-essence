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
#include "compression_routines.hpp"
#include "error_extensions.hpp"
#include "source_location.hpp"

#include <cstdint>

#include <zlib.h>

namespace essence::io {
    namespace {
        void check_error(std::int32_t code, const source_location& location = source_location::current()) {
            if (code != Z_OK) {
                throw source_code_aware_runtime_error{location, U8("Code"), code, U8("Message"), zError(code)};
            }
        }

        void compress(std::span<const std::byte> buffer, const abstract::writable_buffer& result, std::int32_t level) {
            const auto size = compressBound(static_cast<unsigned long>(buffer.size()));

            result.resize(size);

            auto actual_size = static_cast<unsigned long>(result.size_bytes());

            check_error(compress2(reinterpret_cast<std::uint8_t*>(result.data()), &actual_size,
                reinterpret_cast<const std::uint8_t*>(buffer.data()), static_cast<unsigned long>(buffer.size()),
                level));

            result.resize(actual_size);
            result.shrink_to_fit();
        }

        void decompress(std::span<const std::byte> buffer, const abstract::writable_buffer& result) {}

        ES_KEEP_ALIVE [[maybe_unused]] struct init {
            init() {
                add_compression_routines(compression_mode::zlib, compression_routines{
                                                                     .compress   = &compress,
                                                                     .decompress = &decompress,
                                                                 });
            }
        } force_init;
    } // namespace

#ifdef EMSCRIPTEN
    ES_KEEP_ALIVE void (*compression_zlibng_keep_alive)() = [] {};
#endif
} // namespace essence::io
