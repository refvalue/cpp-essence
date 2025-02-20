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

export module essence.serialization:detail.json;
import :basic_json;
import essence.basic;
import essence.meta;
import std;

export namespace essence::serialization::detail {
    template <typename T>
    concept basic_json_context = is_basic_json_context<T>::value;

    template <typename T>
    concept basic_json = is_basic_json<T>::value;

    template <typename T>
    concept json_compatible_type = is_compatible_type<nlohmann_json, T>::value;

    template <typename T>
    concept primitive_json_serializable =
        (json_compatible_type<T> || std::ranges::forward_range<T>) && !std::is_enum_v<T> && !std_optional<T>;

    template <typename T>
    concept non_iterable_object_json_serializable =
        std::is_class_v<T> && !json_compatible_type<T> && !std::ranges::forward_range<T> && !std_optional<T>;

    template <typename T>
    concept iterable_json_serializable = std::ranges::forward_range<T>
                                      && (json_compatible_type<std::ranges::range_value_t<T>>
                                          || non_iterable_object_json_serializable<std::ranges::range_value_t<T>>);

    template <typename T>
    concept has_json_serialization_config = std::is_class_v<T> && requires {
        typename T::json_serialization;
        requires std::is_enum_v<typename T::json_serialization>;
    };

    template <typename T>
    concept has_json_omission_config = std::is_class_v<T> && requires {
        typename T::json_omission;
        requires std::is_class_v<typename T::json_omission>;
    };

    template <typename T>
    concept has_snake_case = std::is_enum_v<T> && requires { T::snake_case; };

    template <typename T>
    concept has_camel_case = std::is_enum_v<T> && requires { T::camel_case; };

    template <typename T>
    concept has_pascal_case = std::is_enum_v<T> && requires { T::pascal_case; };

    template <typename T>
    concept has_enum_to_string = std::is_enum_v<T> && requires { T::enum_to_string; };

    template <typename T>
    concept has_enum_to_string_config = std::is_class_v<T> && has_json_serialization_config<T>
                                     && requires { requires has_enum_to_string<typename T::json_serialization>; };

    template <typename>
    consteval auto get_naming_convention_by_enum() noexcept {
        return meta::naming_convention::snake_case;
    }

    template <has_snake_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return meta::naming_convention::snake_case;
    }

    template <has_camel_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return meta::naming_convention::camel_case;
    }

    template <has_pascal_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return meta::naming_convention::pascal_case;
    }

    template <typename T>
        requires std::is_class_v<T>
    consteval meta::naming_convention get_json_naming_convention() noexcept {
        if constexpr (has_json_serialization_config<T>) {
            return get_naming_convention_by_enum<typename T::json_serialization>();
        } else {
            return meta::naming_convention::snake_case;
        }
    }

    template <typename T>
        requires std::is_class_v<T>
    bool check_json_omitted(std::string_view name) {
        if constexpr (has_json_omission_config<T>) {
            static const auto mapping = [] {
                std::unordered_set<std::string_view> result;

                for (auto&& item : meta::runtime::get_data_member_names<typename T::json_omission>()) {
                    result.emplace(item);
                }

                return result;
            }();

            return mapping.contains(name);
        } else {
            return false;
        }
    }
} // namespace essence::serialization::detail
