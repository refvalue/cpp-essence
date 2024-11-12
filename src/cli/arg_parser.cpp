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

#include "cli/arg_parser.hpp"

#include "char8_t_remediation.hpp"
#include "cli/option.hpp"
#include "common_tokens.hpp"
#include "delegate.hpp"
#include "environment.hpp"
#include "format_remediation.hpp"
#include "generator.hpp"
#include "range.hpp"
#include "string.hpp"

#include <array>
#include <cstddef>
#include <exception>
#include <ranges>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace essence::cli {
    namespace {
        constexpr meta::fingerprint boolean_type_id{std::type_identity<bool>{}};

        const auto help_option = option<bool>{}
                                     .set_bound_name(U8("help"))
                                     .set_description(U8("Show this help."))
                                     .add_aliases(U8("h"), U8("?"))
                                     .as_abstract();

        constexpr std::pair<bool, bool> check_option_prefix(std::string_view name) noexcept {
            const auto has_option_prefix = name.starts_with(common_tokens::option_name_prefix);
            const auto has_option_abbreviation_prefix =
                !has_option_prefix && name.starts_with(common_tokens::option_abbreviation_prefix);

            return {has_option_prefix, has_option_abbreviation_prefix};
        }
    } // namespace

    class arg_parser::impl {
    public:
        impl() : success_{} {
            add_option(help_option);
        }

        explicit operator bool() const noexcept {
            return success_;
        }

        std::span<const abstract::option> options() const noexcept {
            return options_;
        }

        const parse_result_type& cached_result() const noexcept {
            return result_;
        }

        std::span<const abi::string> unmatched_args() const noexcept {
            return unmatched_args_;
        }

        void add_option(abstract::option option) {
            if (option_names_.emplace(option.bound_name()).second) {
                // Adds the consumer's subscribers to the option.
                auto&& new_option = options_.emplace_back(std::move(option));

                new_option.on_error(on_error_.to_function());

                // Sets up a lookup table with the names and aliases of the options as keys.
                option_mapping_.insert_or_assign(new_option.bound_name(), new_option);

                for (auto&& item : new_option.aliases()) {
                    option_mapping_.insert_or_assign(item, new_option);
                }
            }
        }

        void parse() {
            parse_impl(get_command_line_args());
        }

        void parse(std::int32_t argc, char* argv[]) {
            parse_impl(std::span{argv, static_cast<std::size_t>(argc)});
        }

        void parse(std::span<const abi::string> args) {
            parse_impl(args);
        }

        void show_help() const {
            on_output_.try_invoke(common_tokens::fixed_help_content);
            on_output_.try_invoke(format(common_tokens::help_option_header_pattern, U8("Option"), U8("Value"),
                U8("Default"), U8("Description")));

            for (auto&& item : options_) {
                on_output_.try_invoke(format(common_tokens::help_option_header_pattern, item.name_hints(),
                    item.value_hints(), item.default_value_str().value_or(U8("`mandatory`")), item.description()));
            }
        }

        void on_error(const output_handler& handler) {
            on_error_ += handler;
        }

        void on_output(const output_handler& handler) {
            on_output_ += handler;
        }

    private:
        template <std::ranges::forward_range Range>
            requires std::convertible_to<std::ranges::range_value_t<Range>, std::string_view>
        void parse_impl(Range&& args) {
            args_.clear();

            for (auto&& item : args) {
                args_.emplace_back(trim(std::string_view{item}));
            }

            args_.shrink_to_fit();
            parse_args_and_cache();
        }

        void parse_args_and_cache() {
            std::size_t index{};

            result_.clear();
            unmatched_args_.clear();
            present_option_ptrs_.clear();

            while (index < args_.size()) {
                index = parse_argument(index) + 1;
            }

            // Shows the help content and terminates the program.
            if (result_.contains(help_option.bound_name())) {
                show_help();
                std::exit(EXIT_SUCCESS);
            }

            if ((success_ = index <= args_.size())) {
                add_absent_options();
            }

            unmatched_args_.shrink_to_fit();
        }

        void add_absent_options() {
            auto adapter = options_ | std::views::filter([this](const auto& inner) {
                return !present_option_ptrs_.contains(inner.underlying_ptr());
            });

            for (auto&& item : adapter) {
                // Outputs the missing options.
                if (!item.default_value_str()) {
                    on_error_.try_invoke(format(U8("Missing mandatory option: {}."), item.name_hints()));
                    success_ = false;
                }

                result_.insert_or_assign(abi::string{trim(item.bound_name(), common_tokens::option_name_prefix)},
                    option_result{
                        .option    = item,
                        .raw_value = item.default_value_str(),
                    });
            }
        }

        generator<std::tuple<bool, bool, abstract::option>> match_option(
            std::string_view name, bool has_option_prefix, bool has_option_abbreviation_prefix) const {
            // Options like --xxx.
            if (has_option_prefix) {
                if (const auto iter = option_mapping_.find(name); iter != option_mapping_.end()) {
                    co_yield std::tuple{true, iter->second.check_target_type(boolean_type_id), iter->second};
                }
            }

            // Options like -x, -xyz
            if (has_option_abbreviation_prefix) {
                thread_local std::array<char, 3> key{common_tokens::option_abbreviation_prefix.front()};
                auto transformer = [&, this](const auto& inner) -> std::optional<abstract::option> {
                    key[1] = inner;

                    if (const auto iter = option_mapping_.find(key.data()); iter != option_mapping_.end()) {
                        return iter->second;
                    }

                    return std::nullopt;
                };

                auto adapter = name | std::views::drop(1) | std::views::transform(transformer)
                             | std::views::filter([](const auto& inner) { return inner.has_value(); })
                             | std::views::transform([](const auto& inner) { return *inner; });

                // Checks whether the input contains one single option (or an empty value), i.e. --name=value, -x=value.
                const auto single = adapter.empty() || ++adapter.begin() == adapter.end();

                for (auto&& item : adapter) {
                    co_yield std::tuple{single, item.check_target_type(boolean_type_id), item};
                }
            }
        }

        bool validate_option(std::string_view& value, bool single, bool boolean, const abstract::option& option) const {
            // If combined abbreviations are specified, all types of the options must be bool.
            if (!single && !boolean) {
                on_error_.try_invoke(U8("All types of combined abbreviations must be bool."));

                return false;
            }

            // Boolean options like --switch are equivalent to --switch=true and --switch true.
            if (boolean && value.empty()) {
                value = meta::true_string;
            }

            return true;
        }

        std::size_t parse_argument(std::size_t index) {
            std::string_view arg{args_[index]};
            bool next_as_value{};

            // Gets the name-value pair if exists; otherwise only the name is assigned.
            auto&& [name, value] = [&arg] {
                const auto index = arg.find(common_tokens::equal);

                return std::pair{
                    trim(arg.substr(0, index)), index == std::string_view::npos ? U8("") : trim(arg.substr(index + 1))};
            }();

            auto&& [has_option_prefix, has_option_abbreviation_prefix] = check_option_prefix(name);

            // Skips the unmatched argument.
            if (!has_option_prefix && !has_option_abbreviation_prefix) {
                unmatched_args_.emplace_back(arg);

                return index;
            }

            // Options like -xyz=abc is illegal.
            if (has_option_abbreviation_prefix && name.size() > 2 && !value.empty()) {
                on_error_.try_invoke(
                    format(U8("Explicitly assigning values to combined abbreviations is not allowed: {}."), arg));

                return args_.size();
            }

            // Parses the next argument if the current argument does not contain an assignment (--name=value).
            if (value.empty() && index + 1 < args_.size()
                && check_option_prefix(args_[index + 1]) == std::pair{false, false}) {
                value         = args_[index + 1];
                next_as_value = true;
            }

            bool any_boolean{};

            for (auto&& [single, boolean, option] :
                match_option(name, has_option_prefix, has_option_abbreviation_prefix)) {
                if (boolean) {
                    any_boolean = boolean;
                }

                // --option true/false is omitted.
                if (any_boolean && next_as_value) {
                    value = U8("");
                }

                if (!validate_option(value, single, boolean, option)) {
                    return args_.size();
                }

                if (!option.parse_value_and_cache(value)) {
                    on_error_.try_invoke(format(U8("When parsing the command line argument: {}."), arg));
                    on_error_.try_invoke(format(U8("Matched option: {}."), option.name_hints()));

                    return args_.size();
                }

                // Added the underlying pointer of the option to keep track of used options.
                present_option_ptrs_.emplace(option.underlying_ptr());
                result_.insert_or_assign(option.bound_name(), option_result{
                                                                  .option    = option,
                                                                  .raw_value = abi::string{value},
                                                              });
            }

            return index + (any_boolean ? 0 : static_cast<std::size_t>(next_as_value));
        }

        delegate<output_handler> on_error_;
        delegate<output_handler> on_output_;

        bool success_;
        parse_result_type result_;
        abi::vector<abi::string> args_;
        abi::vector<abi::string> unmatched_args_;
        std::vector<abstract::option> options_;
        std::unordered_set<void*> present_option_ptrs_;
        std::unordered_set<abi::string, string_hash, std::equal_to<>> option_names_;
        std::unordered_map<abi::string, abstract::option, string_hash, std::equal_to<>> option_mapping_;
    };

    arg_parser::arg_parser() : impl_{std::make_unique<impl>()} {}

    arg_parser::arg_parser(arg_parser&&) noexcept = default;

    arg_parser::~arg_parser() = default;

    arg_parser& arg_parser::operator=(arg_parser&&) noexcept = default;

    arg_parser::operator bool() const noexcept {
        return static_cast<bool>(*impl_);
    }

    std::span<const abstract::option> arg_parser::options() const noexcept {
        return impl_->options();
    }

    const arg_parser::parse_result_type& arg_parser::cached_result() const noexcept {
        return impl_->cached_result();
    }

    std::span<const abi::string> arg_parser::unmatched_args() const noexcept {
        return impl_->unmatched_args();
    }

    void arg_parser::add_option(abstract::option option) const {
        impl_->add_option(std::move(option));
    }

    void arg_parser::parse() const {
        return impl_->parse();
    }

    void arg_parser::parse(std::int32_t argc, char* argv[]) const {
        return impl_->parse(argc, argv);
    }

    void arg_parser::parse(std::span<const abi::string> args) const {
        return impl_->parse(args);
    }

    void arg_parser::show_help() const {
        impl_->show_help();
    }

    void arg_parser::on_error(const output_handler& handler) const {
        impl_->on_error(handler);
    }

    void arg_parser::on_output(const output_handler& handler) const {
        impl_->on_output(handler);
    }
} // namespace essence::cli
