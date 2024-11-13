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

#include "basic_string.hpp"
#include "globalization/globalized_arg.hpp"
#include "zstring_view.hpp"

#include <iterator>
#include <locale>
#include <string>
#include <string_view>
#include <utility>

#if __has_include(<version> )
#include <version>
#endif

#if __cpp_lib_format >= 201907L
#define ES_WITH_STD_FORMAT
#define ES_FMT_NS std
#include <format>
#else
#define ES_FMT_NS fmt
#include <fmt/format.h>
#include <fmt/xchar.h>
#endif

namespace essence {
#ifdef ES_WITH_STD_FORMAT
    using std::format;
    using std::format_string;
    using std::format_to;
    using std::make_format_args;
    using std::make_wformat_args;
    using std::vformat;
    using std::wformat_string;
#else
    using fmt::format;
    using fmt::format_string;
    using fmt::format_to;
    using fmt::make_format_args;
    using fmt::make_wformat_args;
    using fmt::vformat;
    using fmt::wformat_string;
#endif

    /**
     * @brief Formats a group of arguments with a format string.
     *        All string-view-like arguments are translated to corresponding globalized texts if available.
     * @tparam Args The types of the arguments.
     * @param locale The user-specified locale.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <typename... Args>
    std::string gformat(
        const std::locale& locale, format_string<globalization::globalized_arg_t<Args>...> fmt, Args&&... args) {
        return format(locale, fmt, globalization::make_globalized_arg(locale, std::forward<Args>(args))...);
    }

    /**
     * @brief Formats a group of arguments with a format string to a std::basic_string<>.
     * @tparam S The concere type of the std::basic_string<>.
     * @tparam Args The types of the arguments.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <std_basic_string S = std::string, typename... Args>
    S format_as(format_string<Args...> fmt, Args&&... args)
        requires requires(S str) { format_to(std::back_inserter(str), fmt, std::forward<Args>(args)...); }
    {
        S result;

        format_to(std::back_inserter(result), fmt, std::forward<Args>(args)...);
        result.shrink_to_fit();

        return result;
    }

    /**
     * @brief Formats a group of arguments with a format string to a std::basic_string<>.
     * @tparam S The concere type of the std::basic_string<>.
     * @tparam Args The types of the arguments.
     * @param locale The user-specified locale.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <std_basic_string S = std::string, typename... Args>
    S format_as(const std::locale& locale, format_string<Args...> fmt, Args&&... args)
        requires requires(S str) { format_to(std::back_inserter(str), fmt, std::forward<Args>(args)...); }
    {
        S result;

        format_to(std::back_inserter(result), locale, fmt, std::forward<Args>(args)...);
        result.shrink_to_fit();

        return result;
    }


    /**
     * @brief Formats a group of arguments with a format string to a std::basic_string<>.
     *        All string-view-like arguments are translated to corresponding globalized texts if available.
     * @tparam S The concere type of the std::basic_string<>.
     * @tparam Args The types of the arguments.
     * @param locale The user-specified locale.
     * @param fmt The format string.
     * @param args The arguments.
     * @return The formatted string.
     */
    template <std_basic_string S = std::string, typename... Args>
    S gformat_as(const std::locale& locale, format_string<globalization::globalized_arg_t<Args>...> fmt, Args&&... args)
        requires requires(S str) {
            format_to(
                std::back_inserter(str), fmt, globalization::make_globalized_arg(locale, std::forward<Args>(args))...);
        }
    {
        return format_as<S>(locale, fmt, globalization::make_globalized_arg(locale, std::forward<Args>(args))...);
    }
} // namespace essence

template <typename CharT, typename Traits>
struct ES_FMT_NS::formatter<essence::basic_zstring_view<CharT, Traits>, CharT>
    : formatter<std::basic_string_view<CharT, Traits>, CharT> {
    template <typename FormatContext>
    auto format(essence::basic_zstring_view<CharT, Traits> str, FormatContext& ctx) const {
        return formatter<std::basic_string_view<CharT, Traits>, CharT>::format(
            std::basic_string_view<CharT, Traits>{str}, ctx);
    }
};

// Provides compatibilities for spdlog which internally uses fmt::format instead of std::format.
#ifdef ES_WITH_STD_FORMAT
template <typename CharT, typename Traits>
struct fmt::formatter<essence::basic_zstring_view<CharT, Traits>, CharT>
    : formatter<std::basic_string_view<CharT, Traits>, CharT> {
    template <typename FormatContext>
    auto format(essence::basic_zstring_view<CharT, Traits> str, FormatContext& ctx) const {
        return formatter<std::basic_string_view<CharT, Traits>, CharT>::format(
            std::basic_string_view<CharT, Traits>{str}, ctx);
    }
};
#endif

#undef ES_FMT_NS
#undef ES_WITH_STD_FORMAT
