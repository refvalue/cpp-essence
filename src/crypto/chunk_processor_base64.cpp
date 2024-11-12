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

#include <cstdint>
#include <memory>
#include <type_traits>

#include <openssl/evp.h>

extern "C" void evp_encode_ctx_set_flags(EVP_ENCODE_CTX* ctx, unsigned int flags);

namespace essence::crypto {
    namespace {
        constexpr auto encode_ctx_deleter = [](EVP_ENCODE_CTX* inner) { EVP_ENCODE_CTX_free(inner); };

        const cipher_error_builder encoding_builder{
            .cipher_name  = U8("base64"),
            .routine_name = U8("Encoding"),
        };

        const cipher_error_builder decoding_builder{
            .cipher_name  = U8("base64"),
            .routine_name = U8("Decoding"),
        };

        const chunk_processing_helper base64_encoding_helper{
            .raw_update   = &EVP_EncodeUpdate,
            .raw_finalize = &EVP_EncodeFinal,
            .check_error  = [](std::int32_t code,
                               std::string_view message) { encoding_builder.check_error(code, message); },
        };

        const chunk_processing_helper base64_decoding_helper{
            .raw_update   = &EVP_DecodeUpdate,
            .raw_finalize = &EVP_DecodeFinal,
            .check_error  = [](std::int32_t code,
                               std::string_view message) { decoding_builder.check_error(code, message); },
        };

        template <bool Encoder>
        class base64_processor {
        public:
            using finalize_result_type = std::conditional_t<Encoder, void, std::int32_t>;

            explicit base64_processor(bool newlines)
                : flags_{newlines ? 0U : 1U}, context_{[] {
                      if (auto context = EVP_ENCODE_CTX_new()) {
                          return context;
                      }

                      throw source_code_aware_runtime_error{U8("Failed to allocate the base64 encoding context.")};
                  }()},
                  helper_{[]() -> const auto& {
                      if constexpr (Encoder) {
                          return base64_encoding_helper;
                      } else {
                          return base64_decoding_helper;
                      }
                  }()} {}

            [[nodiscard]] static bool transformer() noexcept {
                return Encoder;
            }

            [[nodiscard]] static abi::string cipher_name() {
                if constexpr (Encoder) {
                    return encoding_builder.cipher_name;
                } else {
                    return decoding_builder.cipher_name;
                }
            }

            [[nodiscard]] static std::size_t buffer_size() noexcept {
                return 4096;
            }

            [[nodiscard]] static std::size_t extra_size() noexcept {
                return 0U;
            }

            [[nodiscard]] static rational size_factor() noexcept {
                return rational{4, 3};
            }

            void init() const {
                if constexpr (Encoder) {
                    EVP_EncodeInit(context_.get());
                    evp_encode_ctx_set_flags(context_.get(), flags_);
                } else {
                    EVP_DecodeInit(context_.get());
                }
            }

            void update(std::span<const std::byte> input, std::span<std::byte>& output) const {
                helper_.update(context_.get(), input, output);
            }

            void finalize(std::span<std::byte>& output) const {
                helper_.finalize(context_.get(), output);
            }

        private:
            std::uint32_t flags_;
            std::unique_ptr<EVP_ENCODE_CTX, decltype(encode_ctx_deleter)> context_;
            const chunk_processing_helper<EVP_ENCODE_CTX, finalize_result_type>& helper_;
        };
    } // namespace

    abstract::chunk_processor make_base64_encoder(bool newlines) {
        return abstract::chunk_processor{base64_processor<true>{newlines}};
    }

    abstract::chunk_processor make_base64_decoder() {
        return abstract::chunk_processor{base64_processor<false>{false}};
    }
} // namespace essence::crypto
