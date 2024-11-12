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

#include "../abi/json.hpp"
#include "../compat.hpp"
#include "common_types.hpp"
#include "http_client_abstract.hpp"
#include "http_client_config.hpp"
#include "http_headers_proxy.hpp"
#include "uri.hpp"

#include <cstddef>
#include <memory>
#include <span>

namespace essence::net {
    /**
     * @brief An HTTP client for JSON input and output.
     */
    class http_client : public http_client_abstract<http_client> {
    public:
        ES_API(CPPESSENCE) explicit http_client(const uri& base_uri);
        ES_API(CPPESSENCE) http_client(const uri& base_uri, const http_client_config& config);
        ES_API(CPPESSENCE) http_client(http_client&&) noexcept;
        ES_API(CPPESSENCE) ~http_client();
        ES_API(CPPESSENCE) http_client& operator=(http_client&&) noexcept;
        [[nodiscard]] ES_API(CPPESSENCE) const uri& base_uri() const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::json commit_json_nop(
            http_method method, const uri& relative_uri, const http_header_handler& header_handler = {}) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::json commit_json(http_method method, const uri& relative_uri,
            const abi::json& params, const http_header_handler& header_handler = {}) const;
        [[nodiscard]] ES_API(CPPESSENCE) abi::json
            commit_bytes(http_method method, const uri& relative_uri, std::string_view content_type,
                std::span<const std::byte> bytes, const http_header_handler& header_handler = {}) const;
        ES_API(CPPESSENCE) void on_progress(const http_progress_handler& handler) const;
        ES_API(CPPESSENCE) void on_percentage(const http_percentage_handler& handler) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::net
