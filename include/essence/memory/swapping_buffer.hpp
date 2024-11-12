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

#include "../scope.hpp"

#include <span>

namespace essence::memory {
    /**
     * @brief A general state machine for foreground and background buffer swapping.
     * @tparam T The type of element of the buffer.
     */
    template <typename T>
    class swapping_buffer {
    public:
        /**
         * @brief Creates an instance.
         */
        swapping_buffer() noexcept : swapping_{} {}

        /**
         * @brief Creates an instance.
         * @param swapped_out The output buffer when the state is swapped.
         * @param unswapped_out The output buffer when the state is unswapped.
         */
        swapping_buffer(std::span<T> swapped_out, std::span<T> unswapped_out) noexcept
            : swapping_{}, swapped_out_{swapped_out}, unswapped_out_{unswapped_out}, current_out_{unswapped_out} {}

        /**
         */
        void reset(std::span<const T> input) noexcept {
            current_in_  = input;
            current_out_ = unswapped_out_;
            swapping_    = false;
        }

        /**
         * @brief Sets the outputs.
         * @param swapped_out The output buffer when the state is swapped.
         * @param unswapped_out The output buffer when the state is unswapped.
         * @param input The input buffer.
         */
        void set_out(std::span<T> swapped_out, std::span<T> unswapped_out, std::span<const T> input = {}) noexcept {
            swapped_out_   = swapped_out;
            unswapped_out_ = unswapped_out;
            reset(input);
        }

        /**
         * @brief Set temporary outputs which is scoped in the current context.
         *        The output buffers will be restored when the return RAII token is out of scope.
         * @param swapped_out The output buffer when the state is swapped.
         * @param unswapped_out The output buffer when the state is unswapped.
         * @param input The input buffer.
         * @return The RAII token.
         *         RAII Token。
         */
        auto set_temporary_out(
            std::span<T> swapped_out, std::span<T> unswapped_out, std::span<const T> input = {}) noexcept {
            const auto entry_handler = [&] { set_out(swapped_out, unswapped_out, input); };
            const auto exit_handler  = [this, swapped = swapped_out_, unswapped = unswapped_out_] {
                set_out(swapped, unswapped);
                reset({});
            };

            return scope_exit{entry_handler, exit_handler};
        };

        /**
         * @brief Swaps the buffers.
         */
        void swap() noexcept {
            current_in_ = current_out_;

            if ((swapping_ = !swapping_)) {
                current_out_ = swapped_out_;
            } else {
                current_out_ = unswapped_out_;
            }
        }

        /**
         * @brief Gets the current input buffer.
         * @return The input buffer.
         */
        [[nodiscard]] std::span<const T> in() const noexcept {
            return current_in_;
        }

        /**
         * @brief Gets the lvalue reference to the current output buffer, which is replaceable with a subspan.
         * @return The lvalue reference to the output buffer.
         */
        [[nodiscard]] std::span<T>& out() noexcept {
            return current_out_;
        }

        /**
         * @brief Gets the original output buffer.
         * @return The output buffer.
         */
        [[nodiscard]] std::span<T> original_out() const noexcept {
            return swapping_ ? swapped_out_ : unswapped_out_;
        }

    private:
        bool swapping_;
        std::span<T> swapped_out_;
        std::span<T> unswapped_out_;
        std::span<const T> current_in_;
        std::span<T> current_out_;
    };
} // namespace essence::memory
