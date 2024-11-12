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

#include "cookie_manager.hpp"

#include "char8_t_remediation.hpp"

#include <mutex>
#include <regex>
#include <vector>

namespace essence::net {
    namespace {
        struct cookie_entry {
            std::string path;
            std::string key;
            std::string value;
        };
    } // namespace

    class cookie_manager::impl {
    public:
        void parse_add(std::string_view set_cookie) {
            thread_local const std::regex pattern{U8(R"((.+?)\=(.+?)[;]?)"),
                std::regex_constants::icase | std::regex_constants::ECMAScript | std::regex_constants::optimize};

            for (std::cregex_iterator iter{set_cookie.data(), set_cookie.data() + set_cookie.size(), pattern}, end;
                 iter != end; ++iter) {
                // TODO: implementation.
            }
        }

        std::string get_for(std::string_view path) const {
            // TODO: implementation.
            return {};
        }

    private:
        mutable std::mutex mutex_;
        std::vector<cookie_entry> cookies_;
    };

    cookie_manager::cookie_manager() : impl_{std::make_unique<impl>()} {}

    cookie_manager::cookie_manager(cookie_manager&&) noexcept = default;

    cookie_manager::~cookie_manager() = default;

    cookie_manager& cookie_manager::operator=(cookie_manager&&) noexcept = default;

    void cookie_manager::parse_add(std::string_view set_cookie) const {
        impl_->parse_add(set_cookie);
    }

    std::string cookie_manager::get_for(std::string_view path) const {
        return impl_->get_for(path);
    }

} // namespace essence::net
