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

#include "jni/iterator.hpp"

#include "error_extensions.hpp"

#include <jni/jvm.hpp>

namespace essence::jni {
    jobject_array_iterator::jobject_array_iterator() noexcept
        : env_{jvm::instance().ensure_env()}, size_{}, index_{}, array_{} {}

    jobject_array_iterator::jobject_array_iterator(jobjectArray array, end_tag)
        : env_{jvm::instance().ensure_env()}, size_{env_->GetArrayLength(array)}, index_{}, array_{array} {}

    jobject_array_iterator::jobject_array_iterator(jobjectArray array, jsize index)
        : env_{jvm::instance().ensure_env()},
          size_{array ? env_->GetArrayLength(array)
                      : throw source_code_aware_runtime_error{U8("The array must be non-null.")}},
          index_{index < size_ ? index : throw source_code_aware_runtime_error{U8("The index is out of range.")}},
          array_{array} {}

    jobject_array_iterator::jobject_array_iterator(const jobject_array_iterator&) = default;

    jobject_array_iterator::jobject_array_iterator(jobject_array_iterator&&) noexcept = default;

    jobject_array_iterator::~jobject_array_iterator() = default;

    jobject_array_iterator& jobject_array_iterator::operator=(const jobject_array_iterator&) = default;

    jobject_array_iterator& jobject_array_iterator::operator=(jobject_array_iterator&&) noexcept = default;

    jobject_array_iterator::value_type jobject_array_iterator::operator*() const {
        return operator[](0);
    }

    jobject_array_iterator& jobject_array_iterator::operator++() noexcept {
        index_++;

        return *this;
    }

    jobject_array_iterator jobject_array_iterator::operator++(int) noexcept {
        jobject_array_iterator duplicate{*this};

        ++*this;

        return duplicate;
    }

    jobject_array_iterator& jobject_array_iterator::operator--() noexcept {
        index_--;

        return *this;
    }

    jobject_array_iterator jobject_array_iterator::operator--(int) noexcept {
        jobject_array_iterator duplicate{*this};

        --*this;

        return duplicate;
    }

    jobject_array_iterator& jobject_array_iterator::operator+=(difference_type off) noexcept {
        return *this;
    }

    jobject_array_iterator jobject_array_iterator::operator+(difference_type off) const noexcept {
        jobject_array_iterator duplicate{*this};

        duplicate.index_ += off;

        return duplicate;
    }

    jobject_array_iterator operator+(
        jobject_array_iterator::difference_type off, jobject_array_iterator next) noexcept {
        next.index_ += off;

        return next;
    }

    jobject_array_iterator& jobject_array_iterator::operator-=(difference_type off) noexcept {
        index_ -= off;

        return *this;
    }

    jobject_array_iterator jobject_array_iterator::operator-(difference_type off) const noexcept {
        jobject_array_iterator duplicate{*this};

        duplicate.index_ -= off;

        return duplicate;
    }

    jobject_array_iterator::difference_type jobject_array_iterator::operator-(
        const jobject_array_iterator& right) const noexcept {
        return index_ - right.index_;
    }

    jobject_array_iterator::value_type jobject_array_iterator::operator[](const difference_type off) const {
        return local_ref{env_->GetObjectArrayElement(array_, index_ + off), true};
    }

    jobject_array_iterator begin(const local_ref_ex<jobjectArray>& array) {
        return ::begin(array.get());
    }

    jobject_array_iterator end(const local_ref_ex<jobjectArray>& array) {
        return ::end(array.get());
    }

    jobject_array_iterator begin(const global_ref_ex<jobjectArray>& array) {
        return ::begin(array.get());
    }

    jobject_array_iterator end(const global_ref_ex<jobjectArray>& array) {
        return ::end(array.get());
    }

    jobject_array_iterator begin(jobject_array_proxy proxy) {
        return begin(proxy.array);
    }

    jobject_array_iterator end(jobject_array_proxy proxy) {
        return end(proxy.array);
    }

} // namespace essence::jni

using namespace essence::jni;

jobject_array_iterator begin(jobjectArray array) {
    return jobject_array_iterator{array, 0};
}

jobject_array_iterator end(jobjectArray array) {
    return jobject_array_iterator{array};
}
