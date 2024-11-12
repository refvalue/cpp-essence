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
     * @brief Represents an IPv6 address.
     */
    class ipv6_address {
    public:
        static constexpr std::size_t value_size        = 16;
        static constexpr std::size_t value_group_count = value_size / 2;
        /**
         * @brief On linux systems, the length of a stringified zone ID must be
         *        not longer than 16 bytes including the null-terminating character.
         *        On Windows, the zone ID must be a decimal natural number.
         * @see https://elixir.bootlin.com/linux/v5.6/source/include/uapi/linux/if.h#L33
         */
        static constexpr std::size_t max_zone_id_size = 16;

        using value_type   = std::array<std::uint8_t, value_size>;
        using zone_id_type = std::array<char, max_zone_id_size>;

        /**
         * @brief Creates an instance.
         */
        constexpr ipv6_address() noexcept : value_{}, zone_id_{} {}


        /**
         * @brief Creates an instance from an initializer list.
         * @param address The IPv6 address.
         * @param zone_id  The zone ID of the scoped address.
         */
        constexpr ipv6_address(std::initializer_list<std::uint8_t> address, std::string_view zone_id = "") noexcept
            : ipv6_address{
                  std::span<const std::uint8_t, value_size>{address.begin(), std::min(address.size(), value_size)},
                  zone_id} {}

        /**
         * @brief Creates an instance from a 16-byte array.
         * @param address The IPv6 address.
         * @param zone_id The zone ID of the scoped address.
         */
        constexpr explicit ipv6_address(
            std::span<const std::uint8_t, value_size> address, std::string_view zone_id = "") noexcept
            : ipv6_address{} {
            std::ranges::copy(address, value_.begin());
            set_zone_id(zone_id);
        }

        constexpr auto operator<=>(const ipv6_address&) const noexcept = default;

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

        /**
         * @brief Gets the zone ID.
         * @return The zone ID.
         */
        [[nodiscard]] constexpr const char* zone_id() const noexcept {
            return zone_id_.data();
        }

        /**
         * @brief Sets the zone ID.
         * @param zone_id The zone ID.
         */
        constexpr void set_zone_id(std::string_view zone_id) noexcept {
            zone_id.copy(zone_id_.data(), std::min(zone_id_.size(), zone_id.size()));
        }

    private:
        value_type value_;
        zone_id_type zone_id_;
    };

    /**
     * @brief Convert a string to an IPv6 address.
     * @param str The string.
     * @return The IPv6 address if succeeds; otherwise std::nullopt.
     */
    ES_API(CPPESSENCE) std::optional<ipv6_address> parse_ipv6_address(std::string_view str);

    /**
     * @brief Convert a string to an IPv6 address.
     * @tparam T The type of the ipv6_address class.
     * @param str The string.
     * @return The IPv6 address if succeeds; otherwise std::nullopt.
     */
    template <std::same_as<ipv6_address> T>
    std::optional<T> from_string(std::string_view str) {
        return parse_ipv6_address(str);
    }

    /**
     * @brief Converts an IPv6 address to a string.
     * @param address The IPv6 address.
     * @return The string.
     */
    ES_API(CPPESSENCE) abi::string to_string(const ipv6_address& address);
} // namespace essence::net
