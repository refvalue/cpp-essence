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

#include <unordered_set>

#include <essence/char8_t_remediation.hpp>
#include <essence/error_extensions.hpp>
#include <essence/range.hpp>

namespace essence::wasm {
    bool is_typed_array(emscripten::val value) {
        static const std::unordered_set<std::string> typed_array_names{U8("Int8Array"), U8("Uint8Array"),
            U8("Uint8ClampedArray"), U8("Int16Array"), U8("Uint16Array"), U8("Int32Array"), U8("Uint32Array"),
            U8("Float32Array"), U8("Float64Array"), U8("BigInt64Array"), U8("BigUint64Array")};

        return typed_array_names.contains(value[U8("constructor")][U8("name")].as<std::string>());
    }

    byte_view get_byte_view(emscripten::val array_or_string) {
        if (array_or_string.isString()) {
            byte_view view{
                .storage = array_or_string.as<std::string>(),
            };

            view.span = as_const_byte_span(std::get<0>(view.storage));

            return view;
        }

        if (array_or_string.isArray() || is_typed_array(array_or_string)) {
            byte_view view{
                .storage = emscripten::convertJSArrayToNumberVector<std::uint8_t>(array_or_string),
            };

            view.span = as_const_byte_span(std::get<1>(view.storage));

            return view;
        }

        throw source_code_aware_runtime_error{U8("The type must be a string or an array.")};
    }

} // namespace essence::wasm
