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

#include "net/http_client_config.hpp"
#include "net/http_listener_config.hpp"
#include "net/uri.hpp"
#include "source_location.hpp"

#include <cpprest/http_client.h>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>

namespace essence::net {
    web::http::client::http_client_config make_cpprest_http_client_config(
        const http_client_config& config, bool no_internal_timeout);

    web::http::experimental::listener::http_listener_config make_cpprest_http_listener_config(
        const http_listener_config& config, bool no_internal_timeout);

    web::uri make_cpprest_uri(const uri& uri);

    uri make_uri(const web::uri& uri);

    bool is_successful_status_code(web::http::status_code code) noexcept;

    void ensure_status_code(
        const web::http::http_response& response, const source_location& location = source_location::current());
} // namespace essence::net
