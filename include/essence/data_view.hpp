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

#include "type_identity.hpp"

#include <array>
#include <cstddef>
#include <iterator>
#include <type_traits>

#if __has_include(<version>)
#include <version>
#endif

#if __cpp_lib_span >= 202002L
#include <span>
#endif

#if __cpp_lib_ranges >= 201911L
#include <ranges>
#endif

namespace essence {
    /**
     * @brief A view of memory data, like std::span.
     * @tparam T The element type.
     */
    template <typename T>
    class data_view {
    public:
        using element_type     = T;
        using value_type       = std::remove_cv_t<T>;
        using size_type        = std::size_t;
        using difference_type  = std::ptrdiff_t;
        using pointer          = T*;
        using const_pointer    = const T*;
        using reference        = T&;
        using const_reference  = const T&;
        using iterator         = T*;
        using reverse_iterator = std::reverse_iterator<iterator>;

        constexpr data_view(T* ptr, size_type size) noexcept : ptr_{ptr}, size_{size} {}

        template <std::size_t N>
        constexpr data_view(type_identity_t<element_type> (&array)[N]) noexcept // NOLINT(*-explicit-constructor)
            : ptr_{array}, size_{N} {}

        template <typename U, std::size_t N>
        constexpr data_view(std::array<U, N>& array) noexcept // NOLINT(*-explicit-constructor)
            : ptr_{array.data()}, size_{array.size()} {}

        template <typename U, std::size_t N>
        constexpr data_view(const std::array<U, N>& array) noexcept // NOLINT(*-explicit-constructor)
            : ptr_{array.data()}, size_{array.size()} {}

#if __cpp_lib_ranges >= 201911L
        template <typename Range>
        constexpr data_view(Range&& range) // NOLINT(*-explicit-constructor, *-forwarding-reference-overload)
            : ptr_{std::ranges::data(range)}, size_{std::ranges::size(range)} {}
#endif

#if __cpp_lib_span >= 202002L
        template <typename U, std::size_t N>
        constexpr data_view( // NOLINT(*-explicit-constructor, *-forwarding-reference-overload)
            const std::span<U, N>& source) noexcept
            : ptr_{source.data()}, size_{source.size()} {}
#endif

        constexpr data_view(const data_view&) noexcept = default;

        constexpr data_view& operator=(const data_view&) noexcept = default;

        constexpr reference operator[](size_type index) const noexcept {
            return ptr_[index];
        }

#if __cpp_lib_span >= 202002L
        constexpr operator std::span<T>() const noexcept { // NOLINT(*-explicit-constructor)
            return std::span<T>{ptr_, size_};
        }
#endif

        [[nodiscard]] constexpr iterator begin() const noexcept {
            return ptr_;
        }

        [[nodiscard]] constexpr iterator end() const noexcept {
            return ptr_ + size_;
        }

        [[nodiscard]] constexpr reverse_iterator rbegin() const noexcept {
            return reverse_iterator{end()};
        }

        [[nodiscard]] constexpr reverse_iterator rend() const noexcept {
            return reverse_iterator{begin()};
        }

        [[nodiscard]] constexpr pointer data() const noexcept {
            return ptr_;
        }

        [[nodiscard]] constexpr size_type size() const noexcept {
            return size_;
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return size_ == 0;
        }

        [[nodiscard]] constexpr size_type size_bytes() const noexcept {
            return size_ * sizeof(T);
        }

        [[nodiscard]] constexpr reference front() const noexcept {
            return ptr_[0];
        }

        [[nodiscard]] constexpr reference back() const noexcept {
            return ptr_[size_ - 1];
        }

        [[nodiscard]] constexpr data_view subview(size_type index, size_type count) const noexcept {
            return data_view{ptr_ + index, count};
        }

    private:
        pointer ptr_;
        size_type size_;
    };

    template <typename T>
    data_view(T* ptr, std::size_t size) -> data_view<T>;

    template <typename T, std::size_t N>
    data_view(T (&)[N]) -> data_view<T>;

    template <typename T, std::size_t N>
    data_view(std::array<T, N>&) -> data_view<T>;

    template <typename T, std::size_t N>
    data_view(const std::array<T, N>&) -> data_view<const T>;

#if __cpp_lib_ranges >= 201911L
    template <typename Range>
    data_view(Range&&) -> data_view<std::remove_reference_t<std::ranges::range_reference_t<Range>>>;
#endif
} // namespace essence
