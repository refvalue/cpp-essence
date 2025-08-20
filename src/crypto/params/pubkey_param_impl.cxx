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

#include <essence/char8_t_remediation.hpp>

#include <openssl/evp.h>

module essence.crypto:params.pubkey_param_impl;
import essence.basic;
import std;

namespace essence::crypto {
    class pubkey_param_impl {
    public:
        pubkey_param_impl(std::span<const zstring_view> cipher_names, std::shared_ptr<void> context)
            : context_{context ? std::static_pointer_cast<EVP_PKEY_CTX>(std::move(context))
                               : throw formatted_runtime_error{"The context must be non-null."}} {
            if (!std::ranges::any_of(cipher_names,
                    [&](zstring_view inner) { return EVP_PKEY_CTX_is_a(context_.get(), inner.c_str()); })) {
                throw formatted_runtime_error{"The public key cipher does not match the given context."};
            }
        }

        [[nodiscard]] EVP_PKEY_CTX* context() const noexcept {
            return context_.get();
        }

    private:
        std::shared_ptr<EVP_PKEY_CTX> context_;
    };

    constexpr auto pubkey_param_impl_deleter = [](void* inner) {
        if (auto impl = static_cast<pubkey_param_impl*>(inner)) {
            delete impl;
        }
    };

    template <typename Deleter>
    pubkey_param_impl& get_impl(const std::unique_ptr<void, Deleter>& opaque) noexcept {
        return *static_cast<pubkey_param_impl*>(opaque.get());
    }
} // namespace essence::crypto
