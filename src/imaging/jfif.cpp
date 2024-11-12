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
#include <optional>
#include <span>
#include <string_view>

namespace essence::imaging {
    namespace {
        constexpr std::array leading_signature{
            std::byte{0xFF},
            std::byte{0xD8},
        };

        constexpr std::array trailing_signature{
            std::byte{0xFF},
            std::byte{0xD9},
        };

        constexpr std::array jfif_sof0_signature{
            std::byte{0xFF},
            std::byte{0xC0},
        };

        constexpr std::size_t jfif_sof0_needed_size = 8;

        struct jfif_hint {
            [[maybe_unused]] static abi::string name() {
                return U8("JFIF");
            }

            [[maybe_unused]] static std::span<const abi::string> file_extensions() noexcept {
                static const std::array<abi::string, 3> extensions{U8(".jfif"), U8(".jpeg"), U8(".jpg")};

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
                /*
                    0xFF, 0xC0,             SOF0 segement
                    0x00, 0x11,             length of segment depends on the number of components
                    0x08,                   bits per pixel
                    0x00, 0x95,             image height
                    0x00, 0xE3,             image width
                    0x03,                   number of components (should be 1 or 3)
                    0x01, 0x22, 0x00,       0x01=Y component, 0x22=sampling factor, quantization table number
                    0x02, 0x11, 0x01,       0x02=Cb component, ...
                    0x03, 0x11, 0x01        0x03=Cr component, ...
                    */
                return image_general_header{
                    .width = (std::to_integer<std::int32_t>(buffer[5]) << 8) + std::to_integer<std::int32_t>(buffer[6]),
                    .height =
                        (std::to_integer<std::int32_t>(buffer[3]) << 8) + std::to_integer<std::int32_t>(buffer[4]),
                    .bits_per_pixel = std::to_integer<std::int32_t>(buffer[2]),
                    .channels       = std::to_integer<std::int32_t>(buffer[7]),
                };
            }

            return std::nullopt;
        }

        struct jfif_header_extractor {
            using impl_type = image_header_extractor_impl<jfif_sof0_needed_size, jfif_sof0_signature>;

            [[maybe_unused]] static io::abstract::bitstream_type_hint hint() {
                static const io::abstract::bitstream_type_hint hint{jfif_hint{}};

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
                register_image_header_extractor(abstract::image_header_extractor{jfif_header_extractor{}});
            }
        } force_init;
    } // namespace
} // namespace essence::imaging
