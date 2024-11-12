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

#include "crypto/chunk_processor.hpp"

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "inout_buffer_pair.hpp"
#include "memory.hpp"
#include "memory/swapping_buffer.hpp"
#include "rational.hpp"

#include <algorithm>
#include <memory>
#include <ranges>
#include <type_traits>
#include <vector>

namespace essence::crypto {
    namespace {
        std::size_t calculate_max_buffer_size(std::span<const abstract::chunk_processor> processors) {
            const auto max_buffer_sizes = processors | std::views::transform([](const auto& inner) {
                return std::ranges::max(inner.buffer_size(), calculate_output_buffer_size(inner));
            });

            return std::ranges::max(max_buffer_sizes);
        }

        class chain_processor {
        public:
            explicit chain_processor(std::span<abstract::chunk_processor> processors)
                : buffer_pair_{processors.size() < 2
                                   ? throw source_code_aware_runtime_error{U8(
                                         "At least two processors are required to be chained together.")}
                                   : calculate_max_buffer_size(processors)},
                  finalization_buffer_{make_unique_array<std::byte>(buffer_pair_.buffer->size())},
                  swapper_{buffer_pair_.in, buffer_pair_.out} {
                if (std::ranges::adjacent_find(
                        processors, std::not_equal_to{}, [](const auto& inner) { return inner.transformer(); })
                    != processors.end()) {
                    throw source_code_aware_runtime_error{
                        U8("All processors must be either transformers or inverse transformers at the same time.")};
                }

                processors_.reserve(processors.size());
                std::ranges::move(processors, std::back_inserter(processors_));
            }

            [[nodiscard]] bool transformer() const {
                return processors_.front().transformer();
            }

            [[nodiscard]] [[maybe_unused]] static abi::string cipher_name() {
                return U8("chain");
            }

            [[nodiscard]] [[maybe_unused]] std::size_t buffer_size() const noexcept {
                return processors_.back().buffer_size();
            }

            [[nodiscard]] [[maybe_unused]] static std::size_t extra_size() noexcept {
                return 0;
            }

            [[nodiscard]] [[maybe_unused]] static rational size_factor() noexcept {
                return rational{0, 1};
            }

            [[maybe_unused]] void init() const {
                for (auto&& item : processors_) {
                    item.init();
                }
            }

            [[maybe_unused]] void update(std::span<const std::byte> input, std::span<std::byte>& output) {
                if (input.empty()) {
                    return (output = {}, void());
                }

                swapper_.reset(input);

                for (auto&& item : processors_ | std::views::take(processors_.size() - 1)) {
                    item.update(swapper_.in(), swapper_.out());
                    swapper_.swap();
                }

                // The final processor without swapping.
                processors_.back().update(swapper_.in(), swapper_.out());
                output = swapper_.out();
            }

            [[maybe_unused]] void finalize(std::span<std::byte>& output) {
                const auto scope = swapper_.set_temporary_out(*finalization_buffer_, *buffer_pair_.buffer);

                // The first iteration.
                processors_.front().finalize(swapper_.out());
                swapper_.swap();

                const auto iterator = [this]<bool Swapping>(
                                          const abstract::chunk_processor& processor, std::bool_constant<Swapping>) {
                    // G1 = F1
                    // Fn = ALGn.Finalize()
                    // Un(x) = ALGn.Update(x)
                    // Gn = Un(Gn-1) + Fn
                    processor.update(swapper_.in(), swapper_.out());
                    swapper_.out() = {
                        std::to_address(swapper_.out().end()), std::to_address(swapper_.original_out().end())};

                    processor.finalize(swapper_.out());
                    swapper_.out() = {
                        std::to_address(swapper_.original_out().begin()), std::to_address(swapper_.out().end())};

                    if constexpr (Swapping) {
                        swapper_.swap();
                    }
                };

                for (auto&& item : processors_ | std::views::drop(1) | std::views::take(processors_.size() - 2)) {
                    iterator(item, std::true_type{});
                }

                iterator(processors_.back(), std::false_type{});
                output = swapper_.out();
            }

        private:
            inout_buffer_pair buffer_pair_;
            unique_array<std::byte> finalization_buffer_;
            memory::swapping_buffer<std::byte> swapper_;
            std::vector<abstract::chunk_processor> processors_;
        };
    } // namespace

    abstract::chunk_processor chain_chunk_processors(std::span<abstract::chunk_processor> processors) {
        return abstract::chunk_processor{chain_processor{processors}};
    }
} // namespace essence::crypto
