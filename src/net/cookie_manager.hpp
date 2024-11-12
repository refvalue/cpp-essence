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

#include <memory>
#include <string>
#include <string_view>

namespace essence::net {
    /**
     * @brief Manages cookies automatically.
     */
    class cookie_manager {
    public:
        cookie_manager();
        cookie_manager(cookie_manager&&) noexcept;
        ~cookie_manager();
        cookie_manager& operator=(cookie_manager&&) noexcept;

        /**
         * @brief Parses a set-cookie string and adds the cookies.
         * @param set_cookie The set-cookie string.
         */
        void parse_add(std::string_view set_cookie) const;

        /**
         * @brief Gets the cookies for a path.
         * @param path The path of the cookie.
         * @return The cookie string.
         */
        std::string get_for(std::string_view path) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::net
