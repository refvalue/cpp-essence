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

#include <zstd.h>
#include <zstd_errors.h>

namespace essence::io {
    namespace {
        std::size_t check_error(
            std::size_t content_size, const source_location& location = source_location::current()) {
            if (ZSTD_isError(content_size)) {
                const auto code = ZSTD_getErrorCode(content_size);

                throw source_code_aware_runtime_error{
                    location, U8("Code"), static_cast<std::int32_t>(code), U8("Message"), ZSTD_getErrorString(code)};
            }

            return content_size;
        }

        void compress(std::span<const std::byte> buffer, const abstract::writable_buffer& result, std::int32_t level) {
            thread_local std::unique_ptr<ZSTD_CCtx, decltype(&ZSTD_freeCCtx)> context(ZSTD_createCCtx(), ZSTD_freeCCtx);
            const auto size = check_error(ZSTD_compressBound(buffer.size()));

            result.resize(size);

            const auto actual_size = check_error(ZSTD_compressCCtx(
                context.get(), result.data(), result.size_bytes(), buffer.data(), buffer.size(), level));

            result.resize(actual_size);
            result.shrink_to_fit();
        }

        void decompress(std::span<const std::byte> buffer, const abstract::writable_buffer& result) {
            const auto content_size = check_error(ZSTD_getFrameContentSize(buffer.data(), buffer.size()));

            result.resize(content_size);
            check_error(ZSTD_decompress(result.data(), result.size_bytes(), buffer.data(), buffer.size()));
        }

        ES_KEEP_ALIVE [[maybe_unused]] struct init {
            init() {
                add_compression_routines(compression_mode::zstd, compression_routines{
                                                                     .compress   = &compress,
                                                                     .decompress = &decompress,
                                                                 });
            }
        } force_init;
    } // namespace

#ifdef EMSCRIPTEN
    ES_KEEP_ALIVE void (*compression_zstd_keep_alive)() = [] {};
#endif
} // namespace essence::io
