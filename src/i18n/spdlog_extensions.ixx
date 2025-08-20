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

export module essence.i18n:spdlog_extensions;

import std;
import :localized_arg;
import essence.basic;

namespace spdlog::i18n::detail {
    constexpr auto spdlog_info_func = []<typename... Args>(
                                          Args&&... args) { spdlog::info(std::forward<Args>(args)...); };

    constexpr auto spdlog_trace_func = []<typename... Args>(
                                           Args&&... args) { spdlog::trace(std::forward<Args>(args)...); };

    constexpr auto spdlog_warn_func = []<typename... Args>(
                                          Args&&... args) { spdlog::warn(std::forward<Args>(args)...); };

    constexpr auto spdlog_error_func = []<typename... Args>(
                                           Args&&... args) { spdlog::error(std::forward<Args>(args)...); };

    template <auto LogArgsFunc, typename... Args>
    void log_args(
        const std::locale& locale, format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        LogArgsFunc(fmt, essence::i18n::make_localized_arg(locale, std::forward<Args>(args))...);
    }

    template <auto LogOneFunc, typename T>
    void log_one(const std::locale& locale, const T& msg) {
        LogOneFunc(essence::i18n::make_localized_arg(locale, msg));
    }
} // namespace spdlog::i18n::detail

export namespace spdlog::i18n {
    template <typename T>
    void info(const std::locale& locale, const T& msg) {
        detail::log_one<detail::spdlog_info_func>(locale, msg);
    }

    template <typename... Args>
    void info(const std::locale& locale, format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        detail::log_args<detail::spdlog_info_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void info(const T& msg) {
        info({}, msg);
    }

    template <typename... Args>
    void info(format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        info({}, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void trace(const std::locale& locale, const T& msg) {
        detail::log_one<detail::spdlog_trace_func>(locale, msg);
    }

    template <typename... Args>
    void trace(
        const std::locale& locale, format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        detail::log_args<detail::spdlog_trace_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void trace(const T& msg) {
        trace({}, msg);
    }

    template <typename... Args>
    void trace(format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        trace({}, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void warn(const std::locale& locale, const T& msg) {
        detail::log_one<detail::spdlog_warn_func>(locale, msg);
    }

    template <typename... Args>
    void warn(const std::locale& locale, format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        detail::log_args<detail::spdlog_warn_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void warn(const T& msg) {
        warn({}, msg);
    }

    template <typename... Args>
    void warn(format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        warn({}, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void error(const std::locale& locale, const T& msg) {
        detail::log_one<detail::spdlog_error_func>(locale, msg);
    }

    template <typename... Args>
    void error(
        const std::locale& locale, format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        detail::log_args<detail::spdlog_error_func>(locale, fmt, std::forward<Args>(args)...);
    }

    template <typename T>
    void error(const T& msg) {
        error({}, msg);
    }

    template <typename... Args>
    void error(format_string_t<essence::i18n::localized_arg_t<Args>...> fmt, Args&&... args) {
        error({}, fmt, std::forward<Args>(args)...);
    }
} // namespace spdlog::i18n
