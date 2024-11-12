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

#include "../boolean.hpp"

#include <concepts>
#include <optional>
#include <string>
#include <string_view>

namespace essence::meta::runtime {
    /**
     * @brief Gets the name of a boolean.
     * @tparam T The type of the boolean.
     * @param value The boolean.
     * @return The name of the boolean.
     */
    template <std::same_as<bool> T>
    std::string to_string(T value) {
        return std::string{meta::to_string(value)};
    }

    /**
     * @brief Parses a boolean from a string.
     * @tparam T The type of the boolean.
     * @param name The name of the boolean.
     * @return The boolean, or std::nullopt if failed.
     */
    template <std::same_as<bool> T>
    constexpr std::optional<T> from_string(std::string_view name) noexcept {
        return meta::from_string<T>(name);
    }
} // namespace essence::meta::runtime
