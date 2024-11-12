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

#include "../common_types.hpp"
#include "../runtime/struct.hpp"

#include <type_traits>
#include <unordered_set>

namespace essence::meta::detail {
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
        return naming_convention::snake_case;
    }

    template <has_snake_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return naming_convention::snake_case;
    }

    template <has_camel_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return naming_convention::camel_case;
    }

    template <has_pascal_case>
    consteval auto get_naming_convention_by_enum() noexcept {
        return naming_convention::pascal_case;
    }

    template <typename T>
        requires std::is_class_v<T>
    consteval naming_convention get_json_naming_convention() noexcept {
        if constexpr (has_json_serialization_config<T>) {
            return get_naming_convention_by_enum<typename T::json_serialization>();
        } else {
            return naming_convention::snake_case;
        }
    }

    template <typename T>
        requires std::is_class_v<T>
    bool check_json_omitted(std::string_view name) {
        if constexpr (has_json_omission_config<T>) {
            static const auto mapping = [] {
                std::unordered_set<std::string_view> result;

                for (auto&& item : runtime::get_data_member_names<typename T::json_omission>()) {
                    result.emplace(item);
                }

                return result;
            }();

            return mapping.contains(name);
        } else {
            return false;
        }
    }
} // namespace essence::meta::detail
