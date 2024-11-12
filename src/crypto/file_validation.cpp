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

#include "crypto/file_validation.hpp"

#include "char8_t_remediation.hpp"
#include "crypto/digest.hpp"
#include "error_extensions.hpp"
#include "string.hpp"

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>

namespace essence::crypto {
    namespace {
        constexpr std::array<std::string_view, 15> digest_mode_texts{
            U8("sha1"),
            U8("sha224"),
            U8("sha256"),
            U8("sha384"),
            U8("sha512"),
            U8("sha512_224"),
            U8("sha512_256"),
            U8("sha3_224"),
            U8("sha3_256"),
            U8("sha3_384"),
            U8("sha3_512"),
            U8("shake128"),
            U8("shake256"),
            U8("md5"),
            U8("sm3"),
        };

        std::filesystem::path make_digest_path(digest_mode mode, std::string_view path) {
            auto extension = digest_mode_texts[static_cast<std::size_t>(mode)];
#ifdef __ANDROID__
            std::filesystem::path fs_path{path};
            auto extension_str = fs_path.extension().generic_u8string();

            fs_path.replace_extension(extension_str.append(".").append(extension));
#else
            std::filesystem::path fs_path{std::u8string{path.begin(), path.end()}};
            auto extension_str = fs_path.extension().generic_u8string();

            fs_path.replace_extension(
                extension_str.append(u8".").append(std::u8string{extension.begin(), extension.end()}));
#endif
            return fs_path;
        }
    } // namespace

    void make_validation_file(digest_mode mode, std::string_view path) {
        const auto digest_path = make_digest_path(mode, path);

        if (std::ofstream stream{digest_path, std::ios::trunc | std::ios::binary | std::ios::out}) {
            const auto digest = make_file_digest(mode, path);

            stream.write(reinterpret_cast<const char*>(digest.c_str()), static_cast<std::streamsize>(digest.size()));
        } else {
            throw source_code_aware_runtime_error{U8("Path"), path, U8("Digest Path"),
                from_u8string(digest_path.u8string()), U8("Message"), U8("Failed to create the validation file.")};
        }
    }

    bool validate_file(digest_mode mode, std::string_view path) {
        const auto digest_path = make_digest_path(mode, path);

        if (std::ifstream stream{digest_path, std::ios::binary | std::ios::in}) {
            const std::string content{std::istreambuf_iterator<char>{stream}, std::istreambuf_iterator<char>{}};

            return icase_string_comparer{}(content, make_file_digest(mode, path));
        }

        return false;
    }
} // namespace essence::crypto
