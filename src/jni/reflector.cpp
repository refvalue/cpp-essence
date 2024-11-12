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

#include "jni/reflector.hpp"

#include "cache_key.hpp"
#include "error_extensions.hpp"
#include "jni/jvm.hpp"
#include "jni/local_ref.hpp"
#include "meta/identifier.hpp"

#include <string_view>
#include <unordered_map>
#include <variant>

#if __has_include(<version> )
#include <version>
#endif

namespace essence::jni {
    class reflector::impl {
    public:
        [[nodiscard]] global_ref_ex<jclass> get_class(std::int32_t key) const {
            return get_item_cache_internal<global_ref_ex<jclass>, jclass>(key);
        }

        [[nodiscard]] jfieldID get_field(std::int32_t key) const {
            return get_item_cache_internal<jfieldID>(key);
        }

        [[nodiscard]] jmethodID get_method(std::int32_t key) const {
            return get_item_cache_internal<jmethodID>(key);
        }

        global_ref_ex<jclass> add_class(std::int32_t key, zstring_view name) {
            // Finds the class.
            if (const auto env = jvm::instance().ensure_env();
                const local_ref_ex<jclass> clazz{env->FindClass(name.c_str()), true}) {
                return std::get<global_ref_ex<jclass>>(
                    cache_.insert_or_assign(make_cache_key<jclass>(key), global_ref_ex<jclass>{clazz.get()})
                        .first->second);
            }

            throw source_code_aware_runtime_error{
                U8("Failed to find the class."), U8("Class Key"), key, U8("Name"), name};
        }

        jfieldID add_field(std::int32_t class_key, const tuple_type& field) {
            return add_member_cache_internal<jfieldID>(
                class_key, std::type_identity<jfieldID>{}, &JNIEnv::GetFieldID, field);
        }

        jmethodID add_method(std::int32_t class_key, const tuple_type& method) {
            return add_member_cache_internal<jmethodID>(
                class_key, std::type_identity<jmethodID>{}, &JNIEnv::GetMethodID, method);
        }

        jfieldID add_static_field(std::int32_t class_key, const tuple_type& field) {
            return add_member_cache_internal<jfieldID>(
                class_key, std::type_identity<jfieldID>{}, &JNIEnv::GetStaticFieldID, field);
        }

        jmethodID add_static_method(std::int32_t class_key, const tuple_type& method) {
            return add_member_cache_internal<jmethodID>(
                class_key, std::type_identity<jmethodID>{}, &JNIEnv::GetStaticMethodID, method);
        }

        void clear() {
            cache_.clear();
        }

    private:
        template <std::constructible_from<std::nullptr_t> T, typename Category = T>
        [[nodiscard]] T get_item_cache_internal(std::int32_t key) const {
            auto iter = cache_.find(make_cache_key<Category>(key));

            return iter != cache_.end() ? std::get<T>(iter->second) : T{nullptr};
        }

        template <std::constructible_from<std::nullptr_t> T, typename CategoryTag>
        T add_member_cache_internal(std::int32_t class_key, std::type_identity<CategoryTag>,
            T (JNICALL JNIEnv::*handler)(jclass class_id, const char* name, const char* signature),
            const tuple_type& item) {
            if (const auto iter = cache_.find(make_cache_key<jclass>(class_key)); iter != cache_.end()) {
                const auto clazz              = std::get<global_ref_ex<jclass>>(iter->second);
                auto&& [key, name, signature] = item;

                if (auto id = (jvm::instance().ensure_env()->*handler)(clazz.get(), name.c_str(), signature.c_str())) {
                    return std::get<T>(cache_.insert_or_assign(make_cache_key<T>(key), id).first->second);
                }

                throw source_code_aware_runtime_error{U8("Class Key"), class_key, U8("jclass"),
                    reinterpret_cast<std::uintptr_t>(clazz.get()), U8("Category"),
                    meta::get_literal_string_t<CategoryTag, meta::identifier_param{.shortened = true}>(), U8("Name"),
                    name, U8("Signature"), signature, U8("Message"), U8("Failed to find the signature.")};
            }

            throw source_code_aware_runtime_error{
                U8("Class Key"), class_key, U8("Message"), U8("Failed to find the class.")};
        }

        std::unordered_map<cache_key, std::variant<global_ref_ex<jclass>, jfieldID, jmethodID>> cache_;
    };

    reflector::reflector(reflector&&) noexcept = default;

    reflector::~reflector() = default;

    reflector& reflector::operator=(reflector&&) noexcept = default;

    const reflector& reflector::instance() {
        static const reflector instance;

        return instance;
    }

    global_ref_ex<jclass> reflector::get_class(std::int32_t key) const {
        return impl_->get_class(key);
    }

    jfieldID reflector::get_field(std::int32_t key) const {
        return impl_->get_field(key);
    }

    jmethodID reflector::get_method(std::int32_t key) const {
        return impl_->get_method(key);
    }

    global_ref_ex<jclass> reflector::add_class(std::int32_t key, zstring_view name) const {
        return impl_->add_class(key, name);
    }

    jfieldID reflector::add_field(std::int32_t class_key, const tuple_type& field) const {
        return impl_->add_field(class_key, field);
    }

    jmethodID reflector::add_method(std::int32_t class_key, const tuple_type& method) const {
        return impl_->add_method(class_key, method);
    }

    jfieldID reflector::add_static_field(std::int32_t class_key, const tuple_type& field) const {
        return impl_->add_static_field(class_key, field);
    }

    jmethodID reflector::add_static_method(std::int32_t class_key, const tuple_type& method) const {
        return impl_->add_static_method(class_key, method);
    }

    void reflector::clear() const {
        impl_->clear();
    }

    reflector::reflector() : impl_{std::make_unique<impl>()} {}
} // namespace essence::jni
