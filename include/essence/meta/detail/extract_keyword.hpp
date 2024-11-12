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

#include "../../uniform_string_finder.hpp"
#include "extraction_param.hpp"

#include <string_view>

namespace essence::meta::detail {
    /**
     * @brief Extracts a substring by parsing a keyword from a literal string.
     * @tparam Mode The searching mode.
     * @param str The literal string.
     * @param keyword The keyword to be searched for.
     * @param param Extra parameters for extraction.
     * @return The substring.
     */
    template <find_mode_type Mode, typename Callable = extraction_param<>::extra_size_func_type>
    consteval std::string_view extract_keyword(
        std::string_view str, std::string_view keyword, const extraction_param<Callable>& param = {}) noexcept {
        const auto prefix_size =
            uniform_find_string<Mode>(str, keyword, string_finder_traits<Mode>::default_index, true);
        auto make_result = [&](std::string_view inner) {
            if (param.preview_first_character) {
                return inner.substr(0, 1);
            }

            return inner;
        };

        if (prefix_size == std::string_view::npos) {
            return param.ensure_correctness ? throw : make_result(str);
        }

        const auto extra_size = param.extra_size_func(str, prefix_size);
        const auto result =
            str.substr(prefix_size + extra_size, str.size() - prefix_size - extra_size - param.suffix_size);

        return make_result(result);
    }
} // namespace essence::meta::detail
