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

module;

#include <essence/compat.hpp>

export module essence.io:compresser;

import std;
import :common_types;
import essence.basic;

export namespace essence::io {
    /**
     * A general compresser.
     */
    class compresser {
    public:
        /**
         * Creates an instance.
         * @param mode The compression mode.
         */
        ES_API(CPPESSENCE) explicit compresser(compression_mode mode);

        ES_API(CPPESSENCE) compresser(compresser&&) noexcept;
        ES_API(CPPESSENCE) ~compresser();
        ES_API(CPPESSENCE) compresser& operator=(compresser&&) noexcept;

        /**
         * Compresses a byte buffer.
         * @param buffer The buffer.
         * @param level The compression level.
         * @return The compressed data.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::vector<std::byte> as_bytes(
            std::span<const std::byte> buffer, std::int32_t level) const;

        /**
         * Compresses a byte buffer as a string.
         * @param buffer The buffer.
         * @param level The compression level.
         * @return The compressed data as a string.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string
            as_string(std::span<const std::byte> buffer, std::int32_t level) const;

        /**
         * Decompress a byte buffer.
         * @param buffer The buffer.
         * @return The decompressed data.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::vector<std::byte> inverse_as_bytes(
            std::span<const std::byte> buffer) const;

        /**
         * Decompresses a byte buffer as a string.
         * @param buffer The buffer.
         * @return The decompressed data.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string inverse_as_string(std::span<const std::byte> buffer) const;

        /**
         * Compresses a byte buffer.
         * @tparam Range The type of the range.
         * @param level The compression level.
         * @return The compressed data.
         */
        template <byte_like_contiguous_range Range>
        abi::vector<std::byte> as_bytes(Range&& range, std::int32_t level) const {
            return as_bytes(as_const_byte_span(range), level);
        }

        /**
         * Compresses a byte buffer as a string.
         * @tparam Range The type of the range.
         * @param level The compression level.
         * @return The compressed data as a string.
         */
        template <byte_like_contiguous_range Range>
        abi::string as_string(Range&& range, std::int32_t level) const {
            return as_string(as_const_byte_span(range), level);
        }

        /**
         * Decompress a byte buffer.
         * @tparam Range The type of the range.
         * @return The decompressed data.
         */
        template <byte_like_contiguous_range Range>
        abi::vector<std::byte> inverse_as_bytes(Range&& range) const {
            return inverse_as_bytes(as_const_byte_span(range));
        }

        /**
         * Decompresses a byte buffer as a string.
         * @tparam Range The type of the range.
         * @return The decompressed data.
         */
        template <byte_like_contiguous_range Range>
        abi::string inverse_as_string(Range&& range) const {
            return inverse_as_string(as_const_byte_span(range));
        }

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::io
