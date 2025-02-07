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

export module essence.basic:flags;
import std;

export namespace essence {
    template <typename T>
        requires std::is_enum_v<T>
    class flags {
    public:
        constexpr flags() noexcept : value_{} {}

        constexpr flags(T value) noexcept // NOLINT(*-explicit-constructor)
            : value_{static_cast<std::underlying_type_t<T>>(value)} {}

        constexpr operator T() const noexcept { // NOLINT(*-explicit-constructor)
            return T{value_};
        }

        constexpr bool operator!() const noexcept {
            return value_ == 0;
        }

        constexpr explicit operator bool() const noexcept {
            return value_ != 0;
        }

        constexpr bool operator==(const flags&) const noexcept  = default;
        constexpr auto operator<=>(const flags&) const noexcept = default;

        constexpr flags& operator&=(const flags& right) noexcept {
            value_ &= right.value_;

            return *this;
        }

        constexpr flags& operator|=(const flags& right) noexcept {
            value_ |= right.value_;

            return *this;
        }

        friend constexpr flags operator&(const flags& left, const flags& right) noexcept {
            return T{left.value_ & right.value_};
        }

        friend constexpr flags operator|(const flags& left, const flags& right) noexcept {
            return T{left.value_ | right.value_};
        }

    private:
        std::underlying_type_t<T> value_;
    };

    template <typename T>
    constexpr flags<T> operator&(T left, T right) noexcept
        requires std::is_enum_v<T>
    {
        return flags{left} & flags{right};
    }

    template <typename T>
    constexpr flags<T> operator|(T left, T right) noexcept
        requires std::is_enum_v<T>
    {
        return flags{left} | flags{right};
    }

    template <typename T>
        requires std::is_enum_v<T>
    flags(T) -> flags<T>;
} // namespace essence
