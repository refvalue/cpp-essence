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
#include <essence/io/compresser.hpp>

#include <emscripten/bind.h>
#include <emscripten/val.h>

namespace essence::wasm {
    namespace {
        class compresser_impl {
        public:
            compresser_impl() : impl_{compression_mode::zstd} {}

            emscripten::val process(emscripten::val buffer) {
                auto result = impl_.as_string(get_byte_view(buffer).span);

                return emscripten::val::array(result.begin(), result.end());
            }

            emscripten::val inverse(emscripten::val buffer) {
                auto result = impl_.inverse_as_string(get_byte_view(buffer).span);

                return emscripten::val::array(result.begin(), result.end());
            }

        private:
            io::compresser impl_;
        };

        std::shared_ptr<compresser_impl> make_compresser() {
            return std::make_shared<compresser_impl>();
        }

        std::string get_error_message(std::intptr_t ptr) {
            return reinterpret_cast<std::exception*>(ptr)->what();
        }
    } // namespace
} // namespace essence::wasm

using namespace essence::wasm;

EMSCRIPTEN_BINDINGS(compression) {
    emscripten::class_<io::compresser_impl>{U8("Compresser")}
        .smart_ptr_constructor(U8("compresser"), &make_compresser)
        .function(U8("process"), &compresser_impl::process)
        .function(U8("inverse"), &compresser_impl::inverse);

    emscripten::function(U8("getErrorMessage"), &get_error_message);
}
