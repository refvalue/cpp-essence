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

#include <coroutine>
#include <cstddef>
#include <exception>
#include <iterator>
#include <memory>
#include <utility>

namespace essence {
    template <typename T>
    class generator {
    public:
        struct promise_type {
            const T* value{};
            std::exception_ptr exception;

            void* operator new(std::size_t size) {
                return ::operator new(size);
            }

            void operator delete(void* ptr, std::size_t size) {
                ::operator delete(ptr, size);
            }

            generator get_return_object() noexcept {
                return generator{*this};
            }

            [[nodiscard]] static std::suspend_always initial_suspend() noexcept {
                return {};
            }

            [[nodiscard]] static std::suspend_always final_suspend() noexcept {
                return {};
            }

            void unhandled_exception() noexcept {
                exception = std::current_exception();
            }

            std::suspend_always yield_value(const T& from) noexcept {
                value = std::addressof(from);

                return {};
            }

            static void return_void() noexcept {}

            void rethrow_if_exception() const {
                if (exception) {
                    std::rethrow_exception(exception);
                }
            }
        };

        class iterator {
        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type   = std::ptrdiff_t;
            using value_type        = T;
            using reference         = const T&;
            using pointer           = const T*;

            iterator() = default;

            explicit iterator(std::coroutine_handle<promise_type> handle) noexcept : handle_{handle} {}

            iterator& operator++() {
                handle_.resume();

                if (handle_.done()) {
                    std::exchange(handle_, {}).promise().rethrow_if_exception();
                }

                return *this;
            }

            void operator++(int) {
                static_cast<void>(operator++());
            }

            bool operator==(const iterator& right) const noexcept {
                return handle_ == right.handle_;
            }

            bool operator!=(const iterator& right) const noexcept {
                return !(*this == right);
            }

            reference operator*() const noexcept {
                return *handle_.promise().value;
            }

            pointer operator->() const noexcept {
                return handle_.promise()._Value;
            }

        private:
            std::coroutine_handle<promise_type> handle_;
        };

        explicit generator(promise_type& promise) noexcept
            : handle_{std::coroutine_handle<promise_type>::from_promise(promise)} {}

        generator(generator&& other) noexcept : handle_{std::exchange(other.handle_, {})} {}

        ~generator() {
            if (handle_) {
                handle_.destroy();
            }
        }

        generator& operator=(generator&& right) noexcept {
            handle_ = std::exchange(right.handle_, nullptr);

            return *this;
        }

        iterator begin() const {
            if (handle_) {
                handle_.resume();

                if (handle_.done()) {
                    handle_.promise().rethrow_if_exception();

                    return {};
                }
            }

            return iterator{handle_};
        }

        static iterator end() noexcept {
            return {};
        }

    private:
        std::coroutine_handle<promise_type> handle_;
    };
} // namespace essence
