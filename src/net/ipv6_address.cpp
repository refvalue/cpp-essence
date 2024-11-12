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

#include "net/ipv6_address.hpp"

#include "char8_t_remediation.hpp"
#include "numeric_conversion.hpp"

#include <ranges>
#include <span>

namespace essence::net {
    static constexpr std::int32_t digit_base     = 16;
    static constexpr auto zone_id_delimiter      = U8('%');
    static constexpr auto ipv6_segment_delimiter = U8(':');
    static constexpr std::string_view ipv6_abbreviated_zero{U8("::")};

    std::optional<ipv6_address> parse_ipv6_address(std::string_view str) {
        return std::nullopt;
    }

    abi::string to_string(const ipv6_address& address) {
        abi::string result;
        auto array = address.get();
        const auto adapter =
            std::views::iota(0U, ipv6_address::value_group_count) | std::views::transform([&](const auto& inner) {
                return static_cast<std::uint16_t>((array[inner * 2] << 8) + array[inner * 2 + 1]);
            });

        bool zero_abbreviated{};
        std::uint16_t last_item{};
        std::size_t consecutive_zero_count{};

        for (auto&& item : adapter) {
            last_item = item;

            if (!zero_abbreviated) {
                if (item == 0) {
                    ++consecutive_zero_count;
                    continue;
                }

                zero_abbreviated = consecutive_zero_count >= 2;
            }

            if (consecutive_zero_count >= 2) {
                // ':" + ':' = "::".
                result.push_back(ipv6_segment_delimiter);
            }

            result.append(essence::to_string(item, digit_base)).push_back(ipv6_segment_delimiter);
            consecutive_zero_count = 0;
        }

        // Handles the trailing zero.
        if (consecutive_zero_count >= 2) {
            result.push_back(ipv6_segment_delimiter);
        } else if (consecutive_zero_count != 0) {
            result.append(essence::to_string(last_item, digit_base));
        }

        // Removes the trailing ':'.
        if (result.ends_with(ipv6_segment_delimiter) && !result.ends_with(ipv6_abbreviated_zero)) {
            result.pop_back();
        }

        // Appends the zone ID.
        if (const std::string_view zone_id{address.zone_id()}; !zone_id.empty()) {
            result.push_back(zone_id_delimiter);
            result.append(zone_id);
        }

        return result;
    }
} // namespace essence::net
