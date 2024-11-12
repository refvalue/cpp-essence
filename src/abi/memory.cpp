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

#include "abi/memory.hpp"

void* es_alloc(std::size_t size) noexcept {
    return ::operator new(size);
}

void* es_aligned_alloc(std::size_t size, std::size_t alignment) noexcept {
    return ::operator new(size, std::align_val_t{alignment});
}

void es_dealloc(void* ptr, std::size_t size) noexcept {
    ::operator delete(ptr, size);
}

void es_aligned_dealloc(void* ptr, std::size_t size, std::size_t alignment) noexcept {
    ::operator delete(ptr, size, std::align_val_t{alignment});
}

namespace essence::abi {
    uniform_allocator_base::uniform_allocator_base() noexcept = default;

    uniform_allocator_base::uniform_allocator_base(const uniform_allocator_base&) noexcept = default;

    uniform_allocator_base::uniform_allocator_base(uniform_allocator_base&&) noexcept = default;

    uniform_allocator_base& uniform_allocator_base::operator=(const uniform_allocator_base&) noexcept = default;

    uniform_allocator_base& uniform_allocator_base::operator=(uniform_allocator_base&&) noexcept = default;

    uniform_allocator_base::~uniform_allocator_base() = default;

    void* uniform_allocator_base::allocate(std::size_t size, std::size_t alignment) noexcept {
        return alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__ ? es_alloc(size) : es_aligned_alloc(size, alignment);
    }

    void uniform_allocator_base::deallocate(void* ptr, std::size_t size, std::size_t alignment) noexcept {
        if (alignment > __STDCPP_DEFAULT_NEW_ALIGNMENT__) {
            es_dealloc(ptr, size);
        } else {
            es_aligned_dealloc(ptr, size, alignment);
        }
    }
} // namespace essence::abi
