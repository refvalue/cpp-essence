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

#include "../abi/string.hpp"
#include "../abi/vector.hpp"
#include "../boolean.hpp"
#include "../compat.hpp"
#include "../functional.hpp"
#include "../zstring_view.hpp"
#include "common_types.hpp"
#include "global_ref.hpp"
#include "iterator.hpp"
#include "local_ref.hpp"

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <new>
#include <optional>
#include <ranges>
#include <span>
#include <string_view>
#include <type_traits>
#include <utility>

#include <jni.h>

namespace essence::jni {
    using jobject_bit_transformer_type = std::function<local_ref(const std::byte* source)>;

    template <typename T>
    concept jni_arg_with_scoped = jni_type<std::decay_t<T>> || local_ref_like<T> || global_ref_like<T>;

    template <typename T>
    concept string_view_able_range =
        std::ranges::contiguous_range<T> && std::convertible_to<std::ranges::range_value_t<T>, std::string_view>;

    ES_API(JNISUPPORT) global_ref_ex<jclass> get_jstring_class();
    ES_API(JNISUPPORT) global_ref_ex<jclass> get_exception_class();

    ES_API(JNISUPPORT) abi::string from_string(jstring str);
    ES_API(JNISUPPORT) abi::vector<jboolean> from_array(jbooleanArray array);
    ES_API(JNISUPPORT) abi::vector<jbyte> from_array(jbyteArray array);
    ES_API(JNISUPPORT) abi::vector<jshort> from_array(jshortArray array);
    ES_API(JNISUPPORT) abi::vector<jint> from_array(jintArray array);
    ES_API(JNISUPPORT) abi::vector<jlong> from_array(jlongArray array);
    ES_API(JNISUPPORT) abi::vector<jchar> from_array(jcharArray array);
    ES_API(JNISUPPORT) abi::vector<float> from_array(jfloatArray array);
    ES_API(JNISUPPORT) abi::vector<double> from_array(jdoubleArray array);
    ES_API(JNISUPPORT) abi::vector<local_ref> from_array(jobjectArray array);
    ES_API(JNISUPPORT) abi::vector<abi::string> from_string_array(jobjectArray array);

    template <std::invocable<const local_ref&> Transformer>
        requires(!std::is_void_v<typename function_traits<Transformer>::return_type>)
    auto from_array(jobjectArray array, Transformer&& transformer) {
        using result_type = abi::vector<typename function_traits<Transformer>::return_type>;

        if (convert_to_boolean(transformer)) {
            jobject_array_proxy proxy{array};
            auto intermediate = proxy | std::views::transform(std::forward<Transformer>(transformer));

            return result_type{intermediate.begin(), intermediate.end()};
        }

        return result_type{};
    }

    ES_API(JNISUPPORT) jstring make_string(std::string_view str);
    ES_API(JNISUPPORT) jbooleanArray make_array(std::span<const bool> buffer);
    ES_API(JNISUPPORT) jbyteArray make_array(std::span<const std::byte> buffer);
    ES_API(JNISUPPORT) jbyteArray make_array(std::span<const std::int8_t> buffer);
    ES_API(JNISUPPORT) jshortArray make_array(std::span<const std::int16_t> buffer);
    ES_API(JNISUPPORT) jintArray make_array(std::span<const std::int32_t> buffer);
    ES_API(JNISUPPORT) jlongArray make_array(std::span<const std::int64_t> buffer);
    ES_API(JNISUPPORT) jbyteArray make_array(std::span<const std::uint8_t> buffer);
    ES_API(JNISUPPORT) jcharArray make_array(std::span<const std::uint16_t> buffer);
    ES_API(JNISUPPORT) jfloatArray make_array(std::span<const float> buffer);
    ES_API(JNISUPPORT) jdoubleArray make_array(std::span<const double> buffer);
#ifdef _MSC_VER
    ES_API(JNISUPPORT) jintArray make_array(std::span<const long> buffer);
#endif
    ES_API(JNISUPPORT)
    jobjectArray make_array(jclass class_id, std::span<const std::byte> buffer, std::size_t unit_size,
        const jobject_bit_transformer_type& transformer);
    ES_API(JNISUPPORT)
    jobject make_object_unsafe(jclass class_id, jmethodID ctor, ...);

    template <std::ranges::contiguous_range Range, typename Transformer>
        requires std::is_invocable_r_v<local_ref, Transformer, const std::ranges::range_reference_t<Range>>
    jobjectArray make_array(jclass class_id, Range&& range, Transformer&& transformer) {
        if (!convert_to_boolean(transformer)) {
            return nullptr;
        }

        return make_array(class_id, std::as_bytes(std::span{range}), sizeof(std::ranges::range_value_t<Range>),
            [&](const std::byte* source) {
                return std::forward<Transformer>(transformer)(
                    *std::launder(reinterpret_cast<const std::ranges::range_value_t<Range>*>(source)));
            });
    }

    template <jni_arg_with_scoped... Args>
    jobject make_object(jclass class_id, jmethodID ctor, Args&&... args) {
        static constexpr auto parse_arg = []<jni_arg_with_scoped T>(T&& arg) -> decltype(auto) {
            if constexpr (local_ref_like<T>) {
                return static_cast<const local_ref&>(std::forward<T>(arg)).get();
            } else if constexpr (global_ref_like<T>) {
                return static_cast<const global_ref&>(std::forward<T>(arg)).get();
            } else {
                return std::forward<T>(arg);
            }
        };

        return make_object_unsafe(class_id, ctor, parse_arg(std::forward<Args>(args))...);
    }

    ES_API(JNISUPPORT) jint throw_exception(const std::exception& ex);
    ES_API(JNISUPPORT) jint throw_exception(jclass exception_type, zstring_view message);

    template <std::default_initializable T>
    T throw_exception(jclass exception_type, zstring_view message, T value = {}) {
        static_assert(sizeof(T) <= 32);
        static_cast<void>(throw_exception(exception_type, message));

        return std::move(value);
    }

    template <std::default_initializable T>
    T throw_exception(const std::exception& ex, T value = {}) {
        static_assert(sizeof(T) <= 32);
        static_cast<void>(throw_exception(ex));

        return std::move(value);
    }

    ES_API(JNISUPPORT) std::optional<abi::string> try_catch_exception();
} // namespace essence::jni

namespace essence::jni::scoped {
    ES_API(JNISUPPORT) local_ref_ex<jstring> make_string(std::string_view str);
    ES_API(JNISUPPORT) local_ref_ex<jbooleanArray> make_array(std::span<const bool> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jbyteArray> make_array(std::span<const std::byte> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jbyteArray> make_array(std::span<const std::int8_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jshortArray> make_array(std::span<const std::int16_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jintArray> make_array(std::span<const std::int32_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jlongArray> make_array(std::span<const std::int64_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jbyteArray> make_array(std::span<const std::uint8_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jcharArray> make_array(std::span<const std::uint16_t> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jfloatArray> make_array(std::span<const float> buffer);
    ES_API(JNISUPPORT) local_ref_ex<jdoubleArray> make_array(std::span<const double> buffer);
#ifdef _MSC_VER
    ES_API(JNISUPPORT) local_ref_ex<jintArray> make_array(std::span<const long> buffer);
#endif
    ES_API(JNISUPPORT)
    local_ref_ex<jobjectArray> make_array(jclass class_id, std::span<const std::byte> buffer, std::size_t unit_size,
        const jobject_bit_transformer_type& transformer);

    template <std::ranges::contiguous_range Range, typename Transformer>
        requires std::is_invocable_r_v<local_ref, Transformer, const std::ranges::range_reference_t<Range>>
    local_ref_ex<jobjectArray> make_array(jclass class_id, Range&& range, Transformer&& transformer) {
        return local_ref_ex<jobjectArray>{
            jni::make_array(class_id, range, std::forward<Transformer>(transformer)), true};
    }

    template <jni_arg_with_scoped... Args>
    local_ref make_object(jclass class_id, jmethodID ctor, Args&&... args) {
        return local_ref{jni::make_object(class_id, ctor, std::forward<Args>(args)...), true};
    }
} // namespace essence::jni::scoped

namespace essence::jni {
    template <string_view_able_range Range>
    jobjectArray make_array(Range&& range) {
        return make_array(
            get_jstring_class().get(), range, [](std::string_view inner) { return scoped::make_string(inner); });
    }
} // namespace essence::jni

namespace essence::jni::scoped {
    template <string_view_able_range Range>
    local_ref_ex<jobjectArray> make_array(Range&& range) {
        return local_ref_ex<jobjectArray>{jni::make_array(range), true};
    }
} // namespace essence::jni::scoped
