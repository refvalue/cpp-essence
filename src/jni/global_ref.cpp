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

#include "jni/global_ref.hpp"

#include "jni/jvm.hpp"

namespace essence::jni {
    global_ref::global_ref() noexcept : ref_{} {}

    global_ref::global_ref(jobject obj) : global_ref{obj, false} {}

    global_ref::global_ref(jobject obj, bool take_over_only) : ref_{} {
        if (const auto env = jvm::instance().get_env(); env && obj) {
            ref_ = take_over_only ? obj : env->NewGlobalRef(obj);
        }
    }

    global_ref::global_ref(const global_ref& other) : global_ref{other.ref_} {}

    global_ref::global_ref(global_ref&& other) noexcept : ref_{std::exchange(other.ref_, nullptr)} {}

    global_ref::~global_ref() {
        if (const auto env = jvm::instance().get_env(); env && ref_) {
            env->DeleteGlobalRef(ref_);
            ref_ = nullptr;
        }
    }

    global_ref::operator bool() const noexcept {
        return ref_;
    }

    global_ref& global_ref::operator=(const global_ref& right) {
        const auto env = jvm::instance().get_env();

        ref_ = env && right.ref_ ? env->NewGlobalRef(right.ref_) : nullptr;

        return *this;
    }

    global_ref& global_ref::operator=(global_ref&& right) noexcept {
        ref_ = std::exchange(right.ref_, nullptr);

        return *this;
    }

    jobject global_ref::get() const noexcept {
        return ref_;
    }

    jobject global_ref::detach() noexcept {
        return std::exchange(ref_, {});
    }
} // namespace essence::jni
