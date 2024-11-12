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

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#if __cpp_char8_t >= 201811L
namespace essence::detail {
    template <std::size_t N>
    struct char8_t_string_literal {
        static constexpr std::size_t size = N;

        bool char_literal;
        std::array<char8_t, N> sequence;

        consteval char8_t_string_literal(char8_t ch) noexcept // NOLINT(*-explicit-constructor)
            : char_literal{true}, sequence{ch} {}

        template <std::size_t... Is>
        consteval char8_t_string_literal(const char8_t (&sequence)[N], std::index_sequence<Is...>) noexcept
            : char_literal{}, sequence{sequence[Is]...} {}

        consteval char8_t_string_literal(const char8_t (&sequence)[N]) noexcept // NOLINT(*-explicit-constructor)
            : char8_t_string_literal{sequence, std::make_index_sequence<N>{}} {}
    };

    char8_t_string_literal(char8_t) -> char8_t_string_literal<1>;

    template <std::size_t N>
    char8_t_string_literal(const char8_t (&)[N]) -> char8_t_string_literal<N>;

    template <char8_t_string_literal Literal, std::size_t... Is>
    inline constexpr const char as_char_array_v[sizeof...(Is)]{static_cast<char>(Literal.sequence[Is])...};

    template <char8_t_string_literal Literal, std::size_t... Is>
    constexpr auto&& make_as_char_array(std::index_sequence<Is...>) noexcept {
        return as_char_array_v<Literal, Is...>;
    }
} // namespace essence::detail

namespace essence {
    template <detail::char8_t_string_literal Literal>
    inline constexpr decltype(auto) as_char_v = []() -> decltype(auto) {
        if constexpr (Literal.char_literal) {
            return static_cast<char>(Literal.sequence.front());
        } else {
            return detail::make_as_char_array<Literal>(std::make_index_sequence<decltype(Literal)::size>{});
        }
    }();

#define U8(x) (essence::as_char_v<(u8##x)>)

    inline std::string from_u8string(std::u8string_view str) {
        return {str.begin(), str.end()};
    }

    inline std::u8string to_u8string(std::string_view str) {
        return {str.begin(), str.end()};
    }
} // namespace essence
#else
#define U8(x) u8##x

namespace essence {
    inline std::string from_u8string(std::string_view str) {
        return std::string{str};
    }

    inline std::string from_u8string(std::string&& str) noexcept {
        return std::move(str);
    }

    inline std::string to_u8string(std::string_view str) {
        return std::string{str};
    }

    inline std::string to_u8string(std::string&& str) noexcept {
        return std::move(str);
    }
} // namespace essence
#endif
