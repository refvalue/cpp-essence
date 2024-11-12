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

#include "argb_color.hpp"

#include <cstdint>

namespace essence {
    /**
     * @brief Describes a YUV color.
     */
    struct alignas(std::uint32_t) yuv_color {
        std::uint8_t y{};
        std::uint8_t u{};
        std::uint8_t v{};
    };

    /**
     * @brief Constructs a YUV color from an ARGB color.
     * @param color The RGB color omitting the alpha channel.
     * @return The YUV color.
     */
    constexpr yuv_color make_yuv_color(argb_color color) noexcept {
        return yuv_color{
            .y = static_cast<std::uint8_t>(0.257 * color.r + 0.504 * color.g + 0.098 * color.b + 16),
            .u = static_cast<std::uint8_t>(-0.148 * color.r - 0.291 * color.g + 0.439 * color.b + 128),
            .v = static_cast<std::uint8_t>(0.439 * color.r - 0.368 * color.g - 0.071 * color.b + 128),
        };
    }
} // namespace essence
