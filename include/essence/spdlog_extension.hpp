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

#include "globalization/globalized_arg.hpp"

#include <utility>

#include <spdlog/spdlog.h>

namespace spdlog::detail {
    inline constexpr auto spdlog_info_func = []<typename... Args>(
                                                 Args&&... args) { spdlog::info(std::forward<Args>(args)...); };

    inline constexpr auto spdlog_trace_func = []<typename... Args>(
                                                  Args&&... args) { spdlog::trace(std::forward<Args>(args)...); };

    inline constexpr auto spdlog_warn_func = []<typename... Args>(
                                                 Args&&... args) { spdlog::warn(std::forward<Args>(args)...); };

    inline constexpr auto spdlog_error_func = []<typename... Args>(
                                                  Args&&... args) { spdlog::error(std::forward<Args>(args)...); };

    template <auto LogArgsFunc, typename... Args>
    void glog_args(const std::locale& locale, format_string_t<essence::globalization::globalized_arg_t<Args>...> fmt,
        Args&&... args) {
        LogArgsFunc(fmt, essence::globalization::make_globalized_arg(locale, std::forward<Args>(args))...);
    }

    template <auto LogOneFunc, typename T>
    void glog_one(const std::locale& locale, const T& msg) {
        LogOneFunc(essence::globalization::make_globalized_arg(locale, msg));
    }
} // namespace spdlog::detail

namespace spdlog {
    template <typename... Args>
    void ginfo(const std::locale& locale, format_string_t<essence::globalization::globalized_arg_t<Args>...> fmt,
        Args&&... args) {
        detail::glog_args<detail::spdlog_info_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void ginfo(const std::locale& locale, const T& msg) {
        detail::glog_one<detail::spdlog_info_func>(locale, msg);
    }

    template <typename... Args>
    void gtrace(const std::locale& locale, format_string_t<essence::globalization::globalized_arg_t<Args>...> fmt,
        Args&&... args) {
        detail::glog_args<detail::spdlog_trace_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void gtrace(const std::locale& locale, const T& msg) {
        detail::glog_one<detail::spdlog_trace_func>(locale, msg);
    }

    template <typename... Args>
    void gwarn(const std::locale& locale, format_string_t<essence::globalization::globalized_arg_t<Args>...> fmt,
        Args&&... args) {
        detail::glog_args<detail::spdlog_warn_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void gwarn(const std::locale& locale, const T& msg) {
        detail::glog_one<detail::spdlog_warn_func>(locale, msg);
    }

    template <typename... Args>
    void gerror(const std::locale& locale, format_string_t<essence::globalization::globalized_arg_t<Args>...> fmt,
        Args&&... args) {
        detail::glog_args<detail::spdlog_error_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void gerror(const std::locale& locale, const T& msg) {
        detail::glog_one<detail::spdlog_error_func>(locale, msg);
    }
} // namespace spdlog
