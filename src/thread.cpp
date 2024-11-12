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

#include "thread.hpp"

#include "abi/memory.hpp"

#include <atomic>
#include <charconv>
#include <future>
#include <mutex>
#include <sstream>
#include <thread>

namespace essence {
    abi::string serialize_thread_id() {
        using abi_stringstream = std::basic_stringstream<char, std::char_traits<char>, abi::uniform_allocator<char>>;
        abi_stringstream stream;

        stream << std::this_thread::get_id();

        return stream.str();
    }

    std::uint64_t decimalize_thread_id_or_zero() {
        std::uint64_t result{};
        const auto thread_id = serialize_thread_id();

        static_cast<void>(std::from_chars(thread_id.data(), thread_id.data() + thread_id.size(), result));

        return result;
    }

    void parallel_for(std::size_t start, std::size_t end, std::size_t thread_count,
        const std::function<void(std::size_t index, std::size_t thread_index, bool& exit)>& handler) {
        if (!handler) {
            return;
        }

        if (thread_count <= 0) {
            thread_count = std::thread::hardware_concurrency();
        }

        if (thread_count == 1) {
            bool exit{};

            for (std::size_t id = start; id < end; id++) {
                if (!exit) {
                    handler(id, 0, exit);
                }
            }
        } else {
            std::vector<std::future<void>> workers;
            std::atomic_size_t current{start};

            std::mutex mutex;
            std::exception_ptr last_exception;

            for (std::size_t i = 0; i < thread_count; ++i) {
                workers.emplace_back(std::async([&, i] {
                    for (bool exit{};;) {
                        const auto id = current.fetch_add(1, std::memory_order::acq_rel);

                        if (id >= end) {
                            break;
                        }

                        try {
                            if (!exit) {
                                handler(id, i, exit);
                            }
                        } catch (...) {
                            std::scoped_lock lock{mutex};

                            last_exception = std::current_exception();

                            /*
                             * This will work even when current is the largest value that
                             * size_t can fit, because fetch_add returns the previous value
                             * before the increment (what will result in overflow
                             * and produce 0 instead of current + 1).
                             */
                            current.store(end, std::memory_order::release);
                            break;
                        }
                    }
                }));
            }

            for (auto&& item : workers) {
                item.get();
            }

            if (last_exception) {
                std::rethrow_exception(last_exception);
            }
        }
    }
} // namespace essence
