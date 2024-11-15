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

#ifndef _MSC_VER
#include "../../char8_t_remediation.hpp"
#endif

#include "../identifier_param.hpp"
#include "../literal_string.hpp"
#include "extract_keyword.hpp"
#include "extraction_param.hpp"
#include "get_signature_suffix_size.hpp"
#include "language_tokens.hpp"

#include <cstddef>
#include <string_view>

#ifdef _MSC_VER
#include <tuple>
#endif

namespace essence::meta::detail {
    /**
     * @brief Gets a short identifier name from a raw name.
     * @param raw_name The raw name.
     * @return The short identifier name.
     */
    constexpr std::string_view get_short_identifier_name(std::string_view raw_name) noexcept {
        return extract_keyword<find_mode_type::full_match_reverse>(
            raw_name, language_tokens::scope, extraction_param{.ensure_correctness = false});
    }

    /**
     * @brief Trims the prefix of the signature of a helper function and returns the remaining part.
     * @param prefix The prefix to be trimmed away.
     * @param signature The signature of the helper function.
     * @param param The parameter for parsing the identifier.
     * @return The remaining part.
     */
    constexpr std::string_view parse_raw_identifier_name(
        std::string_view prefix, std::string_view signature, const identifier_param& param = {}) noexcept {
#ifdef _MSC_VER
        const auto keyword = prefix;
#elif defined(__llvm__) && defined(__clang__)
        const std::string_view keyword{param.type ? U8("[T = ") : U8("[Value = ")};
#elif defined(__GNUC__)
        const std::string_view keyword{param.type ? U8("[with T = ") : U8("[with auto Value = ")};
#else
#error "Unsupported compiler."
#endif

        const auto result = extract_keyword<find_mode_type::full_match>(signature, keyword,
            extraction_param{
                .preview_first_character = param.preview_first_character,
                .ensure_correctness      = param.ensure_correctness,
                .suffix_size             = get_signature_suffix_size(signature),
                .extra_size_func         = [](std::string_view str, std::size_t prefix_size) -> std::size_t {
#ifdef _MSC_VER
                    // Skips 'enum', 'class', 'struct' for MSVC.
                    return std::apply(
                        [&, size = std::size_t{}](const auto&... args) mutable {
                            return ((str.compare(prefix_size, args.size(), args) == 0 ? (size = args.size()) : 0U), ...,
                                size);
                        },
                        language_tokens::type_prefixes);
#else
                    // Skips the possible '&' token for GCC and Clang.
                    return str[prefix_size] == language_tokens::reference.front();
#endif
                },
            });

        if (param.shortened && !param.preview_first_character) {
            return get_short_identifier_name(result);
        }

        // ReSharper disable once CppDFALocalValueEscapesFunction
        return result;
    }
} // namespace essence::meta::detail
