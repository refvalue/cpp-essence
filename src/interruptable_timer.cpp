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

#include "interruptable_timer.hpp"

#include "format_remediation.hpp"

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <utility>

namespace essence {
    namespace {
        struct timer_info {
            std::function<void()> callback;
            std::chrono::milliseconds period;
            std::chrono::milliseconds deferred_time;

            template <typename Callback, typename Period, typename DeferredTime>
            timer_info(Callback&& callback, Period&& period, DeferredTime&& deferred_time)
                : callback{std::forward<Callback>(callback)}, period{std::forward<Period>(period)},
                  deferred_time{std::forward<DeferredTime>(deferred_time)} {}

            explicit operator bool() const noexcept {
                return callback && period > std::chrono::milliseconds::zero();
            }
        };
    } // namespace

    class interruptable_timer::impl {
    public:
        impl()
            : stop_pending_{false}, worker_thread_{[this] { timer_routine(); }}, idle_running_{true}, idle_pending_{true},
              timer_running_{false} {}

        ~impl() {
            change_state(nullptr, false, false);
            stop_pending_.store(true, std::memory_order::release);
        }

        void start(std::int64_t period_in_milliseconds, const std::function<void()>& callback) {
            start(period_in_milliseconds, 0, callback);
        }

        void start(std::int64_t period_in_milliseconds, std::int64_t deferred_milliseconds,
            const std::function<void()>& callback) {
            stop();
            change_state(std::make_shared<timer_info>(callback, std::chrono::milliseconds{period_in_milliseconds},
                             std::chrono::milliseconds{deferred_milliseconds}),
                true, false);
        }

        void stop() {
            change_state(nullptr, false, true);

            std::unique_lock lock{mutex_idle_};

            cv_idle_.wait(lock, [this] { return idle_running_.load(std::memory_order_acquire); });
        }

    private:
        void change_state(const std::shared_ptr<timer_info>& new_state, bool timer_running, bool idle_pending) {
            std::atomic_store_explicit(&timer_info_, new_state, std::memory_order_release);
            timer_running_.store(timer_running, std::memory_order_release);
            idle_pending_.store(idle_pending, std::memory_order_release);
            cv_idle_.notify_all();
            cv_timer_.notify_all();
        }

        void timer_routine() {
            while (!stop_pending_.load(std::memory_order::acquire)) {
                // Reuses this condition variable to notify the subscriber of the stopped state.
                cv_idle_.notify_all();
                {
                    std::unique_lock lock{mutex_idle_};

                    cv_idle_.wait(lock, [this] { return !idle_pending_.load(std::memory_order_acquire); });
                }

                idle_running_.store(false, std::memory_order_release);
                idle_pending_.store(false, std::memory_order_release);

                if (auto info = std::atomic_load_explicit(&timer_info_, std::memory_order_acquire); info && *info) {
                    // Processes deferring.
                    if (timer_running_.load(std::memory_order_acquire)
                        && info->deferred_time > std::chrono::milliseconds::zero()) {

                        std::unique_lock lock{mutex_timer_};

                        cv_timer_.wait_for(lock, info->deferred_time,
                            [this] { return !timer_running_.load(std::memory_order_acquire); });
                    }

                    while (timer_running_.load(std::memory_order_acquire)) {
                        info->callback();

                        std::unique_lock lock{mutex_timer_};

                        cv_timer_.wait_for(
                            lock, info->period, [this] { return !timer_running_.load(std::memory_order_acquire); });
                    }
                }

                idle_running_.store(true, std::memory_order_release);
                idle_pending_.store(false, std::memory_order_release);
            }
        }

        mutable std::mutex mutex_idle_;
        mutable std::mutex mutex_timer_;
        std::atomic_bool stop_pending_;
        std::thread worker_thread_;
        std::atomic_bool idle_running_;
        std::atomic_bool idle_pending_;
        std::atomic_bool timer_running_;
        std::condition_variable cv_idle_;
        std::condition_variable cv_timer_;
        std::shared_ptr<timer_info> timer_info_;
    };

    interruptable_timer::interruptable_timer() : impl_{std::make_unique<impl>()} {}

    interruptable_timer::interruptable_timer(interruptable_timer&&) noexcept = default;

    interruptable_timer::~interruptable_timer() = default;

    interruptable_timer& interruptable_timer::operator=(interruptable_timer&&) noexcept = default;

    void interruptable_timer::start(
        std::int64_t interval_in_milliseconds, const std::function<void()>& callback) const {
        impl_->start(interval_in_milliseconds, callback);
    }

    void interruptable_timer::start(std::int64_t period_in_milliseconds, std::int64_t deferred_milliseconds,
        const std::function<void()>& callback) const {
        impl_->start(period_in_milliseconds, deferred_milliseconds, callback);
    }

    void interruptable_timer::stop() const {
        impl_->stop();
    }
} // namespace essence
