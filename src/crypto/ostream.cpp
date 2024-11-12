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

#include "crypto/ostream.hpp"

#include "inout_buffer_pair.hpp"
#include "io/fs_operator.hpp"

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <memory>
#include <optional>
#include <span>
#include <streambuf>
#include <utility>

#include <openssl/evp.h>

namespace essence::crypto {
    namespace {
        class cipher_streambuf final : public std::streambuf {
        public:
            cipher_streambuf() = default;

            ~cipher_streambuf() override {
                close();
            }

            void init(std::shared_ptr<std::ostream> output_stream, abstract::chunk_processor processor) {
                buffer_pair_   = inout_buffer_pair{processor};
                output_stream_ = std::move(output_stream);
                processor_     = std::move(processor);

                setp(reinterpret_cast<char*>(buffer_pair_.in.data()),
                    reinterpret_cast<char*>(buffer_pair_.in.data()) + buffer_pair_.in.size());

                processor_->init();
            }

            void close() {
                if (is_open()) {
                    finalize();
                    output_stream_.reset();
                    processor_.reset();
                    setp(nullptr, nullptr);
                }
            }

            [[nodiscard]] bool is_open() const noexcept {
                return static_cast<bool>(output_stream_);
            }

            void finalize() {
                if (sync() == 0) {
                    auto output = buffer_pair_.out;

                    // Writes the final data.
                    processor_->finalize(output);
                    output_stream_->write(
                        reinterpret_cast<const char*>(output.data()), static_cast<std::streamsize>(output.size()));
                }
            }

        protected:
            [[nodiscard]] int_type overflow(int_type ch) override {
                if (!is_open() || traits_type::eq_int_type(ch, traits_type::eof())) {
                    return std::streambuf::overflow();
                }

                // Updates the cipher context if the underlying buffer is full.
                if (pptr() >= epptr()) {
                    process_buffer();
                }

                *pptr() = traits_type::to_char_type(ch);
                pbump(1);

                return ch;
            }

            [[nodiscard]] std::int32_t sync() override {
                if (!is_open()) {
                    return -1;
                }

                // Processes the remaining data.
                if (pptr() > pbase()) {
                    process_buffer();
                }

                output_stream_->flush();

                return 0;
            }

        private:
            void process_buffer() {
                if (is_open()) {
                    const std::span input{buffer_pair_.in.data(), static_cast<std::size_t>(pptr() - pbase())};
                    auto output = buffer_pair_.out;

                    processor_->update(input, output);
                    output_stream_->write(
                        reinterpret_cast<const char*>(output.data()), static_cast<std::streamsize>(output.size()));
                    pbump(-static_cast<std::int32_t>(input.size()));
                }
            }

            inout_buffer_pair buffer_pair_;
            std::shared_ptr<std::ostream> output_stream_;
            std::optional<abstract::chunk_processor> processor_;
        };
    } // namespace

    ostream::ostream() : std::ostream{nullptr}, opaque_{new cipher_streambuf} {
        set_rdbuf(static_cast<cipher_streambuf*>(opaque_));
        clear();
    }

    ostream::ostream(std::shared_ptr<std::ostream> output_stream, abstract::chunk_processor processor) : ostream{} {
        open(std::move(output_stream), std::move(processor));
    }

    ostream::ostream(std::string_view path, abstract::chunk_processor processor, openmode mode) : ostream{} {
        open(path, std::move(processor), mode);
    }

    ostream::~ostream() {
        if (opaque_) {
            delete static_cast<cipher_streambuf*>(opaque_);
            opaque_ = nullptr;
        }
    }

    bool ostream::is_open() const noexcept {
        return static_cast<cipher_streambuf*>(opaque_)->is_open();
    }

    void ostream::open(std::shared_ptr<std::ostream> output_stream, abstract::chunk_processor processor) {
        if (output_stream) {
            static_cast<cipher_streambuf*>(opaque_)->init(std::move(output_stream), std::move(processor));
            clear();
        } else {
            setstate(failbit);
        }
    }

    void ostream::open(std::string_view path, abstract::chunk_processor processor, openmode mode) {
        open(io::get_native_fs_operator().open_write(path, mode), std::move(processor));
    }

    void ostream::close() const {
        static_cast<cipher_streambuf*>(opaque_)->close();
    }
} // namespace essence::crypto
