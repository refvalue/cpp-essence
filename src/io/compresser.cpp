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

#include "io/compresser.hpp"

#include "abstract/writable_buffer.hpp"
#include "compression_routines.hpp"

namespace essence::io {
    class compresser::impl {
    public:
        explicit impl(compression_mode mode) : routines_{get_compression_routines(mode)} {}

        [[nodiscard]] abi::vector<std::byte> as_bytes(std::span<const std::byte> buffer, std::int32_t level) const {
            abi::vector<std::byte> result;

            routines_.compress(buffer, abstract::writable_buffer{result}, level);

            return result;
        }

        [[nodiscard]] abi::string as_string(std::span<const std::byte> buffer, std::int32_t level) const {
            abi::string result;

            routines_.compress(buffer, abstract::writable_buffer{result}, level);

            return result;
        }

        [[nodiscard]] abi::vector<std::byte> inverse_as_bytes(std::span<const std::byte> buffer) const {
            abi::vector<std::byte> result;

            routines_.decompress(buffer, abstract::writable_buffer{result});

            return result;
        }

        [[nodiscard]] abi::string inverse_as_string(std::span<const std::byte> buffer) const {
            abi::string result;

            routines_.decompress(buffer, abstract::writable_buffer{result});

            return result;
        }

    private:
        compression_routines routines_;
    };

    compresser::compresser(compression_mode mode) : impl_{std::make_unique<impl>(mode)} {}

    compresser::compresser(compresser&&) noexcept = default;

    compresser::~compresser() = default;

    compresser& compresser::operator=(compresser&&) noexcept = default;

    abi::vector<std::byte> compresser::as_bytes(std::span<const std::byte> buffer, std::int32_t level) const {
        return impl_->as_bytes(buffer, level);
    }

    abi::string compresser::as_string(std::span<const std::byte> buffer, std::int32_t level) const {
        return impl_->as_string(buffer, level);
    }

    abi::vector<std::byte> compresser::inverse_as_bytes(std::span<const std::byte> buffer) const {
        return impl_->inverse_as_bytes(buffer);
    }

    abi::string compresser::inverse_as_string(std::span<const std::byte> buffer) const {
        return impl_->inverse_as_string(buffer);
    }

} // namespace essence::io
