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

#include "char8_t_remediation.hpp"
#include "chunk_processing_helper.hpp"
#include "cipher_error_builder.hpp"
#include "crypto/chunk_processor.hpp"
#include "crypto/symmetric_cipher_util.hpp"

#include <cstdint>
#include <memory>

#include <openssl/evp.h>

namespace essence::crypto {
    namespace {
        constexpr std::array padding_modes{0, EVP_PADDING_PKCS7};
        constexpr auto cipher_ctx_deleter = [](EVP_CIPHER_CTX* inner) { EVP_CIPHER_CTX_free(inner); };

        class symmetric_cipher_processor {
        public:
            symmetric_cipher_processor(zstring_view cipher_name, cipher_padding_mode padding_mode,
                std::span<const std::byte> key, std::span<const std::byte> iv, bool encryption)
                : encryption_{encryption}, padding_mode_{padding_mode},
                  builder_{
                      .cipher_name  = abi::string{cipher_name},
                      .routine_name = encryption ? U8("Encrytion") : U8("Decryption"),
                  },
                  helper_{
                      .raw_update   = &EVP_CipherUpdate,
                      .raw_finalize = &EVP_CipherFinal_ex,
                      .check_error  = [this](std::int32_t code,
                                         std::string_view message) { builder_.check_error(code, message); },
                  },
                  context_{EVP_CIPHER_CTX_new()} {

                if (builder_.cipher_name.empty()) {
                    builder_.raise_error(U8("The cipher name must be non-empty."));
                }

                auto cipher_info = get_symmetric_cipher_info(cipher_name);

                if (!cipher_info) {
                    builder_.raise_error(U8("Could not find the cipher name."));
                }

                if (key.size() != cipher_info->key_length) {
                    builder_.raise_error(U8("Expected Key Length"), cipher_info->key_length, U8("Actual Key Length"),
                        key.size(), U8("Message"),
                        U8("The actual key length must be equal to the expected key length of the cipher."));
                }

                if (iv.size() != cipher_info->iv_length) {
                    builder_.raise_error(U8("Expected IV Length"), cipher_info->iv_length, U8("Actual IV Length"),
                        iv.size(), U8("Message"),
                        U8("The actual IV length must be equal to the expected IV length of the cipher."));
                }

                builder_.check_error(EVP_CipherInit_ex(context_.get(), static_cast<const EVP_CIPHER*>(cipher_info->id),
                                         nullptr, reinterpret_cast<const std::uint8_t*>(key.data()),
                                         reinterpret_cast<const std::uint8_t*>(iv.data()), encryption),
                    U8("An error occurred during the initialization."));
            }

            [[nodiscard]] [[maybe_unused]] bool transformer() const noexcept {
                return encryption_;
            }

            [[nodiscard]] [[maybe_unused]] abi::string cipher_name() const {
                return builder_.cipher_name;
            }

            [[nodiscard]] [[maybe_unused]] static std::size_t buffer_size() noexcept {
                return 4096;
            }

            [[nodiscard]] [[maybe_unused]] static std::size_t extra_size() noexcept {
                return EVP_MAX_BLOCK_LENGTH;
            }

            [[nodiscard]] [[maybe_unused]] static rational size_factor() noexcept {
                return rational{1, 1};
            }

            [[maybe_unused]] void init() const {
                builder_.check_error(EVP_CipherInit_ex(context_.get(), nullptr, nullptr, nullptr, nullptr, encryption_),
                    U8("An error occurred during the re-initialization."));

                // Always succeeds.
                // https://www.openssl.org/docs/man1.0.2/man3/EVP_CIPHER_CTX_set_padding.html
                static_cast<void>(
                    EVP_CIPHER_CTX_set_padding(context_.get(), padding_modes[static_cast<std::size_t>(padding_mode_)]));
            }

            [[maybe_unused]] void update(std::span<const std::byte> input, std::span<std::byte>& output) const {
                helper_.update(context_.get(), input, output);
            }

            [[maybe_unused]] void finalize(std::span<std::byte>& output) const {
                helper_.finalize(context_.get(), output);
            }

        private:
            bool encryption_;
            cipher_padding_mode padding_mode_;
            const cipher_error_builder builder_;
            const chunk_processing_helper<EVP_CIPHER_CTX> helper_;
            std::unique_ptr<EVP_CIPHER_CTX, decltype(cipher_ctx_deleter)> context_;
        };
    } // namespace

    abstract::chunk_processor make_symmetric_cipher_chunk_processor(zstring_view cipher_name,
        cipher_padding_mode padding_mode, std::span<const std::byte> key, std::span<const std::byte> iv,
        bool encryption) {
        return abstract::chunk_processor{symmetric_cipher_processor{cipher_name, padding_mode, key, iv, encryption}};
    }
} // namespace essence::crypto
