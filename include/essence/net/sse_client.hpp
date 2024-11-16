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
#include "sse_message.hpp"
#include "uri.hpp"

#include <memory>

namespace essence::net {
    /**
     * @brief An http client operating on Server-Sent Events.
     * @remark https://html.spec.whatwg.org/multipage/server-sent-events.html#server-sent-events
     */
    class sse_client {
    public:
        ES_API(CPPESSENCE) explicit sse_client(const uri& base_uri);
        ES_API(CPPESSENCE) sse_client(const uri& base_uri, const http_client_config& config);
        ES_API(CPPESSENCE) sse_client(sse_client&&) noexcept;
        ES_API(CPPESSENCE) ~sse_client();
        ES_API(CPPESSENCE) sse_client& operator=(sse_client&&) noexcept;

        /**
         * @brief Connects to the SSE service and starts consuming messages.
         * @param relative_uri The relative uri.
         */
        ES_API(CPPESSENCE) void connect(const uri& relative_uri) const;

        /**
         * @brief Closes the connection.
         */
        ES_API(CPPESSENCE) void close() const;

        /**
         * @brief Registers a message callback.
         * @param handler The message callback.
         */
        ES_API(CPPESSENCE) void on_message(const sse_message_handler& handler) const;

        /**
         * @brief Registers an error callback.
         * @param handler The error callback.
         */
        ES_API(CPPESSENCE) void on_error(const error_handler& handler) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::net
