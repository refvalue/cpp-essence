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

#include "jni/local_ref.hpp"

#include "jni/jvm.hpp"

namespace essence::jni {
    local_ref::local_ref() noexcept : ref_{} {}

    local_ref::local_ref(jobject obj) : local_ref{obj, false} {}

    local_ref::local_ref(jobject obj, bool take_over_only) : ref_{} {
        if (const auto env = jvm::instance().get_env(); env && obj) {
            ref_ = take_over_only ? obj : env->NewLocalRef(obj);
        }
    }

    local_ref::local_ref(const local_ref& other) : local_ref{other.ref_} {}

    local_ref::local_ref(local_ref&& other) noexcept : ref_{std::exchange(other.ref_, nullptr)} {}

    local_ref::~local_ref() {
        if (const auto env = jvm::instance().get_env(); env && ref_) {
            env->DeleteLocalRef(ref_);
            ref_ = nullptr;
        }
    }

    local_ref::operator bool() const noexcept {
        return ref_;
    }

    local_ref& local_ref::operator=(const local_ref& right) {
        const auto env = jvm::instance().get_env();

        ref_ = env && right.ref_ ? env->NewLocalRef(right.ref_) : nullptr;

        return *this;
    }

    local_ref& local_ref::operator=(local_ref&& right) noexcept {
        ref_ = std::exchange(right.ref_, nullptr);

        return *this;
    }

    jobject local_ref::get() const noexcept {
        return ref_;
    }

    jobject local_ref::detach() noexcept {
        return std::exchange(ref_, {});
    }
} // namespace essence::jni
