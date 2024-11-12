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

#include "functional.hpp"
#include "native_handle.hpp"

#include <concepts>
#include <cstdint>
#include <memory>
#include <type_traits>

namespace essence {
    template <typename Callable, typename T = function_arg_type<Callable>>
    concept handle_deleter_type =
        (std::integral<T> && std::invocable<Callable, T>)
        || (std::is_pointer_v<T> && (std::invocable<Callable, T> || std::invocable<Callable, T*>) );

    /**
     * @brief Manages a platform-dependent handle.
     * @tparam Mapped The integral type to map the T value to (default T).
     * @tparam Shared true to use std::shared_ptr; otherwise std::unique_ptr.
     * @tparam T The underlying storage type.
     * @tparam Deleter A callable object to delete the handle.
     * @tparam Validator A callable object to validate the handle.
     */
    template <bool Shared, auto Deleter, handle_stroage_type T = std::uintptr_t, typename Mapped = T,
        auto Validator = &is_valid_handle_value<Mapped>>
        requires(handle_deleter_type<decltype(Deleter)> && handle_validator_type<Validator, Mapped>)
    class basic_managed_handle {
    public:
        using native_type = basic_native_handle<T, Mapped, Validator>;
        using value_type  = std::conditional_t<Shared, std::shared_ptr<void>, std::unique_ptr<void, void (*)(void*)>>;

        basic_managed_handle(std::nullptr_t = nullptr) noexcept // NOLINT(*-explicit-constructor)
            : value_{nullptr, &basic_managed_handle::delete_handle} {}

        /**
         * @brief Constructs the object from a pointer.
         * @tparam U The pointer type.
         * @param value The pointer value.
         */
        template <typename U>
            requires std::is_pointer_v<U>
        explicit basic_managed_handle(U value)
            : value_{reinterpret_cast<void*>(make_pointer_number<T>(value)), &basic_managed_handle::delete_handle} {}

        /**
         * @brief Constructs the object from a mapped number.
         * @param value The mapped number.
         */
        explicit basic_managed_handle(Mapped value)
            : value_{reinterpret_cast<void*>(static_cast<T>(value)), &basic_managed_handle::delete_handle} {}

        basic_managed_handle(const basic_managed_handle&)                = default;
        basic_managed_handle(basic_managed_handle&&) noexcept            = default;
        basic_managed_handle& operator=(const basic_managed_handle&)     = default;
        basic_managed_handle& operator=(basic_managed_handle&&) noexcept = default;

        /**
         * @brief Checks whether the stored value is a valid handle.
         * @remark Assumes zero or negative one is invalid by default.
         */
        explicit operator bool() const noexcept {
            if (value_) {
                return Validator(make_pointer_number<Mapped>(value_.get()));
            }

            return false;
        }

        /**
         * @brief Gets the stored handle.
         * @return The stored handle.
         */
        native_type get() const noexcept {
            return value_.get();
        }

        /**
         * @brief Deletes the handle and resets the value.
         */
        void reset() noexcept {
            value_.reset();
        }

        /**
         * @brief Resets the object with a pointer.
         * @tparam U The pointer type.
         * @param value The pointer value.
         */
        template <typename U>
            requires std::is_pointer_v<U>
        void reset(U value) noexcept {
            if constexpr (Shared) {
                value_.reset(
                    reinterpret_cast<void*>(make_pointer_number<T>(value)), &basic_managed_handle::delete_handle);
            } else {
                value_.reset(reinterpret_cast<void*>(make_pointer_number<T>(value)));
            }
        }

        /**
         * @brief Resets the object with a mapped number.
         * @param value The mapped number.
         */
        void reset(Mapped value) noexcept {
            if constexpr (Shared) {
                value_.reset(reinterpret_cast<void*>(static_cast<T>(value)), &basic_managed_handle::delete_handle);
            } else {
                value_.reset(reinterpret_cast<void*>(static_cast<T>(value)));
            }
        }

        /**
         * @brief Swaps the handle value with another one.
         * @param other The other object.
         */
        void swap(basic_managed_handle& other) noexcept {
            value_.swap(other.value_);
        }

    private:
        static void delete_handle(void* handle) {
            using arg_type = function_arg_type<decltype(Deleter)>;

            if (auto mapped_value = make_pointer_number<Mapped>(handle); Validator(mapped_value)) {
                if constexpr (std::integral<arg_type>) {
                    Deleter(mapped_value);
                } else if constexpr (std::is_pointer_v<std::remove_pointer_t<arg_type>>) {
                    Deleter(&static_cast<std::remove_pointer_t<arg_type>&>(handle));
                } else {
                    Deleter(static_cast<arg_type>(handle));
                }
            }
        }

        value_type value_;
    };

    template <auto Deleter, handle_stroage_type T = std::uintptr_t, typename Mapped = T,
        auto Validator = &is_valid_handle_value<Mapped>>
    using shared_handle = basic_managed_handle<true, Deleter, T, Mapped, Validator>;

    template <auto Deleter, handle_stroage_type T = std::uintptr_t, typename Mapped = T,
        auto Validator = &is_valid_handle_value<Mapped>>
    using unique_handle = basic_managed_handle<false, Deleter, T, Mapped, Validator>;
} // namespace essence
