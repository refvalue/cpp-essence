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

#include "util.hpp"

#include "char8_t_remediation.hpp"
#include "encoding.hpp"
#include "error_extensions.hpp"

#include <chrono>

namespace essence::net {
    namespace {
        template <typename Output, typename Input>
            requires requires(Input input, Output output, Output& output_lvref, std::chrono::years duration) {
                input.assign_to(output_lvref);
                output.set_timeout(duration);
            }
        Output make_cpprest_config(const Input& config, bool no_internal_timeout) {
            Output result;

            config.assign_to(result);

            if (no_internal_timeout) {
                // Timeout is disabled here that is implemented in the sse_client class.
                result.set_timeout(std::chrono::years{100});
            }

            return result;
        }
    } // namespace

    web::http::client::http_client_config make_cpprest_http_client_config(
        const http_client_config& config, bool no_internal_timeout) {
        return make_cpprest_config<web::http::client::http_client_config>(config, no_internal_timeout);
    }

    web::http::experimental::listener::http_listener_config make_cpprest_http_listener_config(
        const http_listener_config& config, bool no_internal_timeout) {
        return make_cpprest_config<web::http::experimental::listener::http_listener_config>(
            config, no_internal_timeout);
    }

    web::uri make_cpprest_uri(const uri& uri) {
        return web::uri{abi::from_abi_string(uri.native_str())};
    }

    uri make_uri(const web::uri& uri) {
        return net::uri{internal::to_utf8_string(uri.to_string())};
    }

    bool is_successful_status_code(web::http::status_code code) noexcept {
        using web::http::status_codes;

        return code == status_codes::OK || code == status_codes::Created || code == status_codes::NoContent;
    }

    void ensure_status_code(const web::http::http_response& response, const source_location& location) {
        if (!is_successful_status_code(response.status_code())) {
            throw source_code_aware_runtime_error{location, U8("HTTP Status Code"), response.status_code(),
                U8("Reason"), internal::to_utf8_string(response.reason_phrase())};
        }
    }


} // namespace essence::net
