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
#include <type_traits>

#include <jni.h>

namespace essence::jni {
    template <typename T>
    concept derived_from_jobject =
        std::is_pointer_v<T> && std::derived_from<std::remove_pointer_t<T>, std::remove_pointer_t<jobject>>;

    template <typename T>
    concept derived_from_jarray =
        std::is_pointer_v<T> && std::derived_from<std::remove_pointer_t<T>, std::remove_pointer_t<jarray>>;

    template <typename T>
    concept derived_from_jobject_array =
        std::is_pointer_v<T> && std::derived_from<std::remove_pointer_t<T>, std::remove_pointer_t<jobjectArray>>;

    template <typename T>
    concept primitve_jarray = derived_from_jarray<T> && !derived_from_jobject_array<T>;

    template <typename T>
    concept jni_primitive_type = std::same_as<T, jbyte> || std::same_as<T, jchar> || std::same_as<T, jshort>
                              || std::same_as<T, jint> || std::same_as<T, jlong> || std::same_as<T, jfloat>
                              || std::same_as<T, jdouble> || std::same_as<T, jboolean> || std::same_as<T, std::int32_t>;

    template <typename T>
    concept jni_type = jni_primitive_type<T> || derived_from_jobject<T>;
} // namespace essence::jni
