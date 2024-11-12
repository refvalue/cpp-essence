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

#include "crypto/params/ecdh_param.hpp"

#include "../util.hpp"
#include "char8_t_remediation.hpp"
#include "pubkey_param_impl.hpp"
#include "zstring_view.hpp"

#include <array>
#include <utility>

#include <openssl/ec.h>

namespace essence::crypto {
    ecdh_param::ecdh_param(std::shared_ptr<void> context)
        : impl_{std::make_unique<pubkey_param_impl>(std::array{zstring_view{U8("ECDH")}}, std::move(context))} {}

    ecdh_param::ecdh_param(ecdh_param&&) noexcept = default;

    ecdh_param::~ecdh_param() = default;

    ecdh_param& ecdh_param::operator=(ecdh_param&&) noexcept = default;

    dh_cofactor_mode ecdh_param::cofactor_mode() const {
        return static_cast<dh_cofactor_mode>(EVP_PKEY_CTX_get_ecdh_cofactor_mode(impl_->context()));
    }

    dh_kdf_type ecdh_param::kdf_mode() const {
        return static_cast<dh_kdf_type>(EVP_PKEY_CTX_get_ecdh_kdf_type(impl_->context()));
    }

    digest_mode ecdh_param::kdf_digest_mode() const {
        return make_digest_mode(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_ecdh_kdf_md, impl_->context()));
    }

    std::int32_t ecdh_param::kdf_digest_outlen() const {
        return evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_ecdh_kdf_outlen, impl_->context());
    }

    void ecdh_param::set_cofactor_mode(dh_cofactor_mode value) const {
        evp_pkey_ctx_set_value(
            &EVP_PKEY_CTX_set_ecdh_cofactor_mode, impl_->context(), static_cast<std::int32_t>(value));
    }

    void ecdh_param::set_kdf_type(dh_kdf_type value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_ecdh_kdf_type, impl_->context(), static_cast<std::int32_t>(value));
    }

    void ecdh_param::set_kdf_digest_mode(digest_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_ecdh_kdf_md, impl_->context(), make_digest_routine(value));
    }

    void ecdh_param::set_kdf_digest_outlen(std::int32_t value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_ecdh_kdf_outlen, impl_->context(), value);
    }

    void ecdh_param::set_kdf_ukm(std::span<const std::byte> value) const {
        evp_pkey_ctx_set_buffer(&EVP_PKEY_CTX_set0_ecdh_kdf_ukm, impl_->context(), value);
    }
} // namespace essence::crypto
