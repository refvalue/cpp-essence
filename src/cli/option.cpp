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

#include "cli/option.hpp"

#include "cli/abstract/option.hpp"
#include "common_tokens.hpp"
#include "delegate.hpp"

#include <array>
#include <unordered_set>

namespace essence::cli {
    namespace {
        struct base_option_impl {
            delegate<output_handler> on_error;
            delegate<validation_handler> on_validation;
        };

        template <std::ranges::forward_range Range>
        auto with_hyphens(Range&& range) {
            return range | std::views::transform([](const auto& inner) {
                auto duplicate = inner;

                std::replace_if(
                    duplicate.begin(), duplicate.end(),
                    [](const auto& inner) { return inner == common_tokens::underscore.front(); },
                    common_tokens::option_abbreviation_prefix.front());

                return std::array{inner, duplicate};
            }) | std::views::join
                 | std::views::common;
        }

        class base_option {
        public:
            base_option() : impl_{std::make_unique<base_option_impl>()} {}

            [[nodiscard]] [[maybe_unused]] abi::string bound_name() const {
                return bound_name_;
            }

            [[maybe_unused]] base_option& set_bound_name(std::string_view name) {
                bound_name_.append(trim_left(name, common_tokens::option_abbreviation_prefix));

                // Adds the hyphened name.
                add_aliases(std::array{bound_name_});

                return *this;
            }

            [[nodiscard]] [[maybe_unused]] abi::string description() const {
                return description_;
            }

            [[maybe_unused]] base_option& set_description(std::string_view description) {
                description_.assign(description);

                return *this;
            }

            [[nodiscard]] [[maybe_unused]] std::span<const abi::string> aliases() const noexcept {
                return aliases_;
            }

            base_option& add_aliases(std::span<const abi::string> aliases) {
                auto adapter = aliases | std::views::transform([](const auto& inner) {
                    return trim_left(inner, common_tokens::option_abbreviation_prefix);
                }) | std::views::filter([](const auto& inner) { return !inner.empty(); })
                             | std::views::transform([](const auto& inner) {
                                   abi::string result{inner.size() > 1 ? common_tokens::option_name_prefix
                                                                       : common_tokens::option_abbreviation_prefix};

                                   result.append(inner);

                                   return result;
                               });

                for (auto&& item : with_hyphens(adapter)) {
                    if (unrepeatable_aliases_.emplace(item).second) {
                        aliases_.emplace_back(item);
                    }
                }

                return *this;
            }

            [[maybe_unused]] static abi::string default_value_str() {
                return {};
            }

            [[maybe_unused]] static std::span<const abi::string> valid_value_strs() noexcept {
                return {};
            }

            [[nodiscard]] abi::string name_hints() const {
                auto joint = join_with(aliases_, std::string_view{U8(",")});

                return {joint.begin(), joint.end()};
            }

            [[nodiscard]] static abi::string value_hints() {
                return {};
            }

            [[maybe_unused]] static bool check_target_type(meta::fingerprint id) noexcept {
                return false;
            }

            [[maybe_unused]] static bool parse_value_and_cache(std::string_view value) noexcept {
                return false;
            }

            [[maybe_unused]] static void set_target_from_cache(void* target) noexcept {}

            [[maybe_unused]] void validate(std::string_view value, validation_result& result) const {
                impl_->on_validation.try_invoke(value, result);
            }

            [[maybe_unused]] void raise_error(std::string_view message) const {
                impl_->on_error.try_invoke(message);
            }

            [[maybe_unused]] void on_validation(const validation_handler& handler) const {
                impl_->on_validation += handler;
            }

            [[maybe_unused]] void on_error(const output_handler& handler) const {
                impl_->on_error += handler;
            }

        private:
            std::unique_ptr<base_option_impl> impl_;

            abi::string bound_name_;
            abi::string description_;
            abi::vector<abi::string> aliases_;
            std::unordered_set<abi::string, string_hash, std::equal_to<>> unrepeatable_aliases_;
        };
    } // namespace

    abstract::option make_base_option() {
        return abstract::option{base_option{}};
    }
} // namespace essence::cli
