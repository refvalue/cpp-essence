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

#include "crypto/pubkey_cipher_provider.hpp"

#include "char8_t_remediation.hpp"
#include "cipher_error_builder.hpp"
#include "crypto/digest.hpp"
#include "util.hpp"

#include <cstdint>

#include <openssl/evp.h>

namespace essence::crypto {
    namespace {
        using asymmetric_process_handler = std::int32_t (*)(EVP_PKEY_CTX* ctx, std::uint8_t* output,
            std::size_t* output_size, const std::uint8_t* input, std::size_t input_size);
    }

    class pubkey_cipher_provider::impl {
    public:
        explicit impl(const asymmetric_key& key)
            : encryption_{key.type() == asymmetric_key_type::pub},
              error_builder_{
                  .cipher_name  = key.name(),
                  .routine_name = encryption_ ? U8("Pubkey Encryption") : U8("Pubkey Decryption"),
              },
              process_routine_{encryption_ ? &EVP_PKEY_encrypt : &EVP_PKEY_decrypt},
              context_{make_evp_pkey_ctx_shared(static_cast<EVP_PKEY*>(key.to_blob()))} {
            error_builder_.check_error(
                encryption_ ? EVP_PKEY_encrypt_init(context_.get()) : EVP_PKEY_decrypt_init(context_.get()),
                U8("Failed to initialize the engine."));
        }

        [[nodiscard]] bool encryptor() const noexcept {
            return encryption_;
        }

        [[nodiscard]] std::shared_ptr<void> context() const {
            return context_;
        }

        [[nodiscard]] abi::vector<std::byte> as_bytes(std::span<const std::byte> buffer) const {
            return process_data<abi::vector<std::byte>>(buffer);
        }

        [[nodiscard]] abi::string as_string(std::span<const std::byte> buffer) const {
            return process_data<abi::string>(buffer);
        }

        [[nodiscard]] abi::string as_base64(std::span<const std::byte> buffer) const {
            return base64_encode(as_bytes(buffer));
        }

        [[nodiscard]] abi::string string_from_base64(std::string_view base64) const {
            return as_string(base64_decode(base64));
        }

        [[nodiscard]] abi::vector<std::byte> bytes_from_base64(std::string_view base64) const {
            return as_bytes(base64_decode(base64));
        }

    private:
        template <byte_like_contiguous_range Container>
        [[nodiscard]] Container process_data(std::span<const std::byte> buffer) const {
            Container result;
            std::size_t output_size{};

            error_builder_.check_error(process_routine_(context_.get(), nullptr, &output_size,
                                           reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()),
                U8("Failed to retrieve the output size."));

            result.resize(output_size);

            error_builder_.check_error(
                process_routine_(context_.get(), reinterpret_cast<std::uint8_t*>(result.data()), &output_size,
                    reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()),
                U8("Failed to process the data."));

            return result;
        }

        bool encryption_;
        cipher_error_builder error_builder_;
        asymmetric_process_handler process_routine_;
        std::shared_ptr<EVP_PKEY_CTX> context_;
    };

    pubkey_cipher_provider::pubkey_cipher_provider(const asymmetric_key& key) : impl_{std::make_unique<impl>(key)} {}

    pubkey_cipher_provider::pubkey_cipher_provider(pubkey_cipher_provider&&) noexcept = default;

    pubkey_cipher_provider::~pubkey_cipher_provider() = default;

    pubkey_cipher_provider& pubkey_cipher_provider::operator=(pubkey_cipher_provider&&) noexcept = default;

    bool pubkey_cipher_provider::encryptor() const noexcept {
        return impl_->encryptor();
    }

    std::shared_ptr<void> pubkey_cipher_provider::context() const {
        return impl_->context();
    }

    abi::vector<std::byte> pubkey_cipher_provider::as_bytes(std::span<const std::byte> buffer) const {
        return impl_->as_bytes(buffer);
    }

    abi::string pubkey_cipher_provider::as_string(std::span<const std::byte> buffer) const {
        return impl_->as_string(buffer);
    }

    abi::string pubkey_cipher_provider::as_base64(std::span<const std::byte> buffer) const {
        return impl_->as_base64(buffer);
    }

    abi::string pubkey_cipher_provider::string_from_base64(std::string_view base64) const {
        return impl_->string_from_base64(base64);
    }

    abi::vector<std::byte> pubkey_cipher_provider::bytes_from_base64(std::string_view base64) const {
        return impl_->bytes_from_base64(base64);
    }
} // namespace essence::crypto
