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

#include "jni/jvm.hpp"

#include "error_extensions.hpp"
#include "thread.hpp"

#include <array>
#include <atomic>
#include <forward_list>
#include <mutex>

namespace essence::jni {
    namespace {
        /// Some JNI functions may contain parameters that are void** or JNIEnv** across different JNI versions.
        /// We provides an adapter here to support auto-casting.
        /// </summary>
        struct env_adapter {
            JNIEnv* env{};

            [[nodiscard]] JNIEnv* get() const noexcept {
                return env;
            }

            operator JNIEnv**() noexcept { // NOLINT(*-explicit-constructor)
                return &env;
            }

            operator void**() noexcept { // NOLINT(*-explicit-constructor)
                return reinterpret_cast<void**>(&env);
            }
        };
    } // namespace

    class jvm::impl {
    public:
        std::optional<jvm_context> init(JavaVM* vm) {
            static constexpr std::array available_versions{
                JNI_VERSION_1_6, JNI_VERSION_1_4, JNI_VERSION_1_2, JNI_VERSION_1_1};

            env_adapter adapter;

            if (const auto iter = std::ranges::find_if(
                    available_versions,
                    [&](std::int32_t inner) { return vm->GetEnv(adapter, inner) == JNI_OK; });
                iter != available_versions.end()) {
                const auto result = std::make_shared<jvm_context>(jvm_context{vm, *iter});

                std::atomic_store_explicit(&context_, result, std::memory_order::release);

                return *result;
            }

            return std::nullopt;
        }

        JNIEnv* get_env() const {
            auto context = std::atomic_load_explicit(&context_, std::memory_order::acquire);

            if (!context) {
                return nullptr;
            }

            thread_local bool attached_in_native_code = false;

            // Creates a thread-local JNIEnv* using a trick.
            thread_local auto creator = [&, context] {
                env_adapter env{nullptr};

                // Attaches the current thread if necessary.
                if (context && context->vm->GetEnv(env, context->version) == JNI_EDETACHED) {
                    return context->vm->AttachCurrentThread(env, nullptr) == JNI_OK
                             ? (attached_in_native_code = true, env.get())
                             : nullptr;
                }

                return env.get();
            };

            thread_local auto freeing_handler = [&, context](JNIEnv*) {
                if (context && attached_in_native_code) {
                    context->vm->DetachCurrentThread();
                }
            };

            thread_local std::unique_ptr<JNIEnv, decltype(freeing_handler)> result{creator(), freeing_handler};

            return result.get();
        }

        JNIEnv* ensure_env() const {
            if (const auto env = get_env()) {
                return env;
            }

            throw source_code_aware_runtime_error{U8("Thread ID"), serialize_thread_id(), U8("Message"),
                U8("Failed to get the JNI environment for the thread.")};
        }

        std::optional<jvm_context> context() const {
            if (const auto current = std::atomic_load_explicit(&context_, std::memory_order::acquire)) {
                return *current;
            }

            return std::nullopt;
        }

        global_ref& add_managed_entry(jobject init) {
            std::scoped_lock lock{entry_mutex_};
            auto&& ref = managed_entries_.emplace_front();

            if (init) {
                ref = global_ref{init};
            }

            return ref;
        }

        void clear_entries() {
            std::scoped_lock lock{entry_mutex_};

            managed_entries_.clear();
        }

    private:
        mutable std::mutex entry_mutex_;
        std::forward_list<global_ref> managed_entries_;

#if __cpp_lib_atomic_shared_ptr >= 201711L
        std::atomic<std::shared_ptr<jvm_context>> context_;
#else
        std::shared_ptr<jvm_context> context_;
#endif
    };

    jvm::jvm(jvm&&) noexcept = default;

    jvm::~jvm() = default;

    jvm& jvm::operator=(jvm&&) noexcept = default;

    const jvm& jvm::instance() {
        static const jvm instance;

        return instance;
    }

    std::optional<jvm_context> jvm::init(JavaVM* vm) const {
        return impl_->init(vm);
    }

    JNIEnv* jvm::get_env() const {
        return impl_->get_env();
    }

    JNIEnv* jvm::ensure_env() const {
        return impl_->ensure_env();
    }

    std::optional<jvm_context> jvm::context() const {
        return impl_->context();
    }

    global_ref& jvm::add_managed_entry(jobject init) const {
        return impl_->add_managed_entry(init);
    }

    void jvm::clear_entries() const {
        impl_->clear_entries();
    }

    jvm::jvm() : impl_{std::make_unique<impl>()} {}
} // namespace essence::jni
