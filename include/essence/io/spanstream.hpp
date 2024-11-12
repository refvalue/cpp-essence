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

#include "spanbuf.hpp"

#include <concepts>
#include <istream>
#include <ranges>
#include <span>
#include <type_traits>

namespace essence::io {
    template <typename BaseStream, std::ios_base::openmode Mode>
    class basic_spanstream_impl : public BaseStream {
    public:
        explicit basic_spanstream_impl(
            std::span<typename BaseStream::char_type> buffer, std::ios_base::openmode mode = Mode)
            : BaseStream{&spanbuf_}, spanbuf_{buffer, (mode | Mode)} {}

        void span(std::span<typename BaseStream::char_type> buffer) noexcept {
            this->spanbuf_.span(buffer);
        }

    protected:
        basic_spanbuf<typename BaseStream::char_type, typename BaseStream::traits_type> spanbuf_;
    };

    template <typename CharT, typename Traits = std::char_traits<CharT>>
    struct basic_ispanstream : basic_spanstream_impl<std::basic_istream<CharT, Traits>, std::ios_base::in> {
        using base_type = basic_spanstream_impl<std::basic_istream<CharT, Traits>, std::ios_base::in>;
        using base_type::basic_spanstream_impl;

        template <std::ranges::borrowed_range Range>
            requires(
                std::convertible_to<Range, std::span<const CharT>> && !std::convertible_to<Range, std::span<CharT>>)
        explicit basic_ispanstream(Range&& range, std::ios_base::openmode mode = std::ios_base::in)
            : base_type{std::span{const_cast<CharT*>(std::ranges::data(range)), std::ranges::size(range)}, mode} {}

        [[nodiscard]] std::span<const typename base_type::char_type> span() const noexcept {
            return this->spanbuf_.span();
        }

        template <std::ranges::borrowed_range Range>
            requires(
                std::convertible_to<Range, std::span<const CharT>> && !std::convertible_to<Range, std::span<CharT>>)
        void span(Range&& range) noexcept {
            return this->spanbuf_.span(
                std::span{const_cast<CharT*>(std::ranges::data(range)), std::ranges::size(range)});
        }
    };

    template <typename CharT, typename Traits = std::char_traits<CharT>>
    struct basic_ospanstream : basic_spanstream_impl<std::basic_ostream<CharT, Traits>, std::ios_base::out> {
        using base_type = basic_spanstream_impl<std::basic_ostream<CharT, Traits>, std::ios_base::out>;
        using base_type::basic_spanstream_impl;

        [[nodiscard]] std::span<typename base_type::char_type> span() const noexcept {
            return this->spanbuf_.span();
        }
    };

    template <typename CharT, typename Traits = std::char_traits<CharT>>
    struct basic_spanstream
        : basic_spanstream_impl<std::basic_iostream<CharT, Traits>, std::ios_base::in | std::ios_base::out> {
        using base_type =
            basic_spanstream_impl<std::basic_iostream<CharT, Traits>, std::ios_base::in | std::ios_base::out>;
        using base_type::basic_spanstream_impl;

        [[nodiscard]] std::span<typename base_type::char_type> span() const noexcept {
            return this->spanbuf_.span();
        }
    };

    using ispanstream = basic_ispanstream<char>;
    using ospanstream = basic_ospanstream<char>;
    using spanstream  = basic_spanstream<char>;
} // namespace essence::io
