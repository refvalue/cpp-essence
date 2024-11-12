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

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

#if UINTPTR_MAX == UINT64_MAX
#define ES_PTR64
#elif UINTPTR_MAX == UINT32_MAX
#define ES_PTR32
#else
#define ES_PTR_UNKNOWN
#endif

namespace essence {
#ifdef ES_PTR32
    template <typename T>
    void hash_combine(std::size_t& result, T&& value) noexcept(
        noexcept(std::declval<std::hash<std::decay_t<T>>>()(std::forward<T>(value)))) {
        using pure_type                    = std::decay_t<T>;
        constexpr std::size_t magic_factor = 0x9E3779B9;
        auto hash                          = std::hash<pure_type>{}(std::forward<T>(value));

        result ^= hash + magic_factor + (result << 6) + (result >> 2);
    }
#elif defined(ES_PTR64)
    template <typename T>
    void hash_combine(std::size_t& result, T&& value) noexcept(
        noexcept(std::declval<std::hash<std::decay_t<T>>>()(std::forward<T>(value)))) {
        using pure_type                    = std::decay_t<T>;
        constexpr std::size_t magic_factor = 0xC6A4A7935BD1E995;
        auto hash                          = std::hash<pure_type>{}(std::forward<T>(value));

        hash *= magic_factor;
        hash ^= hash >> 47;
        hash *= magic_factor;

        result ^= hash;
        result *= magic_factor;

        // Completely arbitrary number, to prevent 0's from hashing to 0.
        result += 0xE6546B64;
    }
#else
#error "Unknown pointer length."
#endif

    template <typename... Args>
    std::size_t hash_arbitrary(Args&&... args) noexcept(
        (noexcept(hash_combine(std::declval<std::size_t&>(), std::forward<Args>(args))) && ...)) {
        std::size_t result{};

        return (hash_combine(result, std::forward<Args>(args)), ..., result);
    }
} // namespace essence
