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

#include "io/stdio_watcher.hpp"

#include "char8_t_remediation.hpp"
#include "delegate.hpp"
#include "error_extensions.hpp"
#include "managed_handle.hpp"
#include "native_handle.hpp"

#include <array>
#include <cstdint>
#include <thread>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#define NOGDI

#include <Windows.h>
#else
#include <fcntl.h>
#include <unistd.h>

#define _fileno fileno
#define _pipe   pipe
#define _dup    dup
#define _dup2   dup2
#define _close  close
#define _read   read
#endif

namespace essence::io {
    namespace {
        using posix_handle = unique_handle<&_close, std::uintptr_t, std::int32_t>;

        constexpr std::uint32_t buffer_size = 4096;
    } // namespace

    class stdio_watcher::impl {
    public:
        explicit impl(stdio_watcher_mode mode) : mode_{mode} {}

        ~impl() {
            stop();
        }

        void start() {
            stop();
            redirect_buffer();
            std::jthread{[this] { dispatch_messages(); }}.swap(worker_);
        }

        void stop() {
            // Restores the original handle and closes the pipe.
            if (origin_) {
                _dup2(origin_.get(), current_stdio_descriptor());
                origin_.reset();
            }

#ifdef _WIN32
            if (original_std_handle_) {
                SetStdHandle(current_std_handle_type(), original_std_handle_);
                original_std_handle_ = {};
            }
#endif

            pipe_write_.reset();

            if (worker_.joinable()) {
                worker_.join();
            }

            pipe_read_.reset();
        }

        void on_message(const stdio_message_handler& handler) {
            on_message_ += handler;
        }

    private:
        [[nodiscard]] std::int32_t current_stdio_descriptor() const noexcept {
            return mode_ == stdio_watcher_mode::output ? _fileno(stdout) : _fileno(stderr);
        }

#ifdef _WIN32
        std::uint32_t current_std_handle_type() const noexcept {
            return mode_ == stdio_watcher_mode::output ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE;
        }
#endif

        void dispatch_messages() const {
            thread_local std::array<char, buffer_size> buffer{};
            using size_type = decltype(_read({}, {}, {}));

            for (size_type bytes_read{};
                (bytes_read = _read(pipe_read_.get(), buffer.data(), static_cast<std::uint32_t>(buffer.size() - 1)))
                > 0;) {
                buffer[bytes_read] = U8('\0');
                on_message_.try_invoke(std::string_view{buffer.data(), static_cast<std::size_t>(bytes_read)});
            }
        }

        void redirect_buffer() {
#ifdef _WIN32
            original_std_handle_ = GetStdHandle(current_std_handle_type());
#endif
            origin_.reset(_dup(current_stdio_descriptor()));

            std::array<std::int32_t, 2> pipe_handles{};

#ifdef _WIN32
            if (_pipe(pipe_handles.data(), buffer_size, _O_TEXT) == -1) {
#else
            if (_pipe(pipe_handles.data()) == -1) {
#endif
                throw source_code_aware_runtime_error{U8("Failed to create a pipe for stdio redirection.")};
            }

            pipe_read_.reset(pipe_handles.front());
            pipe_write_.reset(pipe_handles.back());

            if (_dup2(pipe_write_.get(), current_stdio_descriptor()) == -1) {
                throw source_code_aware_runtime_error{U8("Failed to redirect the stdio buffer to the pipe.")};
            }

#ifdef _WIN32
            if (!SetStdHandle(mode_ == stdio_watcher_mode::output ? STD_OUTPUT_HANDLE : STD_ERROR_HANDLE,
                    reinterpret_cast<HANDLE>(_get_osfhandle(pipe_write_.get())))) {
                throw source_code_aware_runtime_error{U8("Failed to redirect the underlying Win32 standard handle.")};
            }
#endif
        }

        stdio_watcher_mode mode_;
        posix_handle origin_;
        posix_handle pipe_read_;
        posix_handle pipe_write_;
#ifdef _WIN32
        native_handle original_std_handle_;
#endif
        std::jthread worker_;
        delegate<stdio_message_handler> on_message_;
    };

    stdio_watcher::stdio_watcher(stdio_watcher_mode mode) : impl_{std::make_unique<impl>(mode)} {}

    stdio_watcher::stdio_watcher(stdio_watcher&&) noexcept = default;

    stdio_watcher::~stdio_watcher() = default;

    stdio_watcher& stdio_watcher::operator=(stdio_watcher&&) noexcept = default;

    void stdio_watcher::start() const {
        impl_->start();
    }

    void stdio_watcher::stop() const {
        impl_->stop();
    }

    void stdio_watcher::on_message(const stdio_message_handler& handler) const {
        impl_->on_message(handler);
    }
} // namespace essence::io
