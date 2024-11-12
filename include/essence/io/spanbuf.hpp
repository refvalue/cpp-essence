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

#include <cstdint>
#include <span>
#include <streambuf>

namespace essence::io {
    template <typename CharT, typename Traits = std::char_traits<CharT>>
    class basic_spanbuf : public std::basic_streambuf<CharT, Traits> {
    public:
        using char_type   = CharT;
        using traits_type = Traits;
        using int_type    = typename Traits::int_type;
        using pos_type    = typename Traits::pos_type;
        using off_type    = typename Traits::off_type;

        explicit basic_spanbuf() noexcept : basic_spanbuf{std::ios_base::in | std::ios_base::out} {}

        explicit basic_spanbuf(std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) noexcept
            : basic_spanbuf{std::span<CharT>{}, mode} {}

        explicit basic_spanbuf(
            std::span<CharT> buffer, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) noexcept
            : mode_{mode}, buffer_{buffer} {
            this->span(buffer);
        }

        basic_spanbuf(const basic_spanbuf&) = delete;

        basic_spanbuf(basic_spanbuf&&) noexcept = default;

        basic_spanbuf& operator=(const basic_spanbuf&) = delete;

        basic_spanbuf& operator=(basic_spanbuf&&) noexcept = default;

        std::basic_streambuf<CharT, Traits>* setbuf(CharT* buffer, std::streamsize size) override {
            this->span(std::span{buffer, static_cast<std::size_t>(size)});

            return this;
        }

        [[nodiscard]] std::span<CharT> span() const noexcept {
            return buffer_;
        }

        void span(std::span<CharT> buffer) noexcept {
            buffer_ = buffer;

            if ((mode_ & std::ios_base::in) == std::ios_base::in) {
                this->setg(buffer_.data(), buffer_.data(), buffer_.data() + buffer_.size());
            }

            if ((mode_ & std::ios_base::out) == std::ios_base::out) {
                this->setp(buffer_.data(), buffer_.data() + buffer_.size());
                this->pbump(
                    (mode_ & std::ios_base::ate) == std::ios_base::ate ? static_cast<std::int32_t>(buffer_.size()) : 0);
            }
        }

    protected:
        pos_type seekoff(off_type offset, std::ios_base::seekdir direction, std::ios_base::openmode mode) override {
            switch (direction) {
            case std::ios_base::beg:
                return seekpos(pos_type{offset}, mode);
            case std::ios_base::cur:
                // https://stackoverflow.com/questions/6800811/basic-streambufseekoff-what-should-be-returned-when-ios-basein-ios-baseo?rq=3
                // If both in and out are specified and the mode is std::ios_base::cur, it shall fail.
                if ((mode & (std::ios_base::in | std::ios_base::out)) == (std::ios_base::in | std::ios_base::out)) {
                    break;
                }

                if ((mode & std::ios_base::in) == std::ios_base::in) {
                    return seekpos(this->gptr() + offset - this->eback(), std::ios_base::in);
                }

                if ((mode & std::ios_base::out) == std::ios_base::out) {
                    return seekpos(this->pptr() + offset - this->pbase(), std::ios_base::out);
                }

                break;
            case std::ios_base::end:
                return seekpos(pos_type{static_cast<off_type>(buffer_.size()) + offset}, mode);
            default:
                break;
            }

            return std::basic_streambuf<CharT, Traits>::seekoff(offset, direction, mode);
        }

        pos_type seekpos(pos_type pos, std::ios_base::openmode mode) override {
            // Checks validity.
            if (pos < 0 || pos >= buffer_.size()) {
                return std::basic_streambuf<CharT, Traits>::seekpos(pos, mode);
            }

            if ((mode_ & std::ios_base::in) == std::ios_base::in && (mode & std::ios_base::in) == std::ios_base::in) {
                this->setg(buffer_.data(), buffer_.data() + pos, buffer_.data() + buffer_.size());
            }

            if ((mode_ & std::ios_base::out) == std::ios_base::out
                && (mode & std::ios_base::out) == std::ios_base::out) {
                this->setp(buffer_.data(), buffer_.data() + buffer_.size());
                this->pbump(static_cast<std::int32_t>(pos));
            }

            return pos;
        }

    private:
        std::ios_base::openmode mode_;
        std::span<CharT> buffer_;
    };
} // namespace essence::io
