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

#include "../compat.hpp"
#include "abstract/bitstream_type_hint.hpp"

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <optional>
#include <span>

namespace essence::io {
    /**
     * @brief Provides an ability to identify the exact type of bitstream.
     */
    class bitstream_type_judger {
    public:
        /**
         * @brief Creates an instance from well-defined type hints.
         * @param hints The type hints.
         */
        ES_API(CPPESSENCE) explicit bitstream_type_judger(std::span<const abstract::bitstream_type_hint> hints);

        ES_API(CPPESSENCE) bitstream_type_judger(bitstream_type_judger&&) noexcept;
        ES_API(CPPESSENCE) ~bitstream_type_judger();
        ES_API(CPPESSENCE) bitstream_type_judger& operator=(bitstream_type_judger&&) noexcept;

        /**
         * @brief Gets the type hints.
         * @return Type hints.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::span<const abstract::bitstream_type_hint> hints() const noexcept;

        /**
         * @brief Identifies the type of the bitstream from a file.
         * @param path The file path.
         * @return The corresponding type hint, or std::nullopt if not found.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<abstract::bitstream_type_hint> identify(
            std::string_view path) const;

        /**
         * @brief Identifies the type of the bitstream from a standard input stream.
         * @param stream The input stream.
         * @return The corresponding type hint, or std::nullopt if not found.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<abstract::bitstream_type_hint> identify(
            std::istream& stream) const;

        /**
         * @brief Identifies the type of the bitstream from a memory buffer.
         * @param buffer The memory buffer.
         * @return The corresponding type hint, or std::nullopt if not found.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<abstract::bitstream_type_hint> identify(
            std::span<const std::byte> buffer) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::io
