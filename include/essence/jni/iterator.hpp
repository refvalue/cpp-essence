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
#include "local_ref.hpp"

#include <compare>
#include <cstddef>
#include <iterator>
#include <type_traits>

#include <jni.h>

namespace essence::jni {
    struct jobject_array_proxy {
        jobjectArray array;
    };

    class jobject_array_iterator {
    public:
        struct end_tag {};
        using iterator_category = std::random_access_iterator_tag;
        using value_type        = local_ref;
        using pointer           = value_type*;
        using reference         = value_type&;
        using difference_type   = jsize;

        ES_API(JNISUPPORT) jobject_array_iterator() noexcept;
        ES_API(JNISUPPORT) explicit jobject_array_iterator(jobjectArray array, end_tag = {});
        ES_API(JNISUPPORT) jobject_array_iterator(jobjectArray array, jsize index);
        ES_API(JNISUPPORT)
        jobject_array_iterator(const jobject_array_iterator&);
        ES_API(JNISUPPORT)
        jobject_array_iterator(jobject_array_iterator&&) noexcept;
        ES_API(JNISUPPORT) ~jobject_array_iterator();
        ES_API(JNISUPPORT) jobject_array_iterator& operator=(const jobject_array_iterator&);
        ES_API(JNISUPPORT) jobject_array_iterator& operator=(jobject_array_iterator&&) noexcept;
        ES_API(JNISUPPORT) value_type operator*() const;
        ES_API(JNISUPPORT) jobject_array_iterator& operator++() noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator operator++(int) noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator& operator--() noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator operator--(int) noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator& operator+=(difference_type off) noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator operator+(difference_type off) const noexcept;
        ES_API(JNISUPPORT)
        friend jobject_array_iterator operator+(difference_type off, jobject_array_iterator next) noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator& operator-=(difference_type off) noexcept;
        ES_API(JNISUPPORT) jobject_array_iterator operator-(difference_type off) const noexcept;
        ES_API(JNISUPPORT)
        difference_type operator-(const jobject_array_iterator& right) const noexcept;
        ES_API(JNISUPPORT) value_type operator[](difference_type off) const;
        constexpr bool operator==(const jobject_array_iterator&) const noexcept  = default;
        constexpr auto operator<=>(const jobject_array_iterator&) const noexcept = default;

    private:
        JNIEnv* env_;
        jsize size_;
        jsize index_;
        jobjectArray array_;
    };

    ES_API(JNISUPPORT) jobject_array_iterator begin(const local_ref_ex<jobjectArray>& array);
    ES_API(JNISUPPORT) jobject_array_iterator end(const local_ref_ex<jobjectArray>& array);

    ES_API(JNISUPPORT) jobject_array_iterator begin(const global_ref_ex<jobjectArray>& array);
    ES_API(JNISUPPORT) jobject_array_iterator end(const global_ref_ex<jobjectArray>& array);

    ES_API(JNISUPPORT) jobject_array_iterator begin(jobject_array_proxy proxy);
    ES_API(JNISUPPORT) jobject_array_iterator end(jobject_array_proxy proxy);
} // namespace essence::jni

ES_API(JNISUPPORT) essence::jni::jobject_array_iterator begin(jobjectArray array);
ES_API(JNISUPPORT) essence::jni::jobject_array_iterator end(jobjectArray array);
