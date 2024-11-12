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

#include <concepts>
#include <cstddef>
#include <memory>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::io::abstract {
    /**
     * @brief Hints the type of bitstream.
     */
    class bitstream_type_hint {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, bitstream_type_hint>)
        explicit bitstream_type_hint(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Gets the name of the type.
         * @return The name of the type.
         */
        [[nodiscard]] abi::string name() const {
            return wrapper_->name();
        }

        /**
         * @brief Gets the file extensions of the type.
         * @return The file extensions.
         */
        [[nodiscard]] std::span<const abi::string> file_extensions() const {
            return wrapper_->file_extensions();
        }

        /**
         * @brief Gets the size of the leading signature.
         * @return The size of the leading signature.
         */
        [[nodiscard]] std::size_t leading_signature_size() const {
            return wrapper_->leading_signature_size();
        }

        /**
         * @brief Gets the leading byte signature of the type.
         * @return The byte signature.
         */
        [[nodiscard]] std::span<const std::byte> leading_signature() const {
            return wrapper_->leading_signature();
        }

        /**
         * @brief Gets the leading byte signature of the type as a string.
         * @return The leading byte signature as a string.
         */
        [[nodiscard]] std::string_view leading_signature_str() const {
            return wrapper_->leading_signature_str();
        }

        /**
         * @brief Gets the size of the trailing signature.
         * @return The size of the trailing signature.
         */
        [[nodiscard]] std::size_t trailing_signature_size() const {
            return wrapper_->trailing_signature_size();
        }

        /**
         * @brief Gets the trailing byte signature of the type.
         * @return The trailing byte signature.
         */
        [[nodiscard]] std::span<const std::byte> trailing_signature() const {
            return wrapper_->trailing_signature();
        }

        /**
         * @brief Gets the trailing byte signature of the type as a string.
         * @return The trailing byte signature as a string.
         */
        [[nodiscard]] std::string_view trailing_signature_str() const {
            return wrapper_->trailing_signature_str();
        }

    private:
        struct base {
            virtual ~base()                                         = default;
            virtual abi::string name()                              = 0;
            virtual std::span<const abi::string> file_extensions()  = 0;
            virtual std::size_t leading_signature_size()            = 0;
            virtual std::span<const std::byte> leading_signature()  = 0;
            virtual std::string_view leading_signature_str()        = 0;
            virtual std::size_t trailing_signature_size()           = 0;
            virtual std::span<const std::byte> trailing_signature() = 0;
            virtual std::string_view trailing_signature_str()       = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            abi::string name() override {
                return value_.name();
            }

            std::span<const abi::string> file_extensions() override {
                return value_.file_extensions();
            }

            std::size_t leading_signature_size() override {
                return value_.leading_signature_size();
            }

            std::span<const std::byte> leading_signature() override {
                return value_.leading_signature();
            }

            std::string_view leading_signature_str() override {
                return value_.leading_signature_str();
            }

            std::size_t trailing_signature_size() override {
                return value_.trailing_signature_size();
            }

            std::span<const std::byte> trailing_signature() override {
                return value_.trailing_signature();
            }

            std::string_view trailing_signature_str() override {
                return value_.trailing_signature_str();
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::io::abstract
