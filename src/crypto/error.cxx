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

module;

#include <essence/char8_t_remediation.hpp>
#include <essence/compat.hpp>

#include <openssl/err.h>

module essence.crypto:error;
import essence.basic;
import std;

namespace essence::crypto {
    namespace {
        [[maybe_unused]] ES_KEEP_ALIVE const scope_exit crypto_error_strings_scope{
            [] { ERR_load_crypto_strings(); }, [] { ERR_free_strings(); }};
    } // namespace

    std::uint64_t get_last_error_code() {
        std::uint64_t result{};

        // ERR_get_error returns the earliest item in the thread's queue and removes the entry.
        for (std::uint64_t code; (code = ERR_get_error()) != 0;) {
            result = code;
        }

        return result;
    }

    std::string get_error_string(std::uint64_t code) {
        if (code == 0) {
            return U8("None");
        }

        // buf must be at least 120 bytes long.
        // https://www.openssl.org/docs/man1.0.2/man3/ERR_error_string.html
        std::string result(120, U8('\0'));
        const std::string_view intermediate{ERR_error_string(code, result.data())};

        result.resize(intermediate.size());
        result.shrink_to_fit();

        return result;
    };

    std::string get_last_error_string() {
        return get_error_string(get_last_error_code());
    }

    struct crypto_error final : formatted_runtime_error {
        template <typename... Args>
        explicit crypto_error(logging_string_view hint, Args&&... args)
            : crypto_error{hint.location, hint.str, std::forward<Args>(args)...} {}

        template <typename Arg>
        crypto_error(const std::source_location& location, Arg&& arg)
            : formatted_runtime_error{
                  location, U8("Message"), std::forward<Arg>(arg), U8("Internal"), get_last_error_string()} {}

        template <typename... Args>
        explicit crypto_error(const std::source_location& location, Args&&... args)
            : formatted_runtime_error{location, std::forward<Args>(args)..., U8("Internal"), get_last_error_string()} {}
    };
} // namespace essence::crypto
