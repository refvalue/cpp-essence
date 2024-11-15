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
#include <cstdint>
#include <limits>

#ifdef _MSC_VER
#include <immintrin.h>
#endif

namespace essence {
#ifdef _MSC_VER
    template <std::unsigned_integral T>
    constexpr T ceil_power_of_two(T number) noexcept {
        if constexpr (sizeof(T) <= sizeof(std::uint32_t)) {
            return number == 1U ? 1U
                                : static_cast<T>(1U << (std::numeric_limits<std::uint32_t>::digits
                                                        - _lzcnt_u32(static_cast<std::uint32_t>(number) - 1)));
        } else {
            return number == 1U ? 1ULL
                                : static_cast<T>(1ULL << (std::numeric_limits<std::uint64_t>::digits
                                                          - _lzcnt_u64(static_cast<std::uint64_t>(number) - 1)));
        }
    }
#else
    template <std::unsigned_integral T>
    constexpr T ceil_power_of_two(T number) noexcept {
        if constexpr (sizeof(T) <= sizeof(std::uint32_t)) {
            return number == 1U ? 1U
                                : static_cast<T>(1U << (std::numeric_limits<std::uint32_t>::digits
                                                        - __builtin_clz(static_cast<std::uint32_t>(number) - 1)));
        } else {
            return number == 1U ? 1ULL
                                : static_cast<T>(1ULL << (std::numeric_limits<std::uint64_t>::digits
                                                          - __builtin_clzll(static_cast<std::uint64_t>(number) - 1)));
        }
    }
#endif
} // namespace essence
