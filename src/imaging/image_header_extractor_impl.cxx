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

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <istream>
#include <span>
#include <string_view>

namespace essence::imaging {
    /**
     * @brief A helper class template to implement a concrete image header extractor.
     * @tparam Signature A byte array of the signature to locate the beginning of the content.
     * @tparam NeededSize The needed size of the content to be fetched.
     * @tparam BytesToBeSkipped The count of bytes to be skipped before searching for the signature.
     */
    template <std::size_t NeededSize, std::array Signature, std::size_t BytesToBeSkipped = 0>
        requires std::same_as<typename decltype(Signature)::value_type, std::byte>
    struct image_header_extractor_impl {
        static constexpr auto needed_size         = NeededSize;
        static constexpr auto bytes_to_be_skipped = BytesToBeSkipped;
        static constexpr auto&& signature         = Signature;

        [[nodiscard]] static std::span<const std::byte> get(std::istream& stream) {
            thread_local std::array<std::byte, signature.size() + needed_size> buffer{};
            auto read_and_check = [&] {
                return stream.read(
                                 reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(signature.size()))
                               .gcount()
                        == signature.size()
                    && std::equal(buffer.begin(), buffer.begin() + signature.size(), signature.begin())
                    && stream.read(reinterpret_cast<char*>(buffer.data() + signature.size()),
                                 static_cast<std::streamsize>(needed_size))
                               .gcount()
                           == needed_size;
            };

            if (bytes_to_be_skipped != 0) {
                stream.seekg(static_cast<std::streamoff>(bytes_to_be_skipped), std::ios::cur);
            }

            while (!stream.eof() && !read_and_check()) {
            }

            if (stream.eof()) {
                return {};
            }

            return std::span{buffer.data() + signature.size(), needed_size};
        }

        [[nodiscard]] static std::span<const std::byte> get(std::span<const std::byte> buffer) {
            std::string_view str{reinterpret_cast<const char*>(buffer.data()), buffer.size()};
            auto read_and_check = [&] {};

            if (auto index =
                    str.find(reinterpret_cast<const char*>(signature.data()), bytes_to_be_skipped, signature.size());
                index != std::string_view::npos && buffer.size() - index > needed_size) {
                return buffer.subspan(index + signature.size());
            }

            return {};
        }
    };
} // namespace essence::imaging
