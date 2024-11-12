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

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "exception.hpp"
#include "json_compat.hpp"
#include "meta/naming_convention.hpp"
#include "reflection.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence {
    template <typename Visitor>
    decltype(auto) visit_json(const json& value, Visitor&& visitor) {
        using nlohmann::detail::value_t;

        switch (value.type()) {
        case value_t::null:
            return std::forward<Visitor>(visitor)(nullptr);
        case value_t::object:
            return std::forward<Visitor>(visitor)(value);
        case value_t::array:
            return std::forward<Visitor>(visitor)(value.get<json::array_t>());
        case value_t::boolean:
            return std::forward<Visitor>(visitor)(value.get<bool>());
        case value_t::string:
            return std::forward<Visitor>(visitor)(value.get<std::string>());
        case value_t::number_float:
            return std::forward<Visitor>(visitor)(value.get<double>());
        case value_t::number_integer:
            return std::forward<Visitor>(visitor)(value.get<std::int64_t>());
        case value_t::number_unsigned:
            return std::forward<Visitor>(visitor)(value.get<std::uint64_t>());
        default:
            return std::forward<Visitor>(visitor)(value);
        }
    }
} // namespace essence

namespace essence {
    template <meta::reflectable T, meta::naming_convention Convention>
    struct json_serializer {
        using fields_t = meta::get_fields_t<T>;

        static void from_json(const json& value, T& obj) {
            for_each_as_values<fields_t>([&]<typename... Ts>(std::type_identity<Ts>...) {
                // Creates a json null value if not exist, as a std::optional<T> may need this value.
                (value.value(meta::convert_naming_convention(Ts::name, Convention), json{}).get_to(Ts::value_ref(obj)),
                    ...);
            });
        }

        static void to_json(json& value, const T& obj) {
            for_each_as_values<fields_t>([&]<typename... Ts>(std::type_identity<Ts>...) {
                auto handler = [&]<typename U>(std::type_identity<U>) {
                    if (json intermediate(U::value_ref(obj)); !intermediate.is_null()) {
                        value.emplace(meta::convert_naming_convention(U::name, Convention), std::move(intermediate));
                    }
                };

                (handler(std::type_identity<Ts>{}), ...);
            });
        }
    };

    template <json_serializable T>
    T deserialize_json(const std::filesystem::path& path) {
        return throw_nested_and_flatten(
            source_code_aware_runtime_error{U8("Deserialization Failed"), from_u8string(path.u8string())}, [&] {
                std::ifstream stream{path, std::ios::binary};

                stream.exceptions(std::ifstream::failbit);

                return json::parse(stream).get<T>();
            });
    }
} // namespace essence

#define ES_JSON_SERIALIZABLE(naming_convention)                                                 \
    friend void from_json(const essence::json& value, es_meta_this_type& obj) {                \
        essence::json_serializer<es_meta_this_type, naming_convention>::from_json(value, obj); \
    }                                                                                           \
                                                                                                \
    friend void to_json(essence::json& value, const es_meta_this_type& obj) {                  \
        essence::json_serializer<es_meta_this_type, naming_convention>::to_json(value, obj);   \
    }                                                                                           \
                                                                                                \
    ES_MAKE_TERMINATOR(__COUNTER__)

#define ES_JSON_SERIALIZABLE_DEFAULT ES_JSON_SERIALIZABLE(essence::meta::naming_convention::camel)
