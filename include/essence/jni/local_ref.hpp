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
#include "common_types.hpp"

#include <concepts>
#include <type_traits>
#include <utility>

#include <jni.h>

namespace essence::jni {
    class local_ref {
    public:
        ES_API(JNISUPPORT) local_ref() noexcept;
        ES_API(JNISUPPORT) explicit local_ref(jobject obj);
        ES_API(JNISUPPORT) local_ref(jobject obj, bool take_over_only);
        ES_API(JNISUPPORT) local_ref(const local_ref& other);
        ES_API(JNISUPPORT) local_ref(local_ref&& other) noexcept;
        ES_API(JNISUPPORT) ~local_ref();
        ES_API(JNISUPPORT) local_ref& operator=(const local_ref& right);
        ES_API(JNISUPPORT) local_ref& operator=(local_ref&& right) noexcept;
        ES_API(JNISUPPORT) explicit operator bool() const noexcept;
        [[nodiscard]] ES_API(JNISUPPORT) jobject get() const noexcept;
        ES_API(JNISUPPORT) jobject detach() noexcept;

    private:
        jobject ref_;
    };

    template <typename T>
    concept local_ref_like = std::convertible_to<std::decay_t<T>, local_ref>;

    template <derived_from_jobject JObject>
    class local_ref_ex {
    public:
        local_ref_ex() noexcept = default;

        explicit local_ref_ex(JObject obj) : ref_{obj} {}

        local_ref_ex(JObject obj, bool take_over_only) : ref_{obj, take_over_only} {}

        local_ref_ex(const local_ref_ex& other) = default;

        local_ref_ex(local_ref_ex&& other) noexcept : ref_{std::move(other.ref_)} {}

        local_ref_ex& operator=(const local_ref_ex& right) = default;

        local_ref_ex& operator=(local_ref_ex&& right) noexcept {
            ref_ = std::move(right.ref_);

            return *this;
        }

        explicit operator bool() const noexcept {
            return ref_.operator bool();
        }

        operator const local_ref&() const noexcept { // NOLINT(*-explicit-constructor)
            return ref_;
        }

        operator local_ref&() noexcept { // NOLINT(*-explicit-constructor)
            return ref_;
        }

        JObject get() const noexcept {
            return static_cast<JObject>(ref_.get());
        }

        JObject detach() noexcept {
            return static_cast<JObject>(ref_.detach());
        }

    private:
        local_ref ref_;
    };
} // namespace essence::jni
