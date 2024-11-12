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

#include <essence/crypto/digest.hpp>
#include <essence/jni/util.hpp>

#include <jni.h>

using namespace essence;
using namespace essence::crypto;

extern "C" {
/*
 * Class:     com_essence_CryptoUtil
 * Method:    hexEncode
 * Signature: ([B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_hexEncode___3B(JNIEnv*, jclass, jbyteArray buffer) try {
    return jni::make_string(hex_encode(jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    hexDecode
 * Signature: (Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_essence_CryptoUtil_hexDecode__Ljava_lang_String_2(
    JNIEnv*, jclass, jstring hex) try {
    return jni::make_array(hex_decode(jni::from_string(hex)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jbyteArray>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    hexEncode
 * Signature: ([BB)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_hexEncode___3BB(
    JNIEnv*, jclass, jbyteArray buffer, jbyte delimiter) try {
    return jni::make_string(hex_encode(jni::from_array(buffer), delimiter));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    hexDecode
 * Signature: (Ljava/lang/String;B)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_essence_CryptoUtil_hexDecode__Ljava_lang_String_2B(
    JNIEnv*, jclass, jstring hex, jbyte delimiter) try {
    return jni::make_array(hex_decode(jni::from_string(hex), delimiter));
} catch (const std::exception& ex) {
    return jni::throw_exception<jbyteArray>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    base64Encode
 * Signature: ([B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_base64Encode(JNIEnv*, jclass, jbyteArray buffer) try {
    return jni::make_string(base64_encode(jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    base64Decode
 * Signature: (Ljava/lang/String;)[B
 */
JNIEXPORT jbyteArray JNICALL Java_com_essence_CryptoUtil_base64Decode(JNIEnv*, jclass, jstring base64) try {
    return jni::make_array(base64_decode(jni::from_string(base64)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jbyteArray>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    hmacHash
 * Signature: (ILjava/lang/String;[B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_hmacHash__ILjava_lang_String_2_3B(
    JNIEnv*, jclass, jint mode, jstring key, jbyteArray buffer) try {
    return jni::make_string(hmac_hash(static_cast<digest_mode>(mode), jni::from_string(key), jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    hmacHash
 * Signature: (ILjava/lang/String;Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_hmacHash__ILjava_lang_String_2Ljava_lang_String_2(
    JNIEnv*, jclass, jint mode, jstring key, jstring buffer) try {
    return jni::make_string(hmac_hash(static_cast<digest_mode>(mode), jni::from_string(key), jni::from_string(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}

/*
 * Class:     com_essence_CryptoUtil
 * Method:    makeDigest
 * Signature: (I[B)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_makeDigest__I_3B(
    JNIEnv*, jclass, jint mode, jbyteArray buffer) try {
    return jni::make_string(make_digest(static_cast<digest_mode>(mode), jni::from_array(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}
/*
 * Class:     com_essence_CryptoUtil
 * Method:    makeDigest
 * Signature: (ILjava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_com_essence_CryptoUtil_makeDigest__ILjava_lang_String_2(
    JNIEnv*, jclass, jint mode, jstring buffer) try {
    return jni::make_string(make_digest(static_cast<digest_mode>(mode), jni::from_string(buffer)));
} catch (const std::exception& ex) {
    return jni::throw_exception<jstring>(ex);
}
}
