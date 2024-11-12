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

#include "imaging/image_prober.hpp"

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "extractors.hpp"
#include "io/bitstream_type_judger.hpp"
#include "string.hpp"

#include <array>
#include <concepts>
#include <exception>
#include <filesystem>
#include <fstream>
#include <ranges>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace essence::imaging {
    namespace {
        io::bitstream_type_judger make_merged_type_judger(std::span<const abstract::image_header_extractor> left,
            std::span<const abstract::image_header_extractor> right) {
            std::array extractor_group{left, right};
            auto adapter = extractor_group | std::views::join
                         | std::views::transform([](const auto& inner) { return inner.hint(); });

            return io::bitstream_type_judger{
                std::vector<io::abstract::bitstream_type_hint>{adapter.begin(), adapter.end()}};
        }

        template <std::ranges::forward_range Range>
        auto to_vector(Range&& range) {
            return std::vector<std::ranges::range_value_t<Range>>{std::ranges::begin(range), std::ranges::end(range)};
        }
    } // namespace

    class image_prober::impl {
    public:
        impl() : impl{std::array<const abstract::image_header_extractor, 0>{}} {}

        explicit impl(std::span<const abstract::image_header_extractor> extra_extractors)
            : extractors_{to_vector(get_image_header_extractors())},
              judger_{make_merged_type_judger(extractors_, extra_extractors)} {
            for (auto&& item : extractors_) {
                mapping_.insert_or_assign(item.hint().name(), item);
            }
        }

        [[nodiscard]] std::span<const abstract::image_header_extractor> extractors() const noexcept {
            return extractors_;
        }

        [[nodiscard]] std::optional<image_general_header> extract_header(std::string_view path) const {
            std::ifstream stream;

            try {
                stream.exceptions(std::ios::badbit);
                stream.open(std::filesystem::path{to_u8string(path)}, std::ios::in | std::ios::binary);
            } catch (const std::exception& ex) {
                throw source_code_aware_runtime_error{
                    U8("File"), path, U8("Message"), U8("Failed to open the image file."), U8("Internal"), ex.what()};
            }

            return extract_header(stream);
        }

        [[nodiscard]] std::optional<image_general_header> extract_header(std::istream& stream) const {
            auto header = extract_header_impl(stream);

            if (header) {
                header->bytes = stream.seekg(0, std::ios::end).tellg();
            }

            return header;
        }

        [[nodiscard]] std::optional<image_general_header> extract_header(std::span<const std::byte> buffer) const {
            auto header = extract_header_impl(buffer);

            if (header) {
                header->bytes = buffer.size();
            }

            return header;
        }

    private:
        template <typename T>
            requires((std::same_as<std::decay_t<T>, std::istream> && std::is_lvalue_reference_v<T>)
                     || std::convertible_to<T, std::span<const std::byte>>)
        std::optional<image_general_header> extract_header_impl(T&& stream_or_buffer) const {
            if (auto hint = judger_.identify(std::forward<T>(stream_or_buffer))) {
                if (auto iter = mapping_.find(hint->name()); iter != mapping_.end()) {
                    return iter->second.get(std::forward<T>(stream_or_buffer));
                }
            }

            return std::nullopt;
        }

        std::vector<abstract::image_header_extractor> extractors_;
        io::bitstream_type_judger judger_;
        std::unordered_map<abi::string, abstract::image_header_extractor, string_hash, std::equal_to<>> mapping_;
    };

    image_prober::image_prober() : impl_{std::make_unique<impl>()} {}

    image_prober::image_prober(std::span<const abstract::image_header_extractor> extra_extractors)
        : impl_{std::make_unique<impl>(extra_extractors)} {}

    image_prober::image_prober(image_prober&&) noexcept            = default;
    image_prober::~image_prober()                                  = default;
    image_prober& image_prober::operator=(image_prober&&) noexcept = default;

    std::span<const abstract::image_header_extractor> image_prober::extractors() const noexcept {
        return impl_->extractors();
    }

    std::optional<image_general_header> image_prober::extract_header(std::string_view path) const {
        return impl_->extract_header(path);
    }

    std::optional<image_general_header> image_prober::extract_header(std::istream& stream) const {
        return impl_->extract_header(stream);
    }

    std::optional<image_general_header> image_prober::extract_header(std::span<const std::byte> buffer) const {
        return impl_->extract_header(buffer);
    }
} // namespace essence::imaging
