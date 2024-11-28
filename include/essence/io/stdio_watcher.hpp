// Copyright (c) 2024 The RefValue Project
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#pragma once

#include "../compat.hpp"
#include "common_types.hpp"

#include <memory>

namespace essence::io {
    class stdio_watcher {
    public:
        ES_API(CPPESSENCE) explicit stdio_watcher(stdio_watcher_mode mode);
        ES_API(CPPESSENCE) stdio_watcher(stdio_watcher&&) noexcept;
        ES_API(CPPESSENCE) ~stdio_watcher();
        ES_API(CPPESSENCE) stdio_watcher& operator=(stdio_watcher&&) noexcept;
        ES_API(CPPESSENCE) void start() const;
        ES_API(CPPESSENCE) void stop() const;
        ES_API(CPPESSENCE) void on_message(const stdio_message_handler& handler) const;

    private:
        class impl;

        std::unique_ptr<impl> impl_;
    };
} // namespace essence::io
