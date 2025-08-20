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


module essence.crypto;

import std;
import :common_types;
import essence.basic;

namespace essence::crypto {
    namespace {
        constexpr std::array<std::string_view, 15> digest_mode_texts{
            "sha1",
            "sha224",
            "sha256",
            "sha384",
            "sha512",
            "sha512_224",
            "sha512_256",
            "sha3_224",
            "sha3_256",
            "sha3_384",
            "sha3_512",
            "shake128",
            "shake256",
            "md5",
            "sm3",
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

            stream.write(digest.c_str(), static_cast<std::streamsize>(digest.size()));
        } else {
            throw formatted_runtime_error{"Path", path, "Digest Path",
                digest_path.generic_u8string() | std::ranges::to<std::string>(), "Message",
                "Failed to create the validation file."};
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
