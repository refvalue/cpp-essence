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
#include "abstract/sse_connection.hpp"
#include "common_types.hpp"
#include "http_listener_config.hpp"
#include "uri.hpp"

#include <memory>

namespace essence::net {
    /**
     * @brief An HTTP server that implements Server-Sent Events.
     */
    class sse_server {
    public:
        ES_API(CPPESSENCE) explicit sse_server(const uri& listen_uri);
        ES_API(CPPESSENCE) sse_server(const uri& listen_uri, const http_listener_config& config);
        ES_API(CPPESSENCE) sse_server(sse_server&&) noexcept;
        ES_API(CPPESSENCE) ~sse_server();
        ES_API(CPPESSENCE) sse_server& operator=(sse_server&&) noexcept;

        /**
         * @brief Starts accepting requests.
         */
        ES_API(CPPESSENCE) void open() const;

        /**
         * @brief Closes all connections that invalidates all sse_connection objects.
         */
        ES_API(CPPESSENCE) void close() const;

        /**
         * @brief Registers a callback for handling an SSE connection.
         * @param handler The callback.
         */
        ES_API(CPPESSENCE) void on_connection(const sse_connection_handler& handler) const;

        /**
         * @brief Registers a error callback.
         * @param handler The error callback.
         */
        ES_API(CPPESSENCE) void on_error(const error_handler& handler) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::net
