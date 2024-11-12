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

#include "../abi/string.hpp"
#include "../compat.hpp"

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <initializer_list>
#include <optional>
#include <span>
#include <string_view>

namespace essence::net {
    /**
     * @brief Represents an IPv4 address.
     */
    class ipv4_address {
    public:
        static constexpr std::size_t value_size = 4;

        using value_type = std::array<std::uint8_t, value_size>;

        /**
         * @brief Creates an instance.
         */
        constexpr ipv4_address() noexcept : value_{} {}

        /**
         * @brief Creates an instance from an initializer list.
         * @param address The IPv4 address.
         */
        constexpr ipv4_address(std::initializer_list<std::uint8_t> address) noexcept
            : ipv4_address{
                  std::span<const std::uint8_t, value_size>{address.begin(), std::min(address.size(), value_size)}} {}

        /**
         * @brief Creates an instance from a 4-byte array.
         * @param address The IPv4 address.
         */
        constexpr explicit ipv4_address(std::span<const std::uint8_t, value_size> address) noexcept : ipv4_address{} {
            std::ranges::copy(address, value_.begin());
        }

        /**
         * @brief Creates an instance from a 32-bit unsigned integer.
         * @param address The IPv4 address.
         */
        constexpr explicit ipv4_address(std::uint32_t address) noexcept
            : value_{static_cast<std::uint8_t>(address >> 24), static_cast<std::uint8_t>(address >> 16),
                  static_cast<std::uint8_t>(address >> 8), static_cast<std::uint8_t>(address & 0xFF)} {}

        constexpr auto operator<=>(const ipv4_address&) const noexcept = default;

        /**
         * @brief Converts to a 32-bit unsigned integer.
         */
        constexpr explicit operator std::uint32_t() const noexcept {
            return (value_[0] << 24) + (value_[1] << 16) + (value_[2] << 8) + value_[3];
        }

        /**
         * @brief Gets the underlying array.
         * @return The underlying array.
         */
        [[nodiscard]] constexpr value_type& get() noexcept {
            return value_;
        }

        /**
         * @brief Gets the underlying array.
         * @return The underlying array.
         */
        [[nodiscard]] constexpr const value_type& get() const noexcept {
            return value_;
        }

    private:
        value_type value_;
    };

    /**
     * @brief Convert a string to an IPv4 address.
     * @param str The string.
     * @return The IPv4 address if succeeds; otherwise std::nullopt.
     */
    ES_API(CPPESSENCE) std::optional<ipv4_address> parse_ipv4_address(std::string_view str);

    /**
     * @brief Convert a string to an IPv4 address.
     * @tparam T The type of the ipv4_address class.
     * @param str The string.
     * @return The IPv4 address if succeeds; otherwise std::nullopt.
     */
    template <std::same_as<ipv4_address> T>
    std::optional<T> from_string(std::string_view str) {
        return parse_ipv4_address(str);
    }

    /**
     * @brief Converts an IPv4 address to a string.
     * @param address The IPv4 address.
     * @return The string.
     */
    ES_API(CPPESSENCE) abi::string to_string(const ipv4_address& address);
} // namespace essence::net
