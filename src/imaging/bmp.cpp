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

#include "abi/string.hpp"
#include "char8_t_remediation.hpp"
#include "compat.hpp"
#include "extractors.hpp"
#include "image_header_extractor_impl.hpp"
#include "imaging/image_general_header.hpp"
#include "io/abstract/bitstream_type_hint.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>

namespace essence::imaging {
    namespace {
        constexpr std::array leading_signature{
            std::byte{'B'},
            std::byte{'M'},
        };

        constexpr std::array<std::byte, 0> trailing_signature{};

        constexpr std::array bmp_info_header_signature{
            // Little-endian.
            std::byte{0x28},
            std::byte{0},
            std::byte{0},
            std::byte{0},
        };

        constexpr std::size_t bmp_bytes_to_be_skipped     = 12;
        constexpr std::size_t bmp_info_header_needed_size = 12;

        struct bmp_hint {
            [[maybe_unused]] static abi::string name() {
                return U8("BMP");
            }

            [[maybe_unused]] static std::span<const abi::string> file_extensions() noexcept {
                static const std::array<abi::string, 1> extensions{U8(".bmp")};

                return extensions;
            }

            [[maybe_unused]] static std::size_t leading_signature_size() noexcept {
                return imaging::leading_signature.size();
            }

            [[maybe_unused]] static std::span<const std::byte> leading_signature() noexcept {
                return imaging::leading_signature;
            }

            [[maybe_unused]] static std::string_view leading_signature_str() noexcept {
                return std::string_view{reinterpret_cast<const char*>(imaging::leading_signature.data()),
                    imaging::leading_signature.size()};
            }

            [[maybe_unused]] static std::size_t trailing_signature_size() noexcept {
                return imaging::trailing_signature.size();
            }

            [[maybe_unused]] static std::span<const std::byte> trailing_signature() noexcept {
                return imaging::trailing_signature;
            }

            [[maybe_unused]] static std::string_view trailing_signature_str() noexcept {
                return std::string_view{reinterpret_cast<const char*>(imaging::trailing_signature.data()),
                    imaging::trailing_signature.size()};
            }
        };

        std::optional<image_general_header> extract_header(std::span<const std::byte> buffer) noexcept {
            if (!buffer.empty()) {
                return image_general_header{
                    .width = std::to_integer<std::int32_t>(buffer[0]) + (std::to_integer<std::int32_t>(buffer[1]) << 8)
                           + (std::to_integer<std::int32_t>(buffer[2]) << 16)
                           + (std::to_integer<std::int32_t>(buffer[3]) << 24),

                    .height = std::to_integer<std::int32_t>(buffer[4]) + (std::to_integer<std::int32_t>(buffer[5]) << 8)
                            + (std::to_integer<std::int32_t>(buffer[6]) << 16)
                            + (std::to_integer<std::int32_t>(buffer[7]) << 24),

                    .bits_per_pixel =
                        std::to_integer<std::int32_t>(buffer[10]) + (std::to_integer<std::int32_t>(buffer[11]) << 8),
                    .channels =
                        (std::to_integer<std::int32_t>(buffer[10]) + (std::to_integer<std::int32_t>(buffer[11]) << 8))
                        / std::numeric_limits<std::uint8_t>::digits,
                };
            }

            return std::nullopt;
        }

        struct bmp_header_extractor {
            using impl_type = image_header_extractor_impl<bmp_info_header_needed_size, bmp_info_header_signature,
                bmp_bytes_to_be_skipped>;

            [[maybe_unused]] static io::abstract::bitstream_type_hint hint() {
                static const io::abstract::bitstream_type_hint hint{bmp_hint{}};

                return hint;
            }

            [[maybe_unused]] static std::optional<image_general_header> get(std::istream& stream) {
                return extract_header(impl_type::get(stream));
            }

            [[maybe_unused]] static std::optional<image_general_header> get(std::span<const std::byte> buffer) {
                return extract_header(impl_type::get(buffer));
            }
        };

        ES_KEEP_ALIVE [[maybe_unused]] struct init {
            init() {
                register_image_header_extractor(abstract::image_header_extractor{bmp_header_extractor{}});
            }
        } force_init;
    } // namespace
} // namespace essence::imaging
