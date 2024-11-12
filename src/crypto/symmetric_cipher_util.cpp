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

#include "crypto/symmetric_cipher_util.hpp"

#include <openssl/evp.h>

namespace essence::crypto {
    abi::vector<abi::string> get_all_symmetric_cipher_names() {
        abi::vector<abi::string> result;

        EVP_CIPHER_do_all_sorted(
            [](const EVP_CIPHER* cipher, const char* from, const char* to, void* context) {
                static_cast<abi::vector<abi::string>*>(context)->emplace_back(from);
            },
            &result);

        return result;
    }

    std::optional<symmetric_cipher_info> get_symmetric_cipher_info(zstring_view cipher_name) {
        if (const auto cipher = EVP_get_cipherbyname(cipher_name.c_str())) {
            return symmetric_cipher_info{
                .id         = cipher,
                .iv_length  = static_cast<std::size_t>(EVP_CIPHER_iv_length(cipher)),
                .key_length = static_cast<std::size_t>(EVP_CIPHER_key_length(cipher)),
                .block_size = static_cast<std::size_t>(EVP_CIPHER_block_size(cipher)),
            };
        }

        return std::nullopt;
    }
}
