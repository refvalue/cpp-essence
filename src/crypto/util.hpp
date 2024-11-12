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

#include "char.hpp"
#include "crypto/common_types.hpp"
#include "source_location.hpp"
#include "zstring_view.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <functional>
#include <memory>
#include <span>
#include <tuple>
#include <type_traits>
#include <utility>

#include <openssl/evp.h>

namespace essence::crypto {
    using bio_unique_ptr   = std::unique_ptr<BIO, void (*)(BIO*)>;
    using asn1_object_ptr  = std::unique_ptr<ASN1_OBJECT, void (*)(ASN1_OBJECT*)>;
    using evp_pkey_ptr     = std::unique_ptr<EVP_PKEY, void (*)(EVP_PKEY*)>;
    using evp_pkey_ctx_ptr = std::unique_ptr<EVP_PKEY_CTX, void (*)(EVP_PKEY_CTX*)>;

    const EVP_MD* make_digest_routine(digest_mode mode);
    digest_mode make_digest_mode(const EVP_MD* routine);

    bio_unique_ptr make_memory_bio_unique();
    bio_unique_ptr make_memory_bio_unique(std::span<const std::byte> buffer);
    std::span<const std::byte> get_memory_bio_buffer(BIO* bio);

    bio_unique_ptr make_file_bio_unique(FILE* file, bool auto_close = true);
    bio_unique_ptr make_file_bio_unique(zstring_view path, zstring_view mode);

    asn1_object_ptr make_cipher_oid_unique(zstring_view cipher_name);
    evp_pkey_ctx_ptr make_evp_pkey_ctx_unique(zstring_view cipher_name);
    std::shared_ptr<EVP_PKEY_CTX> make_evp_pkey_ctx_shared(EVP_PKEY* pkey);

    void evp_pkey_ctx_operate_value_impl(
        const std::function<std::int32_t()>& handler, const source_location& location = source_location::current());

    void evp_pkey_ctx_set_buffer_impl(const std::function<std::int32_t(std::span<std::byte> result)>& handler,
        std::span<const std::byte> buffer, const source_location& location = source_location::current());

    evp_pkey_ptr evp_pkey_q_keygen_impl(const std::function<EVP_PKEY*()>& handler, zstring_view cipher_name);

    template <std::default_initializable T>
    T evp_pkey_ctx_get_value(std::int32_t (*handler)(EVP_PKEY_CTX* ctx, T* result), EVP_PKEY_CTX* ctx,
        const source_location& location = source_location::current()) {
        T result{};

        if (handler) {
            evp_pkey_ctx_operate_value_impl([&]() { return handler(ctx, &result); }, location);
        }

        return result;
    }

    template <typename T>
    void evp_pkey_ctx_set_value(std::int32_t (*handler)(EVP_PKEY_CTX* ctx, T value), EVP_PKEY_CTX* ctx, T value,
        const source_location& location = source_location::current()) {
        if (handler) {
            evp_pkey_ctx_operate_value_impl([&]() { return handler(ctx, value); }, location);
        }
    }

    template <typename T>
        requires(byte_like<T> || std::is_void_v<T>)
    void evp_pkey_ctx_set_buffer(std::int32_t (*handler)(EVP_PKEY_CTX* ctx, T* buffer, std::int32_t size),
        EVP_PKEY_CTX* ctx, std::span<const std::byte> buffer,
        const source_location& location = source_location::current()) {
        if (handler) {
            evp_pkey_ctx_set_buffer_impl(
                [&](std::span<std::byte> result) {
                    return handler(ctx, reinterpret_cast<T*>(result.data()), static_cast<std::int32_t>(result.size()));
                },
                buffer, location);
        }
    }

    template <typename... Args>
    evp_pkey_ptr evp_pkey_q_keygen(zstring_view cipher_name, Args&&... args) {
        auto tuple = std::forward_as_tuple(std::forward<Args>(args)...);

        return evp_pkey_q_keygen_impl(
            [&cipher_name, &tuple] {
                return std::apply(
                    [&cipher_name]<typename... InnerArgs>(InnerArgs&&... args) {
                        return EVP_PKEY_Q_keygen(
                            nullptr, nullptr, cipher_name.c_str(), std::forward<InnerArgs>(args)...);
                    },
                    std::move(tuple));
            },
            cipher_name);
    }
} // namespace essence::crypto
