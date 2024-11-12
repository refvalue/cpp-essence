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

#include "../abi/string.hpp"
#include "../abi/vector.hpp"
#include "../basic_string.hpp"
#include "../char8_t_remediation.hpp"
#include "../compat.hpp"
#include "../meta/fingerprint.hpp"
#include "../meta/friendly_name_vector.hpp"
#include "../meta/runtime/boolean.hpp"
#include "../meta/runtime/enum.hpp"
#include "../numeric_conversion.hpp"
#include "../range.hpp"
#include "../string.hpp"
#include "abstract/option.hpp"
#include "common_types.hpp"

#include <algorithm>
#include <concepts>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <unordered_set>
#include <utility>

namespace essence::cli {
    /**
     * @brief Creates a base internal implementation of a CLI option class.
     * @return The base implementation.
     */
    ES_API(CPPESSENCE) abstract::option make_base_option();

    /**
     * @brief A CLI option, i.e. --xxx=yyy.
     * @tparam T The data type of the option.
     */
    template <std::default_initializable T>
        requires((std::is_arithmetic_v<range_value_t_or_self_t<T>> || std::is_enum_v<range_value_t_or_self_t<T>>
                     || std_basic_string<range_value_t_or_self_t<T>>)
                 && (std::same_as<range_value_t_or_self_t<T>, T> ? true : extendable_contiguous_range<T>) )
    class option {
    public:
        using element_type  = std::conditional_t<std_basic_string<T>, T, range_value_t_or_self_t<T>>;
        using set_hash_type = std::conditional_t<std_basic_string<element_type>, string_hash, std::hash<element_type>>;

        static constexpr std::string_view delimiter{U8(",")};
        static constexpr std::string_view optional_pattern{U8("[]")};
        static constexpr std::string_view keyword_pattern{U8("``")};
        static constexpr meta::fingerprint type_id{std::type_identity<T>{}};

        /**
         * @brief Creates an instance.
         */
        option() : base_{make_base_option()} {
            // Makes default values for an enumeration.
            if constexpr (std::is_enum_v<element_type>) {
                for (auto&& [name, value] : meta::runtime::get_enum_names<element_type>(true)) {
                    valid_value_strs_.emplace_back(name);
                    valid_values_.emplace(value);
                }
            } else if constexpr (std::same_as<T, bool>) {
                valid_value_strs_.emplace_back(meta::true_string);
                valid_value_strs_.emplace_back(meta::false_string);
                valid_values_.emplace(true);
                valid_values_.emplace(false);
                set_default_value(false);
            }
        }

        [[nodiscard]] abi::string bound_name() const {
            return base_.bound_name();
        }

        option& set_bound_name(std::string_view name) {
            static_cast<void>(base_.set_bound_name(name));

            return *this;
        }

        [[nodiscard]] abi::string description() const {
            return base_.description();
        }

        option& set_description(std::string_view description) {
            static_cast<void>(base_.set_description(description));

            return *this;
        }

        [[nodiscard]] std::span<const abi::string> aliases() const {
            return base_.aliases();
        }

        option& add_aliases(std::span<const abi::string> aliases) {
            static_cast<void>(base_.add_aliases(aliases));

            return *this;
        }

        [[nodiscard]] std::optional<abi::string> default_value_str() const {
            return default_value_str_;
        }

        [[nodiscard]] std::span<const abi::string> valid_value_strs() const noexcept {
            return valid_value_strs_;
        }

        [[nodiscard]] abi::string name_hints() const {
            auto hints = base_.name_hints();

            // An optional option, surrounded by [].
            if (default_value_str()) {
                hints.insert(hints.begin(), optional_pattern.front());
                hints.push_back(optional_pattern.back());
            }

            return hints;
        }

        [[nodiscard]] abi::string value_hints() const {
            abi::string hints;

            if (valid_value_strs().empty()) {
                hints.push_back(keyword_pattern.front());
                hints.append(type_id.friendly_name());
                hints.push_back(keyword_pattern.back());
            } else {
                auto joint = join_with(valid_value_strs_, delimiter);

                hints.assign(joint.begin(), joint.end());
            }

            return hints;
        }

        [[nodiscard]] static bool check_target_type(meta::fingerprint id) noexcept {
            return id == type_id;
        }

        [[nodiscard]] bool parse_value_and_cache(std::string_view value) {
            thread_local bool success = true;

            if constexpr (!std_basic_string<T> && extendable_contiguous_range<T>) {
                parse_range_value_and_cache(value, success);
            } else {
                auto parsed_value = from_element_string(value);

                if ((success = parsed_value.has_value())) {
                    validate_range(*parsed_value, success);
                }

                if (!success) {
                    raise_error(U8("Invalid value."));

                    return success;
                }

                parsed_value.swap(cached_value_);
            }

            // Invokes the consumer's validator.
            if (success) {
                if (validation_result result; !(validate(value, result), result.success)) {
                    success = result.success;

                    raise_error(result.error);
                }
            }

            return success;
        }

        void set_target_from_cache(void* target) {
            if (target && (cached_value_ || default_value_)) {
                auto value = cached_value_ ? *cached_value_ : *default_value_;

                // Selects std::swap or T::swap instead for higher performance if exists.
                if constexpr (requires(T obj) { std::swap(obj, obj); }) {
                    std::swap(*static_cast<T*>(target), value);
                } else if constexpr (requires(T obj) { T{}.swap(obj); }) {
                    value.swap(*static_cast<T*>(target));
                } else {
                    *static_cast<T*>(target) = value;
                }
            }
        }

        void validate(std::string_view value, validation_result& result) const {
            base_.validate(value, result);
        }

        void raise_error(std::string_view message) const {
            base_.raise_error(message);
        }

        void on_validation(const validation_handler& handler) const {
            base_.on_validation(handler);
        }

        void on_error(const output_handler& handler) const {
            base_.on_error(handler);
        }

        template <std::convertible_to<std::string_view>... Args>
        option& add_aliases(Args&&... args) {
            add_aliases(std::array{abi::string{std::string_view{std::forward<Args>(args)}}...});

            return *this;
        }

        template <typename U>
            requires std::constructible_from<T, U>
        option& set_default_value(U&& value) {
            default_value_.emplace(std::forward<U>(value));

            // If T is a range, creates a comma-separated string with its elements.
            if constexpr (!std_basic_string<T> && extendable_contiguous_range<T>) {
                auto strs  = (*default_value_) | std::views::transform(&option::to_element_string);
                auto joint = join_with(strs, delimiter);

                default_value_str_.emplace(joint.begin(), joint.end());
            } else {
                default_value_str_.emplace(to_element_string(*default_value_));
            }

            return *this;
        }

        option& set_valid_values(std::span<const element_type> values) {
            decltype(valid_values_){values.begin(), values.end()}.swap(valid_values_);

            auto adapter = valid_values_ | std::views::transform(&option::to_element_string);

            valid_value_strs_.assign(adapter.begin(), adapter.end());

            return *this;
        }

        template <typename... Args>
            requires(std::constructible_from<element_type, Args> && ...)
        option& set_valid_values(Args&&... args) {
            set_valid_values(std::array{element_type{std::forward<Args>(args)}...});

            return *this;
        }

        abstract::option as_abstract() {
            return abstract::option{std::move(*this)};
        }

    private:
        static abi::string to_element_string(const element_type& item) {
            if constexpr (std::convertible_to<element_type, std::string_view>) {
                return abi::string{std::string_view{item}};
            } else if constexpr (std::is_enum_v<element_type> || std::same_as<element_type, bool>) {
                return abi::to_abi_string(meta::runtime::to_string(item));
            } else if constexpr (std::is_arithmetic_v<element_type>) {
                return abi::to_abi_string(to_string(item));
            } else {
                return {};
            }
        }

        static std::optional<element_type> from_element_string(std::string_view str) {
            if constexpr (std_basic_string<element_type>) {
                return element_type{str};
            } else if constexpr (std::is_enum_v<element_type>) {
                return meta::runtime::from_string<element_type>(str);
            } else if constexpr (std::same_as<element_type, bool>) {
                return meta::runtime::from_string<bool>(str);
            } else if constexpr (std::is_arithmetic_v<element_type>) {
                return from_string<element_type>(str);
            } else {
                return std::nullopt;
            }
        }

        void validate_range(const element_type& value, bool& success) const {
            if (success = valid_values_.empty() ? true : valid_values_.contains(value); !success) {
                auto joint = join_with(valid_value_strs_, delimiter);

                raise_error(U8("The value was out of range."));
                raise_error(U8("One of the following values is allowed:"));
                raise_error(abi::string{joint.begin(), joint.end()});
            }
        }

        void parse_range_value_and_cache(std::string_view value, bool& success) {
            auto validator = [&](const auto& inner) {
                if ((success = inner.has_value())) {
                    validate_range(*inner, success);
                } else {
                    raise_error(U8("Invalid value."));
                }

                return success;
            };

            if constexpr (!std_basic_string<T> && extendable_contiguous_range<T>) {
                auto parts = std::views::split(value, delimiter) | std::views::transform([](const auto& inner) {
                    // The return type of split is implementation-defined, so we should use
                    // the common adapter to convert it to a normal sub-range.
                    auto iter = inner | std::views::common;

                    return from_element_string(abi::string{iter.begin(), iter.end()});
                }) | std::views::take_while(validator)
                           | std::views::transform([](const auto& inner) { return *inner; }) | std::views::common;

                if (success) {
                    cached_value_.emplace();

                    // The extendable_contiguous_range concept ensures that
                    // .clear(), .push_back() and .shrink_to_fit exist.
                    std::ranges::copy(parts, std::back_inserter(*cached_value_));
                    cached_value_->shrink_to_fit();
                } else {
                    cached_value_.reset();
                }
            }
        }

        abstract::option base_;
        std::optional<T> cached_value_;
        std::optional<T> default_value_;
        std::optional<abi::string> default_value_str_;
        abi::vector<abi::string> valid_value_strs_;
        std::unordered_set<element_type, set_hash_type, std::equal_to<>> valid_values_;
    };
} // namespace essence::cli
