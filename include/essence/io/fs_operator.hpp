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

#include "../compat.hpp"
#include "abstract/virtual_fs_operator.hpp"
#include "cmrc_fs_operator.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace essence::io {
    /**
     * @brief Makes a CMRC file system operator.
     * @tparam T The concrete type of the CMRC file system object.
     * @param fs The CMRC file system object.
     * @return The CMRC file system operator.
     */
    template <typename T>
        requires(std::is_class_v<std::decay_t<T>> && std::move_constructible<std::decay_t<T>>)
    abstract::virtual_fs_operator make_cmrc_fs_operator(T&& fs) {
        return abstract::virtual_fs_operator{cmrc_fs_operator{std::forward<T>(fs)}};
    }

    /**
     * @brief Gets the native file system operator.
     * @return The native file system operator.
     */
    ES_API(CPPESSENCE) const abstract::virtual_fs_operator& get_native_fs_operator();
}
