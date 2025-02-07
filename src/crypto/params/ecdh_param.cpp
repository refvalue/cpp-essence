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

#include <essence/char8_t_remediation.hpp>

#include <openssl/ec.h>

module essence.crypto;
import :params.pubkey_param_impl;
import :util;
import essence.basic;

namespace essence::crypto {
    ecdh_param::ecdh_param(std::shared_ptr<void> context)
        : opaque_{new pubkey_param_impl{std::array{zstring_view{U8("ECDH")}}, std::move(context)},
              pubkey_param_impl_deleter} {}

    ecdh_param::ecdh_param(ecdh_param&&) noexcept = default;

    ecdh_param::~ecdh_param() = default;

    ecdh_param& ecdh_param::operator=(ecdh_param&&) noexcept = default;

    dh_cofactor_mode ecdh_param::cofactor_mode() const {
        return static_cast<dh_cofactor_mode>(EVP_PKEY_CTX_get_ecdh_cofactor_mode(get_impl(opaque_).context()));
    }

    dh_kdf_type ecdh_param::kdf_mode() const {
        return static_cast<dh_kdf_type>(EVP_PKEY_CTX_get_ecdh_kdf_type(get_impl(opaque_).context()));
    }

    digest_mode ecdh_param::kdf_digest_mode() const {
        return make_digest_mode(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_ecdh_kdf_md, get_impl(opaque_).context()));
    }

    std::int32_t ecdh_param::kdf_digest_outlen() const {
        return evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_ecdh_kdf_outlen, get_impl(opaque_).context());
    }

    void ecdh_param::set_cofactor_mode(dh_cofactor_mode value) const {
        evp_pkey_ctx_set_value(
            &EVP_PKEY_CTX_set_ecdh_cofactor_mode, get_impl(opaque_).context(), static_cast<std::int32_t>(value));
    }

    void ecdh_param::set_kdf_type(dh_kdf_type value) const {
        evp_pkey_ctx_set_value(
            &EVP_PKEY_CTX_set_ecdh_kdf_type, get_impl(opaque_).context(), static_cast<std::int32_t>(value));
    }

    void ecdh_param::set_kdf_digest_mode(digest_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_ecdh_kdf_md, get_impl(opaque_).context(), make_digest_routine(value));
    }

    void ecdh_param::set_kdf_digest_outlen(std::int32_t value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_ecdh_kdf_outlen, get_impl(opaque_).context(), value);
    }

    void ecdh_param::set_kdf_ukm(std::span<const std::byte> value) const {
        evp_pkey_ctx_set_buffer(&EVP_PKEY_CTX_set0_ecdh_kdf_ukm, get_impl(opaque_).context(), value);
    }
} // namespace essence::crypto
