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

#include "io/bitstream_type_judger.hpp"

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "string.hpp"

#include <algorithm>
#include <exception>
#include <filesystem>
#include <fstream>
#include <numeric>
#include <ranges>
#include <unordered_map>
#include <utility>
#include <vector>

namespace essence::io {
    namespace {
        template <bool Leading>
        std::size_t get_signature_size(const abstract::bitstream_type_hint& hint) noexcept {
            if constexpr (Leading) {
                return hint.leading_signature_size();
            } else {
                return hint.trailing_signature_size();
            }
        }

        void read_trailing_bytes(std::istream& stream, std::span<std::byte> buffer) {
            const auto origin = stream.tellg();

            stream.seekg(0, std::ios::end);

            const auto max_rollback_size =
                std::min(stream.tellg() - origin, static_cast<std::streamoff>(buffer.size()));

            if (max_rollback_size > 0) {
                stream.seekg(-max_rollback_size, std::ios::end)
                    .read(reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(max_rollback_size))
                    .seekg(origin);
            }

            std::fill(buffer.begin() + max_rollback_size, buffer.end(), std::byte{});
        }
    } // namespace

    class bitstream_type_judger::impl {
    public:
        using mapping_type = std::unordered_map<std::string_view, abstract::bitstream_type_hint>;

        explicit impl(std::span<const abstract::bitstream_type_hint> hints)
            : combined_signature_size_{}, max_leading_signature_size_{}, max_trailing_signature_size_{},
              hints_{hints.begin(), hints.end()} {

            if (hints_.empty()) {
                throw source_code_aware_runtime_error{U8("The input type hints cannot be empty.")};
            }

            init_max_signature_size<true>();
            init_max_signature_size<false>();
            init_signature_mapping();
        }

        [[nodiscard]] std::span<const abstract::bitstream_type_hint> hints() const noexcept {
            return hints_;
        }

        [[nodiscard]] std::optional<abstract::bitstream_type_hint> identify(std::string_view path) const {
            std::ifstream stream;

            try {
                stream.exceptions(std::ios::badbit);
                stream.open(std::filesystem::path{to_u8string(path)}, std::ios::in | std::ios::binary);
            } catch (const std::exception& ex) {
                throw source_code_aware_runtime_error{
                    U8("File"), path, U8("Message"), U8("Failed to open the file."), U8("Internal"), ex.what()};
            }

            return identify(stream);
        }

        [[nodiscard]] std::optional<abstract::bitstream_type_hint> identify(std::istream& stream) const {
            thread_local std::vector<std::byte> buffer;

            buffer.resize(combined_signature_size_);

            // Fetches data in the max possible size in one single call to the stream.read function.
            stream.read(
                reinterpret_cast<char*>(buffer.data()), static_cast<std::streamsize>(max_leading_signature_size_));
            read_trailing_bytes(
                stream, std::span{buffer.data() + max_leading_signature_size_, max_trailing_signature_size_});

            return identify(buffer);
        }

        [[nodiscard]] std::optional<abstract::bitstream_type_hint> identify(std::span<const std::byte> buffer) const {
            // Prohibits out-of-range exceptions.
            auto adapter = hints_ | std::views::filter([&](const auto& inner) {
                return inner.leading_signature_size() + inner.trailing_signature_size() <= buffer.size();
            });

            for (auto&& item : adapter) {
                abi::string combined_key{reinterpret_cast<const char*>(buffer.data()), item.leading_signature_size()};

                // Uses a combined key.
                combined_key.append(
                    reinterpret_cast<const char*>(buffer.data() + buffer.size() - item.trailing_signature_size()),
                    item.trailing_signature_size());

                if (auto iter = mapping_.find(combined_key); iter != mapping_.end()) {
                    return item;
                }
            }

            return std::nullopt;
        }

    private:
        template <bool Leading>
        std::size_t& get_max_signature_size() noexcept {
            if constexpr (Leading) {
                return max_leading_signature_size_;
            } else {
                return max_trailing_signature_size_;
            }
        }

        template <bool Leading>
        void init_max_signature_size() {
            static constexpr auto compare = [](const auto& left, const auto& right) {
                return get_signature_size<Leading>(left) < get_signature_size<Leading>(right);
            };

            std::ranges::sort(hints_, compare);

            get_max_signature_size<Leading>() =
                get_signature_size<Leading>(*std::max_element(hints_.begin(), hints_.end(), compare));
        }

        void init_signature_mapping() {
            init_max_signature_size<true>();
            init_max_signature_size<false>();
            combined_signature_size_ = max_leading_signature_size_ + max_trailing_signature_size_;

            // Adds non-empty signatures only.
            auto adapter = hints_ | std::views::filter([](const auto& inner) {
                return inner.leading_signature_size() != 0 || inner.trailing_signature_size() != 0;
            });

            for (auto&& item : adapter) {
                // Uses a combined key.
                mapping_.insert_or_assign(
                    std::move(abi::string{item.leading_signature_str()}.append(item.trailing_signature_str())), item);
            }
        }

        std::size_t combined_signature_size_;
        std::size_t max_leading_signature_size_;
        std::size_t max_trailing_signature_size_;
        std::vector<abstract::bitstream_type_hint> hints_;
        std::unordered_map<abi::string, abstract::bitstream_type_hint, string_hash, std::equal_to<>> mapping_;
    };

    bitstream_type_judger::bitstream_type_judger(std::span<const abstract::bitstream_type_hint> hints)
        : impl_{std::make_unique<impl>(hints)} {}

    bitstream_type_judger::bitstream_type_judger(bitstream_type_judger&&) noexcept = default;

    bitstream_type_judger::~bitstream_type_judger() = default;

    bitstream_type_judger& bitstream_type_judger::operator=(bitstream_type_judger&&) noexcept = default;

    std::span<const abstract::bitstream_type_hint> bitstream_type_judger::hints() const noexcept {
        return impl_->hints();
    }

    std::optional<abstract::bitstream_type_hint> bitstream_type_judger::identify(std::string_view path) const {
        return impl_->identify(path);
    }

    std::optional<abstract::bitstream_type_hint> bitstream_type_judger::identify(std::istream& stream) const {
        return impl_->identify(stream);
    }

    std::optional<abstract::bitstream_type_hint> bitstream_type_judger::identify(
        std::span<const std::byte> buffer) const {
        return impl_->identify(buffer);
    }
} // namespace essence::io
