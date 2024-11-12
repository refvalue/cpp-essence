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

#include "jni/util.hpp"

#include "char8_t_remediation.hpp"
#include "encoding.hpp"
#include "jni/jvm.hpp"
#include "meta/literal_string.hpp"
#include "scope.hpp"
#include "type_list.hpp"

#include <algorithm>
#include <cstdarg>

namespace essence::jni {
    namespace {
        template <meta::literal_string Name>
        global_ref_ex<jclass> get_jclass() {
            static auto&& entry =
                jvm::instance().add_managed_entry(jvm::instance().ensure_env()->FindClass(Name.c_str()));

            return global_ref_ex<jclass>{static_cast<jclass>(entry.get())};
        }

        template <auto Create, auto GetRegion, auto GetElements, auto ReleaseElements>
        struct make_array_routines {
            using array_type = typename function_traits<decltype(GetRegion)>::template arg_type<0>;
            using value_type = std::remove_pointer_t<typename function_traits<decltype(GetElements)>::return_type>;

            inline static const auto create           = std::mem_fn(Create);
            inline static const auto get_region       = std::mem_fn(GetRegion);
            inline static const auto get_elements     = std::mem_fn(GetElements);
            inline static const auto release_elements = std::mem_fn(ReleaseElements);
        };

        template <typename T>
        struct array_routines {};

        template <>
        struct array_routines<bool> : make_array_routines<&JNIEnv::NewBooleanArray, &JNIEnv ::GetBooleanArrayRegion,
                                          &JNIEnv::GetBooleanArrayElements, &JNIEnv::ReleaseBooleanArrayElements> {};

        template <>
        struct array_routines<std::int8_t> : make_array_routines<&JNIEnv::NewByteArray, &JNIEnv::GetByteArrayRegion,
                                                 &JNIEnv::GetByteArrayElements, &JNIEnv::ReleaseByteArrayElements> {};

        template <>
        struct array_routines<std::int16_t> : make_array_routines<&JNIEnv::NewShortArray, &JNIEnv::GetShortArrayRegion,
                                                  &JNIEnv::GetShortArrayElements, &JNIEnv::ReleaseShortArrayElements> {
        };

        template <>
        struct array_routines<std::int32_t> : make_array_routines<&JNIEnv::NewIntArray, &JNIEnv::GetIntArrayRegion,
                                                  &JNIEnv::GetIntArrayElements, &JNIEnv::ReleaseIntArrayElements> {};

#ifdef _MSC_VER
        template <>
        struct array_routines<long> : array_routines<std::int32_t> {};
#endif

        template <>
        struct array_routines<std::int64_t> : make_array_routines<&JNIEnv::NewLongArray, &JNIEnv::GetLongArrayRegion,
                                                  &JNIEnv::GetLongArrayElements, &JNIEnv::ReleaseLongArrayElements> {};

        template <>
        struct array_routines<float> : make_array_routines<&JNIEnv::NewFloatArray, &JNIEnv::GetFloatArrayRegion,
                                           &JNIEnv::GetFloatArrayElements, &JNIEnv::ReleaseFloatArrayElements> {};

        template <>
        struct array_routines<double> : make_array_routines<&JNIEnv::NewDoubleArray, &JNIEnv::GetDoubleArrayRegion,
                                            &JNIEnv::GetDoubleArrayElements, &JNIEnv::ReleaseDoubleArrayElements> {};

        template <>
        struct array_routines<std::uint8_t> : array_routines<std::int8_t> {};

        template <>
        struct array_routines<std::uint16_t> : make_array_routines<&JNIEnv::NewCharArray, &JNIEnv::GetCharArrayRegion,
                                                   &JNIEnv::GetCharArrayElements, &JNIEnv::ReleaseCharArrayElements> {};

        template <typename T>
        struct lookup_array_value_type {
            using list_type =
                type_list<bool, std::int8_t, std::int16_t, std::int32_t, std::int64_t, float, double, std::uint16_t>;

            static constexpr auto judger = []<typename U>(std::type_identity<U>) {
                return std::same_as<typename array_routines<U>::array_type, T>;
            };

            using interface_type  = at_if_t<list_type, judger>;
            using underlying_type = typename array_routines<interface_type>::value_type;
        };

        template <primitve_jarray Array>
        auto from_primitive_array(Array array) {
            using result_type = abi::vector<typename lookup_array_value_type<Array>::underlying_type>;

            if (auto env = jvm::instance().get_env()) {
                auto size = env->GetArrayLength(array);
                result_type result(size);

                array_routines<typename lookup_array_value_type<Array>::interface_type>::get_region(
                    env, array, 0, size, result.data());

                return result;
            }

            return result_type{};
        }

        template <typename T>
        auto make_primitive_array(std::span<const T> buffer) -> typename array_routines<T>::array_type {
            if (auto env = jvm::instance().get_env()) {
                auto array    = array_routines<T>::create(env, static_cast<jsize>(buffer.size()));
                auto elements = array_routines<T>::get_elements(env, array, nullptr);
                scope_exit elements_scope{[&] { array_routines<T>::release_elements(env, array, elements, 0); }};

                std::ranges::copy(buffer.data(), buffer.data() + buffer.size(), elements);

                return array;
            }

            return nullptr;
        }
    } // namespace

    global_ref_ex<jclass> get_jstring_class() {
        return get_jclass<"java/lang/String">();
    }

    global_ref_ex<jclass> get_exception_class() {
        return get_jclass<"java/lang/Exception">();
    }

    abi::string from_string(jstring str) {
        if (const auto env = jvm::instance().get_env(); env && str) {
            const auto jchars = env->GetStringChars(str, nullptr);
            scope_exit jchars_scope{[&] { env->ReleaseStringChars(str, jchars); }};

            return to_utf8_string(std::span<const jchar>{jchars, static_cast<std::size_t>(env->GetStringLength(str))});
        }

        return {};
    }

    abi::vector<jboolean> from_array(jbooleanArray array) {
        return from_primitive_array(array);
    }

    abi::vector<jbyte> from_array(jbyteArray array) {
        return from_primitive_array(array);
    }

    abi::vector<jshort> from_array(jshortArray array) {
        return from_primitive_array(array);
    }

    abi::vector<jint> from_array(jintArray array) {
        return from_primitive_array(array);
    }

    abi::vector<jlong> from_array(jlongArray array) {
        return from_primitive_array(array);
    }

    abi::vector<jchar> from_array(jcharArray array) {
        return from_primitive_array(array);
    }

    abi::vector<float> from_array(jfloatArray array) {
        return from_primitive_array(array);
    }

    abi::vector<double> from_array(jdoubleArray array) {
        return from_primitive_array(array);
    }

    abi::vector<local_ref> from_array(jobjectArray array) {
        return abi::vector<local_ref>(::begin(array), ::end(array));
    }

    abi::vector<abi::string> from_string_array(jobjectArray array) {
        return from_array(array, [](const local_ref& inner) { return from_string(static_cast<jstring>(inner.get())); });
    }

    jstring make_string(std::string_view str) {
        const auto env    = jvm::instance().ensure_env();
        const auto jchars = to_uint16_t_literal(str);

        return env->NewString(jchars.data(), static_cast<jsize>(jchars.size()));
    }

    jbooleanArray make_array(std::span<const bool> buffer) {
        return make_primitive_array(buffer);
    }

    jbyteArray make_array(std::span<const std::byte> buffer) {
        return make_array(
            std::span<const std::uint8_t>{reinterpret_cast<const std::uint8_t*>(buffer.data()), buffer.size()});
    }

    jbyteArray make_array(std::span<const std::int8_t> buffer) {
        return make_primitive_array(buffer);
    }

    jshortArray make_array(std::span<const std::int16_t> buffer) {
        return make_primitive_array(buffer);
    }

    jintArray make_array(std::span<const std::int32_t> buffer) {
        return make_primitive_array(buffer);
    }

    jlongArray make_array(std::span<const std::int64_t> buffer) {
        return make_primitive_array(buffer);
    }

    jbyteArray make_array(std::span<const std::uint8_t> buffer) {
        return make_primitive_array(buffer);
    }

    jcharArray make_array(std::span<const std::uint16_t> buffer) {
        return make_primitive_array(buffer);
    }

    jfloatArray make_array(std::span<const float> buffer) {
        return make_primitive_array(buffer);
    }

    jdoubleArray make_array(std::span<const double> buffer) {
        return make_primitive_array(buffer);
    }
#ifdef _MSC_VER
    jintArray make_array(std::span<const long> buffer) {
        return make_primitive_array(buffer);
    }
#endif
    jobjectArray make_array(jclass class_id, std::span<const std::byte> buffer, std::size_t unit_size,
        const jobject_bit_transformer_type& transformer) {
        if (!transformer) {
            return nullptr;
        }

        jsize index{};
        const auto env    = jvm::instance().ensure_env();
        const auto result = env->NewObjectArray(static_cast<jsize>(buffer.size() / unit_size), class_id, nullptr);

        for (auto ptr = buffer.data(), ptr_end = buffer.data() + buffer.size(); ptr < ptr_end; ptr += unit_size) {
            env->SetObjectArrayElement(result, index++, transformer(ptr).get());
        }

        return result;
    }

    jobject make_object_unsafe(jclass class_id, jmethodID ctor, ...) {
        const auto env = jvm::instance().ensure_env();
        va_list args;
        va_start(args, ctor);

        const auto result = env->NewObjectV(class_id, ctor, args);

        va_end(args);

        return result;
    }

    jint throw_exception(const std::exception& ex) {
        const auto env = jvm::instance().ensure_env();

        return env->ThrowNew(get_exception_class().get(), ex.what());
    }

    jint throw_exception(jclass exception_type, zstring_view message) {
        const auto env = jvm::instance().ensure_env();

        return env->ThrowNew(exception_type, message.c_str());
    }
} // namespace essence::jni

namespace essence::jni::scoped {
    local_ref_ex<jstring> make_string(std::string_view str) {
        return local_ref_ex<jstring>{jni::make_string(str), true};
    }

    local_ref_ex<jbooleanArray> make_array(std::span<const bool> buffer) {
        return local_ref_ex<jbooleanArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jbyteArray> make_array(std::span<const std::byte> buffer) {
        return local_ref_ex<jbyteArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jbyteArray> make_array(std::span<const std::int8_t> buffer) {
        return local_ref_ex<jbyteArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jshortArray> make_array(std::span<const std::int16_t> buffer) {
        return local_ref_ex<jshortArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jintArray> make_array(std::span<const std::int32_t> buffer) {
        return local_ref_ex<jintArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jlongArray> make_array(std::span<const std::int64_t> buffer) {
        return local_ref_ex<jlongArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jbyteArray> make_array(std::span<const std::uint8_t> buffer) {
        return local_ref_ex<jbyteArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jcharArray> make_array(std::span<const std::uint16_t> buffer) {
        return local_ref_ex<jcharArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jfloatArray> make_array(std::span<const float> buffer) {
        return local_ref_ex<jfloatArray>{jni::make_array(buffer), true};
    }

    local_ref_ex<jdoubleArray> make_array(std::span<const double> buffer) {
        return local_ref_ex<jdoubleArray>{jni::make_array(buffer), true};
    }
#ifdef _MSC_VER
    local_ref_ex<jintArray> make_array(std::span<const long> buffer) {
        return local_ref_ex<jintArray>{jni::make_array(buffer), true};
    }
#endif
    local_ref_ex<jobjectArray> make_array(jclass class_id, std::span<const std::byte> buffer, std::size_t unit_size,
        const jobject_bit_transformer_type& transformer) {
        return local_ref_ex<jobjectArray>{jni::make_array(class_id, buffer, unit_size, transformer), true};
    }
} // namespace essence::jni::scoped
