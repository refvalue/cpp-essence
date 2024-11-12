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

#include "../abi/map.hpp"
#include "../abi/string.hpp"
#include "../compat.hpp"
#include "../meta/runtime/struct.hpp"
#include "abstract/option.hpp"
#include "common_types.hpp"
#include "option_result.hpp"

#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace essence::cli {
    /**
     * @brief A parser to handle the command line arguments.
     */
    class arg_parser {
    public:
        using parse_result_type = abi::map<abi::string, option_result, std::less<>>;

        /**
         * @brief Creates an instance.
         */
        ES_API(CPPESSENCE) arg_parser();
        ES_API(CPPESSENCE) arg_parser(arg_parser&&) noexcept;
        ES_API(CPPESSENCE) ~arg_parser();
        ES_API(CPPESSENCE) arg_parser& operator=(arg_parser&&) noexcept;

        /**
         * @brief Checks whether the previous parsing operation succeeds.
         * @return True if succeeds; otherwise false.
         */
        ES_API(CPPESSENCE) explicit operator bool() const noexcept;

        /**
         * @brief Gets the added options.
         * @return The added options.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::span<const abstract::option> options() const noexcept;

        /**
         * @brief Gets the cached result after parsing.
         * @return The cached result as a map, or an empty map if not exists.
         */
        [[nodiscard]] ES_API(CPPESSENCE) const parse_result_type& cached_result() const noexcept;

        /**
         * @brief Gets the unmatched arguments after parsing.
         * @return The unmatched arguments.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::span<const abi::string> unmatched_args() const noexcept;

        /**
         * @brief Adds a new  option.
         * @param option The CLI option.
         */
        ES_API(CPPESSENCE) void add_option(abstract::option option) const; // NOLINT(*-use-nodiscard)

        /**
         * @brief Parses the startup command line arguments.
         */
        ES_API(CPPESSENCE) void parse() const;

        /**
         * @brief Parses the input command line arguments.
         * @param argc The count of the arguments.
         * @param argv The array of the arguments.
         */
        ES_API(CPPESSENCE) void parse(std::int32_t argc, char* argv[]) const;

        /**
         * @brief Parses the input command line arguments.
         * @param args The arguments.
         */
        ES_API(CPPESSENCE) void parse(std::span<const abi::string> args) const;

        /**
         * @brief Prints the help string.
         */
        ES_API(CPPESSENCE) void show_help() const;

        /**
         * @brief Subscribes an event to be invoked when an error occurs.
         * @param handler The handler.
         */
        ES_API(CPPESSENCE) void on_error(const output_handler& handler) const;

        /**
         * @brief Subscribes an event to be invoked when a normal output is ready.
         * @param handler The handler.
         */
        ES_API(CPPESSENCE) void on_output(const output_handler& handler) const;

        /**
         * @brief Fill a data model with the parsed result.
         * @tparam T The type of the model.
         * @return The filled model.
         */
        template <std::default_initializable T>
            requires std::is_class_v<T>
        std::optional<T> to_model() const {
            T result{};
            auto&& mapping = cached_result();

            if (mapping.empty()) {
                return std::nullopt;
            }

            // Sets the data members of the entity.
            auto handler = [&](const auto& item) {
                if (const auto iter = mapping.find(std::string_view{item.name}); iter != mapping.end()) {
                    iter->second.option.set_target_from_cache(item.reference);
                }
            };

            meta::runtime::enumerate_data_members(result, [&](const auto&... members) { (handler(members), ...); });

            return result;
        }

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::cli
