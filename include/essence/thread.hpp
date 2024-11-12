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

#include "abi/string.hpp"
#include "compat.hpp"
#include "data_view.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>

#if __cpp_concepts >= 201907L
#include <concepts>
#endif

namespace essence {
    /**
     * @brief Serializes the current thread ID to a string.
     * @return The string.
     */
    ES_API(CPPESSENCE) abi::string serialize_thread_id();

    /**
     * @brief Converts the current thread ID to a decimal.
     * @return The decimal and zero if the source is invalid.
     */
    ES_API(CPPESSENCE) std::uint64_t decimalize_thread_id_or_zero();

    /**
     * @brief Iterates a numeric range in parallel.
     *        The exceptions will be transferred to the calling thread.
     * @param start The start index.
     * @param end The end index.
     * @param thread_count The count of worker threads.
     * @param handler The iteration handler.
     */
    ES_API(CPPESSENCE)
    void parallel_for(std::size_t start, std::size_t end, std::size_t thread_count,
        const std::function<void(std::size_t index, std::size_t thread_index, bool& exit)>& handler);

    /**
     * @brief Iterates a data_view<T> in parallel.
     *        The exceptions will be transferred to the calling thread.
     * @tparam T The element type.
     * @param view The data_view<T>.
     *             data_view<T>。
     * @param thread_count The count of worker threads.
     * @param handler The iteration handler.
     */
#if __cpp_concepts >= 201907L
    template <typename T, std::invocable<T&, std::size_t, std::size_t, bool&> Callable>
#else
    template <typename T, typename Callable>
#endif
    void parallel_for(data_view<T> view, std::size_t thread_count, Callable&& handler) {
        parallel_for(0, view.size(), thread_count, [&](std::size_t index, std::size_t thread_index, bool& exit) {
            std::forward<Callable>(handler)(view[index], index, thread_index, exit);
        });
    }
} // namespace essence
