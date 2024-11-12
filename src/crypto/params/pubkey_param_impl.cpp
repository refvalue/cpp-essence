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

#include "pubkey_param_impl.hpp"

#include "char8_t_remediation.hpp"
#include "error_extensions.hpp"

#include <algorithm>

namespace essence::crypto {
    pubkey_param_impl::pubkey_param_impl(std::span<const zstring_view> cipher_names, std::shared_ptr<void> context)
        : context_{context ? std::static_pointer_cast<EVP_PKEY_CTX>(std::move(context))
                           : throw source_code_aware_runtime_error{U8("The context must be non-null.")}} {
        if (!std::ranges::any_of(
                cipher_names,
                [&](zstring_view inner) { return EVP_PKEY_CTX_is_a(context_.get(), inner.c_str()); })) {
            throw source_code_aware_runtime_error{U8("The public key cipher does not match the given context.")};
        }
    }

    pubkey_param_impl::~pubkey_param_impl() = default;

    EVP_PKEY_CTX* pubkey_param_impl::context() const noexcept {
        return context_.get();
    }

} // namespace essence::crypto
