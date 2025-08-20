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

export module essence.imaging:abstract.image_header_extractor;
import :image_general_header;
import essence.io;
import std;

export namespace essence::imaging::abstract {
    /**
     * Extracts a general image header from image file data.
     */
    class image_header_extractor {
    public:
        template <typename T>
            requires(!std::same_as<std::decay_t<T>, image_header_extractor>)
        explicit image_header_extractor(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * Gets the underlying type hint.
         * @return The type hint.
         */
        [[nodiscard]] ES_API(CPPESSENCE) io::abstract::bitstream_type_hint hint() const {
            return wrapper_->hint();
        }

        /**
         * Extracts the general image header from a standard input stream.
         * @param stream The input stream.
         * @return The image header if succeeds; otherwise std::nullopt.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<image_general_header> get(std::istream& stream) const {
            return wrapper_->get(stream);
        }

        /**
         * Extracts the general image header from a memory buffer.
         * @param buffer The memory buffer.
         * @return The image header if succeeds; otherwise std::nullopt.
         */
        [[nodiscard]] ES_API(CPPESSENCE) std::optional<image_general_header> get(std::span<const std::byte> buffer) const {
            return wrapper_->get(buffer);
        }

    private:
        struct base {
            virtual ~base()                                                                    = default;
            virtual io::abstract::bitstream_type_hint hint()                                   = 0;
            virtual std::optional<image_general_header> get(std::istream& stream)              = 0;
            virtual std::optional<image_general_header> get(std::span<const std::byte> buffer) = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            io::abstract::bitstream_type_hint hint() override {
                return value_.hint();
            }

            std::optional<image_general_header> get(std::istream& stream) override {
                return value_.get(stream);
            }

            std::optional<image_general_header> get(std::span<const std::byte> buffer) override {
                return value_.get(buffer);
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::imaging::abstract
