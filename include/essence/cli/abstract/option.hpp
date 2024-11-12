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

#include "../../abi/string.hpp"
#include "../../meta/fingerprint.hpp"
#include "../common_types.hpp"
#include "../validation_result.hpp"

#include <array>
#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::cli::abstract {
    /**
     * @brief A CLI option, i.e. --xxx=yyy.
     * @tparam T The data type of the option.
     */
    class option {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, option>)
        explicit option(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        [[nodiscard]] abi::string bound_name() const {
            return wrapper_->bound_name();
        }

        const option& set_bound_name(std::string_view name) const { // NOLINT(*-use-nodiscard)
            wrapper_->set_bound_name(name);

            return *this;
        }

        [[nodiscard]] abi::string description() const {
            return wrapper_->description();
        }

        [[nodiscard]] const option& set_description(std::string_view description) const {
            wrapper_->set_description(description);

            return *this;
        }

        [[nodiscard]] std::span<const abi::string> aliases() const {
            return wrapper_->aliases();
        }

        const option& add_aliases(std::span<const abi::string> aliases) const { // NOLINT(*-use-nodiscard)
            wrapper_->add_aliases(aliases);

            return *this;
        }

        [[nodiscard]] std::optional<abi::string> default_value_str() const {
            return wrapper_->default_value_str();
        }

        [[nodiscard]] std::span<const abi::string> valid_value_strs() const {
            return wrapper_->valid_value_strs();
        }

        [[nodiscard]] abi::string name_hints() const {
            return wrapper_->name_hints();
        }

        [[nodiscard]] abi::string value_hints() const {
            return wrapper_->value_hints();
        }

        [[nodiscard]] bool check_target_type(meta::fingerprint id) const {
            return wrapper_->check_target_type(id);
        }

        [[nodiscard]] bool parse_value_and_cache(std::string_view value) const {
            return wrapper_->parse_value_and_cache(value);
        }

        void validate(std::string_view value, validation_result& result) const {
            wrapper_->validate(value, result);
        }

        void raise_error(std::string_view message) const {
            wrapper_->raise_error(message);
        }

        [[nodiscard]] void* underlying_ptr() const noexcept {
            return wrapper_.get();
        }

        void on_validation(const validation_handler& handler) const {
            wrapper_->on_validation(handler);
        }

        void on_error(const output_handler& handler) const {
            wrapper_->on_error(handler);
        }

        template <std::convertible_to<std::string_view>... Args>
        const option& add_aliases(Args&&... args) const {
            return add_aliases(std::array{abi::string{std::string_view{std::forward<Args>(args)...}}});
        }

        template <typename T>
        void set_target_from_cache(T& target) const {
            static constexpr meta::fingerprint id{std::type_identity<T>{}};

            if (check_target_type(id)) {
                wrapper_->set_target_from_cache(&target);
            }
        }

    private:
        struct base {
            virtual ~base()                                                          = default;
            virtual abi::string bound_name()                                               = 0;
            virtual void set_bound_name(std::string_view name)                             = 0;
            virtual abi::string description()                                        = 0;
            virtual void set_description(std::string_view description)               = 0;
            virtual std::span<const abi::string> aliases()                           = 0;
            virtual void add_aliases(std::span<const abi::string> aliases)           = 0;
            virtual std::optional<abi::string> default_value_str()                   = 0;
            virtual std::span<const abi::string> valid_value_strs()                  = 0;
            virtual abi::string name_hints()                                         = 0;
            virtual abi::string value_hints()                                        = 0;
            virtual bool check_target_type(meta::fingerprint id)                     = 0;
            virtual bool parse_value_and_cache(std::string_view value)               = 0;
            virtual void set_target_from_cache(void* target)                         = 0;
            virtual void validate(std::string_view value, validation_result& result) = 0;
            virtual void raise_error(std::string_view message)                       = 0;
            virtual void on_validation(const validation_handler& handler)            = 0;
            virtual void on_error(const output_handler& handler)                     = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            abi::string bound_name() override {
                return value_.bound_name();
            }

            void set_bound_name(std::string_view name) override {
                value_.set_bound_name(name);
            }

            abi::string description() override {
                return value_.description();
            }

            void set_description(std::string_view description) override {
                value_.set_description(description);
            }

            std::span<const abi::string> aliases() override {
                return value_.aliases();
            }

            void add_aliases(std::span<const abi::string> aliases) override {
                value_.add_aliases(aliases);
            }

            std::optional<abi::string> default_value_str() override {
                return value_.default_value_str();
            }

            std::span<const abi::string> valid_value_strs() override {
                return value_.valid_value_strs();
            }

            abi::string name_hints() override {
                return value_.name_hints();
            }

            abi::string value_hints() override {
                return value_.value_hints();
            }

            bool check_target_type(meta::fingerprint id) override {
                return value_.check_target_type(id);
            }

            bool parse_value_and_cache(std::string_view value) override {
                return value_.parse_value_and_cache(value);
            }

            void set_target_from_cache(void* target) override {
                value_.set_target_from_cache(target);
            }

            void validate(std::string_view value, validation_result& result) override {
                value_.validate(value, result);
            }

            void raise_error(std::string_view message) override {
                value_.raise_error(message);
            }

            void on_validation(const validation_handler& handler) override {
                value_.on_validation(handler);
            }

            void on_error(const output_handler& handler) override {
                value_.on_error(handler);
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::cli::abstract
