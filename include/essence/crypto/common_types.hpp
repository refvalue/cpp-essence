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

#include <concepts>
#include <cstddef>
#include <functional>

namespace essence::crypto {
    struct use_public_tag {};
    struct use_private_tag {};

    inline static constexpr use_public_tag use_public{};
    inline static constexpr use_private_tag use_private{};

    /**
     * @brief The available padding modes of a symmetric cipher.
     */
    enum class cipher_padding_mode {
        none,
        pkcs7,
    };

    /**
     * @brief The available digest algorithms.
     */
    enum class digest_mode {
        sha1,
        sha224,
        sha256,
        sha384,
        sha512,
        sha512_224,
        sha512_256,
        sha3_224,
        sha3_256,
        sha3_384,
        sha3_512,
        shake128,
        shake256,
        md5,
        sm3,
    };

    /**
     */
    enum class rsa_padding_mode {
        pkcs1 = 1,
        none  = 3,
        pkcs1_oaep,
        x931,
        pkcs1_pss,
        pkcs1_with_tls,
    };

    enum class rsa_pss_saltlen {
        automatic = -2,
        digest,
        max = -3,
    };

    enum class dh_cofactor_mode {
        none = -1,
        disabled,
        enabled,
    };

    enum class dh_kdf_type {
        none = 1,
        x9_63,
    };

    enum dh_nid {
        none,
        ffdhe2048 = 1126,
        ffdhe3072,
        ffdhe4096,
        ffdhe6144,
        ffdhe8192,
        modp_1536 = 1212,
        modp_2048,
        modp_3072,
        modp_4096,
        modp_6144,
        modp_8192,
    };

    enum class dhx_rfc5114 {
        none,
        section_2_1,
        section_2_2,
        section_2_3,
    };

    /**
     * @brief The types of an asymmetric key.
     */
    enum class asymmetric_key_type {
        pub,
        pair,
    };

    template <typename T>
    concept pubkey_keygen_param = requires(const T obj) {
        { obj.generate_key_blob() } -> std::convertible_to<void*>;
    };

    using password_request_handler = std::function<abi::string(std::size_t max_size, bool& cancelled)>;
} // namespace essence::crypto
