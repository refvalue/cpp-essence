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

#include "../../abi/string.hpp"
#include "../../rational.hpp"

#include <concepts>
#include <cstddef>
#include <memory>
#include <span>
#include <type_traits>
#include <utility>

namespace essence::crypto::abstract {
    /**
     * @brief Provides a uniform interface for processing crypto chunks.
     * @remark This interface always keeps uniquely referenced.
     */
    class chunk_processor {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, chunk_processor>)
        explicit chunk_processor(T&& value) : wrapper_{std::make_unique<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Indicates whether this is a forward transformer.
         * @return True if this is a forward transformer; false if an inverse transformer.
         */
        [[nodiscard]] bool transformer() const {
            return wrapper_->transformer();
        }

        /**
         * @brief Gets the name of the cipher.
         * @return The name of the cipher.
         */
        [[nodiscard]] abi::string cipher_name() const {
            return wrapper_->cipher_name();
        }

        /**
         * @brief Gets the size of the input buffer.
         * @return The size of the input buffer.
         */
        [[nodiscard]] std::size_t buffer_size() const {
            return wrapper_->buffer_size();
        }

        /**
         * @brief Gets the extra size of the output buffer.
         * @return The extra size of the output buffer.
         */
        [[nodiscard]] std::size_t extra_size() const {
            return wrapper_->extra_size();
        }

        /**
         * @brief Gets the factor to be multiplied to the input buffer size and the result is the size of the output
         *        buffer.
         * @return The factor described as a rational number.
         */
        [[nodiscard]] rational size_factor() const {
            return wrapper_->size_factor();
        }

        /**
         * @brief Initializes the processor.
         */
        void init() const {
            wrapper_->init();
        }

        /**
         * @brief Processes a memory chunk.
         * @param input The input chunk.
         * @param output The output chunk, which may be replaced with a subspan of the given span.
         */
        void update(std::span<const std::byte> input, std::span<std::byte>& output) const {
            wrapper_->update(input, output);
        }

        /**
         * @brief Gets the final chunk.
         * @param output The output chunk, which may be replaced with a subspan of the given span.
         */
        void finalize(std::span<std::byte>& output) const {
            wrapper_->finalize(output);
        }

    private:
        struct base {
            virtual ~base()                                                                     = default;
            virtual bool transformer()                                                          = 0;
            virtual abi::string cipher_name()                                                   = 0;
            virtual std::size_t buffer_size()                                                   = 0;
            virtual std::size_t extra_size()                                                    = 0;
            virtual rational size_factor()                                                      = 0;
            virtual void init()                                                                 = 0;
            virtual void update(std::span<const std::byte> input, std::span<std::byte>& output) = 0;
            virtual void finalize(std::span<std::byte>& output)                                 = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            bool transformer() override {
                return value_.transformer();
            }

            abi::string cipher_name() override {
                return value_.cipher_name();
            }

            std::size_t buffer_size() override {
                return value_.buffer_size();
            }

            std::size_t extra_size() override {
                return value_.extra_size();
            }

            rational size_factor() override {
                return value_.size_factor();
            }

            void init() override {
                value_.init();
            }

            void update(std::span<const std::byte> input, std::span<std::byte>& output) override {
                value_.update(input, output);
            }

            void finalize(std::span<std::byte>& output) override {
                value_.finalize(output);
            }

        private:
            T value_;
        };

        std::unique_ptr<base> wrapper_;
    };
} // namespace essence::crypto::abstract
