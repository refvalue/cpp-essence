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

#include "crypto/symmetric_cipher_provider.hpp"

#include "crypto/digest.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <ranges>

#include <openssl/evp.h>

namespace essence::crypto {
    namespace {
        struct update_tag {};
        struct finalization_tag {};
    } // namespace

    class symmetric_cipher_provider::impl {
    public:
        explicit impl(abstract::chunk_processor processor) : processor_{std::move(processor)} {}

        [[nodiscard]] abi::string cipher_name() const {
            return processor_.cipher_name();
        }

        [[nodiscard]] bool encryptor() const noexcept {
            return processor_.transformer();
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
            static constexpr std::size_t intermediate_size = 4096;
            thread_local std::array<std::byte, intermediate_size + EVP_MAX_BLOCK_LENGTH> intermediate;

            Container result;

            auto intermediate_data =
                reinterpret_cast<const std::ranges::range_value_t<Container>*>(intermediate.data());

            processor_.init();

            auto process_chunk = [&]<typename Tag>(Tag, const std::byte* chunk, std::size_t size) {
                std::span<std::byte> output{intermediate};

                if constexpr (std::same_as<Tag, update_tag>) {
                    processor_.update(std::span{chunk, size}, output);
                } else {
                    processor_.finalize(output);
                }

                std::ranges::copy(intermediate_data, intermediate_data + output.size(), std::back_inserter(result));
            };

            // Processes the complete chunks.
            const auto complete_ptr_end = buffer.data() + buffer.size() / intermediate_size * intermediate_size;

            for (auto ptr = buffer.data(); ptr < complete_ptr_end; ptr += intermediate_size) {
                process_chunk(update_tag{}, ptr, intermediate_size);
            }

            // Processes the remaining chunk.
            if (const auto ptr_end = buffer.data() + buffer.size(); complete_ptr_end < ptr_end) {
                process_chunk(update_tag{}, complete_ptr_end, ptr_end - complete_ptr_end);
            }

            process_chunk(finalization_tag{}, nullptr, 0U);

            return result;
        }

        abstract::chunk_processor processor_;
    };

    symmetric_cipher_provider::symmetric_cipher_provider(abstract::chunk_processor processor)
        : impl_{std::make_unique<impl>(std::move(processor))} {}

    symmetric_cipher_provider::symmetric_cipher_provider(symmetric_cipher_provider&&) noexcept = default;

    symmetric_cipher_provider::~symmetric_cipher_provider() = default;

    symmetric_cipher_provider& symmetric_cipher_provider::operator=(symmetric_cipher_provider&&) noexcept = default;

    abi::string symmetric_cipher_provider::cipher_name() const {
        return impl_->cipher_name();
    }

    bool symmetric_cipher_provider::encryptor() const noexcept {
        return impl_->encryptor();
    }

    abi::vector<std::byte> symmetric_cipher_provider::as_bytes(std::span<const std::byte> buffer) const {
        return impl_->as_bytes(buffer);
    }

    abi::string symmetric_cipher_provider::as_string(std::span<const std::byte> buffer) const {
        return impl_->as_string(buffer);
    }

    abi::string symmetric_cipher_provider::as_base64(std::span<const std::byte> buffer) const {
        return impl_->as_base64(buffer);
    }

    abi::string symmetric_cipher_provider::string_from_base64(std::string_view base64) const {
        return impl_->string_from_base64(base64);
    }

    abi::vector<std::byte> symmetric_cipher_provider::bytes_from_base64(std::string_view base64) const {
        return impl_->bytes_from_base64(base64);
    }
} // namespace essence::crypto
