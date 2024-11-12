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
#include "../enum.hpp"
#include "../naming_convention.hpp"

#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace essence::meta::runtime {
    /**
     * @brief Gets the pairs of names and values of an enumeration.
     * @tparam T The type of the enumeration.
     * @tparam Tag The tag for extra features.
     * @param short_name Whether to get a short name (without a namespace).
     * @return The pairs.
     */
    template <typename T, typename Tag = void>
        requires std::is_enum_v<T>
    std::span<const std::pair<std::string, T>> get_enum_names(bool short_name = true) {
        static constexpr std::array naming_conventions{
            naming_convention::camel_case,
            naming_convention::pascal_case,
            naming_convention::snake_case,
        };

        static constexpr auto make_cache = [](bool short_name) {
            std::vector<std::pair<std::string, T>> result;
            auto names = short_name ? meta::probe_enum_names<T, true>() : meta::probe_enum_names<T, false>();

            for (auto&& [name, value] : names) {
                result.emplace_back(std::string{name}, value);

                // Adds extra names according to the naming conventions.
                if constexpr (std::same_as<Tag, all_naming_conventions_tag>) {
                    auto extra_names = naming_conventions | std::views::transform([&](const auto& inner) {
                        return convert_naming_convention(name, inner);
                    }) | std::views::filter([&](const auto& inner) { return inner != name; });

                    for (auto&& item : extra_names) {
                        result.emplace_back(std::move(item), value);
                    }
                }
            }

            return result;
        };

        static const auto short_cache = make_cache(true);
        static const auto cache       = make_cache(false);

        if (short_name) {
            return short_cache;
        }

        return cache;
    }

    /**
     * @brief Gets the pairs of names and values of an enumeration.
     * @tparam T The type of the enumeration.
     * @tparam Tag The tag for extra features.
     * @param short_name Whether to get a short name (without a namespace).
     * @return The names.
     */
    template <typename T, typename Tag = void>
        requires std::is_enum_v<T>
    std::span<const std::string> get_enum_names_only(bool short_name = false) {
        static const auto make_cache = [](bool short_name) {
            std::vector<std::string> result;

            for (auto&& [name, value] : get_enum_names<T, Tag>(short_name)) {
                result.emplace_back(name);
            }

            return result;
        };

        static const auto short_cache = make_cache(true);
        static const auto cache       = make_cache(false);

        if (short_name) {
            return short_cache;
        }

        return cache;
    }

    /**
     * @brief Gets the name of an enumeration.
     * @tparam T The type of the enumeration.
     * @param value The enumeration.
     * @return The name of the enumeration, or an empty string if not found.
     */
    template <typename T>
        requires std::is_enum_v<T>
    std::string to_string(T value) {
        static const auto mapping = [] {
            std::unordered_map<T, std::string> result;

            for (auto&& [name, value] : get_enum_names<T>()) {
                result.insert_or_assign(value, name);
            }

            return result;
        }();

        if (auto iter = mapping.find(value); iter != mapping.end()) {
            return iter->second;
        }

        return {};
    }

    /**
     * @brief Parses an enumeration from a string.
     * @tparam T The type of the enumeration.
     * @param name The name of the enumeration.
     * @return The enumeration or std::nullopt if failed.
     */
    template <typename T>
        requires std::is_enum_v<T>
    std::optional<T> from_string(std::string_view name) {
        static const auto mapping = [] {
            std::unordered_map<std::string_view, T> result;

            for (auto&& [name, value] : get_enum_names<T, all_naming_conventions_tag>()) {
                result.insert_or_assign(name, value);
            }

            return result;
        }();

        if (auto iter = mapping.find(name); iter != mapping.end()) {
            return iter->second;
        }

        return std::nullopt;
    }
} // namespace essence::meta::runtime
