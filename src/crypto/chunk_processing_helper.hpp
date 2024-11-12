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

#include "char8_t_remediation.hpp"

#include <cstddef>
#include <cstdint>
#include <functional>
#include <span>
#include <string_view>

namespace essence::crypto {
    template <typename T, typename R = std::int32_t>
    struct chunk_processing_helper {
        std::int32_t (*raw_update)(
            T* ctx, std::uint8_t* out, std::int32_t* outl, const std::uint8_t* in, std::int32_t inl);

        R (*raw_finalize)(T* ctx, std::uint8_t* out, std::int32_t* outl);

        std::function<void(std::int32_t code, std::string_view message)> check_error;

        void update(T* context, std::span<const std::byte> input, std::span<std::byte>& output) const {
            auto size = static_cast<std::int32_t>(output.size());

            check_error(
                raw_update(context, reinterpret_cast<std::uint8_t*>(output.data()), &size,
                    reinterpret_cast<const std::uint8_t*>(input.data()), static_cast<std::int32_t>(input.size())),
                U8("An error occurred during the update."));

            output = output.subspan(0, size);
        }

        void finalize(T* context, std::span<std::byte>& output) const {
            auto size   = static_cast<std::int32_t>(output.size());
            auto result = [&] {
                if constexpr (std::is_void_v<R>) {
                    // Here one is for success.
                    return (raw_finalize(context, reinterpret_cast<std::uint8_t*>(output.data()), &size), 1);
                } else {
                    return raw_finalize(context, reinterpret_cast<std::uint8_t*>(output.data()), &size);
                }
            }();

            check_error(result, U8("An error occurred during the finalization."));
            output = output.subspan(0, size);
        }
    };
} // namespace essence::crypto
