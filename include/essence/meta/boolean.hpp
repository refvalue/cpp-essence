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

#include "identifier.hpp"

#include <concepts>
#include <optional>
#include <string_view>

namespace essence::meta {
    /**
     * @brief A string representing the boolean true.
     */
    inline constexpr auto true_string = get_literal_string_v<bool, true>();

    /**
     * @brief A string representing the boolean false.
     */
    inline constexpr auto false_string = get_literal_string_v<bool, false>();

    /**
     * @brief Gets the name of a boolean as a std::string_view at compile-time.
     * @param value The boolean.
     * @return The name of the boolean.
     */
    constexpr std::string_view to_string(bool value) noexcept {
        if (value) {
            return true_string;
        } else {
            return false_string;
        }
    }

    /**
     * @brief Parses a boolean from a string at compile-time.
     * @tparam T The type of the enumeration.
     * @param name The name of the boolean.
     * @return The boolean, or std::nullopt if failed.
     */
    template <std::same_as<bool> T>
    constexpr std::optional<T> from_string(std::string_view name) noexcept {
        if (name == true_string) {
            return true;
        }

        if (name == false_string) {
            return false;
        }

        return std::nullopt;
    }
} // namespace essence::meta
