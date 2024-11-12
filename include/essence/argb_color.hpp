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

#include <cstdint>
#include <limits>
#include <type_traits>

namespace essence {
    /**
     * @brief Describes a ARGB color.
     */
    struct alignas(std::uint32_t) argb_color {
        std::uint8_t r{};
        std::uint8_t g{};
        std::uint8_t b{};
        std::uint8_t alpha{0xFF};

        /**
         * @brief Converts a 32-bit unsigned integer.
         */
        constexpr explicit operator std::uint32_t() const noexcept {
            return (static_cast<std::uint32_t>(alpha) << 24) + (static_cast<std::uint32_t>(b) << 16)
                 + (static_cast<std::uint32_t>(g) << 8) + r;
        }

        /**
         * @brief Normalizes the data to a new range.
         * @tparam T The type of the data in the new range.
         * @param min The lower bound.
         * @param max The upper bound.
         * @return The normalized color.
         */
        template <typename T>
            requires std::is_arithmetic_v<T>
        constexpr auto normalize(T min, T max) const noexcept {
            constexpr auto source_range =
                std::numeric_limits<std::uint8_t>::max() - std::numeric_limits<std::uint8_t>::min();
            auto convert = [&](std::uint8_t value) { return value * (max - min) / static_cast<T>(source_range) + min; };

            struct normalized_color {
                T r;
                T g;
                T b;
                T alpha;
            } result{convert(r), convert(g), convert(b), convert(alpha)};

            return result;
        }
    };

    /**
     * @brief Constructs an ARGB color from a 32-bit unsigned integer.
     * @param color The color in a 32-bit unsigned integer.
     * @return ARGB color.
     */
    constexpr argb_color make_argb_color(std::uint32_t color) noexcept {
        return argb_color{
            .r     = static_cast<std::uint8_t>(color),
            .g     = static_cast<std::uint8_t>(color >> 8),
            .b     = static_cast<std::uint8_t>(color >> 16),
            .alpha = static_cast<std::uint8_t>(color >> 24),
        };
    }

    /**
     * @brief Common predefined colors.
     * @remark http://www.flounder.com/csharp_color_table.htm
     */
    struct argb_colors {
        static constexpr argb_color alice_blue{
            .r = 240,
            .g = 248,
            .b = 255,
        };

        static constexpr argb_color light_salmon{
            .r = 255,
            .g = 160,
            .b = 122,
        };

        static constexpr argb_color antique_white{
            .r = 250,
            .g = 235,
            .b = 215,
        };

        static constexpr argb_color light_sea_green{
            .r = 32,
            .g = 178,
            .b = 170,
        };

        static constexpr argb_color aqua{
            .r = 0,
            .g = 255,
            .b = 255,
        };

        static constexpr argb_color light_sky_blue{
            .r = 135,
            .g = 206,
            .b = 250,
        };

        static constexpr argb_color aquamarine{
            .r = 127,
            .g = 255,
            .b = 212,
        };

        static constexpr argb_color light_slate_gray{
            .r = 119,
            .g = 136,
            .b = 153,
        };

        static constexpr argb_color azure{
            .r = 240,
            .g = 255,
            .b = 255,
        };

        static constexpr argb_color light_steel_blue{
            .r = 176,
            .g = 196,
            .b = 222,
        };

        static constexpr argb_color beige{
            .r = 245,
            .g = 245,
            .b = 220,
        };

        static constexpr argb_color light_yellow{
            .r = 255,
            .g = 255,
            .b = 224,
        };

        static constexpr argb_color bisque{
            .r = 255,
            .g = 228,
            .b = 196,
        };

        static constexpr argb_color lime{
            .r = 0,
            .g = 255,
            .b = 0,
        };

        static constexpr argb_color black{
            .r = 0,
            .g = 0,
            .b = 0,
        };

        static constexpr argb_color lime_green{
            .r = 50,
            .g = 205,
            .b = 50,
        };

        static constexpr argb_color blanched_almond{
            .r = 255,
            .g = 255,
            .b = 205,
        };

        static constexpr argb_color linen{
            .r = 250,
            .g = 240,
            .b = 230,
        };

        static constexpr argb_color blue{
            .r = 0,
            .g = 0,
            .b = 255,
        };

        static constexpr argb_color magenta{
            .r = 255,
            .g = 0,
            .b = 255,
        };

        static constexpr argb_color blue_violet{
            .r = 138,
            .g = 43,
            .b = 226,
        };

        static constexpr argb_color maroon{
            .r = 128,
            .g = 0,
            .b = 0,
        };

        static constexpr argb_color brown{
            .r = 165,
            .g = 42,
            .b = 42,
        };

        static constexpr argb_color medium_aquamarine{
            .r = 102,
            .g = 205,
            .b = 170,
        };

        static constexpr argb_color burly_wood{
            .r = 222,
            .g = 184,
            .b = 135,
        };

        static constexpr argb_color medium_blue{
            .r = 0,
            .g = 0,
            .b = 205,
        };

        static constexpr argb_color cadet_blue{
            .r = 95,
            .g = 158,
            .b = 160,
        };

        static constexpr argb_color medium_orchid{
            .r = 186,
            .g = 85,
            .b = 211,
        };

        static constexpr argb_color chartreuse{
            .r = 127,
            .g = 255,
            .b = 0,
        };

        static constexpr argb_color medium_purple{
            .r = 147,
            .g = 112,
            .b = 219,
        };

        static constexpr argb_color chocolate{
            .r = 210,
            .g = 105,
            .b = 30,
        };

        static constexpr argb_color medium_sea_green{
            .r = 60,
            .g = 179,
            .b = 113,
        };

        static constexpr argb_color coral{
            .r = 255,
            .g = 127,
            .b = 80,
        };

        static constexpr argb_color medium_slate_blue{
            .r = 123,
            .g = 104,
            .b = 238,
        };

        static constexpr argb_color cornflower_blue{
            .r = 100,
            .g = 149,
            .b = 237,
        };

        static constexpr argb_color medium_spring_green{
            .r = 0,
            .g = 250,
            .b = 154,
        };

        static constexpr argb_color cornsilk{
            .r = 255,
            .g = 248,
            .b = 220,
        };

        static constexpr argb_color medium_turquoise{
            .r = 72,
            .g = 209,
            .b = 204,
        };

        static constexpr argb_color crimson{
            .r = 220,
            .g = 20,
            .b = 60,
        };

        static constexpr argb_color medium_violet_red{
            .r = 199,
            .g = 21,
            .b = 112,
        };

        static constexpr argb_color cyan{
            .r = 0,
            .g = 255,
            .b = 255,
        };

        static constexpr argb_color midnight_blue{
            .r = 25,
            .g = 25,
            .b = 112,
        };

        static constexpr argb_color dark_blue{
            .r = 0,
            .g = 0,
            .b = 139,
        };

        static constexpr argb_color mint_cream{
            .r = 245,
            .g = 255,
            .b = 250,
        };

        static constexpr argb_color dark_cyan{
            .r = 0,
            .g = 139,
            .b = 139,
        };

        static constexpr argb_color misty_rose{
            .r = 255,
            .g = 228,
            .b = 225,
        };

        static constexpr argb_color dark_goldenrod{
            .r = 184,
            .g = 134,
            .b = 11,
        };

        static constexpr argb_color moccasin{
            .r = 255,
            .g = 228,
            .b = 181,
        };

        static constexpr argb_color dark_gray{
            .r = 169,
            .g = 169,
            .b = 169,
        };

        static constexpr argb_color navajo_white{
            .r = 255,
            .g = 222,
            .b = 173,
        };

        static constexpr argb_color dark_green{
            .r = 0,
            .g = 100,
            .b = 0,
        };

        static constexpr argb_color navy{
            .r = 0,
            .g = 0,
            .b = 128,
        };

        static constexpr argb_color dark_khaki{
            .r = 189,
            .g = 183,
            .b = 107,
        };

        static constexpr argb_color old_lace{
            .r = 253,
            .g = 245,
            .b = 230,
        };

        static constexpr argb_color dark_magena{
            .r = 139,
            .g = 0,
            .b = 139,
        };

        static constexpr argb_color olive{
            .r = 128,
            .g = 128,
            .b = 0,
        };

        static constexpr argb_color dark_olive_green{
            .r = 85,
            .g = 107,
            .b = 47,
        };

        static constexpr argb_color olive_drab{
            .r = 107,
            .g = 142,
            .b = 45,
        };

        static constexpr argb_color dark_orange{
            .r = 255,
            .g = 140,
            .b = 0,
        };

        static constexpr argb_color orange{
            .r = 255,
            .g = 165,
            .b = 0,
        };

        static constexpr argb_color dark_orchid{
            .r = 153,
            .g = 50,
            .b = 204,
        };

        static constexpr argb_color orange_red{
            .r = 255,
            .g = 69,
            .b = 0,
        };

        static constexpr argb_color dark_red{
            .r = 139,
            .g = 0,
            .b = 0,
        };

        static constexpr argb_color orchid{
            .r = 218,
            .g = 112,
            .b = 214,
        };

        static constexpr argb_color dark_salmon{
            .r = 233,
            .g = 150,
            .b = 122,
        };

        static constexpr argb_color pale_goldenrod{
            .r = 238,
            .g = 232,
            .b = 170,
        };

        static constexpr argb_color dark_sea_green{
            .r = 143,
            .g = 188,
            .b = 143,
        };

        static constexpr argb_color pale_green{
            .r = 152,
            .g = 251,
            .b = 152,
        };

        static constexpr argb_color dark_slate_blue{
            .r = 72,
            .g = 61,
            .b = 139,
        };

        static constexpr argb_color pale_turquoise{
            .r = 175,
            .g = 238,
            .b = 238,
        };

        static constexpr argb_color dark_slate_gray{
            .r = 40,
            .g = 79,
            .b = 79,
        };

        static constexpr argb_color pale_violet_red{
            .r = 219,
            .g = 112,
            .b = 147,
        };

        static constexpr argb_color dark_turquoise{
            .r = 0,
            .g = 206,
            .b = 209,
        };

        static constexpr argb_color papaya_whip{
            .r = 255,
            .g = 239,
            .b = 213,
        };

        static constexpr argb_color dark_violet{
            .r = 148,
            .g = 0,
            .b = 211,
        };

        static constexpr argb_color peach_puff{
            .r = 255,
            .g = 218,
            .b = 155,
        };

        static constexpr argb_color deep_pink{
            .r = 255,
            .g = 20,
            .b = 147,
        };

        static constexpr argb_color peru{
            .r = 205,
            .g = 133,
            .b = 63,
        };

        static constexpr argb_color deep_sky_blue{
            .r = 0,
            .g = 191,
            .b = 255,
        };

        static constexpr argb_color pink{
            .r = 255,
            .g = 192,
            .b = 203,
        };

        static constexpr argb_color dim_gray{
            .r = 105,
            .g = 105,
            .b = 105,
        };

        static constexpr argb_color plum{
            .r = 221,
            .g = 160,
            .b = 221,
        };

        static constexpr argb_color dodger_blue{
            .r = 30,
            .g = 144,
            .b = 255,
        };

        static constexpr argb_color powder_blue{
            .r = 176,
            .g = 224,
            .b = 230,
        };

        static constexpr argb_color firebrick{
            .r = 178,
            .g = 34,
            .b = 34,
        };

        static constexpr argb_color purple{
            .r = 128,
            .g = 0,
            .b = 128,
        };

        static constexpr argb_color floral_white{
            .r = 255,
            .g = 250,
            .b = 240,
        };

        static constexpr argb_color red{
            .r = 255,
            .g = 0,
            .b = 0,
        };

        static constexpr argb_color forest_green{
            .r = 34,
            .g = 139,
            .b = 34,
        };

        static constexpr argb_color rosy_brown{
            .r = 188,
            .g = 143,
            .b = 143,
        };

        static constexpr argb_color fuschia{
            .r = 255,
            .g = 0,
            .b = 255,
        };

        static constexpr argb_color royal_blue{
            .r = 65,
            .g = 105,
            .b = 225,
        };

        static constexpr argb_color gainsboro{
            .r = 220,
            .g = 220,
            .b = 220,
        };

        static constexpr argb_color saddle_brown{
            .r = 139,
            .g = 69,
            .b = 19,
        };

        static constexpr argb_color ghost_white{
            .r = 248,
            .g = 248,
            .b = 255,
        };

        static constexpr argb_color salmon{
            .r = 250,
            .g = 128,
            .b = 114,
        };

        static constexpr argb_color gold{
            .r = 255,
            .g = 215,
            .b = 0,
        };

        static constexpr argb_color sandy_brown{
            .r = 244,
            .g = 164,
            .b = 96,
        };

        static constexpr argb_color goldenrod{
            .r = 218,
            .g = 165,
            .b = 32,
        };

        static constexpr argb_color sea_green{
            .r = 46,
            .g = 139,
            .b = 87,
        };

        static constexpr argb_color gray{
            .r = 128,
            .g = 128,
            .b = 128,
        };

        static constexpr argb_color seashell{
            .r = 255,
            .g = 245,
            .b = 238,
        };

        static constexpr argb_color green{
            .r = 0,
            .g = 128,
            .b = 0,
        };

        static constexpr argb_color sienna{
            .r = 160,
            .g = 82,
            .b = 45,
        };

        static constexpr argb_color green_yellow{
            .r = 173,
            .g = 255,
            .b = 47,
        };

        static constexpr argb_color silver{
            .r = 192,
            .g = 192,
            .b = 192,
        };

        static constexpr argb_color honeydew{
            .r = 240,
            .g = 255,
            .b = 240,
        };

        static constexpr argb_color sky_blue{
            .r = 135,
            .g = 206,
            .b = 235,
        };

        static constexpr argb_color hot_pink{
            .r = 255,
            .g = 105,
            .b = 180,
        };

        static constexpr argb_color slate_blue{
            .r = 106,
            .g = 90,
            .b = 205,
        };

        static constexpr argb_color indian_red{
            .r = 205,
            .g = 92,
            .b = 92,
        };

        static constexpr argb_color slate_gray{
            .r = 112,
            .g = 128,
            .b = 144,
        };

        static constexpr argb_color indigo{
            .r = 75,
            .g = 0,
            .b = 130,
        };

        static constexpr argb_color snow{
            .r = 255,
            .g = 250,
            .b = 250,
        };

        static constexpr argb_color ivory{
            .r = 255,
            .g = 240,
            .b = 240,
        };

        static constexpr argb_color spring_green{
            .r = 0,
            .g = 255,
            .b = 127,
        };

        static constexpr argb_color khaki{
            .r = 240,
            .g = 230,
            .b = 140,
        };

        static constexpr argb_color steel_blue{
            .r = 70,
            .g = 130,
            .b = 180,
        };

        static constexpr argb_color lavender{
            .r = 230,
            .g = 230,
            .b = 250,
        };

        static constexpr argb_color tan{
            .r = 210,
            .g = 180,
            .b = 140,
        };

        static constexpr argb_color lavender_blush{
            .r = 255,
            .g = 240,
            .b = 245,
        };

        static constexpr argb_color teal{
            .r = 0,
            .g = 128,
            .b = 128,
        };

        static constexpr argb_color lawn_green{
            .r = 124,
            .g = 252,
            .b = 0,
        };

        static constexpr argb_color thistle{
            .r = 216,
            .g = 191,
            .b = 216,
        };

        static constexpr argb_color lemon_chiffon{
            .r = 255,
            .g = 250,
            .b = 205,
        };

        static constexpr argb_color tomato{
            .r = 253,
            .g = 99,
            .b = 71,
        };

        static constexpr argb_color light_blue{
            .r = 173,
            .g = 216,
            .b = 230,
        };

        static constexpr argb_color turquoise{
            .r = 64,
            .g = 224,
            .b = 208,
        };

        static constexpr argb_color light_coral{
            .r = 240,
            .g = 128,
            .b = 128,
        };

        static constexpr argb_color violet{
            .r = 238,
            .g = 130,
            .b = 238,
        };

        static constexpr argb_color light_cyan{
            .r = 224,
            .g = 255,
            .b = 255,
        };

        static constexpr argb_color wheat{
            .r = 245,
            .g = 222,
            .b = 179,
        };

        static constexpr argb_color light_goldenrod_yellow{
            .r = 250,
            .g = 250,
            .b = 210,
        };

        static constexpr argb_color white{
            .r = 255,
            .g = 255,
            .b = 255,
        };

        static constexpr argb_color light_green{
            .r = 144,
            .g = 238,
            .b = 144,
        };

        static constexpr argb_color white_smoke{
            .r = 245,
            .g = 245,
            .b = 245,
        };

        static constexpr argb_color light_gray{
            .r = 211,
            .g = 211,
            .b = 211,
        };

        static constexpr argb_color yellow{
            .r = 255,
            .g = 255,
            .b = 0,
        };

        static constexpr argb_color light_pink{
            .r = 255,
            .g = 182,
            .b = 193,
        };

        static constexpr argb_color yellow_green{
            .r = 154,
            .g = 205,
            .b = 50,
        };
    };
} // namespace essence
