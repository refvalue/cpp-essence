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

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <type_traits>
#include <utility>

namespace essence::detail {
    template <typename T>
    consteval auto rational_bindable_count(T obj) noexcept {
        auto&& [a, b]                = obj;
        constexpr std::size_t type_1 = std::same_as<std::decay_t<decltype(a)>, std::int64_t> ? 1U : 0U;
        constexpr std::size_t type_2 = std::same_as<std::decay_t<decltype(b)>, std::int64_t> ? 1U : 0U;

        return std::integral_constant<std::size_t, type_1 + type_2>{};
    }
} // namespace essence::detail

namespace essence {
    /**
     * @brief Checks if an object is similar to essence::rational.
     */
    template <typename T>
    concept similar_rational = requires(std::int64_t a, std::int64_t b, T obj) {
        T{a, b};
        { detail::rational_bindable_count(obj) } -> std::same_as<std::integral_constant<std::size_t, 2U>>;
    };

    /**
     * @brief Represents a rational number consisting of its corresponding numerator and denominator.
     */
    struct rational {
        std::int64_t numerator;
        std::int64_t denominator;

        constexpr rational() noexcept : numerator{}, denominator{} {}

        explicit constexpr rational(std::int64_t numerator) noexcept : rational{numerator, 1} {}

        constexpr rational(std::int64_t numerator, std::int64_t denominator) noexcept
            : numerator{numerator}, denominator{denominator} {
            simplify();
        }

        template <typename T>
            requires(std::is_aggregate_v<std::decay_t<T>> && similar_rational<std::decay_t<T>>)
        explicit rational(T&& object) noexcept : rational{} {
            auto&& [numerator, denominator] = std::forward<T>(object);

            this->numerator   = numerator;
            this->denominator = denominator;
        }

        /**
         * @brief Converts to a similar object.
         * @tparam T The type of the similar object.
         */
        template <typename T>
            requires similar_rational<T>
        constexpr operator T() const noexcept(similar_rational<T>) { // NOLINT(*-explicit-constructor)
            return T{numerator, denominator};
        }

        /**
         */
        constexpr void simplify() noexcept {
            const auto gcd = std::gcd(numerator, denominator);

            numerator /= gcd;
            denominator /= gcd;

            // A/-B --> -A/B
            // -A/-B --> A/B
            if (denominator < 0) {
                numerator   = -numerator;
                denominator = -denominator;
            }
        }

        /**
         * @brief Gets the reciprocal of the rational.
         * @return The reciprocal.
         */
        [[nodiscard]] constexpr rational reciprocal() const noexcept {
            return rational{denominator, numerator};
        }

        /**
         * @brief Converts to a floating-point number.
         * @tparam T The type of the floating-point number.
         */
        template <std::floating_point T>
        explicit constexpr operator T() const noexcept {
            return static_cast<T>(numerator) / static_cast<T>(denominator);
        }

        constexpr bool operator<(const rational& right) const noexcept {
            return numerator * right.denominator < right.numerator * denominator;
        }

        constexpr bool operator>(const rational& right) const noexcept {
            return numerator * right.denominator > right.numerator * denominator;
        }

        constexpr bool operator==(const rational& right) const noexcept {
            return numerator * right.denominator == right.numerator * denominator;
        }

        constexpr bool operator!=(const rational& right) const noexcept {
            return !(*this == right);
        }

        constexpr bool operator<=(const rational& right) const noexcept {
            return !(*this > right);
        }

        constexpr bool operator>=(const rational& right) const noexcept {
            return !(*this < right);
        }

        friend constexpr rational operator-(const rational& value) noexcept {
            return rational{-value.numerator, value.denominator};
        }

        friend constexpr rational operator+(const rational& left, const rational& right) noexcept {
            const auto lcm = std::lcm(left.denominator, right.denominator);
            rational result{
                left.numerator * (lcm / left.denominator) + right.numerator * (lcm / right.denominator), lcm};

            return (result.simplify(), result);
        }

        friend constexpr rational operator-(const rational& left, const rational& right) noexcept {
            return left + (-right);
        }

        friend constexpr rational operator*(const rational& left, const rational& right) noexcept {
            rational result{left.numerator * right.numerator, left.denominator * right.denominator};

            return (result.simplify(), result);
        }

        friend constexpr rational operator/(const rational& left, const rational& right) noexcept {
            return left * right.reciprocal();
        }
    };

    /**
     * @brief Gets the nearest integer larger than the given rational.
     * @param number The rational.
     * @return The nearest larger integer.
     */
    constexpr std::int64_t ceil(rational number) noexcept {
        return (number.numerator + number.denominator - 1) / number.denominator;
    }
} // namespace essence
