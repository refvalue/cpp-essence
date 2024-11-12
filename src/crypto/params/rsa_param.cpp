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

#include "crypto/params/rsa_param.hpp"

#include "../util.hpp"
#include "char8_t_remediation.hpp"
#include "pubkey_param_impl.hpp"

#include <array>
#include <cstdint>
#include <utility>

#include <openssl/rsa.h>

namespace essence::crypto {
    rsa_param::rsa_param(std::shared_ptr<void> context)
        : impl_{std::make_unique<pubkey_param_impl>(
            std::array{zstring_view{U8("RSA")}, zstring_view{U8("RSA2")}, zstring_view{U8("RSA-PSS")}},
            std::move(context))} {}

    rsa_param::rsa_param(rsa_param&&) noexcept = default;

    rsa_param::~rsa_param() = default;

    rsa_param& rsa_param::operator=(rsa_param&&) noexcept = default;

    rsa_padding_mode rsa_param::padding_mode() const {
        return static_cast<rsa_padding_mode>(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_rsa_padding, impl_->context()));
    }

    rsa_pss_saltlen rsa_param::pss_saltlen() const {
        return static_cast<rsa_pss_saltlen>(
            evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_rsa_pss_saltlen, impl_->context()));
    }

    digest_mode rsa_param::mgf1_digest_mode() const {
        return make_digest_mode(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_rsa_mgf1_md, impl_->context()));
    }

    digest_mode rsa_param::oaep_digest_mode() const {
        return make_digest_mode(evp_pkey_ctx_get_value(&EVP_PKEY_CTX_get_rsa_oaep_md, impl_->context()));
    }

    void rsa_param::set_padding_mode(rsa_padding_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_rsa_padding, impl_->context(), static_cast<std::int32_t>(value));
    }

    void rsa_param::set_pss_saltlen(rsa_pss_saltlen value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_rsa_pss_saltlen, impl_->context(), static_cast<std::int32_t>(value));
    }

    void rsa_param::set_mgf1_digest_mode(digest_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_rsa_mgf1_md, impl_->context(), make_digest_routine(value));
    }

    void rsa_param::set_oaep_digest_mode(digest_mode value) const {
        evp_pkey_ctx_set_value(&EVP_PKEY_CTX_set_rsa_oaep_md, impl_->context(), make_digest_routine(value));
    }

    void rsa_param::set_oaep_label(std::span<const std::byte> value) const {
        evp_pkey_ctx_set_buffer(&EVP_PKEY_CTX_set0_rsa_oaep_label, impl_->context(), value);
    }
} // namespace essence::crypto
