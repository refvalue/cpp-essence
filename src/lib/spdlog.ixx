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

module;

#include <spdlog/pattern_formatter.h>
#include <spdlog/sinks/callback_sink.h>
#include <spdlog/sinks/null_sink.h>
#include <spdlog/sinks/ostream_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

export module lib.spdlog;

export namespace spdlog {
    using spdlog::color_mode;
    using spdlog::debug;
    using spdlog::default_factory;
    using spdlog::default_logger;
    using spdlog::default_logger_raw;
    using spdlog::error;
    using spdlog::format_string_t;
    using spdlog::formatter;
    using spdlog::get_level;
    using spdlog::info;
    using spdlog::logger;
    using spdlog::memory_buf_t;
    using spdlog::pattern_formatter;
    using spdlog::set_automatic_registration;
    using spdlog::set_default_logger;
    using spdlog::set_error_handler;
    using spdlog::set_formatter;
    using spdlog::set_level;
    using spdlog::set_pattern;
    using spdlog::sink_ptr;
    using spdlog::trace;
    using spdlog::warn;
} // namespace spdlog

export namespace spdlog::details {
    using details::log_msg;
}

export namespace spdlog::sinks {
    using sinks::callback_sink;
    using sinks::callback_sink_mt;
    using sinks::callback_sink_st;
    using sinks::null_sink;
    using sinks::null_sink_mt;
    using sinks::null_sink_st;
    using sinks::ostream_sink;
    using sinks::ostream_sink_mt;
    using sinks::ostream_sink_st;
    using sinks::rotating_file_sink;
    using sinks::rotating_file_sink_mt;
    using sinks::rotating_file_sink_st;
    using sinks::sink;
    using sinks::stderr_color_sink_mt;
    using sinks::stderr_color_sink_st;
    using sinks::stdout_color_sink_mt;
    using sinks::stdout_color_sink_st;
} // namespace spdlog::sinks
