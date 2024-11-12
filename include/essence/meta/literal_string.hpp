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

#include "../zstring_view.hpp"

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <span>
#include <string_view>

namespace essence::meta {
    /**
     * @brief A literal type of collection of chars.
     * @tparam N The number of chars.
     */
    template <std::size_t N>
    struct literal_string {
        std::array<char, N + 1> elements;

        consteval literal_string() noexcept : elements{} {}

        consteval literal_string(const char (&data)[N + 1]) noexcept // NOLINT(*-explicit-constructor)
            : elements{std::to_array(data)} {} // NOLINT(*-explicit-constructor)

        template <std::size_t... Ns>
        consteval literal_string(std::span<const char, Ns>... data) noexcept // NOLINT(*-explicit-constructor)
            : elements{} { // NOLINT(*-explicit-constructor)
            auto iter = elements.begin();

            ((iter = std::ranges::copy(data, iter).out), ..., static_cast<void>(iter));
        }

        template <std::size_t... Ns>
        consteval literal_string(const literal_string<Ns>&... data) noexcept // NOLINT(*-explicit-constructor)
            : elements{} { // NOLINT(*-explicit-constructor)
            auto iter = elements.begin();

            ((iter = std::ranges::copy(data, iter).out), ..., static_cast<void>(iter));
        }

        template <std::same_as<char>... Ts>
            requires(sizeof...(Ts) == N)
        consteval literal_string(Ts... chars) noexcept : elements{chars...} {} // NOLINT(*-explicit-constructor)

        constexpr char& operator[](std::size_t index) noexcept {
            return elements[index];
        }

        constexpr const char& operator[](std::size_t index) const noexcept {
            return elements[index];
        }

        constexpr operator std::string_view() const noexcept { // NOLINT(*-explicit-constructor)
            return std::string_view{elements.data(), size()};
        }

        constexpr operator zstring_view() const noexcept { // NOLINT(*-explicit-constructor)
            return zstring_view{elements.data(), size()};
        }

        [[nodiscard]] static constexpr bool empty() noexcept {
            return size() == 0;
        }

        [[nodiscard]] static constexpr std::size_t size() noexcept {
            return N;
        }

        [[nodiscard]] constexpr char& front() noexcept {
            return elements.front();
        }

        [[nodiscard]] constexpr const char& front() const noexcept {
            return elements.front();
        }

        [[nodiscard]] constexpr char& back() noexcept {
            return elements[size() - 1];
        }

        [[nodiscard]] constexpr const char& back() const noexcept {
            return elements[size() - 1];
        }

        [[nodiscard]] constexpr auto begin() noexcept {
            return elements.begin();
        }

        [[nodiscard]] constexpr auto begin() const noexcept {
            return elements.begin();
        }

        [[nodiscard]] constexpr auto end() noexcept {
            return elements.begin() + size();
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return elements.begin() + size();
        }

        [[nodiscard]] constexpr char* data() noexcept {
            return elements.data();
        }

        [[nodiscard]] constexpr const char* data() const noexcept {
            return elements.data();
        }

        [[nodiscard]] constexpr const char* c_str() const noexcept {
            return elements.data();
        }

        [[nodiscard]] constexpr bool contains(char c) const noexcept {
            return std::find(begin(), end(), c) != end();
        }

        [[nodiscard]] constexpr bool contains(std::string_view str) const noexcept {
            return str.size() <= size() ? std::search(begin(), end(), str.begin(), str.end()) != end() : false;
        }

        [[nodiscard]] constexpr auto to_span() const noexcept {
            return std::span<const char, N>{data(), size()};
        }
    };

    template <std::size_t N>
    literal_string(const char (&)[N]) -> literal_string<N - 1>;

    template <std::size_t... Ns>
    literal_string(std::span<const char, Ns>...) -> literal_string<(Ns + ...)>;

    template <std::size_t... Ns>
    literal_string(const literal_string<Ns>&...) -> literal_string<(Ns + ...)>;

    template <std::same_as<char>... Ts>
    literal_string(Ts...) -> literal_string<sizeof...(Ts)>;

    template <std::size_t M, std::size_t N>
    constexpr auto operator<=>(const literal_string<M>& left, const literal_string<N>& right) noexcept {
        return static_cast<std::string_view>(left).compare(static_cast<std::string_view>(right)) <=> 0;
    }

    template <std::size_t M, std::size_t N>
    constexpr bool operator==(const literal_string<M>& left, const literal_string<N>& right) noexcept {
        return static_cast<std::string_view>(left) == static_cast<std::string_view>(right);
    }

    template <std::size_t M, std::size_t N>
    consteval auto operator+(const literal_string<M>& left, const literal_string<N>& right) noexcept {
        return literal_string{left, right};
    }
} // namespace essence::meta
