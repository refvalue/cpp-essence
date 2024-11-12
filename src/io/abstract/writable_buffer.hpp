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
#include <memory>
#include <ranges>

namespace essence::io::abstract {
    class writable_buffer {
    public:
        template <std::ranges::contiguous_range T>
        explicit writable_buffer(T& ref) : wrapper_{std::make_unique<wrapper<T>>(ref)} {}

        void resize(std::size_t new_size) const {
            wrapper_->resize(new_size);
        }

        [[nodiscard]] std::byte* data() const {
            return wrapper_->data();
        }

        [[nodiscard]] std::size_t size_bytes() const {
            return wrapper_->size_bytes();
        }

        void shrink_to_fit() const {
            wrapper_->shrink_to_fit();
        }

    private:
        struct base {
            virtual ~base()                           = default;
            virtual void resize(std::size_t new_size) = 0;
            virtual std::byte* data()                 = 0;
            virtual std::size_t size_bytes()          = 0;
            virtual void shrink_to_fit()              = 0;
        };

        template <std::ranges::contiguous_range T>
        class wrapper final : public base {
        public:
            explicit wrapper(T& ref) : ref_{ref} {}

            void resize(std::size_t new_size) override {
                ref_.resize(new_size);
            }

            std::byte* data() override {
                return reinterpret_cast<std::byte*>(ref_.data());
            }

            std::size_t size_bytes() override {
                return ref_.size() * sizeof(std::ranges::range_value_t<T>);
            }

            void shrink_to_fit() override {
                ref_.shrink_to_fit();
            }

        private:
            T& ref_;
        };

        std::unique_ptr<base> wrapper_;
    };
} // namespace essence::io::abstract
