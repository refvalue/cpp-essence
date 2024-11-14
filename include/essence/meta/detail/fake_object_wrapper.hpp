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

namespace essence::meta::detail {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 7631)
#elif defined(__llvm__) && defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wundefined-var-template"
#pragma clang diagnostic ignored "-Wundefined-internal"
#elif defined(__GNUC__)
#pragma GCC system_header
#endif

    template <typename T>
    struct fake_object_wrapper {
        using type = T;

        const T value;
        static const fake_object_wrapper<T> shadow;
    };

    template <typename T>
    fake_object_wrapper(T) -> fake_object_wrapper<T>;

    template <typename T>
    consteval const T& make_fake_object_wrapper() noexcept {
        return fake_object_wrapper<T>::shadow.value;
    }

    template <typename T>
    consteval auto make_fake_object_wrapper(const T& obj) noexcept {
        return fake_object_wrapper{obj};
    }

#ifdef _MSC_VER
#pragma warning(pop)
#elif defined(__llvm__) && defined(__clang__)
#pragma clang diagnostic pop
#endif
} // namespace essence::meta::detail
