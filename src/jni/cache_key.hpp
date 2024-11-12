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

#include "hashing.hpp"

#include <compare>
#include <concepts>
#include <functional>
#include <type_traits>

#include <jni.h>

namespace essence::jni {
    struct cache_key {
        std::int32_t class_id;
        std::int32_t field_id;
        std::int32_t method_id;

        constexpr auto operator<=>(const cache_key&) const noexcept = default;
    };
} // namespace essence::jni

namespace essence::jni {
    template <auto Enum>
        requires std::is_enum_v<decltype(Enum)>
    inline constexpr auto arg_enum_v = static_cast<std::underlying_type_t<decltype(Enum)>>(Enum);

    template <typename T>
    constexpr auto make_cache_key(std::int32_t key) noexcept {
        if constexpr (std::same_as<T, jclass>) {
            return cache_key{key};
        } else if constexpr (std::same_as<T, jfieldID>) {
            return cache_key{0, key};
        } else if constexpr (std::same_as<T, jmethodID>) {
            return cache_key{0, 0, key};
        } else {
            return cache_key{};
        }
    }
} // namespace essence::jni

template <>
struct std::hash<essence::jni::cache_key> {
    auto operator()(const essence::jni::cache_key& item) const noexcept {
        return essence::hash_arbitrary(item.class_id, item.field_id, item.method_id);
    }
}; // namespace std
