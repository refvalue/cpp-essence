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

#include "net/http_headers_proxy.hpp"

#include "encoding.hpp"

#include <cpprest/http_headers.h>

namespace essence::net {
    http_headers_proxy::http_headers_proxy(web::http::http_headers& ref) : ref_{ref} {}

    std::size_t http_headers_proxy::size() const {
        return ref_.size();
    }

    bool http_headers_proxy::empty() const {
        return ref_.empty();
    }

    bool http_headers_proxy::contains(std::string_view name) const {
        return ref_.has(internal::to_native_string(name));
    }

    void http_headers_proxy::add(std::string_view name, std::string_view value) const {
        ref_.add(internal::to_native_string(name), internal::to_native_string(value));
    }

    void http_headers_proxy::remove(std::string_view name) const {
        ref_.remove(internal::to_native_string(name));
    }

    std::optional<abi::string> http_headers_proxy::get_value(std::string_view name) const {
        if (const auto iter = ref_.find(internal::to_native_string(name)); iter != ref_.end()) {
            return to_utf8_string(iter->second);
        }

        return std::nullopt;
    }

    void http_headers_proxy::clear() const {
        ref_.clear();
    }

    abi::map<abi::string, abi::string> http_headers_proxy::items() const {
        abi::map<abi::string, abi::string> result;

        for (auto&& [key, value] : ref_) {
            result.insert_or_assign(to_utf8_string(key), to_utf8_string(value));
        }

        return result;
    }
} // namespace essence::net
