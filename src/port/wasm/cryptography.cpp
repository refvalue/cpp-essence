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

#include "util.hpp"

#include <cstdint>
#include <memory>
#include <string>

#include <essence/char8_t_remediation.hpp>
#include <essence/crypto/digest.hpp>
#include <essence/crypto/symmetric_cipher_provider.hpp>

#include <emscripten/bind.h>
#include <emscripten/val.h>

using namespace essence::crypto;

namespace essence::wasm {
    namespace {
        class encrypter_impl {
        public:
            encrypter_impl(emscripten::val cipher_name, emscripten::val key, emscripten::val iv)
                : provider_{cipher_name.as<std::string>(), cipher_padding_mode::pkcs7, get_byte_view(key).span,
                    get_byte_view(iv).span} {}

            emscripten::val process(emscripten::val buffer) const {
                auto result = provider_.as_base64(get_byte_view(buffer).span);

                return emscripten::val::u8string(result.c_str());
            }

        private:
            symmetric_cipher_provider provider_;
        };

        class decrypter_impl {
        public:
            decrypter_impl(emscripten::val cipher_name, emscripten::val key, emscripten::val iv)
                : provider_{cipher_name.as<std::string>(), cipher_padding_mode::pkcs7, get_byte_view(key).span,
                    get_byte_view(iv).span, false} {}

            emscripten::val process(emscripten::val base64) const {
                auto result = provider_.string_from_base64(base64.as<std::string>());

                return emscripten::val::array(result.begin(), result.end());
            }

        private:
            symmetric_cipher_provider provider_;
        };

        std::shared_ptr<encrypter_impl> make_encryptor(
            emscripten::val cipher_name, emscripten::val key, emscripten::val iv) {
            return std::make_shared<encrypter_impl>(cipher_name, key, iv);
        }

        std::shared_ptr<decrypter_impl> make_decryptor(
            emscripten::val cipher_name, emscripten::val key, emscripten::val iv) {
            return std::make_shared<decrypter_impl>(cipher_name, key, iv);
        }

        emscripten::val digest_sm3(emscripten::val buffer) {
            return emscripten::val::u8string(make_digest(digest_mode::sm3, get_byte_view(buffer).span).c_str());
        }

        std::string get_error_message(std::intptr_t ptr) {
            return reinterpret_cast<std::exception*>(ptr)->what();
        }
    } // namespace
} // namespace essence::wasm

using namespace essence::wasm;

EMSCRIPTEN_BINDINGS(crypto) {
    emscripten::class_<encrypter_impl>(U8("Encryptor"))
        .smart_ptr_constructor(U8("encryptor"), &make_encryptor)
        .function(U8("process"), &encrypter_impl::process);

    emscripten::class_<decrypter_impl>(U8("Decryptor"))
        .smart_ptr_constructor(U8("decryptor"), &make_decryptor)
        .function(U8("process"), &decrypter_impl::process);

    emscripten::function(U8("digestSm3"), &digest_sm3);
    emscripten::function(U8("getErrorMessage"), &get_error_message);
}
