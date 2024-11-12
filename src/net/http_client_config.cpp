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

#include "net/http_client_config.hpp"

#include "encoding.hpp"

#include <chrono>

#include <cpprest/http_client.h>

namespace essence::net {
    http_client_config http_client_config::get_default() {
        return http_client_config{
            .timeout                 = 10U,
            .validate_certificates   = false,
            .https_to_http_redirects = true,
        };
    }

    http_client_config http_client_config::get_default_no_timeout() {
        return http_client_config{
            .timeout                 = no_timeout,
            .validate_certificates   = false,
            .https_to_http_redirects = true,
        };
    }

    web::http::client::http_client_config& http_client_config::assign_to(
        web::http::client::http_client_config& opaque) const {
        if (proxy) {
            opaque.set_proxy(web::web_proxy{internal::to_native_string(*proxy)});
        }

        opaque.set_timeout(std::chrono::seconds{timeout});
        opaque.set_validate_certificates(validate_certificates);
        opaque.set_https_to_http_redirects(https_to_http_redirects);

        return opaque;
    }
} // namespace essence::net
