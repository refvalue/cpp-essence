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

#include "../abi/string.hpp"
#include "../compat.hpp"
#include "../zstring_view.hpp"
#include "common_types.hpp"

#include <cstddef>
#include <memory>
#include <span>
#include <string_view>

namespace essence::crypto {
    /**
     * @brief Represents an asymmetric key or key pair for a pubkey encryption algorithm.
     * @see pubkey_cipher_provider
     */
    class asymmetric_key {
    public:
        /**
         * @brief Creates an instance from an internal blob.
         * @warning DO NOT USE this constructor unless you know what you are doing.
         * @param blob a pointer to the internal blob.
        */
        ES_API(CPPESSENCE) explicit asymmetric_key(void* blob);

        /**
         * @brief Creates an instance from a public key stored on the disk.
         * @param tag The hint tag for overloading, should always be essence::crypto::use_public.
         * @param path The path of the public key file.
         * @param handler An optional callable object to retrieve the password having encrypted the key.
         * @see password_request_handler
         */
        ES_API(CPPESSENCE)
        asymmetric_key(use_public_tag tag, zstring_view path, const password_request_handler& handler = {});

        /**
         * @brief Creates an instance from a memory buffer storing a public key.
         * @param tag the hint tag for overloading, should always be essence::crypto::use_public.
         * @param buffer the memory buffer containing the public key.
         * @param handler an optional callable object to retrieve the password having encrypted the key.
         */
        ES_API(CPPESSENCE)
        asymmetric_key(
            use_public_tag tag, std::span<const std::byte> buffer, const password_request_handler& handler = {});

        /**
         * @brief Creates an instance from a private key stored on the disk.
         * @param tag The hint tag for overloading, should always be essence::crypto::use_private.
         * @param path The path of the private key file.
         * @param handler An optional callable object to retrieve the password having encrypted the key.
         * @see password_request_handler
         */
        ES_API(CPPESSENCE)
        asymmetric_key(use_private_tag tag, zstring_view path, const password_request_handler& handler = {});

        /**
         * @brief Creates an instance from a memory buffer storing a private key.
         * @param tag The hint tag for overloading, should always be essence::crypto::use_private.
         * @param buffer The memory buffer containing the private key.
         * @param handler An optional callable object to retrieve the password having encrypted the key.
         */
        ES_API(CPPESSENCE)
        asymmetric_key(
            use_private_tag tag, std::span<const std::byte> buffer, const password_request_handler& handler = {});

        ES_API(CPPESSENCE) asymmetric_key(asymmetric_key&&) noexcept;
        ES_API(CPPESSENCE) ~asymmetric_key();
        ES_API(CPPESSENCE) asymmetric_key& operator=(asymmetric_key&&) noexcept;

        /**
         * @brief Gets the type of the asymmetric key.
         * @return The type of the asymmetric key.
         * @see asymmetric_key_type
         */
        [[nodiscard]] ES_API(CPPESSENCE) asymmetric_key_type type() const;

        /**
         * @brief Gets the name of the asymmetric key.
         * @return The name of the asymmetric key.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string name() const;

        /**
         * @brief Gets the description of the asymmetric key.
         * @return The description of the asymmetric key.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string description() const;

        /**
         * @brief Gets the internal blob.
         * @return The internal blob.
         */
        [[nodiscard]] ES_API(CPPESSENCE) void* to_blob() const noexcept;

        /**
         * @brief Saves the public key to a string.
         * @return The string containing the public key.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string save_public() const;

        /**
         * @brief Saves the public key to a file.
         * @param path The path of the file.
         */
        ES_API(CPPESSENCE) void save_public(zstring_view path) const;

        /**
         * @brief Saves the private key to a string.
         * @return The string containing the private key.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string save_private() const;

        /**
         * @brief Encrypts and saves the private key to a string.
         * @param cipher_name The name of a symmetric cipher to encrypt the key.
         * @param password The password coded in displayable characters to encrypt the key.
         * @return The string containing the encrypted private key.
         */
        [[nodiscard]] ES_API(CPPESSENCE) abi::string
            save_private(zstring_view cipher_name, std::string_view password) const;

        /**
         * @brief Saves the private key to a file.
         * @param path The path of the file.
         */
        ES_API(CPPESSENCE) void save_private(zstring_view path) const;

        /**
         * @brief Encrypts and saves the private key to a file.
         * @param path The path of the file.
         * @param cipher_name The name of a symmetric cipher to encrypt the key.
         * @param password The password coded in displayable characters to encrypt the key.
         */
        ES_API(CPPESSENCE)
        void save_private(zstring_view path, zstring_view cipher_name, std::string_view password) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };

    /**
     * @brief Generates an asymmetric key pair.
     * @tparam T The parameter type matching the pubkey_keygen_param concept.
     * @param param The parameter object.
     * @return The generated asymmetric key pair.
     * @see ec_keygen_param
     * @see ed448_keygen_param
     * @see ed25519_keygen_param
     * @see rsa_keygen_param
     * @see sm2_keygen_param
     * @see x448_keygen_param
     * @see x25519_keygen_param
     */
    template <pubkey_keygen_param T>
    asymmetric_key generate_asymmetric_key_pair(const T& param) {
        return asymmetric_key{param.generate_key_blob()};
    }
} // namespace essence::crypto
