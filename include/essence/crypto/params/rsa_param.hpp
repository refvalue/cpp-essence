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

#include "../../compat.hpp"
#include "../common_types.hpp"

#include <cstddef>
#include <memory>
#include <span>

namespace essence::crypto {
    class pubkey_param_impl;

    class rsa_param {
    public:
        ES_API(CPPESSENCE) explicit rsa_param(std::shared_ptr<void> context);
        ES_API(CPPESSENCE) rsa_param(rsa_param&&) noexcept;
        ES_API(CPPESSENCE) ~rsa_param();
        ES_API(CPPESSENCE) rsa_param& operator=(rsa_param&&) noexcept;

        [[nodiscard]] ES_API(CPPESSENCE) rsa_padding_mode padding_mode() const;
        [[nodiscard]] ES_API(CPPESSENCE) rsa_pss_saltlen pss_saltlen() const;
        [[nodiscard]] ES_API(CPPESSENCE) digest_mode mgf1_digest_mode() const;
        [[nodiscard]] ES_API(CPPESSENCE) digest_mode oaep_digest_mode() const;

        ES_API(CPPESSENCE) void set_padding_mode(rsa_padding_mode value) const;
        ES_API(CPPESSENCE) void set_pss_saltlen(rsa_pss_saltlen value) const;
        ES_API(CPPESSENCE) void set_mgf1_digest_mode(digest_mode value) const;
        ES_API(CPPESSENCE) void set_oaep_digest_mode(digest_mode value) const;
        ES_API(CPPESSENCE) void set_oaep_label(std::span<const std::byte> value) const;

    private:
        std::unique_ptr<pubkey_param_impl> impl_;
    };
} // namespace essence::crypto
