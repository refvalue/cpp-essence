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

#include "../compat.hpp"
#include "common_types.hpp"
#include "http_client_config.hpp"
#include "http_headers_proxy.hpp"
#include "uri.hpp"

namespace essence::net {
    /**
     * @brief Downloads a file from an HTTP server.
     * @param absolute_uri The absolute URI.
     * @param header_handler The handler to process HTTP headers.
     * @return The downloaded data.
     */
    ES_API(CPPESSENCE)
    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_header_handler& header_handler = {});

    /**
     * @brief Downloads a file from an HTTP server.
     * @param absolute_uri The absolute URI.
     * @param config The HTTP configuration.
     * @param header_handler The handler to process HTTP headers.
     * @return The downloaded data.
     */
    ES_API(CPPESSENCE)
    abi::vector<std::byte> download_file(
        const uri& absolute_uri, const http_client_config& config, const http_header_handler& header_handler = {});

    /**
     * @brief Downloads a file from an HTTP server.
     * @param absolute_uri The absolute URI.
     * @param progress_handlers The handlers to process both byte and percentage progresses.
     * @param header_handler The handler to process HTTP headers.
     * @return The downloaded data.
     */
    ES_API(CPPESSENCE)
    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_progress_handlers& progress_handlers,
        const http_header_handler& header_handler = {});

    /**
     * @brief Downloads a file from an HTTP server.
     * @param absolute_uri The absolute URI.
     * @param config The HTTP configuration.
     * @param progress_handlers The handlers to process both byte and percentage progresses.
     * @param header_handler The handler to process HTTP headers.
     * @return The downloaded data.
     */
    ES_API(CPPESSENCE)
    abi::vector<std::byte> download_file(const uri& absolute_uri, const http_client_config& config,
        const http_progress_handlers& progress_handlers, const http_header_handler& header_handler = {});
} // namespace essence::net
