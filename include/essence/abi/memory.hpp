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

#include <cstddef>
#include <limits>
#include <memory>
#include <new>

extern "C" {
ES_API(CPPESSENCE) void* es_alloc(std::size_t size) noexcept;
ES_API(CPPESSENCE) void* es_aligned_alloc(std::size_t size, std::size_t alignment) noexcept;
ES_API(CPPESSENCE) void es_dealloc(void* ptr, std::size_t size) noexcept;
ES_API(CPPESSENCE) void es_aligned_dealloc(void* ptr, std::size_t size, std::size_t alignment) noexcept;
}

namespace essence::abi {
    struct uniform_allocator_base {
        ES_API(CPPESSENCE) uniform_allocator_base() noexcept;
        ES_API(CPPESSENCE) uniform_allocator_base(const uniform_allocator_base&) noexcept;
        ES_API(CPPESSENCE) uniform_allocator_base(uniform_allocator_base&&) noexcept;
        ES_API(CPPESSENCE) ~uniform_allocator_base();
        ES_API(CPPESSENCE) uniform_allocator_base& operator=(const uniform_allocator_base&) noexcept;
        ES_API(CPPESSENCE) uniform_allocator_base& operator=(uniform_allocator_base&&) noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) static void* allocate(std::size_t size, std::size_t alignment) noexcept;
        ES_API(CPPESSENCE) static void deallocate(void* ptr, std::size_t size, std::size_t alignment) noexcept;
    };

    template <typename T>
    struct uniform_allocator : private uniform_allocator_base {
        using value_type                             = T;
        using size_type                              = std::size_t;
        using difference_type                        = std::ptrdiff_t;
        using propagate_on_container_move_assignment = std::true_type;

        constexpr uniform_allocator() noexcept = default;

        constexpr uniform_allocator(const uniform_allocator&) noexcept = default;

        constexpr uniform_allocator(uniform_allocator&&) noexcept = default;

        template <typename U>
        constexpr uniform_allocator(const uniform_allocator<U>&) noexcept {}

        ~uniform_allocator() = default;

        uniform_allocator& operator=(const uniform_allocator&) noexcept = default;

        uniform_allocator& operator=(uniform_allocator&&) noexcept = default;

        [[nodiscard]] static T* allocate(std::size_t size) {
            if (size > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
                throw std::bad_array_new_length{};
            }

            auto uninitialized_ptr = uniform_allocator_base::allocate(size * sizeof(T), alignof(T));

            if (uninitialized_ptr == nullptr) {
                throw std::bad_alloc{};
            }

            return static_cast<T*>(uninitialized_ptr);
        }

        static void deallocate(T* ptr, std::size_t size) noexcept {
            uniform_allocator_base::deallocate(ptr, size * sizeof(T), alignof(T));
        }

        constexpr bool operator==(const uniform_allocator& right) const noexcept {
            return true;
        }

        constexpr bool operator!=(const uniform_allocator& right) const noexcept {
            return false;
        }
    };
} // namespace essence::abi
