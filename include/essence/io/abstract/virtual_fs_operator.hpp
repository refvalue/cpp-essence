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

#include <concepts>
#include <istream>
#include <memory>
#include <ostream>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::io::abstract {
    /**
     * @brief Provides an ability to operator files on a virtual file system defined by the user.
     */
    class virtual_fs_operator {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, virtual_fs_operator>)
        explicit virtual_fs_operator(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Checks whether a file exists.
         * @param path The path of the file.
         * @return True if the file exists; otherwise false.
         */
        [[nodiscard]] bool exists(std::string_view path) const {
            return wrapper_->exists(path);
        }

        /**
         * @brief Checks whether a path names a regular file.
         * @param path The path.
         * @return True if the path names a regular file; otherwise false.
         */
        [[nodiscard]] bool is_file(std::string_view path) const {
            return wrapper_->is_file(path);
        }

        /**
         * @brief Checks whether a path names a directory.
         * @param path The path.
         * @return True if the path names a directory; otherwise false.
         */
        [[nodiscard]] bool is_directory(std::string_view path) const {
            return wrapper_->is_directory(path);
        }

        /**
         * @brief Opens a file in read-write mode.
         * @param path The path of the file.
         * @param mode The open mode.
         * @return A std::iostream to read or write the file.
         */
        [[nodiscard]] std::unique_ptr<std::iostream> open(
            std::string_view path, std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) const {
            return wrapper_->open(path, mode);
        }

        /**
         * @brief Opens a file in read-only mode.
         * @param path The path of the file.
         * @param mode The open mode.
         * @return A std::istream to read the file.
         */
        [[nodiscard]] std::unique_ptr<std::istream> open_read(
            std::string_view path, std::ios_base::openmode mode = std::ios_base::in) const {
            return wrapper_->open_read(path, mode);
        }

        /**
         * @brief Opens a file in write-only mode.
         * @param path The path of the file.
         * @param mode The open mode.
         * @return A std::istream to write the file.
         */
        [[nodiscard]] std::unique_ptr<std::ostream> open_write(
            std::string_view path, std::ios_base::openmode mode = std::ios_base::out) const {
            return wrapper_->open_write(path, mode);
        }

    private:
        struct base {
            virtual ~base()                                                                                  = default;
            virtual bool exists(std::string_view path)                                                       = 0;
            virtual bool is_file(std::string_view path)                                                      = 0;
            virtual bool is_directory(std::string_view path)                                                 = 0;
            virtual std::unique_ptr<std::iostream> open(std::string_view path, std::ios_base::openmode mode) = 0;
            virtual std::unique_ptr<std::istream> open_read(std::string_view path, std::ios_base::openmode mode)  = 0;
            virtual std::unique_ptr<std::ostream> open_write(std::string_view path, std::ios_base::openmode mode) = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            bool exists(std::string_view path) override {
                return value_.exists(path);
            }

            bool is_file(std::string_view path) override {
                return value_.is_file(path);
            }

            bool is_directory(std::string_view path) override {
                return value_.is_directory(path);
            }

            std::unique_ptr<std::iostream> open(std::string_view path, std::ios_base::openmode mode) override {
                return value_.open(path, mode);
            }

            std::unique_ptr<std::istream> open_read(std::string_view path, std::ios_base::openmode mode) override {
                return value_.open_read(path, mode);
            }

            std::unique_ptr<std::ostream> open_write(std::string_view path, std::ios_base::openmode mode) override {
                return value_.open_write(path, mode);
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::io::abstract
