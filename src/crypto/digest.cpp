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

#include "crypto/digest.hpp"

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "util.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <utility>

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>

namespace essence::crypto {
    namespace {
        template <std::invocable<EVP_MD_CTX*> Callable>
        abi::string make_digest_impl(digest_mode mode, Callable&& update_handler) {
            std::unique_ptr<EVP_MD_CTX, decltype([](EVP_MD_CTX* inner) { EVP_MD_CTX_free(inner); })> context{
                EVP_MD_CTX_new()};

            if (!EVP_DigestInit_ex2(context.get(), make_digest_routine(mode), nullptr)) {
                throw source_code_aware_runtime_error{U8("Failed to initialize the digest.")};
            }

            std::forward<Callable>(update_handler)(context.get());

            std::uint32_t hash_size{};
            thread_local std::array<std::byte, EVP_MAX_MD_SIZE> hash{};

            if (!EVP_DigestFinal_ex(context.get(), reinterpret_cast<std::uint8_t*>(hash.data()), &hash_size)) {
                throw source_code_aware_runtime_error{U8("Failed to finalize the digest.")};
            }

            return hex_encode(std::span<const std::byte>{hash.data(), hash_size});
        }
    } // namespace

    abi::string hex_encode(std::span<const std::byte> buffer, std::optional<char> delimiter) {
        std::size_t size{};
        const auto inner_delimiter = delimiter ? *delimiter : U8('\0');

        if (OPENSSL_buf2hexstr_ex(
                nullptr, 0, &size, reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size(), inner_delimiter)
            == 0) {
            throw source_code_aware_runtime_error{
                U8("Failed to calculate the required size of the hexadecimal buffer.")};
        }

        abi::string result(size, U8('\0'));

        if (OPENSSL_buf2hexstr_ex(result.data(), result.size(), &size,
                reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size(), inner_delimiter)
            == 0) {
            throw source_code_aware_runtime_error{U8("Failed to generate the hexadecimal string.")};
        }

        if (!result.empty()) {
            result.pop_back();
        }

        return result;
    }

    abi::vector<std::byte> hex_decode(zstring_view hex, std::optional<char> delimiter) {
        std::size_t size{};
        const auto inner_delimiter = delimiter ? *delimiter : U8('\0');

        if (OPENSSL_hexstr2buf_ex(nullptr, 0, &size, hex.c_str(), inner_delimiter) == 0) {
            throw source_code_aware_runtime_error{U8("Failed to calculate the required size of the original buffer.")};
        }

        abi::vector<std::byte> result(size);

        if (OPENSSL_hexstr2buf_ex(
                reinterpret_cast<std::uint8_t*>(result.data()), result.size(), &size, hex.c_str(), inner_delimiter)
            == 0) {
            throw source_code_aware_runtime_error{U8("Failed to decode the hexadecimal string.")};
        }

        return result;
    }

    abi::string md5_hash(std::span<const std::byte> buffer) {
        return make_digest(digest_mode::md5, buffer);
    }

    abi::string base64_encode(std::span<const std::byte> buffer) {
        if (buffer.empty()) {
            return abi::string{};
        }

        // https://www.openssl.org/docs/man3.0/man3/EVP_EncodeBlock.html
        abi::string result((buffer.size() + 2) / 3 * 4, U8('\0'));
        auto actual_size = EVP_EncodeBlock(reinterpret_cast<std::uint8_t*>(result.data()),
            reinterpret_cast<const std::uint8_t*>(buffer.data()), static_cast<std::int32_t>(buffer.size()));

        if (result.size() != static_cast<std::size_t>(actual_size)) {
            throw source_code_aware_runtime_error{U8("Excepted Size"), result.size(), U8("Actual Size"), actual_size,
                U8("Message"), U8("The expected size must be equal to the actual size.")};
        }

        return result;
    }

    abi::vector<std::byte> base64_decode(std::string_view encoded_text) {
        if (encoded_text.empty()) {
            return {};
        }

        if (encoded_text.size() % 4 != 0) {
            throw source_code_aware_runtime_error{U8("Base64 Text Length"), encoded_text.size(), U8("Message"),
                U8("Illegal length of the base64 text, which should be divisible by 4.")};
        }

        // Detects all paddings.
        const auto padding_size = [&]() -> std::size_t {
            if (const auto iter =
                    std::find_if_not(encoded_text.rbegin(), encoded_text.rend(), [](char c) { return c == U8('='); });
                iter != encoded_text.rend()) {
                return static_cast<std::size_t>(std::distance(encoded_text.rbegin(), iter));
            }

            return 0;
        }();

        // https://www.openssl.org/docs/man3.0/man3/EVP_DecodeBlock.html
        abi::vector<std::byte> result(encoded_text.size() / 4 * 3);
        auto actual_size = EVP_DecodeBlock(reinterpret_cast<std::uint8_t*>(result.data()),
            reinterpret_cast<const std::uint8_t*>(encoded_text.data()), static_cast<std::int32_t>(encoded_text.size()));

        if (actual_size == -1) {
            throw source_code_aware_runtime_error{U8("An error occurred when invoking \"EVP_DecodeBlock\".")};
        }

        if (result.size() != static_cast<std::size_t>(actual_size)) {
            throw source_code_aware_runtime_error{U8("Excepted Size"), result.size(), U8("Actual Size"), actual_size,
                U8("Message"), U8("The actual size must be equal to the expected size.")};
        }

        // Removes all padding zeros.
        if (result.size() >= padding_size) {
            result.resize(result.size() - padding_size);
        }

        return result;
    }

    abi::string hmac_hash(digest_mode mode, std::string_view key, std::span<const std::byte> buffer) {
        std::uint32_t hash_size{};
        std::array<std::byte, EVP_MAX_MD_SIZE> hash{};

        const auto result = HMAC(make_digest_routine(mode), key.data(), static_cast<std::int32_t>(key.size()),
            reinterpret_cast<const std::uint8_t*>(buffer.data()), static_cast<std::int32_t>(buffer.size()),
            reinterpret_cast<std::uint8_t*>(hash.data()), &hash_size);

        if (result == nullptr) {
            throw source_code_aware_runtime_error{U8("An error occurred when invoking \"HMAC\".")};
        }

        return base64_encode(std::span{hash.data(), hash_size});
    }

    abi::string make_digest(digest_mode mode, std::span<const std::byte> buffer) {
        if (buffer.data() == nullptr) {
            throw source_code_aware_runtime_error{U8("A input buffer with null data is not allowed.")};
        }

        return make_digest_impl(mode, [&](EVP_MD_CTX* context) {
            if (!EVP_DigestUpdate(context, buffer.data(), buffer.size())) {
                throw source_code_aware_runtime_error{U8("Failed to update the digest.")};
            }
        });
    }

    abi::string make_file_digest(digest_mode mode, std::string_view path) {
#ifdef __ANDROID__
        std::filesystem::path fs_path{path};
#else
        std::filesystem::path fs_path{std::u8string{path.begin(), path.end()}};
#endif
        std::ifstream stream{fs_path, std::ios::in | std::ios::binary};

        if (!stream) {
            throw source_code_aware_runtime_error{U8("Path"), path, U8("Message"), U8("Failed to open the file.")};
        }

        thread_local std::array<char, 4096> chunk{};

        return make_digest_impl(mode, [&](EVP_MD_CTX* context) {
            // Hashing the whole file block by block.
            while (!stream.eof()) {

                if (auto actual_size = stream.read(chunk.data(), static_cast<std::streamsize>(chunk.size())).gcount();
                    !EVP_DigestUpdate(context, chunk.data(), actual_size)) {
                    throw source_code_aware_runtime_error{U8("Chunk size"), actual_size, U8("Message"),
                        U8("Failed to update the digest by the current chunk.")};
                }
            }
        });
    }
} // namespace essence::crypto
