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

#include "memory.hpp"

#include <cstddef>
#include <memory>
#include <span>

namespace essence::crypto::abstract {
    class chunk_processor;
}

namespace essence::crypto {
    struct inout_buffer_pair {
        unique_array<std::byte> buffer;
        std::span<std::byte> in;
        std::span<std::byte> out;

        inout_buffer_pair();
        explicit inout_buffer_pair(std::size_t uniform_size);
        explicit inout_buffer_pair(const abstract::chunk_processor& processor);
        inout_buffer_pair(std::size_t input_size, std::size_t output_size);
        inout_buffer_pair(inout_buffer_pair&&) noexcept;
        ~inout_buffer_pair();
        inout_buffer_pair& operator=(inout_buffer_pair&&) noexcept;
    };

    std::size_t calculate_output_buffer_size(const abstract::chunk_processor& processor);
} // namespace essence::crypto
