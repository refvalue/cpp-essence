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

#include "../../char8_t_remediation.hpp"
#include "../../json.hpp"
#include "../../optional.hpp"
#include "../detail/json.hpp"
#include "../fingerprint.hpp"
#include "../naming_convention.hpp"
#include "enum.hpp"
#include "struct.hpp"

#include <concepts>
#include <exception>
#include <ranges>
#include <type_traits>
#include <utility>

namespace essence::meta::runtime {
    /**
     * @brief A JSON serializer by using the meta reflection implementation in this project.
     * @tparam T The type of the value.
     */
    template <typename T, typename = void>
    struct json_serializer {
        template <typename U, typename BasicJsonContext>
            requires nlohmann::detail::is_basic_json_context<BasicJsonContext>::value
        [[noreturn]] static void throw_exception(const U& member, std::string_view json_key, std::string_view message,
            std::string_view internal, BasicJsonContext context) {

            throw nlohmann::detail::parse_error::create(112, 0U,
                nlohmann::detail::concat(U8("Accessing class data member '"), member.enclosing_type_name, U8("."),
                    member.name, U8("' required by JSON key '"), json_key, U8("' -- "), message,
                    U8("\nInternal error: "), internal),
                context);
        }

        /**
         * @brief Serializes a primitive value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The primitive value.
         */
        template <typename BasicJson, typename U = T>
            requires(
                nlohmann::detail::is_basic_json<BasicJson>::value
                && (nlohmann::detail::is_compatible_type<nlohmann::json, U>::value || std::ranges::forward_range<U>)
                && !std::is_enum_v<U>)
        static void to_json(BasicJson& json, const U& value) {
            nlohmann::to_json(json, value);
        }

        /**
         * @brief Deserializes a JSON value to a primitive value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The primitive value.
         */
        template <typename BasicJson, typename U = T>
            requires(
                nlohmann::detail::is_basic_json<BasicJson>::value
                && (nlohmann::detail::is_compatible_type<nlohmann::json, U>::value || std::ranges::forward_range<U>)
                && !std::is_enum_v<U>)
        static void from_json(const BasicJson& json, U& value) {
            nlohmann::from_json(json, value);
        }

        /**
         * @brief Serializes a std::optional<> value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The std::optional<> value.
         */
        template <typename BasicJson, typename U = T>
            requires(nlohmann::detail::is_basic_json<BasicJson>::value && std_optional<U>)
        static void to_json(BasicJson& json, const U& value) {
            if (value) {
                to_json(json, *value);
            } else {
                json = nullptr;
            }
        }

        /**
         * @brief Deserializes a JSON value to a std::optional<> value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The std::optional<> value.
         */
        template <typename BasicJson, typename U = T>
            requires(nlohmann::detail::is_basic_json<BasicJson>::value && std_optional<U>)
        static void from_json(const BasicJson& json, U& value) {
            if (json.is_null()) {
                value.reset();
            } else {
                typename U::value_type new_value{};

                from_json(json, new_value);
                value.emplace(std::move(new_value));
            }
        }

        /**
         * @brief Serializes a non-iterable class object to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The class object.
         */
        template <typename BasicJson, typename U = T>
            requires(std::is_class_v<U> && nlohmann::detail::is_basic_json<BasicJson>::value
                     && !nlohmann::detail::is_compatible_type<nlohmann::json, U>::value
                     && !std::ranges::forward_range<U> && !std_optional<U>)
        static void to_json(BasicJson& json, const U& value) {
            auto handler = [&](const auto& item) {
                try {
                    if (!detail::check_json_omitted<U>(item.raw_name)) {
                        BasicJson subjson;

                        to_json(subjson, item.reference);
                        json.emplace(item.name, std::move(subjson));
                    }
                } catch (const std::exception& ex) {
                    throw_exception(
                        item, item.name, U8("Failed to serialize the data member to a JSON value."), ex.what(), &json);
                }
            };

            enumerate_data_members<detail::get_json_naming_convention<U>()>(
                value, [&](const auto&... members) { (handler(members), ...); });
        }

        /**
         * @brief Deserializes a JSON value to a non-iterable class object.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The class object.
         */
        template <typename BasicJson, typename U = T>
            requires(std::is_class_v<U> && nlohmann::detail::is_basic_json<BasicJson>::value
                     && !nlohmann::detail::is_compatible_type<nlohmann::json, U>::value
                     && !std::ranges::forward_range<U> && !std_optional<U>)
        static void from_json(const BasicJson& json, U& value) {
            auto handler = [&](const auto& item) {
                try {
                    if (!detail::check_json_omitted<U>(item.raw_name)) {
                        from_json(json.value(item.name, BasicJson{}), item.reference);
                    }
                } catch (const std::exception& ex) {
                    throw_exception(item, item.name, U8("Failed to deserialize the JSON value to the data member."),
                        ex.what(), &json);
                }
            };

            enumerate_data_members<detail::get_json_naming_convention<U>()>(
                value, [&](const auto&... members) { (handler(members), ...); });
        }

        /**
         * @brief Serializes an enumeration value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The enumeration value.
         */
        template <typename BasicJson, typename U = T>
            requires(nlohmann::detail::is_basic_json<BasicJson>::value
                     && std::same_as<typename BasicJson::string_t::value_type, char> && std::is_enum_v<U>)
        static void to_json(BasicJson& json, const U& value) {
            if constexpr (detail::has_enum_to_string_config<T>) {
                nlohmann::to_json(
                    json, convert_naming_convention(to_string(value), detail::get_json_naming_convention<T>()));
            } else {
                nlohmann::to_json(json, value);
            }
        }

        /**
         * @brief Deserializes a JSON value to an enumeration value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The enumeration value.
         */
        template <typename BasicJson, typename U = T>
            requires(nlohmann::detail::is_basic_json<BasicJson>::value
                     && std::same_as<typename BasicJson::string_t::value_type, char> && std::is_enum_v<U>)
        static void from_json(const BasicJson& json, U& value) {
            if (json.is_string()) {
                const auto name = json.template get_ptr<const typename BasicJson::string_t*>();

                if (auto enum_value = from_string<U>(*name)) {
                    value = *enum_value;
                } else {
                    throw nlohmann::detail::parse_error::create(112, 0U,
                        nlohmann::detail::concat(U8("invalid enumeration name '"), *name, U8("' of type '"),
                            fingerprint{std::type_identity<U>{}}.friendly_name(), U8("'.")),
                        &json);
                }
            } else {
                nlohmann::from_json(json, value);
            }
        }
    };
} // namespace essence::meta::runtime
