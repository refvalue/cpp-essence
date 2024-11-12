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

#include "util.hpp"

#include "crypto/abstract/chunk_processor.hpp"
#include "encoding.hpp"
#include "error.hpp"
#include "error_extensions.hpp"

#include <algorithm>

#include <openssl/buffer.h>
#include <openssl/crypto.h>

namespace essence::crypto {
    namespace {
        constexpr std::array digest_routines{&EVP_sha1, &EVP_sha224, &EVP_sha256, &EVP_sha384, &EVP_sha512,
            &EVP_sha512_224, &EVP_sha512_256, &EVP_sha3_224, &EVP_sha3_256, &EVP_sha3_384, &EVP_sha3_512, &EVP_shake128,
            &EVP_shake256, &EVP_md5, &EVP_sm3};
    }

    const EVP_MD* make_digest_routine(digest_mode mode) {
        return digest_routines.at(static_cast<std::size_t>(mode))();
    }

    digest_mode make_digest_mode(const EVP_MD* routine) {
        if (const auto iter =
                std::ranges::find_if(digest_routines, [&](const EVP_MD* (*inner)()) { return inner() == routine; });
            iter != digest_routines.end()) {
            return static_cast<digest_mode>(std::distance(digest_routines.begin(), iter));
        }

        throw source_code_aware_runtime_error{U8("Invalid digest routine.")};
    }

    bio_unique_ptr make_memory_bio_unique() {
        if (bio_unique_ptr result{BIO_new(BIO_s_mem()), &BIO_free_all}) {
            return result;
        }

        throw crypto_error{U8("Failed to create a BIO based on a memory stream.")};
    }

    bio_unique_ptr make_memory_bio_unique(std::span<const std::byte> buffer) {
        if (bio_unique_ptr result{
                BIO_new_mem_buf(buffer.data(), static_cast<std::int32_t>(buffer.size())), &BIO_free_all}) {
            return result;
        }

        throw crypto_error{U8("Failed to create a memory BIO from the fixed buffer.")};
    }

    std::span<const std::byte> get_memory_bio_buffer(BIO* bio) {
        if (bio == nullptr) {
            throw source_code_aware_runtime_error{U8("The BIO must be non-null")};
        }

        BUF_MEM* memory{};

        if (BIO_get_mem_ptr(bio, &memory) == 1) {
            return {reinterpret_cast<const std::byte*>(memory->data), memory->length};
        }

        throw crypto_error{U8("Failed to get the underlying buffer of the BIO.")};
    }

    bio_unique_ptr make_file_bio_unique(FILE* file, bool auto_close) {
        if (bio_unique_ptr result{BIO_new_fp(file, auto_close ? BIO_CLOSE : BIO_NOCLOSE), &BIO_free_all}) {
            return result;
        }

        throw crypto_error{U8("Failed to create a FILE BIO.")};
    }

    bio_unique_ptr make_file_bio_unique(zstring_view path, zstring_view mode) {
#ifdef _WIN32
        // Workaround for the issue of narrow-string encoding on Windows.
        if (auto file = _wfopen(internal::to_native_string(path).c_str(), internal::to_native_string(mode).c_str()))
#else
        if (auto file = std::fopen(path.c_str(), mode.c_str()))
#endif
        {
            return make_file_bio_unique(file);
        }

        throw source_code_aware_runtime_error{
            U8("Path"), path, U8("Mode"), mode, U8("Message"), U8("Failed to create a FILE.")};
    }

    asn1_object_ptr make_cipher_oid_unique(zstring_view cipher_name) {
        if (const auto cipher = EVP_get_cipherbyname(cipher_name.c_str())) {
            if (asn1_object_ptr result{OBJ_nid2obj(EVP_CIPHER_get_nid(cipher)), &ASN1_OBJECT_free}) {
                return result;
            }

            throw crypto_error{U8("Cipher"), cipher_name, U8("Message"),
                U8("Failed to create an OID representing the symmetric cipher.")};
        }

        throw crypto_error{U8("Cipher"), cipher_name, U8("Message"), U8("Cannot find the symmetric cipher.")};
    }

    evp_pkey_ctx_ptr make_evp_pkey_ctx_unique(zstring_view cipher_name) {
        if (evp_pkey_ctx_ptr result{
                EVP_PKEY_CTX_new_from_name(nullptr, cipher_name.c_str(), nullptr), &EVP_PKEY_CTX_free}) {
            return result;
        }

        throw crypto_error{U8("Cipher"), cipher_name, U8("Message"), U8("Failed to create a EVP_PKEY_CTX.")};
    }

    std::shared_ptr<EVP_PKEY_CTX> make_evp_pkey_ctx_shared(EVP_PKEY* pkey) {
        if (pkey == nullptr) {
            throw source_code_aware_runtime_error{U8("The pkey must be non-null")};
        }

        if (std::shared_ptr<EVP_PKEY_CTX> result{
                EVP_PKEY_CTX_new_from_pkey(nullptr, pkey, nullptr), &EVP_PKEY_CTX_free}) {
            return result;
        }

        throw crypto_error{U8("Failed to create a EVP_PKEY_CTX from the existing pkey.")};
    }

    void evp_pkey_ctx_operate_value_impl(
        const std::function<std::int32_t()>& handler, const source_location& location) {
        if (handler && handler() <= 0) {
            throw crypto_error{location, U8("Failed to get/set the property value of the EVP_PKEY_CTX.")};
        }
    }

    void evp_pkey_ctx_set_buffer_impl(const std::function<std::int32_t(std::span<std::byte> result)>& handler,
        std::span<const std::byte> buffer, const source_location& location) {
        if (handler) {
            if (std::unique_ptr<std::byte, decltype([](std::byte* inner) { OPENSSL_free(inner); })> duplicate_buffer{
                    static_cast<std::byte*>(OPENSSL_malloc(buffer.size()))}) {
                std::ranges::copy(buffer, duplicate_buffer.get());

                evp_pkey_ctx_operate_value_impl(
                    [&] {
                        return handler(std::span{duplicate_buffer.get(), buffer.size()});
                    },
                    location);

                // Lets OpenSSL owns the buffer.
                static_cast<void>(duplicate_buffer.release());
            }
        }
    }

    evp_pkey_ptr evp_pkey_q_keygen_impl(const std::function<EVP_PKEY*()>& handler, zstring_view cipher_name) {
        if (const auto blob = handler()) {
            return evp_pkey_ptr{blob, &EVP_PKEY_free};
        }

        throw crypto_error{
            U8("Cipher Name"), cipher_name, U8("Message"), U8("Failed to generate an asymmetric key pair.")};
    }
} // namespace essence::crypto
