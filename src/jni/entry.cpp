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

#include "jni/entry.hpp"

#include "char8_t_remediation.hpp"
#include "jni/jvm.hpp"
#include "jni/reflector.hpp"

#include <exception>

#include <spdlog/spdlog.h>

namespace essence::jni {
    void jvm_main();
    void jvm_exit();

    jint do_jni_on_load(JavaVM* vm, void*) try {
        if (auto context = jvm::instance().init(vm)) {
            spdlog::info(U8("C++ Essence Java VM Version: {:08x}."), context->version);
            jvm_main();

            return context->version;
        }

        return JNI_ERR;
    } catch (const std::exception& ex) {
        spdlog::error(ex.what());

        return JNI_ERR;
    }

    void do_jni_on_unload(JavaVM* vm, void*) try {
        spdlog::info(U8("Unloading C++ Essence Java VM..."));
        jvm_exit();
        jvm::instance().clear_entries();
        reflector::instance().clear();
    } catch (const std::exception& ex) {
        spdlog::error(ex.what());
    }
} // namespace essence::jni
