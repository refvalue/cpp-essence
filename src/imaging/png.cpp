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
            std::byte{137},
            std::byte{80},
            std::byte{78},
            std::byte{71},
            std::byte{13},
            std::byte{10},
            std::byte{26},
            std::byte{10},
        };

        constexpr std::array<std::byte, 0> trailing_signature{};

        constexpr std::array png_ihdr_signature{
            std::byte{73},
            std::byte{72},
            std::byte{68},
            std::byte{82},
        };

        constexpr std::size_t png_ihdr_needed_size = 9;

        struct png_hint {
            [[maybe_unused]] static abi::string name() {
                return U8("PNG");
            }

            [[maybe_unused]] static std::span<const abi::string> file_extensions() noexcept {
                static const std::array<abi::string, 1> extensions{U8(".png")};

                return extensions;
            }

            static std::size_t leading_signature_size() noexcept {
                return imaging::leading_signature.size();
            }

            static std::span<const std::byte> leading_signature() noexcept {
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
                /*
                    The IHDR chunk shall be the first chunk in the PNG datastream. It contains:
                    Width                4 bytes
                    Height               4 bytes
                    Bit depth	         1 byte
                    Colour type	         1 byte
                    Compression method   1 byte
                    Filter method        1 byte
                    Interlace method     1 byte
                */
                return image_general_header{
                    .width = (std::to_integer<std::int32_t>(buffer[0]) << 24)
                           + (std::to_integer<std::int32_t>(buffer[1]) << 16)
                           + (std::to_integer<std::int32_t>(buffer[2]) << 8) + std::to_integer<std::int32_t>(buffer[3]),

                    .height = (std::to_integer<std::int32_t>(buffer[4]) << 24)
                            + (std::to_integer<std::int32_t>(buffer[5]) << 16)
                            + (std::to_integer<std::int32_t>(buffer[6]) << 8)
                            + std::to_integer<std::int32_t>(buffer[7]),

                    .bits_per_pixel = std::to_integer<std::int32_t>(buffer[8]),
                    .channels = std::to_integer<std::int32_t>(buffer[8]) / std::numeric_limits<std::uint8_t>::digits,
                };
            }

            return std::nullopt;
        }

        struct png_header_extractor {
            using impl_type = image_header_extractor_impl<png_ihdr_needed_size, png_ihdr_signature>;

            [[maybe_unused]] static io::abstract::bitstream_type_hint hint() {
                static const io::abstract::bitstream_type_hint hint{png_hint{}};

                return hint;
            }

            [[maybe_unused]] static std::optional<image_general_header> get(std::istream& stream) {
                return extract_header(impl_type::get(stream));
            }

            [[maybe_unused]] static std::optional<image_general_header> get(std::span<const std::byte> buffer) {
                return extract_header(impl_type::get(buffer));
            }
        };

        [[maybe_unused]] ES_KEEP_ALIVE struct init {
            init() {
                register_image_header_extractor(abstract::image_header_extractor{png_header_extractor{}});
            }
        } force_init;
    } // namespace
} // namespace essence::imaging
