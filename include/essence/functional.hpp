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

#include "boolean.hpp"

#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <type_traits>
#include <utility>

namespace essence::detail {
    template <typename R, typename T, typename... Args>
    struct member_function_wrapper_base {
        using std_function_type = std::function<R(Args...)>;

        template <auto Function, std::convertible_to<T> U>
            requires std::is_member_function_pointer_v<decltype(Function)>
                  && std::is_invocable_r_v<R, decltype(std::mem_fn(Function)), U, Args...>
        static std::function<R(Args...)> make(U&& obj) {
            return
                [&]<typename... Unbound>(Unbound&&... args) { return (obj.*Function)(std::forward<Unbound>(args)...); };
        }
    };

    template <typename Function>
    struct member_function_wrapper {};

#define MAKE_MEMBER_FUNCTION_WRAPPER(...)               \
    template <typename R, typename T, typename... Args> \
    struct member_function_wrapper<R (T::*)(Args...) __VA_ARGS__> : member_function_wrapper_base<R, T, Args...> {}

    MAKE_MEMBER_FUNCTION_WRAPPER();
    MAKE_MEMBER_FUNCTION_WRAPPER(const);
    MAKE_MEMBER_FUNCTION_WRAPPER(volatile);
    MAKE_MEMBER_FUNCTION_WRAPPER(const volatile);
    MAKE_MEMBER_FUNCTION_WRAPPER(&);
    MAKE_MEMBER_FUNCTION_WRAPPER(const&);
    MAKE_MEMBER_FUNCTION_WRAPPER(volatile&);
    MAKE_MEMBER_FUNCTION_WRAPPER(const volatile&);
    MAKE_MEMBER_FUNCTION_WRAPPER(&&);
    MAKE_MEMBER_FUNCTION_WRAPPER(const&&);
    MAKE_MEMBER_FUNCTION_WRAPPER(volatile&&);
    MAKE_MEMBER_FUNCTION_WRAPPER(const volatile&&);
} // namespace essence::detail

namespace essence {
    template <auto Function, typename T>
        requires std::is_member_function_pointer_v<decltype(Function)>
    auto make_member_function(T&& obj) {
        return detail::member_function_wrapper<decltype(Function)>::template make<Function>(std::forward<T>(obj));
    }

    template <typename Function>
    struct make_std_function {
        using type = decltype(std::function{std::declval<Function>()});
    };

    template <typename Function>
        requires std::is_member_function_pointer_v<Function>
    struct make_std_function<Function> {
        using type = typename detail::member_function_wrapper<Function>::std_function_type;
    };

    template <typename Function>
    using make_std_function_t = typename make_std_function<Function>::type;

    template <typename StdFunction>
    struct std_function_traits {};

    template <typename R, typename... Args>
    struct std_function_traits<std::function<R(Args...)>> {
        using return_type = R;

        template <std::size_t I>
        using arg_type = std::tuple_element_t<I, std::tuple<Args...>>;

        static constexpr std::size_t arg_count = sizeof...(Args);
    };

    template <typename Function>
    struct function_traits {
        using std_func_type = make_std_function_t<Function>;
        using return_type   = typename std_function_traits<std_func_type>::return_type;

        template <std::size_t I>
        using arg_type = typename std_function_traits<std_func_type>::template arg_type<I>;

        static constexpr std::size_t arg_count = std_function_traits<std_func_type>::arg_count;
    };

    template <typename Function>
    using function_return_type = typename function_traits<Function>::return_type;

    template <typename Function, std::size_t I = 0>
    using function_arg_type = typename function_traits<Function>::template arg_type<I>;

    template <typename Callable, typename... Args>
    concept invocable = std::invocable<Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept invocable_r = std::is_invocable_r_v<R, Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept invocable_not_r = invocable<Callable, Args...> && !std::same_as<std::invoke_result_t<Callable, Args...>, R>;

    template <typename Callable, typename... Args>
    concept nothrow_invocable = std::is_nothrow_invocable_v<Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nothrow_invocable_r = std::is_nothrow_invocable_r_v<R, Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nothrow_invocable_not_r =
        nothrow_invocable<Callable, Args...> && !std::same_as<std::invoke_result_t<Callable, Args...>, R>;

    template <typename Callable, typename... Args>
    concept nullptr_or_invocable = std::is_null_pointer_v<std::decay_t<Callable>> || invocable<Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nullptr_or_invocable_r =
        std::is_null_pointer_v<std::decay_t<Callable>> || invocable_r<Callable, R, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nullptr_or_invocable_not_r =
        std::is_null_pointer_v<std::decay_t<Callable>> || invocable_not_r<Callable, R, Args...>;

    template <typename Callable, typename... Args>
    concept nullptr_or_nothrow_invocable =
        std::is_null_pointer_v<std::decay_t<Callable>> || nothrow_invocable<Callable, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nullptr_or_nothrow_invocable_r =
        std::is_null_pointer_v<std::decay_t<Callable>> || nothrow_invocable_r<Callable, R, Args...>;

    template <typename Callable, typename R, typename... Args>
    concept nullptr_or_nothrow_invocable_not_r =
        std::is_null_pointer_v<std::decay_t<Callable>> || nothrow_invocable_not_r<Callable, R, Args...>;

    template <typename Callable, typename... Args>
        requires nullptr_or_invocable<Callable, Args...>
    constexpr auto invoke_optional(Callable&& callable, Args&&... args) noexcept(
        nullptr_or_nothrow_invocable<Callable, Args...>) {
        // Simply returns nullptr if the type of std::decay_t<Callable>> is std::nullptr_t.
        if constexpr (std::is_null_pointer_v<std::decay_t<Callable>>) {
            return nullptr;
        } else {
            using result_type = std::invoke_result_t<Callable, Args...>;

            auto valid = convert_to_boolean(callable);

            if constexpr (std::is_void_v<result_type>) {
                if (valid) {
                    std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
                }
            } else {
                if (valid) {
                    return std::optional{std::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...)};
                }

                // Uses a concrete type std::optional<T> instead of std::nullopt unless the return type is explicitly
                // specified by std::optional<T>.
                return std::optional<result_type>{};
            }
        }
    }

    template <typename Callable, typename R, typename... Args>
        requires nullptr_or_invocable_not_r<Callable, void, Args...>
    constexpr auto invoke_optional_or(Callable&& callable, R&& default_value, Args&&... args) noexcept(
        nullptr_or_nothrow_invocable_not_r<Callable, void, Args...>) {
        // Due to the "requires" clause, it never returns void.
        auto result = invoke_optional(std::forward<Callable>(callable), std::forward<Args>(args)...);

        if constexpr (std::is_null_pointer_v<decltype(result)>) {
            return std::forward<R>(default_value);
        } else {
            return std::move(result).value_or(std::forward<R>(default_value));
        }
    }
} // namespace essence
