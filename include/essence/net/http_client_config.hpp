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

#include "../abi/string.hpp"
#include "../compat.hpp"

#include <cstdint>
#include <limits>
#include <optional>

namespace web::http::client {
    class http_client_config;
}

namespace essence::net {
    struct http_client_config {
        static constexpr auto no_timeout = std::numeric_limits<std::uint32_t>::max();

        std::uint32_t timeout{};
        bool validate_certificates{};
        bool https_to_http_redirects{};
        std::optional<abi::string> proxy;

        ES_API(CPPESSENCE) static http_client_config get_default();
        ES_API(CPPESSENCE) static http_client_config get_default_no_timeout();
        ES_API(CPPESSENCE)
        web::http::client::http_client_config& assign_to(web::http::client::http_client_config& opaque) const;
    };
} // namespace essence::net
