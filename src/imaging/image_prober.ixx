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

export module essence.imaging:image_prober;

import std;
import :abstract.image_header_extractor;
import :image_general_header;

export namespace essence::imaging {
    /**
     * Probes the type of image file and extracts fundamental information about it.
     */
    class image_prober {
    public:
        /**
         * Creates an instance from builtin image header extractors.
         */
        ES_API(CPPESSENCE) image_prober();

        /**
         * Creates an instance from builtin and additional image header extractors.
         * @param extra_extractors The extra extractors.
         */
        ES_API(CPPESSENCE) explicit image_prober(std::span<const abstract::image_header_extractor> extra_extractors);

        ES_API(CPPESSENCE) image_prober(image_prober&&) noexcept;
        ES_API(CPPESSENCE) ~image_prober();
        ES_API(CPPESSENCE) image_prober& operator=(image_prober&&) noexcept;

        /**
         * Gets the extractors.
         * @return The extractors.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::span<const abstract::image_header_extractor> extractors() const noexcept;

        /**
         * Extracts the general image header from a file.
         * @param path The file path.
         * @return The image header if succeeds; otherwise std::nullopt.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<image_general_header> extract_header(
            std::string_view path) const;

        /**
         * Extracts the general image header from a standard input stream.
         * @param stream The input stream.
         * @return The image header if succeeds; otherwise std::nullopt.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<image_general_header> extract_header(std::istream& stream) const;

        /**
         * Extracts the general image header from a memory buffer.
         * @param buffer The memory buffer.
         * @return The image header if succeeds; otherwise std::nullopt.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<image_general_header> extract_header(
            std::span<const std::byte> buffer) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::imaging
