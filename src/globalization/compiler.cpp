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

#include "globalization/compiler.hpp"

#include "abi/json.hpp"
#include "abi/string.hpp"
#include "abi/vector.hpp"
#include "char8_t_remediation.hpp"
#include "common_constants.hpp"
#include "crypto/digest.hpp"
#include "error_extensions.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <span>
#include <string_view>
#include <utility>

namespace essence::globalization {
    namespace {
        auto get_key_value_pairs(const decltype(std::declval<const abi::json>().items())& items) {
            return items | std::views::filter([](const auto& inner) { return inner.value().is_string(); })
                 | std::views::transform([](const auto& inner) {
                       return std::pair{inner.key(), inner.value().template get<abi::string>()};
                   });
        }

        struct default_compiler {
            [[maybe_unused]] static std::uint32_t version() noexcept {
                return common_constants::language_file_version_number;
            }

            [[maybe_unused]] static void to_file(const abi::json& json, std::string_view path) {
                std::ofstream stream;

                try {
                    stream.exceptions(std::ios::badbit | std::ios::failbit);
                    stream.open(std::filesystem::path{to_u8string(path)}, std::ios::out | std::ios::binary);
                } catch (const std::exception& ex) {
                    throw source_code_aware_runtime_error{U8("Language File"), path, U8("Message"),
                        U8("Failed to create the language file."), U8("Internal"), ex.what()};
                }

                // Writes the header.
                stream.write(common_constants::language_file_magic_flag.data(),
                    static_cast<std::streamsize>(common_constants::language_file_magic_flag.size()));

                stream.write(reinterpret_cast<const char*>(common_constants::language_file_version.data()),
                    static_cast<std::streamsize>(common_constants::language_file_version.size()));

                // Writes translated texts.

                for (const auto items = json.items(); auto&& [key, value] : get_key_value_pairs(items)) {
                    stream.write(key.data(), static_cast<std::streamsize>(key.size()));
                    stream.put(common_constants::language_key_value_delimiter);
                    stream.write(value.data(), static_cast<std::streamsize>(value.size()));
                    stream.put(common_constants::language_key_value_terminator);
                }
            }

            static abi::vector<std::byte> to_bytes(const abi::json& json) {
                abi::vector<std::byte> result;

                // Writes the header.
                std::ranges::copy(
                    std::as_bytes(std::span{common_constants::language_file_magic_flag}), std::back_inserter(result));

                std::ranges::copy(
                    std::as_bytes(std::span{common_constants::language_file_version}), std::back_inserter(result));

                // Writes translated texts.

                for (const auto items = json.items(); auto&& [key, value] : get_key_value_pairs(items)) {
                    std::ranges::copy(std::as_bytes(std::span{key}), std::back_inserter(result));
                    result.emplace_back(static_cast<std::byte>(common_constants::language_key_value_delimiter));
                    std::ranges::copy(std::as_bytes(std::span{value}), std::back_inserter(result));
                    result.emplace_back(static_cast<std::byte>(common_constants::language_key_value_terminator));
                }

                return result;
            }

            [[maybe_unused]] static abi::string to_base64(const abi::json& json) {
                return crypto::base64_encode(to_bytes(json));
            }
        };
    } // namespace

    abstract::compiler make_default_compiler() {
        return abstract::compiler{default_compiler{}};
    }
} // namespace essence::globalization
