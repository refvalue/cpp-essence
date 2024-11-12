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

#include <algorithm>
#include <compare>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <utility>

namespace essence {
    template <typename T>
        requires std::is_arithmetic_v<T>
    struct implicit_number_casting_operator {
        T value;

        constexpr implicit_number_casting_operator(T value) noexcept : value{value} {} // NOLINT(*-explicit-constructor)

        template <typename U>
        constexpr operator U() const noexcept // NOLINT(*-explicit-constructor)
            requires std::is_arithmetic_v<U>
        {
            return static_cast<U>(value);
        }
    };

    template <typename T>
    concept equivalent_rect = std::is_aggregate_v<T> && requires(T obj) {
        requires std::is_arithmetic_v<decltype(obj.x)>;
        requires std::is_arithmetic_v<decltype(obj.y)>;
        requires std::is_arithmetic_v<decltype(obj.width)>;
        requires std::is_arithmetic_v<decltype(obj.height)>;
        T{
            .x      = {},
            .y      = {},
            .width  = {},
            .height = {},
        };
    };

    template <typename T>
    concept bottom_right_rect = std::is_aggregate_v<T> && requires(T obj) {
        requires std::is_arithmetic_v<decltype(obj.left)>;
        requires std::is_arithmetic_v<decltype(obj.top)>;
        requires std::is_arithmetic_v<decltype(obj.right)>;
        requires std::is_arithmetic_v<decltype(obj.bottom)>;
        T{
            .left   = {},
            .top    = {},
            .right  = {},
            .bottom = {},
        };
    };

    template <typename T>
    concept width_height_pair = std::is_aggregate_v<T> && requires(T obj) {
        requires std::is_arithmetic_v<decltype(obj.width)>;
        requires std::is_arithmetic_v<decltype(obj.height)>;
        T{
            .width  = {},
            .height = {},
        };
    };

    template <typename T>
    concept nothrow_equivalent_rect = requires {
        requires equivalent_rect<T>;
        {
            T{
                .x      = {},
                .y      = {},
                .width  = {},
                .height = {},
            }
        } noexcept;
    };

    template <typename T>
    concept nothrow_bottom_right_rect = requires {
        requires bottom_right_rect<T>;
        {
            T{
                .left   = {},
                .top    = {},
                .right  = {},
                .bottom = {},
            }
        } noexcept;
    };

    template <typename T>
    concept nothrow_width_height_pair = requires {
        requires width_height_pair<T>;
        {
            T{
                .width  = {},
                .height = {},
            }
        } noexcept;
    };

    /**
     * @brief Indicates which rectangle to base the ratio calculation on.
     */
    enum class rect_ratio_base {
        /**
         * @brief The left operand.
         */
        left,
        /**
         * @brief The right operand.
         */
        right,
        /**
         * @brief The smaller rectangle.
         */
        smaller,
    };

    /**
     * @brief Illustrates a rectangle.
     * @tparam Number The numeric type of the member variables.
     */
    template <typename Number>
        requires std::is_arithmetic_v<Number>
    struct rect {
        Number x{};
        Number y{};
        Number width{};
        Number height{};

        constexpr auto operator<=>(const rect&) const noexcept = default;

        constexpr Number right() const noexcept {
            return x + width;
        }

        constexpr Number bottom() const noexcept {
            return y + height;
        }

        constexpr Number area() const noexcept {
            return width * height;
        }

        /**
         * @brief Converts to a compatible rectangle that contains member variables left, top, right, bottom.
         * @tparam T The type of the compatible rectangle.
         */
        template <typename T>
        constexpr operator T() const // NOLINT(*-explicit-constructor)
            noexcept(nothrow_bottom_right_rect<std::decay_t<T>>) // NOLINT(*-explicit-constructor)
            requires bottom_right_rect<std::decay_t<T>>
        {
            return std::decay_t<T>{
                .left   = implicit_number_casting_operator{x},
                .top    = implicit_number_casting_operator{y},
                .right  = implicit_number_casting_operator{right()},
                .bottom = implicit_number_casting_operator{bottom()},
            };
        }

        /**
         * @brief Converts to an equivalent rectangle that contains member variables x, y, width, height.
         * @tparam T The type of the equivalent rectangle.
         */
        template <typename T>
        constexpr operator T() const // NOLINT(*-explicit-constructor)
            noexcept(nothrow_equivalent_rect<std::decay_t<T>>) // NOLINT(*-explicit-constructor)
            requires equivalent_rect<std::decay_t<T>>
        {
            return std::decay_t<T>{
                .x      = implicit_number_casting_operator{x},
                .y      = implicit_number_casting_operator{y},
                .width  = implicit_number_casting_operator{width},
                .height = implicit_number_casting_operator{height},
            };
        }

        /**
         * @brief Retrieves the width-height pair.
         * @tparam T The type of the pair that contains member variables width and height.
         * @return The weight-height pair.
         */
        template <width_height_pair T>
        constexpr T to_extent() const noexcept(nothrow_width_height_pair<T>) {
            return T{
                .width  = implicit_number_casting_operator{width},
                .height = implicit_number_casting_operator{height},
            };
        }

        /**
         * @brief Sets the width and height from a width-height pair.
         * @tparam T The type of the pair that contains member variables width and height.
         * @param pair The weight-height pair.
         */
        template <width_height_pair T>
        constexpr void assign_from_extent(const T& pair) noexcept {
            width  = implicit_number_casting_operator{pair.width};
            height = implicit_number_casting_operator{pair.height};
        }

        /**
         * @brief Checks whether the area of the rectangle is zero (i.e. collapsed to a zero-dimensional point).
         * @return True if the area is zero; otherwise false.
         */
        [[nodiscard]] constexpr bool collapsed() const noexcept {
            return width <= std::numeric_limits<Number>::epsilon() || height <= std::numeric_limits<Number>::epsilon();
        }

        /**
         * @brief Makes a rectangle from a compatible rectangle that contains member variables x, y, width, height.
         * @tparam T The type of the compatible rectangle.
         * @param bottom_right_rect The compatible rectangle.
         * @return The rectangle.
         */
        template <bottom_right_rect T>
        static constexpr rect from(const T& bottom_right_rect) noexcept {
            return rect{
                .x      = implicit_number_casting_operator{bottom_right_rect.left},
                .y      = implicit_number_casting_operator{bottom_right_rect.top},
                .width  = implicit_number_casting_operator{bottom_right_rect.right - bottom_right_rect.left},
                .height = implicit_number_casting_operator{bottom_right_rect.bottom - bottom_right_rect.top},
            };
        }

        /**
         * @brief Makes a rectangle from an equivalent rectangle that contains member variables x, y, bottom, right.
         * @tparam T The type of the equivalent rectangle.
         * @param equivalent_rect The equivalent rectangle。
         * @return The rectangle.
         */
        template <equivalent_rect T>
        static constexpr rect from(const T& equivalent_rect) noexcept {
            return rect{
                .x      = implicit_number_casting_operator{equivalent_rect.x},
                .y      = implicit_number_casting_operator{equivalent_rect.y},
                .width  = implicit_number_casting_operator{equivalent_rect.width},
                .height = implicit_number_casting_operator{equivalent_rect.height},
            };
        }
    };

    /**
     * @brief Checks if one rectangle intersects with another rectangle.
     * @tparam T The numeric type of the member variables.
     * @param left The first rectangle.
     * @param right The second rectangle.
     * @return True if the first rectangle intersects with the second one; otherwise false.
     */
    template <typename T>
        requires std::is_arithmetic_v<T>
    constexpr bool intersect_with(const rect<T>& left, const rect<T>& right) noexcept {
        return std::max(left.x, right.x) <= std::min(left.right(), right.right())
            && std::max(left.y, right.y) <= std::min(left.bottom(), right.bottom());
    }

    /**
     * @brief Calculates the overlapped area of two rectangles.
     * @tparam T The numeric type of the member variables.
     * @param left The first rectangle.
     * @param right The second rectangle.
     * @return The overlapped area of two rectangles.
     */
    template <typename T>
        requires std::is_arithmetic_v<T>
    constexpr T calc_overlapped_area(const rect<T>& left, const rect<T>& right) noexcept {
        auto dx = std::min(left.right(), right.right()) - std::max(left.x, right.x);
        auto dy = std::min(left.bottom(), right.bottom()) - std::max(left.y, right.y);

        return dx >= 0 && dy >= 0 ? dx * dy : 0;
    }

    /**
     * @brief Calculates the ratio of the overlapped area.
     * @tparam T The numeric type of the member variables.
     * @param left The first rectangle.
     * @param right The second rectangle.
     * @param ratio_base Indicates which rectangle to base the ratio calculation on.
     * @return The overlapped ratio.
     */
    template <typename T>
        requires std::is_arithmetic_v<T>
    constexpr double calc_overlapped_ratio(
        const rect<T>& left, const rect<T>& right, rect_ratio_base ratio_base) noexcept {
        auto area = static_cast<double>(calc_overlapped_area(left, right));

        switch (ratio_base) {
        case rect_ratio_base::left:
            return area / left.area();
        case rect_ratio_base::right:
            return area / right.area();
        case rect_ratio_base::smaller:
            return area / std::min(left.area(), right.area());
        default:
            return 0.0;
        }
    }

    using recti = rect<std::int32_t>;
    using rectf = rect<float>;
    using rectd = rect<double>;
} // namespace essence
