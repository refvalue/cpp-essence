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

#include "../compat.hpp"
#include "../range.hpp"
#include "../zstring_view.hpp"
#include "abstract/chunk_processor.hpp"
#include "common_types.hpp"

#include <array>
#include <cstddef>
#include <span>
#include <utility>

namespace essence::crypto {
    /**
     * @brief Creates a chunk processor for Base64 encoding.
     * @param newlines Whether to append a newline every 64 characters to satisfy the requirements of the PEM format.
     * @return The chunk processor.
     */
    ES_API(CPPESSENCE) abstract::chunk_processor make_base64_encoder(bool newlines = false);

    /**
     * @brief Creates a chunk processor for Base64 decoding.
     * @return The chunk processor.
     */
    ES_API(CPPESSENCE) abstract::chunk_processor make_base64_decoder();

    /**
     * @brief Creates a chunk processor for a symmetric cipher.
     * @param cipher_name The name of the cipher.
     * @param padding_mode The padding mode.
     * @param key The symmetric key.
     * @param iv The initialization vector.
     * @param encryption True to create an encryptor; otherwise a decryptor.
     * @return The chunk processor.
     */
    ES_API(CPPESSENCE)
    abstract::chunk_processor make_symmetric_cipher_chunk_processor(zstring_view cipher_name,
        cipher_padding_mode padding_mode, std::span<const std::byte> key, std::span<const std::byte> iv,
        bool encryption = true);

    /**
     * @brief Creates a chunk processor for a symmetric cipher.
     * @tparam KeyRange The type of the key range.
     * @tparam IVRange The type of the IV range.
     * @param cipher_name The name of the cipher.
     * @param padding_mode The padding mode.
     * @param key The symmetric key.
     * @param iv The initialization vector.
     * @param encryption True to create an encryptor; otherwise a decryptor.
     * @return The chunk processor.
     */
    template <byte_like_contiguous_range KeyRange, byte_like_contiguous_range IVRange>
    abstract::chunk_processor make_symmetric_cipher_chunk_processor(zstring_view cipher_name,
        cipher_padding_mode padding_mode, KeyRange&& key, IVRange&& iv, bool encryption = true) {
        return make_symmetric_cipher_chunk_processor(
            cipher_name, padding_mode, as_const_byte_span(key), as_const_byte_span(iv), encryption);
    }

    /**
     * @brief Chains multiple chunk processor together sequentially and returns a new single chunk processor.
     * @param processors The processors to be chained.
     * @return The new single chunk processors.
     */
    ES_API(CPPESSENCE)
    abstract::chunk_processor chain_chunk_processors(std::span<abstract::chunk_processor> processors);

    /**
     * @brief Chains multiple chunk processor together sequentially and returns a new single chunk processor.
     * @tparam Args The types of given chunk processors.
     * @param args The processors to be chained.
     * @return The new single chunk processors.
     */
    template <typename... Args>
        requires(std::same_as<std::decay_t<Args>, abstract::chunk_processor> && ...)
    abstract::chunk_processor chain_chunk_processors(Args&&... args) {
        std::array processors{std::move(args)...};

        return chain_chunk_processors(processors);
    }
} // namespace essence::crypto
