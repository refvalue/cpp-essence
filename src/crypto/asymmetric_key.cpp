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

#include "crypto/asymmetric_key.hpp"

#include "char8_t_remediation.hpp"
#include "error.hpp"
#include "util.hpp"

#include <algorithm>
#include <concepts>
#include <optional>
#include <utility>

#include <openssl/evp.h>
#include <openssl/pem.h>

extern "C" {
int evp_keymgmt_util_has(EVP_PKEY* pk, int selection);
}

namespace essence::crypto {
    namespace {
        struct encrypted_key_info {
            zstring_view cipher_name;
            std::string_view password;
        };

        template <typename Tag>
        using conditional_encrypted_key_info = std::conditional_t<std::same_as<Tag, use_public_tag>,
            std::type_identity<void>, std::optional<encrypted_key_info>>;

        template <typename Tag>
        EVP_PKEY* load_key_impl(Tag, BIO* bio, const password_request_handler& handler) {
            static constexpr auto pair = [] {
                if constexpr (std::same_as<Tag, use_public_tag>) {
                    return std::pair{U8("Public Key"), &PEM_read_bio_PUBKEY};
                } else {
                    return std::pair{U8("Private Key"), &PEM_read_bio_PrivateKey};
                }
            }();

            static constexpr auto pem_password_handler = [](char* buffer, std::int32_t size, std::int32_t rw_flag,
                                                             void* user_data) {
                if (const auto handler = static_cast<const password_request_handler*>(user_data);
                    handler && *handler && rw_flag == 0) {
                    bool cancelled{};
                    auto password = (*handler)(static_cast<std::size_t>(size), cancelled);

                    if (cancelled) {
                        return -1;
                    }

                    const auto final_size = std::min(static_cast<std::size_t>(size), password.size());

                    std::ranges::copy_n(password.begin(), static_cast<std::ptrdiff_t>(final_size), buffer);

                    return static_cast<std::int32_t>(final_size);
                }

                return PEM_def_callback(buffer, size, rw_flag, nullptr);
            };

            // The read routines return either a pointer to the structure read or NULL if an error occurred.
            if (EVP_PKEY * result{};
                pair.second(bio, &result, pem_password_handler, const_cast<password_request_handler*>(&handler))) {
                return result;
            }

            throw crypto_error{U8("Category"), pair.first, U8("Message"), U8("Failed to load the key from the BIO.")};
        }

        template <typename Tag>
        void save_key_impl(
            Tag, BIO* bio, const EVP_PKEY* blob, conditional_encrypted_key_info<Tag> encrypted_key_info) {
            static constexpr std::string_view category{
                std::same_as<Tag, use_public_tag> ? U8("Public Key") : U8("Private Key")};

            // The write routines return 1 for success or 0 for failure.
            if (auto code =
                    [&] {
                        if constexpr (std::same_as<Tag, use_public_tag>) {
                            return PEM_write_bio_PUBKEY(bio, blob);
                        } else {
                            if (!encrypted_key_info) {
                                return PEM_write_bio_PKCS8PrivateKey(bio, blob, nullptr, nullptr, 0, nullptr, nullptr);
                            }

                            if (auto cipher = EVP_get_cipherbyname(encrypted_key_info->cipher_name.c_str())) {
                                return PEM_write_bio_PKCS8PrivateKey(bio, blob, cipher,
                                    encrypted_key_info->password.data(),
                                    static_cast<std::int32_t>(encrypted_key_info->password.size()), nullptr, nullptr);
                            }

                            throw crypto_error{U8("Category"), category, U8("Cipher"), encrypted_key_info->cipher_name,
                                U8("Message"), U8("Failed to find the symmetric cipher.")};
                        }
                    }();
                code != 1) {
                throw crypto_error{U8("Category"), category, U8("Message"), U8("Failed to save the key into the BIO.")};
            }
        }

        template <typename Tag>
        EVP_PKEY* load_key(Tag tag, zstring_view path, const password_request_handler& handler) {
            const auto bio = make_file_bio_unique(path, U8("rb"));

            return load_key_impl(tag, bio.get(), handler);
        }

        template <typename Tag>
        EVP_PKEY* load_key(Tag tag, std::span<const std::byte> buffer, const password_request_handler& handler) {
            const auto bio = make_memory_bio_unique(buffer);

            return load_key_impl(tag, bio.get(), handler);
        }

        template <typename Tag>
        void save_key(
            Tag tag, zstring_view path, const EVP_PKEY* blob, conditional_encrypted_key_info<Tag> cipher_info = {}) {
            const auto bio = make_file_bio_unique(path, U8("wb"));

            save_key_impl(tag, bio.get(), blob, cipher_info);
        }

        template <typename Tag>
        abi::string save_key(Tag tag, const EVP_PKEY* blob, conditional_encrypted_key_info<Tag> cipher_info = {}) {
            const auto bio = make_memory_bio_unique();

            save_key_impl(tag, bio.get(), blob, cipher_info);

            const auto buffer = get_memory_bio_buffer(bio.get());

            return abi::string{reinterpret_cast<const char*>(buffer.data()),
                reinterpret_cast<const char*>(buffer.data()) + buffer.size()};
        }
    } // namespace

    class asymmetric_key::impl {
    public:
        explicit impl(void* blob)
            : blob_{blob ? static_cast<EVP_PKEY*>(blob)
                         : throw source_code_aware_runtime_error{U8("The blob must be non-null.")}} {}

        [[nodiscard]] EVP_PKEY* blob() const noexcept {
            return blob_.get();
        }

    private:
        std::unique_ptr<EVP_PKEY, decltype([](EVP_PKEY* inner) { EVP_PKEY_free(inner); })> blob_;
    };

    asymmetric_key::asymmetric_key(void* blob) : impl_{std::make_unique<impl>(blob)} {}

    asymmetric_key::asymmetric_key(use_public_tag tag, zstring_view path, const password_request_handler& handler)
        : asymmetric_key{load_key(tag, path, handler)} {}

    asymmetric_key::asymmetric_key(
        use_public_tag tag, std::span<const std::byte> buffer, const password_request_handler& handler)
        : asymmetric_key{load_key(tag, buffer, handler)} {}

    asymmetric_key::asymmetric_key(use_private_tag tag, zstring_view path, const password_request_handler& handler)
        : asymmetric_key{load_key(tag, path, handler)} {}

    asymmetric_key::asymmetric_key(
        use_private_tag tag, std::span<const std::byte> buffer, const password_request_handler& handler)
        : asymmetric_key{load_key(tag, buffer, handler)} {}

    asymmetric_key::asymmetric_key(asymmetric_key&&) noexcept = default;

    asymmetric_key::~asymmetric_key() = default;

    asymmetric_key& asymmetric_key::operator=(asymmetric_key&&) noexcept = default;

    asymmetric_key_type asymmetric_key::type() const {
        // Uses the undocumented function.
        return evp_keymgmt_util_has(impl_->blob(), OSSL_KEYMGMT_SELECT_PRIVATE_KEY) ? asymmetric_key_type::pair
                                                                                    : asymmetric_key_type::pub;
    }

    abi::string asymmetric_key::name() const {
        return EVP_PKEY_get0_type_name(impl_->blob());
    }

    abi::string asymmetric_key::description() const {
        return EVP_PKEY_get0_description(impl_->blob());
    }

    void* asymmetric_key::to_blob() const noexcept {
        return impl_->blob();
    }

    abi::string asymmetric_key::save_public() const {
        return save_key(use_public, impl_->blob());
    }

    void asymmetric_key::save_public(zstring_view path) const {
        save_key(use_public, path, impl_->blob());
    }

    abi::string asymmetric_key::save_private() const {
        return save_key(use_private, impl_->blob());
    }

    abi::string asymmetric_key::save_private(zstring_view cipher_name, std::string_view password) const {
        return save_key(use_private, impl_->blob(), encrypted_key_info{cipher_name, password});
    }

    void asymmetric_key::save_private(zstring_view path) const {
        save_key(use_private, path, impl_->blob());
    }

    void asymmetric_key::save_private(zstring_view path, zstring_view cipher_name, std::string_view password) const {
        save_key(use_private, path, impl_->blob(), encrypted_key_info{cipher_name, password});
    }
} // namespace essence::crypto
