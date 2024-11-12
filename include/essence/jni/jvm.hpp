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
#include "global_ref.hpp"

#include <cstdint>
#include <memory>
#include <optional>

#include <jni.h>

namespace essence::jni {
    /**
     * @brief A Java VM runtime context.
     */
    struct jvm_context {
        JavaVM* vm;
        std::int32_t version;
    };

    /**
     * @brief Manages a Java VM.
     */
    class jvm {
    public:
        ES_API(JNISUPPORT) jvm(jvm&&) noexcept;
        ES_API(JNISUPPORT) ~jvm();
        ES_API(JNISUPPORT) jvm& operator=(jvm&&) noexcept;
        ES_API(JNISUPPORT) static const jvm& instance();
        ES_API(JNISUPPORT) std::optional<jvm_context> init(JavaVM* vm) const;
        [[nodiscard]] ES_API(JNISUPPORT) JNIEnv* get_env() const;
        [[nodiscard]] ES_API(JNISUPPORT) JNIEnv* ensure_env() const;
        [[nodiscard]] ES_API(JNISUPPORT) std::optional<jvm_context> context() const;
        ES_API(JNISUPPORT) global_ref& add_managed_entry(jobject init = nullptr) const;
        ES_API(JNISUPPORT) void clear_entries() const;

    private:
        jvm();

        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::jni
