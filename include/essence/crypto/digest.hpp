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

#include "../abi/string.hpp"
#include "../abi/vector.hpp"
#include "../compat.hpp"
#include "../range.hpp"
#include "../zstring_view.hpp"
#include "common_types.hpp"

#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

namespace essence::crypto {
    /**
     * @brief Encodes a memory buffer into a hexadecimal string.
     * @param buffer The memory buffer.
     * @param delimiter An optional delimiter to be inserted between two hexadecimal units.
     * @return The hexadecimal string.
     * @see hex_decode()
     * @code{.cpp}
     *     constexpr std::array mac_addr{ 0xE4, 0x54, 0xE8, 0x81, 0xFC, 0xFD };
     *     auto mac_str = hex_encode(mac_addr, ':');
     *
     *     printf("%s\n", mac_str.c_str());
     * @endcode
     */
    ES_API(CPPESSENCE) abi::string hex_encode(std::span<const std::byte> buffer, std::optional<char> delimiter = {});

    /**
     * @brief Decodes a hexadecimal string into a byte array.
     * @param hex The hexadecimal string.
     * @param delimiter An optional delimiter assumed existing between two hexadecimal units within "hex".
     * @return The byte array.
     * @see hex_encode()
     */
    ES_API(CPPESSENCE) abi::vector<std::byte> hex_decode(zstring_view hex, std::optional<char> delimiter = {});

    /**
     * @brief Digests a memory buffer in MD5 mode.
     * @deprecated Use make_digest instead.
     * @param buffer The memory buffer.
     * @return The MD5 digest encoded in a hexadecimal string.
     * @see make_digest()
     */
    ES_API(CPPESSENCE) abi::string md5_hash(std::span<const std::byte> buffer);

    /**
     * @brief Encodes a memory buffer into a base64 string.
     * @param buffer The memory buffer.
     * @return The base64 encoded string.
     * @see base64_decode()
     */
    ES_API(CPPESSENCE) abi::string base64_encode(std::span<const std::byte> buffer);

    /**
     * @brief Decodes a base64 encoded string.
     * @param encoded_text The base64 encoded string.
     * @return The decoded memory buffer.
     * @see base64_encode()
     */
    ES_API(CPPESSENCE) abi::vector<std::byte> base64_decode(std::string_view encoded_text);

    /**
     * @brief Calculates a HMAC hash.
     * @param mode The hashing mode.
     * @param key The HMAC key.
     * @param buffer The memory buffer.
     * @return The hash code.
     */
    ES_API(CPPESSENCE)
    abi::string hmac_hash(digest_mode mode, std::string_view key, std::span<const std::byte> buffer);

    /**
     * @brief Calculates a hash.
     * @param mode The hashing mode.
     * @param buffer The memory buffer.
     * @return The hash code.
     */
    ES_API(CPPESSENCE) abi::string make_digest(digest_mode mode, std::span<const std::byte> buffer);

    /**
     * @brief Calculates the hash of a file.
     * @param mode The hashing mode.
     * @param path The file path.
     * @return The hash code.
     */
    ES_API(CPPESSENCE) abi::string make_file_digest(digest_mode mode, std::string_view path);

    template <byte_like_contiguous_range Range>
    abi::string hex_encode(
        Range&& range, std::optional<char> delimiter = {}) {
        return hex_encode(as_const_byte_span(range), delimiter);
    }

    template <byte_like_contiguous_range Range>
    abi::string md5_hash(Range&& range) {
        return md5_hash(as_const_byte_span(range));
    }

    template <byte_like_contiguous_range Range>
    abi::string base64_encode(Range&& range) {
        return base64_encode(as_const_byte_span(range));
    }

    template <byte_like_contiguous_range Range>
    abi::string hmac_hash(
        digest_mode mode, std::string_view key, Range&& range) {
        return hmac_hash(mode, key, as_const_byte_span(range));
    }

    template <byte_like_contiguous_range Range>
    abi::string make_digest(digest_mode mode, Range&& range) {
        return make_digest(mode, as_const_byte_span(range));
    }
} // namespace essence::crypto
