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

#include "crypto/params/dh_param.hpp"

#include "../util.hpp"
#include "char8_t_remediation.hpp"
#include "pubkey_param_impl.hpp"

#include <array>
#include <utility>

#include <openssl/dh.h>

namespace essence::crypto {
    dh_param::dh_param(std::shared_ptr<void> context)
        : impl_{std::make_unique<pubkey_param_impl>(
            std::array{zstring_view{U8("DH")}, zstring_view{U8("DHX")}}, std::move(context))} {}

    dh_param::dh_param(dh_param&&) noexcept = default;

    dh_param::~dh_param() = default;

    dh_param& dh_param::operator=(dh_param&&) noexcept = default;

    dh_kdf_type dh_param::kdf_mode() const {
        return static_cast<dh_kdf_type>(EVP_PKEY_CTX_get_dh_kdf_type(impl_->context()));
    }

    digest_mode dh_param::kdf_digest_mode() const {
        return make_digest_mode(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_dh_kdf_md, impl_->context()));
    }

    std::int32_t dh_param::kdf_digest_outlen() const {
        return evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_dh_kdf_outlen, impl_->context());
    }

    void dh_param::set_padding(bool value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_pad, impl_->context(), static_cast<std::int32_t>(value));
    }

    void dh_param::set_nid(dh_nid value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_nid, impl_->context(), static_cast<std::int32_t>(value));
    }

    void dh_param::set_rfc5114(dhx_rfc5114 value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_rfc5114, impl_->context(), static_cast<std::int32_t>(value));
    }

    void dh_param::set_kdf_type(dh_kdf_type value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_kdf_type, impl_->context(), static_cast<std::int32_t>(value));
    }

    void dh_param::set_kdf_cipher_name(zstring_view value) const {
        if (auto oid = make_cipher_oid_unique(value)) {
            evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set0_dh_kdf_oid, impl_->context(), oid.get());
            static_cast<void>(oid.release());
        }
    }

    void dh_param::set_kdf_digest_mode(digest_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_kdf_md, impl_->context(), make_digest_routine(value));
    }

    void dh_param::set_kdf_digest_outlen(std::int32_t value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_dh_kdf_outlen, impl_->context(), value);
    }

    void dh_param::set_kdf_ukm(std::span<const std::byte> value) const {
        evp_pkey_ctx_set_buffer(&EVP_PKEY_CTX_set0_dh_kdf_ukm, impl_->context(), value);
    }
} // namespace essence::crypto
