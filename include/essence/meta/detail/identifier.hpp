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

#include "../../char8_t_remediation.hpp"
#include "parse_qualified_function_name.hpp"
#include "parse_raw_identifier_name.hpp"

#include <type_traits>

/*
 * With GCC 11, __PRETTY_FUNCTION__ provides more detailed information than
 * std::source_location::current().function_name().
 */
#ifdef _MSC_VER
#define ES_UNIFORM_FUNCSIG __FUNCSIG__
#elif defined(__GNUC__) || defined(__clang__)
#define ES_UNIFORM_FUNCSIG __PRETTY_FUNCTION__
#else
#error "Unsupported compiler."
#endif

namespace essence::meta::detail {
    /**
     * @brief Gets the literal string of a type.
     * @tparam T The type.
     * @tparam Param The parameter for parsing the identifier.
     * @return The literal string.
     */
    template <typename T, identifier_param Param = {}>
    consteval auto get_literal_string() noexcept {
        return parse_raw_identifier_name(U8("essence::meta::detail::get_literal_string<"), ES_UNIFORM_FUNCSIG,
            identifier_param{
                .ensure_correctness      = Param.ensure_correctness,
                .preview_first_character = Param.preview_first_character,
            });
    }

    /**
     * @brief Gets the literal string of a literal value attached to a concrete type.
     * @tparam Value The literal value.
     * @tparam T The type, to which the literal value is attached.
     * @tparam Param The parameter for parsing the identifier.
     * @return The literal string.
     */
    template <auto Value, typename T, identifier_param Param = {}>
    consteval auto get_literal_string() noexcept {
        return parse_raw_identifier_name(U8("essence::meta::detail::get_literal_string<"), ES_UNIFORM_FUNCSIG,
            identifier_param{
                .type                    = false,
                .ensure_correctness      = Param.ensure_correctness,
                .preview_first_character = Param.preview_first_character,
            });
    }

    /**
     * @brief Gets the short literal string of a type (without a namespace).
     * @tparam T The type.
     * @tparam Param The parameter for parsing the identifier.
     * @return The short literal string.
     */
    template <typename T, identifier_param Param = {}>
    consteval auto get_short_literal_string() noexcept {
        return parse_raw_identifier_name(U8("essence::meta::detail::get_short_literal_string<"), ES_UNIFORM_FUNCSIG,
            identifier_param{
                .shortened               = true,
                .ensure_correctness      = Param.ensure_correctness,
                .preview_first_character = Param.preview_first_character,
            });
    }

    /**
     * @brief Gets the short literal string of a literal value attached to a concrete type without a namespace.
     * @tparam Value The literal value.
     * @tparam T The type, to which the literal value is attached.
     * @tparam Param The parameter for parsing the identifier.
     * @return The short literal string.
     */
    template <auto Value, typename T, identifier_param Param = {}>
    consteval auto get_short_literal_string() noexcept {
        return parse_raw_identifier_name(U8("essence::meta::detail::get_short_literal_string<"), ES_UNIFORM_FUNCSIG,
            identifier_param{
                .type                    = false,
                .shortened               = true,
                .ensure_correctness      = Param.ensure_correctness,
                .preview_first_character = Param.preview_first_character,
            });
    }

    /**
     * @brief Gets the friendly name of a function without arguments.
     * @tparam Value The function pointer.
     * @tparam Param The parameter for parsing the identifier.
     * @return The friendly name.
     */
    template <auto Value, identifier_param Param = {}>
        requires(std::is_pointer_v<decltype(Value)> && std::is_function_v<std::remove_pointer_t<decltype(Value)>>)
    consteval auto get_function_name() noexcept {
        return parse_qualified_function_name(
            parse_raw_identifier_name(U8("essence::meta::detail::get_function_name<"), ES_UNIFORM_FUNCSIG,
                identifier_param{
                    .type                    = false,
                    .ensure_correctness      = Param.ensure_correctness,
                    .preview_first_character = Param.preview_first_character,
                }));
    }

    /**
     * @brief Gets the short friendly name of a function without arguments (and with the namespace part trimmed).
     * @tparam Value The function pointer.
     * @tparam Param The parameter for parsing the identifier.
     * @return The short friendly name.
     */
    template <auto Value, identifier_param Param = {}>
        requires(std::is_pointer_v<decltype(Value)> && std::is_function_v<std::remove_pointer_t<decltype(Value)>>)
    consteval auto get_short_function_name() noexcept {
        return get_short_identifier_name(get_function_name<Value, Param>());
    }
} // namespace essence::meta::detail
