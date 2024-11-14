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

#include "friendly_name.hpp"
#include "identifier.hpp"
#include "literal_string.hpp"

#include <compare>
#include <span>
#include <string_view>

namespace essence::meta {
    /**
     * @brief A unique identifier of a type, i.e. a fingerprint.
     */
    class fingerprint {
        template <typename T>
        struct typename_cache {
            static constexpr auto origin = get_literal_string_t<T>();
            static constexpr literal_string value{std::span<const char, origin.size()>{origin.data(), origin.size()}};
        };

    public:
        /**
         * @brief Create a fingerprint for a type.
         * @tparam T The type.
         */
        template <typename T>
        explicit consteval fingerprint(std::type_identity<T>) noexcept
            : typename_{typename_cache<T>::value}, friendly_name_{friendly_name_v<T>} {}

        constexpr auto operator<=>(const fingerprint&) const noexcept = default;

        /**
         * @brief Gets the name of the type.
         * @return The name of the type.
         */
        [[nodiscard]] constexpr const char* name() const noexcept {
            return typename_.data();
        }

        /**
         * @brief Gets the friendly name of the type.
         * @return The friendly name of the type if exists; otherwise the ordinary name.
         */
        [[nodiscard]] constexpr const char* friendly_name() const noexcept {
            return friendly_name_.empty() ? typename_.data() : friendly_name_.data();
        }

    private:
        std::string_view typename_;
        std::string_view friendly_name_;
    };
} // namespace essence::meta
