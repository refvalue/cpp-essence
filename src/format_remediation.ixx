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

export module essence.basic:format_remediation;
import :basic_string;
import :zstring_view;
import std;

export namespace essence {
    using std::format;
    using std::format_string;
    using std::format_to;
    using std::make_format_args;
    using std::make_wformat_args;
    using std::vformat;
    using std::wformat_string;

    /**
     * Formats a group of arguments with a format string to a std::basic_string<>.
     * @tparam S The concrete type of the std::basic_string<>.
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
     * Formats a group of arguments with a format string to a std::basic_string<>.
     * @tparam S The concrete type of the std::basic_string<>.
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
} // namespace essence

export template <typename CharT, typename Traits>
struct std::formatter<essence::basic_zstring_view<CharT, Traits>, CharT>
    : formatter<std::basic_string_view<CharT, Traits>, CharT> {
    template <typename FormatContext>
    auto format(essence::basic_zstring_view<CharT, Traits> str, FormatContext& ctx) const {
        return formatter<std::basic_string_view<CharT, Traits>, CharT>::format(
            std::basic_string_view<CharT, Traits>{str}, ctx);
    }
};
