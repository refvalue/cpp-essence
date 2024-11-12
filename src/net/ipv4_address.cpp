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

#include "net/ipv4_address.hpp"

#include "char8_t_remediation.hpp"
#include "format_remediation.hpp"
#include "numeric_conversion.hpp"

#include <ranges>

namespace essence::net {
    std::optional<ipv4_address> parse_ipv4_address(std::string_view str) {
        auto adapter = std::views::split(str, U8('.')) | std::views::transform([](const auto& inner) {
            auto iter = inner | std::views::common;

            return essence::from_string<std::uint8_t>(std::string{iter.begin(), iter.end()});
        }) | std::views::take(ipv4_address::value_size);

        auto result = std::make_optional<ipv4_address>();
        auto iter   = result->get().begin();

        for (auto&& item : adapter) {
            if (!item) {
                break;
            }

            (*iter++) = *item;
        }

        // Checks an illegal IPv4 address.
        if (iter != result->get().end()) {
            result.reset();
        }

        return result;
    }

    abi::string to_string(const ipv4_address& address) {
        auto&& array = address.get();

        return format_as<abi::string>(U8("{}.{}.{}.{}"), array[0], array[1], array[2], array[3]);
    }
} // namespace essence::net
