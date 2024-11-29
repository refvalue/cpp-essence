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

#include "abi/string.hpp"
#include "abi/vector.hpp"
#include "compat.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

namespace essence {
    inline constexpr std::int32_t default_nested_exception_indent = 4;

    class aggregate_error : public std::runtime_error {
    public:
        using value_type       = std::exception_ptr;
        using iterator         = const value_type*;
        using reverse_iterator = std::reverse_iterator<iterator>;

        [[nodiscard]] ES_API(CPPESSENCE) iterator begin() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) iterator end() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) reverse_iterator rbegin() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) reverse_iterator rend() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) bool empty() const noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) std::size_t size() const noexcept;
        [[noreturn]] ES_API(CPPESSENCE) static void flatten_and_throw(
            const std::exception_ptr& root, std::int32_t indent = default_nested_exception_indent);

        template <typename E>
        [[noreturn]] ES_API(CPPESSENCE) static void throw_nested(
            E&& ex, std::int32_t indent = default_nested_exception_indent) try {
            std::throw_with_nested(std::forward<E>(ex));
        } catch (const std::exception&) {
            flatten_and_throw(std::current_exception(), indent);
        }

        template <typename E, bool Reverse = false>
        std::optional<E> extract() const {
            auto iter = [this] {
                if constexpr (Reverse) {
                    return rbegin();
                } else {
                    return begin();
                }
            }();
            auto iter_end = [this] {
                if constexpr (Reverse) {
                    return rend();
                } else {
                    return end();
                }
            }();

            for (; iter != iter_end; ++iter) {
                try {
                    std::rethrow_exception(*iter);
                } catch (const std::exception& ex) {
                    if (auto concrete = dynamic_cast<const E*>(&ex)) {
                        return *concrete;
                    }
                }
            }

            return std::nullopt;
        }

    private:
        aggregate_error(abi::vector<value_type>&& exceptions, std::string_view what);

        abi::vector<value_type> exceptions_;
    };

    ES_API(CPPESSENCE)
    abi::string serialize_nested_exceptions(
        const std::exception_ptr& root, std::int32_t indent = default_nested_exception_indent);
} // namespace essence
