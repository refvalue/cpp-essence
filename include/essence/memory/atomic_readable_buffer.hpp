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

#include <algorithm>
#include <array>
#include <atomic>
#include <concepts>
#include <cstddef>
#include <memory>
#include <memory_resource>
#include <ranges>
#include <utility>

#if __has_include(<version> )
#include <version>
#endif

namespace essence {
    /**
     * @brief A safe buffer that can be atomically replaced and read.
     * @tparam T The element type of the buffer.
     */
    template <std::default_initializable T>
    class atomic_readable_buffer {
    public:
        using allocator_traits_type = std::allocator_traits<std::pmr::polymorphic_allocator<T>>;

        /**
         * @brief Constructs the object with a PMR resource.
         * @param resource The PMR resource.
         */
        explicit atomic_readable_buffer(std::pmr::memory_resource* resource = std::pmr::new_delete_resource())
            : size_{}, allocator_{resource ? resource : std::pmr::new_delete_resource()} {
            reset();
        }

        /**
         * @brief Resets the buffer.
         */
        void reset() {
            update(std::array<T, 0>{});
        }

        /**
         * @brief Updated the entire buffer.
         * @tparam Range The type of the input range.
         * @param range The input range.
         */
        template <std::ranges::forward_range Range>
            requires(std::convertible_to<std::ranges::range_value_t<Range>, T> && std::ranges::sized_range<Range>)
        void update(Range&& range) {
            auto size = std::ranges::size(range);

            auto storage = allocator_traits_type::allocate(allocator_, size);

            for (auto ptr = storage, ptr_end = storage + size; ptr < ptr_end; ++ptr) {
                std::construct_at(ptr);
            }

            std::shared_ptr<T> buffer{storage, [size, allocator = allocator_](T* inner) mutable {
                                          std::destroy(inner, inner + size);
                                          allocator_traits_type::deallocate(allocator, inner, size);
                                      }};

            std::ranges::copy(range, buffer.get());
            size_ = size;

            std::atomic_store_explicit(&buffer_, buffer, std::memory_order::release);
        }

        /**
         * @brief Retrieves the current buffer.
         * @return The current buffer.
         */
        std::pair<std::shared_ptr<T>, std::size_t> get() const {
            auto buffer = std::atomic_load_explicit(&buffer_, std::memory_order::acquire);

            return {buffer, size_};
        }

    private:
        std::size_t size_;
        std::pmr::polymorphic_allocator<T> allocator_;

#if __cpp_lib_atomic_shared_ptr >= 201711L
        std::atomic<std::shared_ptr<T>> buffer_;
#else
        std::shared_ptr<T> buffer_;
#endif
    };
} // namespace essence
