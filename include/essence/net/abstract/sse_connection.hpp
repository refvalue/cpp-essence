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

#include "../../abi/string.hpp"
#include "../sse_message.hpp"
#include "../uri.hpp"

#include <concepts>
#include <functional>
#include <memory>
#include <type_traits>
#include <utility>

namespace essence::net::abstract {
    /**
     * @brief An interface: manages an SSE connection and its lifetime.
     */
    class sse_connection {
    public:
        sse_connection() = default;

        template <typename T>
            requires(!std::same_as<std::decay_t<T>, sse_connection>)
        explicit sse_connection(T&& value) : wrapper_{std::make_shared<wrapper<T>>(std::forward<T>(value))} {}

        /**
         * @brief Checks whether the internal operation has been canceled and if true
                  the user must stop all invocations to this object.
         * @return true if the internal operation has been canceled; otherwise false.
         */
        [[nodiscard]] bool canceled() const {
            return wrapper_->canceled();
        }

        /**
         * @brief Gets the request URI.
         * @return The URI.
         */
        [[nodiscard]] uri request_uri() const {
            return wrapper_->request_uri();
        }

        /**
         * @brief Gets the remote address.
         * @return The remote address.
         */
        [[nodiscard]] abi::string remote_address() const {
            return wrapper_->remote_address();
        }

        /**
         * @brief Sends a message to the client.
         */
        void send_message(const sse_message& message) const {
            wrapper_->send_message(message);
        }

        /**
         * @brief Keep alive and raise an error when the network is disconnected.
         */
        void tick() const {
            wrapper_->tick();
        }

        /**
         * @brief Closes the connection.
         */
        void close() const {
            wrapper_->close();
        }

        /**
         * @brief Gets the underlying pointer.
         * @return The underlying pointer.
         */
        [[nodiscard]] void* underlying_ptr() const {
            return wrapper_->underlying_ptr();
        }

    private:
        struct base {
            virtual ~base()                                       = default;
            virtual bool canceled()                               = 0;
            virtual uri request_uri()                             = 0;
            virtual abi::string remote_address()                  = 0;
            virtual void send_message(const sse_message& message) = 0;
            virtual void tick()                                   = 0;
            virtual void close()                                  = 0;
            virtual void* underlying_ptr()                        = 0;
        };

        template <typename T>
        class wrapper final : public base {
        public:
            template <std::convertible_to<T> U>
            explicit wrapper(U&& value) : value_{std::forward<U>(value)} {}

            bool canceled() override {
                return value_.canceled();
            }

            uri request_uri() override {
                return value_.request_uri();
            }

            abi::string remote_address() override {
                return value_.remote_address();
            }

            void send_message(const sse_message& message) override {
                value_.send_message(message);
            }

            void tick() override {
                value_.tick();
            }

            void close() override {
                value_.close();
            }

            void* underlying_ptr() override {
                return &value_;
            }

        private:
            T value_;
        };

        std::shared_ptr<base> wrapper_;
    };
} // namespace essence::net::abstract

namespace essence::net {
    using sse_connection_handler = std::function<void(abstract::sse_connection connection)>;
}
