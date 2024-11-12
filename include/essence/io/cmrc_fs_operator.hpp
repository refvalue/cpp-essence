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

#include "../char8_t_remediation.hpp"
#include "../error_extensions.hpp"
#include "spanstream.hpp"

#include <concepts>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::io {
    template <std::move_constructible T>
        requires std::is_class_v<T>
    class cmrc_fs_operator {
    public:
        template <std::convertible_to<T> U>
            requires(!std::same_as<std::decay_t<U>, cmrc_fs_operator>)
        explicit cmrc_fs_operator(U&& impl) : impl_{std::forward<U>(impl)} {}

        [[nodiscard]] bool exists(std::string_view path) const {
            return impl_.exists(std::string{path});
        }

        [[nodiscard]] bool is_file(std::string_view path) const {
            return impl_.is_file(std::string{path});
        }

        [[nodiscard]] bool is_directory(std::string_view path) const {
            return impl_.is_directory(std::string{path});
        }

        static std::unique_ptr<std::iostream> open(
            [[maybe_unused]] std::string_view path, [[maybe_unused]] std::ios_base::openmode mode) {
            throw source_code_aware_runtime_error{
                U8("This CMRC file is read-only and cannot be opened as std::iostream.")};
        }

        [[nodiscard]] std::unique_ptr<std::istream> open_read(
            std::string_view path, std::ios_base::openmode mode) const {
            auto file = impl_.open(std::string{path});

            return std::make_unique<ispanstream>(std::span{file.begin(), file.end()}, mode);
        }

        static std::unique_ptr<std::ostream> open_write(
            [[maybe_unused]] std::string_view path, [[maybe_unused]] std::ios_base::openmode mode) {
            throw source_code_aware_runtime_error{
                U8("This CMRC file is read-only and cannot be opened as std::ostream.")};
        }

    private:
        T impl_;
    };

    template <typename T>
    cmrc_fs_operator(T&&) -> cmrc_fs_operator<std::decay_t<T>>;
} // namespace essence::io
