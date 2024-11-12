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
#include "asymmetric_key.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

namespace essence::crypto {
    class pubkey_cipher_provider {
    public:
        ES_API(CPPESSENCE) explicit pubkey_cipher_provider(const asymmetric_key& key);
        ES_API(CPPESSENCE) pubkey_cipher_provider(pubkey_cipher_provider&&) noexcept;
        ES_API(CPPESSENCE) ~pubkey_cipher_provider();
        ES_API(CPPESSENCE) pubkey_cipher_provider& operator=(pubkey_cipher_provider&&) noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) bool encryptor() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) std::shared_ptr<void> context() const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::vector<std::byte> as_bytes(std::span<const std::byte> buffer) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::string as_string(std::span<const std::byte> buffer) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::string as_base64(std::span<const std::byte> buffer) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::string string_from_base64(std::string_view base64) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::vector<std::byte> bytes_from_base64(std::string_view base64) const;

        template <byte_like_contiguous_range Range>
        abi::vector<std::byte> as_bytes(Range&& range) const {
            return as_bytes(as_const_byte_span(range));
        }

        template <byte_like_contiguous_range Range>
        abi::string as_string(Range&& range) const {
            return as_string(as_const_byte_span(range));
        }

        template <byte_like_contiguous_range Range>
        abi::string as_base64(Range&& range) const {
            return as_base64(as_const_byte_span(range));
        }

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::crypto
