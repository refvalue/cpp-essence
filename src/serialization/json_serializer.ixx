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

module;

#include <essence/char8_t_remediation.hpp>

export module essence.serialization:json_serializer;
import :basic_json;
import :detail.json;
import essence.basic;
import essence.meta;
import std;

export namespace essence::serialization {
    /**
     * Stashes the current 'enum_to_string' and 'naming_convention' configurations during the serialization.
     */
    struct json_serializer_base {
        static bool& get_enum_to_string_ref() noexcept {
            thread_local bool enabled{};

            return enabled;
        }

        static meta::naming_convention& get_naming_convention_ref() noexcept {
            thread_local auto convention = detail::get_json_naming_convention<std::type_identity<void>>();

            return convention;
        }

        static void reset() noexcept {
            get_enum_to_string_ref()    = {};
            get_naming_convention_ref() = detail::get_json_naming_convention<std::type_identity<void>>();
        }
    };

    /**
     * A JSON serializer by using the meta reflection implementation in this project.
     * @tparam T The type of the value.
     */
    template <typename T, typename = void>
    struct json_serializer : json_serializer_base {
        template <typename U, detail::basic_json_context BasicJsonContext>
        [[noreturn]] static void throw_exception(const U& member, std::string_view json_key, std::string_view message,
            std::string_view internal, BasicJsonContext context) {

            throw json_parse_error::create(112, 0U,
                concat(U8("Accessing class data member '"), member.enclosing_type_name, U8("."), member.name,
                    U8("' required by JSON key '"), json_key, U8("' -- "), message, U8("\nInternal error: "), internal),
                context);
        }

        /**
         * Serializes a primitive value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The primitive value.
         */
        template <detail::basic_json BasicJson, detail::primitive_json_serializable U = T>
        static void to_json(BasicJson& json, const U& value) {
            nlohmann_to_json(json, value);
        }

        /**
         * Deserializes a JSON value to a primitive value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The primitive value.
         */
        template <detail::basic_json BasicJson, detail::primitive_json_serializable U = T>
        static void from_json(const BasicJson& json, U& value) {
            nlohmann_from_json(json, value);
        }

        /**
         * Serializes a std::optional<> value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The std::optional<> value.
         */
        template <detail::basic_json BasicJson, std_optional U = T>
        static void to_json(BasicJson& json, const U& value) {
            if (value) {
                to_json(json, *value);
            } else {
                json = nullptr;
            }
        }

        /**
         * Deserializes a JSON value to a std::optional<> value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The std::optional<> value.
         */
        template <detail::basic_json BasicJson, std_optional U = T>
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
         * Serializes a non-iterable class object to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The class object.
         */
        template <detail::basic_json BasicJson, detail::non_iterable_object_json_serializable U = T>
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

            get_enum_to_string_ref()    = detail::has_enum_to_string_config<U>;
            get_naming_convention_ref() = detail::get_json_naming_convention<U>();

            meta::runtime::enumerate_data_members<detail::get_json_naming_convention<U>()>(
                value, [&](const auto&... members) { (handler(members), ...); });

            reset();
        }

        /**
         * Deserializes a JSON value to a non-iterable class object.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The class object.
         */
        template <detail::basic_json BasicJson, detail::non_iterable_object_json_serializable U = T>
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

            meta::runtime::enumerate_data_members<detail::get_json_naming_convention<U>()>(
                value, [&](const auto&... members) { (handler(members), ...); });
        }

        /**
         * Serializes an enumeration value to a JSON value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The enumeration value.
         */
        template <detail::basic_json BasicJson, typename U = T>
            requires(std::same_as<typename BasicJson::string_t::value_type, char> && std::is_enum_v<U>)
        static void to_json(BasicJson& json, const U& value) {
            if (get_enum_to_string_ref()) {
                nlohmann_to_json(json, meta::convert_naming_convention(meta::runtime::to_string(value), get_naming_convention_ref()));
            } else {
                nlohmann_to_json(json, value);
            }
        }

        /**
         * Deserializes a JSON value to an enumeration value.
         * @tparam BasicJson The type of the JSON.
         * @tparam U A type placeholder to support overload resolution.
         * @param json The JSON value.
         * @param value The enumeration value.
         */
        template <detail::basic_json BasicJson, typename U = T>
            requires(std::same_as<typename BasicJson::string_t::value_type, char> && std::is_enum_v<U>)
        static void from_json(const BasicJson& json, U& value) {
            if (json.is_string()) {
                const auto name = json.template get_ptr<const typename BasicJson::string_t*>();

                if (auto enum_value = meta::runtime::from_string<U>(*name)) {
                    value = *enum_value;
                } else {
                    throw json_parse_error::create(112, 0U,
                        concat(U8("invalid enumeration name '"), *name, U8("' of type '"),
                            meta::fingerprint{std::type_identity<U>{}}.friendly_name(), U8("'.")),
                        &json);
                }
            } else {
                nlohmann_from_json(json, value);
            }
        }
    };
} // namespace essence::serialization
