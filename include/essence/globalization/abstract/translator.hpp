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
#include "../../io/abstract/virtual_fs_operator.hpp"

#include <concepts>
#include <cstdint>
#include <memory>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::globalization::abstract {
    /**
     * @brief A translator for retrieving texts in multi-languages.
     */
    class translator {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, translator>)
        explicit translator(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Gets the version of the translator.
         * @return The version.
         */
        [[nodiscard]] std::uint32_t version() const {
            return wrapper_->version();
        }

        /**
         * @brief Gets the underlying virtual filesystem operator.
         * @return The virtual filesystem operator.
         */
        [[nodiscard]] io::abstract::virtual_fs_operator virtual_fs() const {
            return wrapper_->virtual_fs();
        }

        /**
         * @brief Sets the underlying virtual filesystem operator.
         * @param fs_operator The virtual filesystem operator.
         */
        void set_virtual_fs(io::abstract::virtual_fs_operator fs_operator) const {
            wrapper_->set_virtual_fs(std::move(fs_operator));
        }

        /**
         * @brief Gets the working directory.
         * @return The working directory.
         */
        [[nodiscard]] abi::string working_directory() const {
            return wrapper_->working_directory();
        }

        /**
         * @brief Sets the working directory.
         * @param directory The working directory.
         */
        void set_working_directory(std::string_view directory) const {
            wrapper_->set_working_directory(directory);
        }

        /**
         * @brief Gets the current language.
         * @return The current language.
         */
        [[nodiscard]] abi::string language() const {
            return wrapper_->language();
        }

        /**
         * @brief Sets the current language.
         * @param name The current language.
         */
        void set_language(std::string_view name) const {
            wrapper_->set_language(name);
        }

        /**
         * @brief Gets a text in the current language.
         * @param name The unique name of the text.
         * @return The text or the name if not exists.
         */
        [[nodiscard]] abi::string get_text(std::string_view name) const {
            return wrapper_->get_text(name);
        }

    private:
        struct base {
            virtual ~base()                                                            = default;
            virtual std::uint32_t version()                                            = 0;
            virtual io::abstract::virtual_fs_operator virtual_fs()                     = 0;
            virtual void set_virtual_fs(io::abstract::virtual_fs_operator fs_operator) = 0;
            virtual abi::string working_directory()                                    = 0;
            virtual void set_working_directory(std::string_view directory)             = 0;
            virtual abi::string language()                                             = 0;
            virtual void set_language(std::string_view name)                           = 0;
            virtual abi::string get_text(std::string_view name)                        = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            std::uint32_t version() override {
                return value_.version();
            }

            io::abstract::virtual_fs_operator virtual_fs() override {
                return value_.virtual_fs();
            }

            void set_virtual_fs(io::abstract::virtual_fs_operator fs_operator) override {
                value_.set_virtual_fs(std::move(fs_operator));
            }

            abi::string working_directory() override {
                return value_.working_directory();
            }

            void set_working_directory(std::string_view directory) override {
                value_.set_working_directory(directory);
            }

            abi::string language() override {
                return value_.language();
            }

            void set_language(std::string_view name) override {
                value_.set_language(name);
            }

            abi::string get_text(std::string_view name) override {
                return value_.get_text(name);
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::globalization::abstract
