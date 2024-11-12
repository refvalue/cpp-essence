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

#include "../../abi/json.hpp"
#include "../../abi/vector.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::globalization::abstract {
    /**
     * @brief A compiler to translate globalized texts into particular binary sequences.
     */
    class compiler {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, compiler>)
        explicit compiler(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Gets the version of the compiler.
         * @return The version.
         */
        [[nodiscard]] std::uint32_t version() const {
            return wrapper_->version();
        }

        /**
         * @brief Compiles a JSON value into a language file.
         * @param json The JSON value.
         * @param path The path of the file.
         */
        void to_file(const abi::json& json, std::string_view path) const {
            wrapper_->to_file(json, path);
        }

        /**
         * @brief Compiles a JSON value into a byte array.
         * @param json The JSON value.
         * @return The byte array.
         */
        [[nodiscard]] abi::vector<std::byte> to_bytes(const abi::json& json) const {
            return wrapper_->to_bytes(json);
        }

        /**
         * @brief Compiles a JSON value into a base64 string.
         * @param json The JSON value.
         * @return The base64 string.
         */
        [[nodiscard]] abi::string to_base64(const abi::json& json) const {
            return wrapper_->to_base64(json);
        }

    private:
        struct base {
            virtual ~base()                                                    = default;
            virtual std::uint32_t version()                                    = 0;
            virtual void to_file(const abi::json& json, std::string_view path) = 0;
            virtual abi::vector<std::byte> to_bytes(const abi::json& json)     = 0;
            virtual abi::string to_base64(const abi::json& json)               = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            std::uint32_t version() override {
                return value_.version();
            }

            void to_file(const abi::json& json, std::string_view path) override {
                value_.to_file(json, path);
            }

            abi::vector<std::byte> to_bytes(const abi::json& json) override {
                return value_.to_bytes(json);
            }

            abi::string to_base64(const abi::json& json) override {
                return value_.to_base64(json);
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::globalization::abstract
