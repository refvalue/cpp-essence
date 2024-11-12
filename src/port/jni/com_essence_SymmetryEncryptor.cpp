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

#include "entry.hpp"

#include <exception>

#include <essence/crypto/chunk_processor.hpp>
#include <essence/crypto/symmetric_cipher_provider.hpp>
#include <essence/error_extensions.hpp>
#include <essence/jni/util.hpp>

#include <jni.h>

using namespace essence;
using namespace essence::java;
using namespace essence::crypto;

namespace {
    const symmetric_cipher_provider* get_impl_ptr(JNIEnv* env, jobject self) {
        return reinterpret_cast<symmetric_cipher_provider*>(
            env->GetLongField(self, jni_classes::symmetry_encryptor::impl));
    }

    const symmetric_cipher_provider& get_impl(JNIEnv* env, jobject self) {
        const auto impl = get_impl_ptr(env, self);

        return impl ? *impl : throw source_code_aware_runtime_error{U8("Uninitialized or released implemention.")};
    }
} // namespace

extern "C" {
/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    getCipherName
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_SymmetryEncryptor_getCipherName(JNIEnv* env, jobject self) try {
    return jni::make_string(get_impl(env, self).cipher_name());
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    asBytes
 * Signature: ([B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_essence_SymmetryEncryptor_asBytes___3B(
    JNIEnv* env, jobject self, jbyteArray buffer) try {
    return jni::make_array(get_impl(env, self).as_bytes(jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jbyteArray>(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    asBytes
 * Signature: (Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_essence_SymmetryEncryptor_asBytes__Ljava_lang_String_2(
    JNIEnv* env, jobject self, jstring buffer) try {
    return jni::make_array(get_impl(env, self).as_bytes(jni::from_string(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jbyteArray>(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    asBase64
 * Signature: ([B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_SymmetryEncryptor_asBase64___3B(
    JNIEnv* env, jobject self, jbyteArray buffer) try {
    return jni::make_string(get_impl(env, self).as_base64(jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    asBase64
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_SymmetryEncryptor_asBase64__Ljava_lang_String_2(
    JNIEnv* env, jobject self, jstring buffer) try {
    return jni::make_string(get_impl(env, self).as_base64(jni::from_string(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    init
 * Signature: (Ljava/lang/String;I[B[B)V
 */
JNIEXPORT void JNICALL Java_com_essence_SymmetryEncryptor_init__Ljava_lang_String_2I_3B_3B(
    JNIEnv* env, jobject self, jstring cipher_name, jint mode, jbyteArray key, jbyteArray iv) try {
    env->SetLongField(self, jni_classes::symmetry_encryptor::impl,
        reinterpret_cast<jlong>(
            new symmetric_cipher_provider{make_symmetric_cipher_chunk_processor(jni::from_string(cipher_name),
                static_cast<cipher_padding_mode>(mode), jni::from_array(key), jni::from_array(iv))}));
} catch (const std::exception& ex) {
    jni::throw_exception(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    init
 * Signature: (Ljava/lang/String;ILjava/lang/String;Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
    Java_com_essence_SymmetryEncryptor_init__Ljava_lang_String_2ILjava_lang_String_2Ljava_lang_String_2(
        JNIEnv* env, jobject self, jstring cipher_name, jint mode, jstring key, jstring iv) try {
    env->SetLongField(self, jni_classes::symmetry_encryptor::impl,
        reinterpret_cast<jlong>(
            new symmetric_cipher_provider{make_symmetric_cipher_chunk_processor(jni::from_string(cipher_name),
                static_cast<cipher_padding_mode>(mode), jni::from_string(key), jni::from_string(iv))}));
} catch (const std::exception& ex) {
    jni::throw_exception(ex);
}

/*
 * Class:     com_essence_SymmetryEncryptor
 * Method:    release
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_essence_SymmetryEncryptor_release(JNIEnv* env, jobject self) try {
    if (auto impl = get_impl_ptr(env, self)) {
        delete impl;
        env->SetLongField(self, jni_classes::symmetry_encryptor::impl, jlong{});
    }
} catch (const std::exception& ex) {
    jni::throw_exception(ex);
}
}
