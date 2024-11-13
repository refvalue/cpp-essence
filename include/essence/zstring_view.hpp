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

#if __has_include(<compare>)
#include <compare>
#endif

#include <string>
#include <string_view>

namespace essence {
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_zstring_view {
    public:
        using underlying_type        = std::basic_string_view<CharT, Traits>;
        using traits_type            = typename underlying_type::traits_type;
        using value_type             = typename underlying_type::value_type;
        using pointer                = typename underlying_type::pointer;
        using const_pointer          = typename underlying_type::const_pointer;
        using reference              = typename underlying_type::reference;
        using const_reference        = typename underlying_type::const_reference;
        using const_iterator         = typename underlying_type::const_iterator;
        using iterator               = typename underlying_type::iterator;
        using const_reverse_iterator = typename underlying_type::const_reverse_iterator;
        using reverse_iterator       = typename underlying_type::reverse_iterator;
        using size_type              = typename underlying_type::size_type;
        using difference_type        = typename underlying_type::difference_type;

        static constexpr size_type npos = underlying_type::npos;

        constexpr basic_zstring_view() noexcept                          = default;
        constexpr basic_zstring_view(const basic_zstring_view&) noexcept = default;
        constexpr basic_zstring_view(const CharT* str) : view_{str} {} // NOLINT(*-explicit-constructor)
        constexpr basic_zstring_view(const CharT* str, size_type size) : view_{str, size} {}

        template <typename Allocator>
        constexpr basic_zstring_view( // NOLINT(*-explicit-constructor)
            const std::basic_string<CharT, Traits, Allocator>& str)
            : view_{str} {}

#ifndef __ANDROID__
        template <typename Iter, typename End>
        constexpr basic_zstring_view(Iter first, End last) : view_{first, last} {}
#endif

        constexpr explicit basic_zstring_view(std::nullptr_t) = delete;

        constexpr operator underlying_type() const noexcept { // NOLINT(*-explicit-constructor)
            return view_;
        }

        constexpr const_iterator begin() const noexcept {
            return view_.begin();
        }

        constexpr const_iterator cbegin() const noexcept {
            return view_.cbegin();
        }

        constexpr const_iterator end() const noexcept {
            return view_.end();
        }

        constexpr const_iterator cend() const noexcept {
            return view_.cend();
        }

        constexpr const_reverse_iterator rbegin() const noexcept {
            return view_.rbegin();
        }

        constexpr const_reverse_iterator crbegin() const noexcept {
            return view_.crbegin();
        }

        constexpr const_reverse_iterator rend() const noexcept {
            return view_.rend();
        }

        constexpr const_reverse_iterator crend() const noexcept {
            return view_.crend();
        }

        constexpr const_reference operator[](size_type index) const {
            return view_[index];
        }

        constexpr const_reference at(size_type index) const {
            return view_.at(index);
        }

        constexpr const_reference front() const {
            return view_.front();
        }

        constexpr const_reference back() const {
            return view_.back();
        }

        constexpr const_pointer data() const noexcept {
            return view_.data();
        }

        constexpr const_pointer c_str() const noexcept {
            return view_.data();
        }

        constexpr size_type size() const noexcept {
            return view_.size();
        }

        constexpr size_type length() const noexcept {
            return view_.length();
        }

        constexpr size_type max_size() const noexcept {
            return view_.max_size();
        }

        [[nodiscard]] constexpr bool empty() const noexcept {
            return view_.empty();
        }

        constexpr size_type find(basic_zstring_view view, size_type offset = 0) const noexcept {
            return view_.find(view, offset);
        }

        constexpr size_type find(CharT ch, size_type offset = 0) const noexcept {
            return view_.find(ch, offset);
        }

        constexpr size_type find(const CharT* str, size_type offset, size_type count) const {
            return view_.find(str, offset, count);
        }

        constexpr size_type find(const CharT* str, size_type offset = 0) const {
            return view_.rfind(str, offset);
        }

        constexpr size_type rfind(basic_zstring_view view, size_type offset = npos) const noexcept {
            return view_.rfind(view, offset);
        }

        constexpr size_type rfind(CharT ch, size_type offset = npos) const noexcept {
            return view_.rfind(ch, offset);
        }

        constexpr size_type rfind(const CharT* str, size_type offset, size_type count) const {
            return view_.rfind(str, offset, count);
        }

        constexpr size_type rfind(const CharT* str, size_type offset = npos) const {
            return view_.rfind(str, offset);
        }

        constexpr size_type find_first_of(basic_zstring_view view, size_type offset = 0) const noexcept {
            return view_.find_first_of(view, offset);
        }

        constexpr size_type find_first_of(CharT ch, size_type offset = 0) const noexcept {
            return view_.find_first_of(ch, offset);
        }

        constexpr size_type find_first_of(const CharT* str, size_type offset, size_type count) const {
            return view_.find_first_of(str, offset, count);
        }

        constexpr size_type find_first_of(const CharT* str, size_type offset = 0) const {
            return view_.find_first_of(str, offset);
        }

        constexpr size_type find_last_of(basic_zstring_view view, size_type offset = npos) const noexcept {
            return view_.find_last_of(view, offset);
        }

        constexpr size_type find_last_of(CharT ch, size_type offset = npos) const noexcept {
            return view_.find_last_of(ch, offset);
        }

        constexpr size_type find_last_of(const CharT* str, size_type offset, size_type count) const {
            return view_.find_last_of(str, offset, count);
        }

        constexpr size_type find_last_of(const CharT* str, size_type offset = npos) const {
            return view_.find_last_of(str, offset);
        }

        constexpr size_type find_first_not_of(basic_zstring_view view, size_type offset = 0) const noexcept {
            return view_.find_first_not_of(view, offset);
        }

        constexpr size_type find_first_not_of(CharT ch, size_type offset = 0) const noexcept {
            return view_.find_first_not_of(ch, offset);
        }

        constexpr size_type find_first_not_of(const CharT* str, size_type offset, size_type count) const {
            return view_.find_first_not_of(str, offset, count);
        }

        constexpr size_type find_first_not_of(const CharT* str, size_type offset = 0) const {
            return view_.find_first_not_of(str, offset);
        }

        constexpr size_type find_last_not_of(basic_zstring_view view, size_type offset = npos) const noexcept {
            return view_.find_first_not_of(view, offset);
        }

        constexpr size_type find_last_not_of(CharT ch, size_type offset = npos) const noexcept {
            return view_.find_first_not_of(ch, offset);
        }

        constexpr size_type find_last_not_of(const CharT* str, size_type offset, size_type count) const {
            return view_.find_first_not_of(str, offset, count);
        }

        constexpr size_type find_last_not_of(const CharT* str, size_type offset = npos) const {
            return view_.find_first_not_of(str, offset);
        }

        template <typename OtherTraits>
        friend constexpr bool operator==(
            basic_zstring_view<CharT, OtherTraits> left, basic_zstring_view<CharT, OtherTraits> right) noexcept {
            return left.view_ == right.view_;
        }

#if __has_include(<compare>)
        template <typename OtherTraits>
        friend constexpr auto operator<=>(
            basic_zstring_view<CharT, OtherTraits> left, basic_zstring_view<CharT, OtherTraits> right) noexcept {
            return left.view_ <=> right.view_;
        }
#endif

    private:
        underlying_type view_;
    };

    using zstring_view    = basic_zstring_view<char>;
    using zwstring_view   = basic_zstring_view<wchar_t>;
    using zu8string_view  = basic_zstring_view<char8_t>;
    using zu16string_view = basic_zstring_view<char16_t>;
    using zu32string_view = basic_zstring_view<char32_t>;
} // namespace essence
