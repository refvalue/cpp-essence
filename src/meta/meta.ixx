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

export module essence.meta;

export import :detail.data_member_binding;
export import :detail.data_member_matching;
export import :detail.extract_keyword;
export import :detail.extraction_param;
export import :detail.fake_object_wrapper;
export import :detail.find_at_depth;
export import :detail.get_signature_suffix_size;
export import :detail.identifier;
export import :detail.language_tokens;
export import :detail.naming_convention;
export import :detail.parse_data_member_name;
export import :parse_qualified_function_name;
export import :detail.parse_raw_identifier_name;

export import :boolean;
export import :common_types;
export import :enumeration;
export import :fingerprint;
export import :friendly_name_base;
export import :friendly_name_vector;
export import :identifier;
export import :identifier_param;
export import :literal_string;
export import :literal_string_util;
export import :naming_convention;
export import :string_constant;
export import :structure;
export import :trivial_pair;

export import :runtime.boolean;
export import :runtime.enumeration;
export import :runtime.structure;
