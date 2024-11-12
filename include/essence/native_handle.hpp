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

#include <compare>
#include <concepts>
#include <cstdint>

namespace essence {
    template <typename T>
    concept handle_stroage_type = std::integral<T> && sizeof(T) >= sizeof(std::uintptr_t);

    template <auto Callable, typename T>
    concept handle_validator_type = requires {
        requires std::integral<T>;
        { Callable } -> std::predicate<T>;
    };

    template <std::integral T>
    T make_pointer_number(const void* pointer) noexcept {
        return static_cast<T>(reinterpret_cast<std::uintptr_t>(pointer));
    }

    /**
     * @brief Checks whether the stored value is a valid handle (assuming zero or negative one is invalid).
     * @tparam T The integral type.
     * @param value The integer to store the handle value.
     * @return true if the value is valid; otherwise false.
     */
    template <std::integral T>
    constexpr bool is_valid_handle_value(T value) noexcept {
        return value != 0 && value != -1;
    }

    /**
     * @brief Stores a platform-dependent handle.
     * @tparam T The underlying storage type.
     * @tparam Mapped The integral type to map the T value to (default T).
     * @tparam Validator A callable object to validate the handle.
     */
    template <handle_stroage_type T = std::uintptr_t, typename Mapped = T,
        auto Validator = &is_valid_handle_value<Mapped>>
        requires handle_validator_type<Validator, Mapped>
    class basic_native_handle {
    public:
        constexpr basic_native_handle(std::nullptr_t = nullptr) noexcept : value_{} {} // NOLINT(*-explicit-constructor)

        /**
         * @brief Constructs the object from a pointer.
         * @tparam U The pointer type.
         * @param value The pointer value.
         */
        template <typename U>
            requires std::is_pointer_v<U>
        basic_native_handle(U value) noexcept : value_{make_pointer_number<T>(value)} {} // NOLINT(*-explicit-constructor)

        /**
         * @brief Constructs the object from a mapped number.
         * @param value The mapped number.
         */
        constexpr basic_native_handle(Mapped value) noexcept : value_{static_cast<T>(value)} {} // NOLINT(*-explicit-constructor)

        constexpr bool operator==(const basic_native_handle&) const noexcept  = default;
        constexpr auto operator<=>(const basic_native_handle&) const noexcept = default;

        /**
         * @brief Checks whether the stored value is a valid handle.
         * @remark Assmues zero or negative one is invalid by default.
         */
        explicit constexpr operator bool() const noexcept {
            return Validator(static_cast<Mapped>(value_));
        }

        /**
         * @brief Converts to a pointer.
         * @tparam U The pointer type.
         */
        template <typename U>
            requires std::is_pointer_v<U>
        operator U() const noexcept { // NOLINT(*-explicit-constructor)
            return reinterpret_cast<U>(value_);
        }

        /**
         * @brief Converts to a mapped number.
         */
        constexpr operator Mapped() const noexcept { // NOLINT(*-explicit-constructor)
            return static_cast<Mapped>(value_);
        }

    private:
        T value_;
    };

    using native_handle = basic_native_handle<>;
} // namespace essence
