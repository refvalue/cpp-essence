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

#include "net/uri.hpp"

#include "encoding.hpp"

#include <cpprest/uri.h>

namespace essence::net {
    class uri::impl {
    public:
        impl() = default;

        explicit impl(std::string_view uri_str) : uri_{internal::to_native_string(uri_str)} {}

        impl(const impl&) = default;

        web::uri& uri() noexcept {
            return uri_;
        }

    private:
        web::uri uri_;
    };

    uri::uri() : impl_{std::make_unique<impl>()} {}

    uri::uri(std::string_view uri_str) : impl_{std::make_unique<impl>(uri_str)} {}

    uri::uri(const uri& other) : impl_{std::make_unique<impl>(*other.impl_)} {}

    uri::uri(uri&&) noexcept = default;

    uri::~uri() = default;

    uri& uri::operator=(const uri& right) {
        impl_ = std::make_unique<impl>(*right.impl_);

        return *this;
    }

    uri& uri::operator=(uri&&) noexcept = default;

    bool uri::operator==(const uri& right) const {
        return impl_->uri() == right.impl_->uri();
    }

    bool uri::operator!=(const uri& right) const {
        return !(*this == right);
    }

    abi::string uri::str() const {
        return to_utf8_string(impl_->uri().to_string());
    }

    abi::nstring uri::native_str() const {
        return abi::to_abi_string(impl_->uri().to_string());
    }

    bool uri::empty() const {
        return impl_->uri().is_empty();
    }

    abi::string uri::scheme() const {
        return to_utf8_string(impl_->uri().scheme());
    }

    abi::string uri::user_info() const {
        return to_utf8_string(impl_->uri().user_info());
    }

    abi::string uri::host() const {
        return to_utf8_string(impl_->uri().host());
    }

    std::int32_t uri::port() const {
        return impl_->uri().port();
    }

    abi::string uri::path() const {
        return to_utf8_string(impl_->uri().path());
    }

    abi::string uri::query() const {
        return to_utf8_string(impl_->uri().query());
    }

    abi::string uri::fragment() const {
        return to_utf8_string(impl_->uri().fragment());
    }

    abi::vector<abi::string> uri::split_path() const {
        abi::vector<abi::string> result;

        for (auto&& item : web::uri::split_path(impl_->uri().path())) {
            result.emplace_back(to_utf8_string(item));
        }

        return result;
    }

    abi::map<abi::string, abi::string> uri::split_query() const {
        abi::map<abi::string, abi::string> result;

        for (auto&& [key, value] : web::uri::split_query(impl_->uri().query())) {
            result.insert_or_assign(to_utf8_string(key), to_utf8_string(value));
        }

        return result;
    }

    abi::string uri::resolve_uri(std::string_view relative_uri) const {
        return to_utf8_string(impl_->uri().resolve_uri(internal::to_native_string(relative_uri)));
    }

    abi::string encode_uri_data_string(std::string_view str) {
        return to_utf8_string(web::uri::encode_data_string(internal::to_native_string(str)));
    }

    abi::string decode_uri_data_string(std::string_view str) {
        return to_utf8_string(web::uri::decode(internal::to_native_string(str)));
    }
} // namespace essence::net
