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

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"
#include "scope.hpp"

#ifndef __EMSCRIPTEN__
#include <atomic>
#endif

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#if __has_include(<version> )
#include <version>
#endif

namespace essence {
    template <typename Function>
    class delegate {};

    template <typename Signature>
        requires std::is_function_v<Signature>
    class delegate<std::function<Signature>> : public delegate<Signature> {};

    template <typename R, typename... Args>
    class delegate<R(Args...)> {
    public:
        using function_type = std::function<R(Args...)>;
        using nothrow_function_type =
            std::conditional_t<std::is_void_v<R>, function_type, std::function<std::optional<R>(Args...)>>;

        delegate() : lifetime_observer_{std::make_shared<std::byte>()} {
            update_readable_buffer();
        }

        delegate(const delegate&)                = delete;
        delegate(delegate&&) noexcept            = delete;
        delegate& operator=(const delegate&)     = delete;
        delegate& operator=(delegate&&) noexcept = delete;

        explicit operator bool() const noexcept {
#ifdef __EMSCRIPTEN__
            auto func = readable_buffer_;
#else
            auto func = std::atomic_load_explicit(&readable_buffer_, std::memory_order::acquire);
#endif

            return func && static_cast<bool>(*func);
        }

        decltype(auto) operator+=(const delegate & right) {
            for (auto&& [id, func] : right.listeners_) {
                add_listener<false>(func);
            }

            update_readable_buffer();

            return *this;
        }

        template <std::convertible_to<function_type> Callable>
        decltype(auto) operator+=(Callable && handler) {
            add_listener<true>(std::forward<Callable>(handler));

            return *this;
        }

        template <typename... Equivalents>
            requires std::is_invocable_r_v<R, function_type, Equivalents...>
        auto try_invoke(Equivalents&&... args) const {
#ifdef __EMSCRIPTEN__
            auto buffer = readable_buffer_;
#else
            // Fetches the buffer automatically.
            auto buffer = std::atomic_load_explicit(&readable_buffer_, std::memory_order::acquire);
#endif

            if constexpr (std::is_void_v<R>) {
                if (buffer) {
                    for (auto ptr = buffer.get(); *ptr; ++ptr) {
                        (*ptr)(std::forward<Equivalents>(args)...);
                    }
                }
            } else {
                if (!buffer) {
                    return std::optional<R>{};
                }

                auto ptr     = buffer.get();
                auto tuple   = std::forward_as_tuple(std::forward<Equivalents>(args)...);
                auto handler = [&](auto inner) -> std::optional<R> { return std::apply(*inner, std::move(tuple)); };

                // Exactly one single subscriber.
                if (!ptr[1]) {
                    return handler(ptr);
                }

                for (; *(ptr + 1); ++ptr) {
                    static_cast<void>(handler(ptr));
                }

                // Returns the result of the final subscriber.
                return handler(ptr);
            }
        }

        template <typename... Equivalents>
            requires std::is_invocable_r_v<R, function_type, Equivalents...>
        R operator()(Equivalents&&... args) const {
            if constexpr (std::is_void_v<R>) {
                return try_invoke(std::forward<Equivalents>(args)...);
            } else {
                auto result = try_invoke(std::forward<Equivalents>(args)...);

                if (!result) {
                    throw source_code_aware_runtime_error{
                        U8("A delegate with a return value cannot be invoked, within which no subscriber exists.")};
                }

                return std::move(*result);
            }
        }

        template <std::convertible_to<function_type> Callable>
        auto add_listener(Callable&& handler) {
            auto id               = add_listener<true>(std::forward<Callable>(handler));
            auto lifetime_handler = [this, id, observer = std::weak_ptr<void>{lifetime_observer_}] {
                if (auto lock = observer.lock()) {
                    remove_listener(id);
                }
            };

            return std::make_shared<scope_exit<decltype(lifetime_handler)>>(std::move(lifetime_handler));
        }

        function_type to_function() const {
            return [this]<typename... Equivalents>(
                       Equivalents&&... args) { return (*this)(std::forward<Equivalents>(args)...); };
        }

        nothrow_function_type to_nothrow_function() const {
            return [this]<typename... Equivalents>(
                       Equivalents&&... args) { return this->try_invoke(std::forward<Equivalents>(args)...); };
        }

    private:
        template <bool Update, std::convertible_to<function_type> Callable>
        auto add_listener(Callable&& handler) {
#ifdef __EMSCRIPTEN__
            auto counter = ++global_counter_;
#else
            auto counter = global_counter_.fetch_add(1, std::memory_order::acq_rel) + 1;
#endif

            auto id = listeners_.emplace_back(counter, std::forward<Callable>(handler)).first;

            if constexpr (Update) {
                update_readable_buffer();
            }

            return id;
        }

        void remove_listener(std::uint64_t id) {
            if (auto iter = std::find_if(listeners_.begin(), listeners_.end(),
                    [&](const std::pair<std::uint64_t, function_type>& inner) { return inner.first == id; });
                iter != listeners_.end()) {
                listeners_.erase(iter);
                update_readable_buffer();
            }
        }

        void update_readable_buffer() {
            // This statement is exception-safe here.
            // An unbounded array is not considered because there is no corresponding overload for
            // std::atomic<std::shared_ptr<T>>::load.
#if __cpp_lib_smart_ptr_for_overwrite >= 202002L
            auto buffer = std::static_pointer_cast<function_type>(
                std::make_shared_for_overwrite<function_type[]>(listeners_.size() + 1));
#else
            std::shared_ptr<function_type> buffer{
                new function_type[listeners_.size() + 1], std::default_delete<function_type[]> {}};
#endif
            std::transform(listeners_.begin(), listeners_.end(), buffer.get(),
                [](const std::pair<std::uint64_t, function_type>& inner) { return inner.second; });

#ifdef __EMSCRIPTEN__
            readable_buffer_ = std::move(buffer);
#else
            std::atomic_store_explicit(&readable_buffer_, std::move(buffer), std::memory_order::release);
#endif
        }

        std::vector<std::pair<std::uint64_t, function_type>> listeners_;

#ifdef __EMSCRIPTEN__
        std::shared_ptr<function_type> readable_buffer_;
#elif __cpp_lib_atomic_shared_ptr >= 201711L
        std::atomic<std::shared_ptr<function_type>> readable_buffer_;
#else
        std::shared_ptr<function_type> readable_buffer_;
#endif

        std::shared_ptr<void> lifetime_observer_;

#ifdef __EMSCRIPTEN__
        inline static std::uint64_t global_counter_;
#else
        inline static std::atomic_uint64_t global_counter_;
#endif
    };
} // namespace essence
