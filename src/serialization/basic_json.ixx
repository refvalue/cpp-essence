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

module;

#include <essence/compat.hpp>

export module essence.serialization:basic_json;
import lib.nlohmann_json;

export namespace essence {
    using nlohmann::adl_serializer;
    using nlohmann::basic_json;
    using nlohmann::json_pointer;
    using nlohmann::detail::is_basic_json;
    using nlohmann::detail::is_basic_json_context;
    using nlohmann::detail::is_compatible_type;
    using nlohmann_json    = nlohmann::json;
    using json_parse_error = nlohmann::detail::parse_error;
    using nlohmann::detail::concat;

    ES_API(CPPESSENCE) constexpr auto&& nlohmann_from_json = nlohmann::from_json;
    ES_API(CPPESSENCE) constexpr auto&& nlohmann_to_json   = nlohmann::to_json;
} // namespace essence
