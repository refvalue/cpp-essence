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
#include "io/fs_operator.hpp"

#include <concepts>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string_view>
#include <system_error>

namespace essence::io {
    namespace {
        template <typename T>
            requires(std::derived_from<T, std::istream> || std::derived_from<T, std::ostream>)
        std::unique_ptr<T> make_unique_stream(std::string_view path, std::ios_base::openmode mode) {
            auto stream = std::make_unique<T>();

            stream->exceptions(std::ios_base::badbit);
            stream->open(std::filesystem::path{to_u8string(path)}, mode);

            return stream;
        }

        struct native_fs_operator {
            [[nodiscard]] [[maybe_unused]] static bool exists(std::string_view path) noexcept {
                std::error_code code;

                return std::filesystem::exists(to_u8string(path), code);
            }

            [[nodiscard]] [[maybe_unused]] static bool is_file(std::string_view path) noexcept {
                std::error_code code;

                return std::filesystem::is_regular_file(to_u8string(path), code);
            }

            [[nodiscard]] [[maybe_unused]] static bool is_directory(std::string_view path) noexcept {
                std::error_code code;

                return std::filesystem::is_directory(to_u8string(path), code);
            }

            [[nodiscard]] [[maybe_unused]] static std::unique_ptr<std::iostream> open(
                std::string_view path, std::ios_base::openmode mode) {
                return make_unique_stream<std::fstream>(path, mode);
            }

            [[nodiscard]] [[maybe_unused]] static std::unique_ptr<std::istream> open_read(
                std::string_view path, std::ios_base::openmode mode) {
                return make_unique_stream<std::ifstream>(path, mode);
            }

            [[nodiscard]] [[maybe_unused]] static std::unique_ptr<std::ostream> open_write(
                std::string_view path, std::ios_base::openmode mode) {
                return make_unique_stream<std::ofstream>(path, mode);
            }
        };
    } // namespace

    const abstract::virtual_fs_operator& get_native_fs_operator() {
        static const abstract::virtual_fs_operator fs_operator{native_fs_operator{}};

        return fs_operator;
    }
} // namespace essence::io
