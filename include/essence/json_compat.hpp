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

#include "json.hpp"
#include "meta/runtime/json_serializer.hpp"

#include <concepts>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace essence {
    using nlohmann::adl_serializer;
    using nlohmann::basic_json;
    using nlohmann::json_pointer;
    using json = basic_json<std::map, std::vector, std::string, bool, std::int64_t, std::uint64_t, double,
        std::allocator, meta::runtime::json_serializer>;

    template <typename T>
    concept json_serializable = requires(const json& json, T& value) {
        { json.get<T>() } -> std::same_as<T>;
        json.get_to(value);
        essence::json(value);
    };
} // namespace essence
