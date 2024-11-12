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
#include "../../zstring_view.hpp"
#include "../common_types.hpp"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace essence::crypto {
    class pubkey_param_impl;

    class dh_param {
    public:
        ES_API(CPPESSENCE) explicit dh_param(std::shared_ptr<void> context);
        ES_API(CPPESSENCE) dh_param(dh_param&&) noexcept;
        ES_API(CPPESSENCE) ~dh_param();
        ES_API(CPPESSENCE) dh_param& operator=(dh_param&&) noexcept;

        [[nodiscard]] ES_API(CPPESSENCE) dh_kdf_type kdf_mode() const;
        [[nodiscard]] ES_API(CPPESSENCE) digest_mode kdf_digest_mode() const;
        [[nodiscard]] ES_API(CPPESSENCE) std::int32_t kdf_digest_outlen() const;

        ES_API(CPPESSENCE) void set_padding(bool value) const;
        ES_API(CPPESSENCE) void set_nid(dh_nid value) const;
        ES_API(CPPESSENCE) void set_rfc5114(dhx_rfc5114 value) const;
        ES_API(CPPESSENCE) void set_kdf_type(dh_kdf_type value) const;
        ES_API(CPPESSENCE) void set_kdf_cipher_name(zstring_view value) const;
        ES_API(CPPESSENCE) void set_kdf_digest_mode(digest_mode value) const;
        ES_API(CPPESSENCE) void set_kdf_digest_outlen(std::int32_t value) const;
        ES_API(CPPESSENCE) void set_kdf_ukm(std::span<const std::byte> value) const;

    private:
        std::unique_ptr<pubkey_param_impl> impl_;
    };
} // namespace essence::crypto
