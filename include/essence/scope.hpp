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

#include "noncopyable.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

namespace essence {
    template <std::invocable Callable>
        requires(
            std::destructible<Callable> && (std::move_constructible<Callable> || std::copy_constructible<Callable>) )
    class scope_exit : public noncopyable {
    public:
        template <std::invocable ExitCallable>
            requires std::convertible_to<Callable, ExitCallable>
        explicit scope_exit(ExitCallable&& on_exit) noexcept(noexcept(Callable{std::forward<ExitCallable>(on_exit)}))
            : attached_{true}, on_exit_{std::forward<ExitCallable>(on_exit)} {}

        template <std::invocable EntryCallable, std::invocable ExitCallable>
            requires std::convertible_to<Callable, ExitCallable>
        scope_exit(EntryCallable&& on_entry, ExitCallable&& on_exit) noexcept(
            noexcept(Callable{std::forward<ExitCallable>(on_exit)}))
            : scope_exit{std::forward<ExitCallable>(on_exit)} {
            std::forward<EntryCallable>(on_entry)();
        }

        scope_exit(const scope_exit&)     = delete;
        scope_exit(scope_exit&&) noexcept = delete;

        ~scope_exit() {
            if (attached_) {
                on_exit_();
            }
        }

        scope_exit& operator=(const scope_exit&)     = delete;
        scope_exit& operator=(scope_exit&&) noexcept = delete;

        void release() noexcept {
            attached_ = false;
        }

    private:
        bool attached_;
        Callable on_exit_;
    };

    template <typename T>
    scope_exit(T&&) -> scope_exit<std::decay_t<T>>;

    template <typename T, typename U>
    scope_exit(T&&, U&&) -> scope_exit<std::decay_t<U>>;
} // namespace essence
