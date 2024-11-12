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
#include <functional>
#include <memory>
#include <new>
#include <span>

namespace essence {
    template <typename T>
    using unique_array = std::unique_ptr<std::span<T>, std::function<void(std::span<T>*)>>;

    /**
     * @brief Creates a unique array as a std::span<T>.
     * @tparam T The element type.
     * @param size The size of the array.
     * @return A std::unique_ptr<std::span<T>> object.
     */
    template <typename T>
    auto make_unique_array(std::size_t size) {
        auto buffer  = std::make_unique_for_overwrite<std::byte[]>(sizeof(std::span<T>) + size * sizeof(T));
        auto span    = new (buffer.get()) std::span{new (buffer.get() + sizeof(std::span<T>)) T[size]{}, size};
        auto deleter = [size](std::span<T>* inner) {
            auto data = std::launder(reinterpret_cast<T*>(reinterpret_cast<std::byte*>(inner) + sizeof(std::span<T>)));

            inner->~span();
            std::ranges::destroy(data, data + size);
            std::default_delete<std::byte[]>{}(reinterpret_cast<std::byte*>(inner));
        };

        buffer.release();

        return unique_array<T>{span, deleter};
    }
} // namespace essence
