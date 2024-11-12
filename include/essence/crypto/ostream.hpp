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
#include "abstract/chunk_processor.hpp"

#include <istream>
#include <memory>
#include <string_view>

namespace essence::crypto {
    /**
     * @brief An output stream to transform input data by using a chunk processor(i.e. a chunk encryptor).
     */
    class ostream : public std::ostream {
    public:
        /**
         * @brief Creates an empty instance, of which the initialization is delayed when the open function is called.
         */
        ES_API(CPPESSENCE) ostream();


        /**
         * @brief Creates an instance.
         * @param output_stream The output stream to receive the transformed data.
         * @param processor The chunk processor.
         */
        ES_API(CPPESSENCE) ostream(std::shared_ptr<std::ostream> output_stream, abstract::chunk_processor processor);


        /**
         * @brief Creates an instance.
         * @param path The path of the output file into which the transformed data will be written.
         * @param processor The chunk processor.
         * @param mode The open mode of the output file.
         */
        ES_API(CPPESSENCE)
        ostream(std::string_view path, abstract::chunk_processor processor, openmode mode = out | binary);

        ostream(const ostream&)     = delete;
        ostream(ostream&&) noexcept = delete;
        ES_API(CPPESSENCE) ~ostream() override;
        ostream& operator=(const ostream&)     = delete;
        ostream& operator=(ostream&&) noexcept = delete;

        /**
         * @brief Checks whether the stream is open.
         * @return True if the stream is open; otherwise false.
         */
        [[nodiscard]] ES_API(CPPESSENCE) bool is_open() const noexcept;

        /**
         * @brief Resets all internal states and opens a new output stream to write the transformed data.
         * @param output_stream The output stream to receive the transformed data.
         * @param processor The chunk processor.
         */
        ES_API(CPPESSENCE)
        void open(std::shared_ptr<std::ostream> output_stream, abstract::chunk_processor processor);

        /**
         * @brief Resets all internal states and opens a new file to write the transformed data.
         * @param path The path of the output file into which the transformed data will be written.
         * @param processor The chunk processor.
         * @param mode The open mode of the output file.
         */
        ES_API(CPPESSENCE)
        void open(std::string_view path, abstract::chunk_processor processor, openmode mode = out | binary);

        /**
         * @brief Closes the current file or stream.
         */
        ES_API(CPPESSENCE) void close() const;

    private:
        void* opaque_;
    };
} // namespace essence::crypto
