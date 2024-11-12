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

#include "char.hpp"

#include <array>
#include <concepts>
#include <ranges>
#include <span>
#include <type_traits>

namespace essence::detail {
    /**
     * @brief Gets the type of range value.
     * @tparam Range The type of the range.
     * @return A type_identity<T> object where the T is std::ranges::range_value_t<Range>.
     * @remark This trick works around non-satisfaction of the concepts of std::ranges::range_value_t<>.
     */
    template <std::ranges::range Range>
    consteval auto get_range_value_t_or_self() noexcept {
        return std::type_identity<std::ranges::range_value_t<Range>>{};
    }

    /**
     * @brief Gets the type of non-ranged value.
     * @tparam T The type of the value.
     * @return A type_identity<T> object.
     * @remark This trick works around non-satisfaction of the concepts of std::ranges::range_value_t<>.
     */
    template <typename T>
    consteval auto get_range_value_t_or_self() noexcept {
        return std::type_identity<T>{};
    }
} // namespace essence::detail

namespace essence {
    template <typename T>
    concept byte_like_contiguous_range = std::ranges::contiguous_range<T> && byte_like<std::ranges::range_value_t<T>>;

    template <typename T>
    concept extendable_contiguous_range = requires(T obj, std::size_t size, typename T::value_type value) {
        requires std::ranges::contiguous_range<T>;
        obj.clear();
        obj.resize(size);
        obj.shrink_to_fit();
        obj.push_back(value);
    };

    /**
     * @brief Gets the type of range value, or the type itself if the type is non-ranged.
     * @tparam T The type.
     */
    template <typename T>
    struct range_value_t_or_self {
        using type = typename decltype(detail::get_range_value_t_or_self<T>())::type;
    };

    /**
     * @brief Gets the type of range value, or the type itself if the type is non-ranged.
     * @tparam T The type.
     */
    template <typename T>
    using range_value_t_or_self_t = typename range_value_t_or_self<T>::type;

    template <byte_like_contiguous_range Range>
    constexpr auto as_const_byte_span(Range&& range) noexcept {
        // Deals with a narrow/UTF-8 string literal correctly (excluding the trailing null-terminating character).
        if constexpr (std::convertible_to<Range, std::string_view>) {
            const std::string_view view{range};

            return std::span{reinterpret_cast<const std::byte*>(view.data()), view.size()};

        } else if constexpr (std::convertible_to<Range, std::u8string_view>) {
            const std::u8string_view view{range};

            return std::span{reinterpret_cast<const std::byte*>(view.data()), view.size()};
        } else {
            return std::span{reinterpret_cast<const std::byte*>(std::ranges::cdata(range)),
                std::ranges::size(range) * sizeof(std::ranges::range_value_t<Range>)};
        }
    }

    /**
     * @brief Joins a range of items with a delimiter.
     * @tparam Range The type of the range.
     * @tparam Delimiter The type of the delimiter.
     * @param range The range.
     * @param delimiter The delimiter.
     * @return The result view.
     */
    template <std::ranges::forward_range Range, std::ranges::forward_range Delimiter>
        requires(std::ranges::contiguous_range<std::ranges::range_value_t<Range>>
                 && std::ranges::contiguous_range<Delimiter>)
    constexpr auto join_with(Range&& range, Delimiter&& delimiter) {
        using cache_type =
            std::array<std::span<const std::ranges::range_value_t<std::ranges::range_value_t<Range>>>, 2>;

        cache_type cache;

        cache.back() = std::span{std::ranges::data(delimiter), std::ranges::size(delimiter)};

        auto adapter = range | std::views::transform([cache](const auto& inner) mutable {
            cache.front() = std::span{std::ranges::data(inner), std::ranges::size(inner)};

            return cache | std::views::join;
        }) | std::views::join;

        auto size = std::ranges::distance(adapter);

        return adapter | std::views::take(size == 0 ? 0 : size - 1) | std::views::common;
    }
} // namespace essence
