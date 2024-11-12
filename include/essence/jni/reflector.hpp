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

#include "../compat.hpp"
#include "../zstring_view.hpp"
#include "global_ref.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

#include <jni.h>

namespace essence::jni::detail {
    template <typename T>
    struct is_enum_tuple : std::false_type {};

    template <typename T, typename... Args>
        requires(std::is_enum_v<T> || std::integral<T>)
    struct is_enum_tuple<std::tuple<T, Args...>> : std::true_type {};

    template <typename T>
    concept enum_tuple = is_enum_tuple<std::decay_t<T>>::value;

    template <enum_tuple Tuple>
    constexpr auto from_enum_tuple(const Tuple& source) {
        return [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            return std::tuple{static_cast<std::int32_t>(std::get<0>(source)), std::get<(Is + 1)>(source)...};
        }(std::make_index_sequence<std::tuple_size_v<Tuple> - 1>{});
    }
} // namespace essence::jni::detail

namespace essence::jni {
    class reflector {
    public:
        using tuple_type = std::tuple<std::int32_t, zstring_view, zstring_view>;

        ES_API(JNISUPPORT) reflector(reflector&&) noexcept;
        ES_API(JNISUPPORT) ~reflector();
        ES_API(JNISUPPORT) reflector& operator=(reflector&&) noexcept;
        ES_API(JNISUPPORT) static const reflector& instance();
        [[nodiscard]] ES_API(JNISUPPORT) global_ref_ex<jclass> get_class(std::int32_t key) const;
        [[nodiscard]] ES_API(JNISUPPORT) jfieldID get_field(std::int32_t key) const;
        [[nodiscard]] ES_API(JNISUPPORT) jmethodID get_method(std::int32_t key) const;
        [[nodiscard]] ES_API(JNISUPPORT) global_ref_ex<jclass> add_class(std::int32_t key, zstring_view name) const;
        [[nodiscard]] ES_API(JNISUPPORT) jfieldID add_field(std::int32_t class_key, const tuple_type& field) const;
        [[nodiscard]] ES_API(JNISUPPORT) jmethodID add_method(std::int32_t class_key, const tuple_type& method) const;
        [[nodiscard]] ES_API(JNISUPPORT) jfieldID
            add_static_field(std::int32_t class_key, const tuple_type& field) const;
        [[nodiscard]] ES_API(JNISUPPORT) jmethodID
            add_static_method(std::int32_t class_key, const tuple_type& method) const;
        ES_API(JNISUPPORT) void clear() const;

        template <typename T, detail::enum_tuple Tuple>
            requires std::is_enum_v<T>
        jfieldID add_field(T class_key, const Tuple& field) const {
            return add_field(static_cast<std::int32_t>(class_key), detail::from_enum_tuple(field));
        }

        template <typename T, detail::enum_tuple Tuple>
            requires std::is_enum_v<T>
        jmethodID add_method(T class_key, const Tuple& method) const {
            return add_method(static_cast<std::int32_t>(class_key), detail::from_enum_tuple(method));
        }

        template <typename T, detail::enum_tuple Tuple>
            requires std::is_enum_v<T>
        jfieldID add_static_field(T class_key, const Tuple& field) const {
            return add_static_field(static_cast<std::int32_t>(class_key), detail::from_enum_tuple(field));
        }

        template <typename T, detail::enum_tuple Tuple>
            requires std::is_enum_v<T>
        jmethodID add_static_method(T class_key, const Tuple& method) const {
            return add_static_method(static_cast<std::int32_t>(class_key), detail::from_enum_tuple(method));
        }

        template <typename T, detail::enum_tuple... Args>
            requires(std::is_enum_v<T> || std::integral<T>)
        auto add_fields(T class_key, Args&&... fields) const {
            return std::tuple{add_field(class_key, std::forward<Args>(fields))...};
        }

        template <typename T, detail::enum_tuple... Args>
            requires(std::is_enum_v<T> || std::integral<T>)
        auto add_methods(T class_key, Args&&... methods) const {
            return std::tuple{add_method(class_key, std::forward<Args>(methods))...};
        }

        template <typename T, detail::enum_tuple... Args>
            requires(std::is_enum_v<T> || std::integral<T>)
        auto add_static_fields(T class_key, Args&&... fields) const {
            return std::tuple{add_static_field(class_key, std::forward<Args>(fields))...};
        }

        template <typename T, detail::enum_tuple... Args>
            requires(std::is_enum_v<T> || std::integral<T>)
        auto add_static_methods(T class_key, Args&&... methods) const {
            return std::tuple{add_static_method(class_key, std::forward<Args>(methods))...};
        }

    private:
        reflector();

        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::jni
