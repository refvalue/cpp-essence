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

#include "globalization/translator.hpp"

#include "char8_t_remediation.hpp"
#include "common_constants.hpp"
#include "error_extensions.hpp"
#include "globalization/abstract/translator.hpp"
#include "io/abstract/virtual_fs_operator.hpp"
#include "io/fs_operator.hpp"
#include "string.hpp"

#include <atomic>
#include <cstddef>
#include <exception>
#include <filesystem>
#include <fstream>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include <spdlog/spdlog.h>

namespace essence::globalization {
    namespace {
        /**
         * @brief A translator whose working directory and language are immutable for thread safety.
         */
        class immutable_translator {
        public:
            immutable_translator(io::abstract::virtual_fs_operator fs_operator, std::string_view working_directory,
                std::string_view language)
                : language_{language}, working_directory_{working_directory}, fs_operator_{std::move(fs_operator)} {
                auto path = std::filesystem::path{to_u8string(working_directory)} / to_u8string(language);

                path.replace_extension(common_constants::language_file_extension);

                auto path_str = from_u8string(path.generic_u8string());

                if (!fs_operator_.exists(path_str)) {
                    throw source_code_aware_runtime_error{
                        U8("Language File"), path_str, U8("Message"), U8("The language file does not exist.")};
                }

                parse_language_file(path_str);

                spdlog::info(
                    U8("Successfully loaded {} entries from the language file: {}."), entries_.size(), path_str);
            }

            static std::uint32_t version() noexcept {
                return common_constants::language_file_version_number;
            }

            [[nodiscard]] io::abstract::virtual_fs_operator virtual_fs() const {
                return fs_operator_;
            }

            [[nodiscard]] abi::string working_directory() const {
                return working_directory_;
            }

            [[nodiscard]] abi::string language() const {
                return language_;
            }

            [[nodiscard]] abi::string get_text(std::string_view name) const {
                if (const auto iter = entries_.find(name); iter != entries_.end()) {
                    return iter->second;
                }

                return abi::string{name};
            }

        private:
            void parse_language_file(std::string_view path) {
                auto raise_error = [&]<typename... Args>(Args&&... args) {
                    throw source_code_aware_runtime_error{
                        U8("Language File"), path, U8("Message"), std::forward<Args>(args)...};
                };

                // Attempts to open the language file and handles the possible errors.
                const auto stream = [&] {
                    try {
                        return fs_operator_.open_read(path, std::ios::in | std::ios::binary);
                    } catch (const std::exception& ex) {
                        raise_error(U8("Failed to open the language file."), U8("Internal"), ex.what());
                        throw;
                    }
                }();

                thread_local std::remove_const_t<decltype(common_constants::language_file_magic_flag)> magic_flag{};

                // Analyzes the header.
                if (stream->read(magic_flag.data(), magic_flag.size()).gcount() < magic_flag.size()
                    || magic_flag != common_constants::language_file_magic_flag) {
                    raise_error(U8("Invalid magic flag."));
                }

                thread_local std::remove_const_t<decltype(common_constants::language_file_version)> version{};

                // Verifies the version.
                if (stream->read(reinterpret_cast<char*>(version.data()), version.size()).gcount()
                        < sizeof(common_constants::language_file_version)
                    || version != common_constants::language_file_version) {
                    raise_error(U8("Invalid file version."));
                }

                abi::string chunk;

                // Parses the body --> key1\30value1\31key2\30value2\31
                while (std::getline(*stream, chunk, common_constants::language_key_value_terminator)) {
                    std::string_view view{chunk};

                    if (const auto key_size = view.find(common_constants::language_key_value_delimiter);
                        key_size != std::string_view::npos) {
                        auto key   = view.substr(0, key_size);
                        auto value = view.substr(key_size + 1);

                        entries_.insert_or_assign(abi::string{key}, abi::string{value});
                    }
                }
            }

            abi::string language_;
            abi::string working_directory_;
            io::abstract::virtual_fs_operator fs_operator_;
            std::unordered_map<abi::string, abi::string, string_hash, std::equal_to<>> entries_;
        };

        /**
         * @brief A thread-safe implementation of the default translator without locks.
         */
        class default_translator {
        public:
            explicit default_translator(io::abstract::virtual_fs_operator fs_operator = io::get_native_fs_operator())
                : impl_{std::make_shared<immutable_translator>(std::move(fs_operator),
                      common_constants::default_working_folder, common_constants::default_language)} {}

            [[nodiscard]] [[maybe_unused]] std::uint32_t version() const noexcept {
                return current()->version();
            }

            [[nodiscard]] io::abstract::virtual_fs_operator virtual_fs() const {
                return current()->virtual_fs();
            }

            [[maybe_unused]] void set_virtual_fs(const io::abstract::virtual_fs_operator& fs_operator) {
                update(fs_operator, current()->working_directory(), current()->language());
            }

            [[nodiscard]] [[maybe_unused]] abi::string working_directory() const {
                return current()->working_directory();
            }

            [[maybe_unused]] void set_working_directory(std::string_view directory) {
                update(current()->virtual_fs(), directory, current()->language());
            }

            [[nodiscard]] [[maybe_unused]] abi::string language() const {
                return current()->language();
            }

            [[maybe_unused]] void set_language(std::string_view name) {
                update(current()->virtual_fs(), current()->working_directory(), name);
                spdlog::info(U8("Switched to {}."), name);
            }

            [[nodiscard]] [[maybe_unused]] abi::string get_text(std::string_view name) const {
                return current()->get_text(name);
            }

        private:
            void update(const io::abstract::virtual_fs_operator& fs_operator, std::string_view working_directory,
                std::string_view language) {
                std::atomic_store_explicit(&impl_,
                    std::make_shared<immutable_translator>(fs_operator, working_directory, language),
                    std::memory_order::release);
            }

            [[nodiscard]] std::shared_ptr<immutable_translator> current() const noexcept {
                return std::atomic_load_explicit(&impl_, std::memory_order::acquire);
            }

            std::shared_ptr<immutable_translator> impl_;
        };
    } // namespace

    abstract::translator make_translator() {
        return abstract::translator{default_translator{}};
    }

    abstract::translator make_translator(io::abstract::virtual_fs_operator fs_operator) {
        return abstract::translator{default_translator{std::move(fs_operator)}};
    }
} // namespace essence::globalization
