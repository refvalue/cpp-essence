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

export module essence.i18n:format_extensions;
import :localized_arg;
import essence.basic;
import std;

export namespace essence::i18n {
    /**
     * Formats a group of arguments with a format string.
     *        All string-view-like arguments are translated to corresponding localized texts if available.
     * @tparam Args The types of the arguments.
     * @param locale The user-specified locale.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <typename... Args>
    std::string format(const std::locale& locale, format_string<localized_arg_t<Args>...> fmt, Args&&... args) {
        return format(locale, fmt, i18n::make_localized_arg(locale, std::forward<Args>(args))...);
    }

    /**
     * Formats a group of arguments with a format string.
     *        All string-view-like arguments are translated to corresponding localized texts if available.
     *        The current global locale is used.
     * @tparam Args The types of the arguments.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <typename... Args>
    std::string format(format_string<localized_arg_t<Args>...> fmt, Args&&... args) {
        return format(std::locale{}, fmt, std::forward<Args>(args)...);
    }

    /**
     * Formats a group of arguments with a format string to a std::basic_string<>.
     *        All string-view-like arguments are translated to corresponding localized texts if available.
     * @tparam S The concrete type of the std::basic_string<>.
     * @tparam Args The types of the arguments.
     * @param locale The user-specified locale.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <std_basic_string S = std::string, typename... Args>
    S format_as(const std::locale& locale, format_string<localized_arg_t<Args>...> fmt, Args&&... args)
        requires requires(S str) {
            format_to(std::back_inserter(str), fmt, i18n::make_localized_arg(locale, std::forward<Args>(args))...);
        }
    {
        return format_as<S>(locale, fmt, i18n::make_localized_arg(locale, std::forward<Args>(args))...);
    }

    /**
     * Formats a group of arguments with a format string to a std::basic_string<>.
     *        All string-view-like arguments are translated to corresponding localized texts if available.
     *        The current global locale is used.
     * @tparam S The concrete type of the std::basic_string<>.
     * @tparam Args The types of the arguments.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <std_basic_string S = std::string, typename... Args>
    S format_as(format_string<localized_arg_t<Args>...> fmt, Args&&... args)
        requires requires(S str) {
            format_to(std::back_inserter(str), fmt,
                i18n::make_localized_arg(std::declval<std::locale>(), std::forward<Args>(args))...);
        }
    {
        return format_as<S>(std::locale{}, fmt, std::forward<Args>(args)...);
    }
} // namespace essence::i18n
