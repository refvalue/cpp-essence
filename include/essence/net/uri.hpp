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
#include "../abi/vector.hpp"
#include "../compat.hpp"

#include <concepts>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

namespace essence::net {
    class uri {
    public:
        ES_API(CPPESSENCE) uri();
        ES_API(CPPESSENCE) uri(std::string_view uri_str);

        template <std::convertible_to<std::string_view> S>
            requires(!std::same_as<std::decay_t<S>, std::string_view>)
        uri(S&& uri_str) : uri{std::string_view{std::forward<S>(uri_str)}} {}

        ES_API(CPPESSENCE) uri(const uri& other);
        ES_API(CPPESSENCE) uri(uri&&) noexcept;
        ES_API(CPPESSENCE) ~uri();
        ES_API(CPPESSENCE) uri& operator=(const uri& right);
        ES_API(CPPESSENCE) uri& operator=(uri&&) noexcept;
        ES_API(CPPESSENCE) bool operator==(const uri& right) const;
        ES_API(CPPESSENCE) bool operator!=(const uri& right) const;
        ES_API(CPPESSENCE) abi::string str() const;
        ES_API(CPPESSENCE) abi::nstring native_str() const;
        ES_API(CPPESSENCE) bool empty() const;
        ES_API(CPPESSENCE) abi::string scheme() const;
        ES_API(CPPESSENCE) abi::string user_info() const;
        ES_API(CPPESSENCE) abi::string host() const;
        ES_API(CPPESSENCE) std::int32_t port() const;
        ES_API(CPPESSENCE) abi::string path() const;
        ES_API(CPPESSENCE) abi::string query() const;
        ES_API(CPPESSENCE) abi::string fragment() const;
        ES_API(CPPESSENCE) abi::vector<abi::string> split_path() const;
        ES_API(CPPESSENCE) abi::map<abi::string, abi::string> split_query() const;
        ES_API(CPPESSENCE) abi::string resolve_uri(std::string_view relative_uri) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };

    ES_API(CPPESSENCE) abi::string encode_uri_data_string(std::string_view str);
    ES_API(CPPESSENCE) abi::string decode_uri_data_string(std::string_view str);
} // namespace essence::net
