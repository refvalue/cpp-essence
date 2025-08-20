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

export module essence.meta:parse_qualified_function_name;

import std;
import :detail.find_at_depth;
import :detail.language_tokens;
import :detail.parse_raw_identifier_name;
import :literal_string;
import essence.basic;

export namespace essence::meta::detail {
#if defined(_MSC_VER) && defined(_WIN64)
    constexpr std::array calling_conventions{"__cdecl", "__vectorcall"};
#elif defined(_MSC_VER) && defined(_WIN32)
    constexpr std::array calling_conventions{"__cdecl", "__stdcall", "__fastcall", "__vectorcall", "__thiscall"};
#elif defined(__clang__) || defined(__GNUC__)
    constexpr std::array<const char*, 0> calling_conventions{};
#else
#error "Unsupported compiler."
#endif

    /**
     * Parses a function signature into a friendly function name.
     * @param signature The signature.
     * @return The function name.
     */
    consteval std::string_view parse_qualified_function_name(std::string_view signature) noexcept {
        // When using MSVC, finds the start index of the function name past the
        // top-level calling convention token (e.g. __cdecl/__vectorcall for x64,
        // __stdcall/__cdecl/__fastcall/__vectorcall/__thiscall for x86).
        // When using GCC or Clang, returns zero immediately.
        const auto start_index = [&]() -> std::size_t {
            // Workaround for failing compilation on x86-64 Clang assertions trunk.
            // std::array<T, 0>::begin() and std::array<T, 0>::end() are not
            // odr-usable.
            if constexpr (!calling_conventions.empty()) {
                for (auto&& item : calling_conventions) {
                    if (const auto index = find_at_depth<find_mode_type::full_match>(signature, item);
                        index != std::string_view::npos) {
                        return index + 1;
                    }
                }
            }

            return 0;
        }();

        // Finds the end index of the function name before the top-level left
        // "<" or "(" indicating the start of the template or function arguments.
        const auto function_name_start_index =
            std::max(start_index, find_at_depth<find_mode_type::full_match_reverse>(signature, language_tokens::scope));

        const auto end_index = find_at_depth<find_mode_type::any_of>(signature,
            literal_string{language_tokens::left_parentheses, language_tokens::left_angle_bracket}, 1,
            function_name_start_index != std::string_view::npos ? function_name_start_index : start_index);

        const auto final_size =
            end_index != std::string_view::npos ? (end_index - 1 - start_index) : (signature.size() - start_index);

        return signature.substr(start_index, final_size);
    }
} // namespace essence::meta::detail
