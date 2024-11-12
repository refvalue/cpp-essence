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

#include "../abi/map.hpp"
#include "../abi/string.hpp"
#include "../compat.hpp"
#include "uri.hpp"

#include <cstddef>
#include <functional>
#include <optional>
#include <string_view>

namespace web::http {
    class http_headers;
}

namespace essence::net {
    class http_headers_proxy {
    public:
        ES_API(CPPESSENCE) explicit http_headers_proxy(web::http::http_headers& ref);
        [[nodiscard]] ES_API(CPPESSENCE) std::size_t size() const;
        [[nodiscard]] ES_API(CPPESSENCE) bool empty() const;
        [[nodiscard]] ES_API(CPPESSENCE) bool contains(std::string_view name) const;
        ES_API(CPPESSENCE) void add(std::string_view name, std::string_view value) const;
        ES_API(CPPESSENCE) void remove(std::string_view name) const;
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<abi::string> get_value(std::string_view name) const;
        ES_API(CPPESSENCE) void clear() const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::map<abi::string, abi::string> items() const;

    private:
        web::http::http_headers& ref_;
    };

    using http_header_handler = std::function<void(const uri& request_uri, const http_headers_proxy& headers)>;
} // namespace essence::net
