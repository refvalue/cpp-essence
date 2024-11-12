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

#include "compat.hpp"

#include <functional>
#include <memory>

namespace essence {
    class interruptable_timer {
    public:
        ES_API(CPPESSENCE) interruptable_timer();
        ES_API(CPPESSENCE) interruptable_timer(interruptable_timer&&) noexcept;
        ES_API(CPPESSENCE) ~interruptable_timer();
        ES_API(CPPESSENCE) interruptable_timer& operator=(interruptable_timer&&) noexcept;

        ES_API(CPPESSENCE)
        void start(std::int64_t period_in_milliseconds, const std::function<void()>& callback) const;

        ES_API(CPPESSENCE)
        void start(std::int64_t period_in_milliseconds, std::int64_t deferred_milliseconds,
            const std::function<void()>& callback) const;

        ES_API(CPPESSENCE) void stop() const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence
