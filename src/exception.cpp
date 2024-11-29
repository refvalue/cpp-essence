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

#include "exception.hpp"

#include "char8_t_remediation.hpp"
#include "format_remediation.hpp"
#include "functional.hpp"

#include <stdexcept>
#include <string>
#include <tuple>

#include <spdlog/spdlog.h>

namespace essence {
    namespace {
        template <invocable<const std::exception&> ExceptionCallback>
        void unwind_exception_ptr(const std::exception_ptr& ex, ExceptionCallback&& exception_callback) {
            if (ex) {
                try {
                    std::rethrow_exception(ex);
                } catch (const std::exception& inner_ex) {
                    invoke_optional(std::forward<ExceptionCallback>(exception_callback), inner_ex);
                } catch (...) {
                    invoke_optional(
                        std::forward<ExceptionCallback>(exception_callback), std::runtime_error{U8("Unknown error.")});
                }
            }
        }

        template <nullptr_or_invocable<std::exception_ptr> LevelCallback = std::nullptr_t,
            nullptr_or_invocable<abi::string&> CompleteCallback          = std::nullptr_t>
        void flatten_nested_exceptions(const std::exception_ptr& root, std::int32_t indent,
            LevelCallback&& level_callback = nullptr, CompleteCallback&& complete_callback = nullptr) {
            abi::string result;

            auto level_tuple           = std::forward_as_tuple(std::forward<LevelCallback>(level_callback));
            auto invoke_level_callback = [&](const std::exception_ptr& ex) {
                invoke_optional(std::get<0>(std::move(level_tuple)), ex);
            };

            auto complete_tuple           = std::forward_as_tuple(std::forward<CompleteCallback>(complete_callback));
            auto invoke_complete_callback = [&] { invoke_optional(std::get<0>(std::move(complete_tuple)), result); };

            auto appender = [&]<typename Callable>(
                                Callable&& self, const std::exception_ptr& current, std::int32_t level = 0) -> void {
                bool recursive{};
                auto tuple = std::forward_as_tuple(std::forward<Callable>(self));

                unwind_exception_ptr(current, [&](const std::exception& ex) {
                    if (const auto aggregate = dynamic_cast<const aggregate_error*>(&ex)) {
                        for (auto&& item : *aggregate) {
                            invoke_level_callback(item);
                        }
                    } else {
                        invoke_level_callback(current);
                    }

                    result.append(format(
                        U8("{}{}\n"), std::string(static_cast<std::size_t>(indent) * level, U8('-')), ex.what()));

                    try {
                        std::rethrow_if_nested(ex);
                    } catch (const std::exception&) {
                        recursive = true;

                        return std::apply(
                            [&level]<typename InnerCallable>(InnerCallable&& self) {
                                return std::forward<InnerCallable>(self)(
                                    std::forward<InnerCallable>(self), std::current_exception(), level + 1);
                            },
                            std::move(tuple));

                    } catch (...) {
                        spdlog::info(U8("Unknown error."));
                    }
                });

                if (!recursive) {
                    // Removes the redundant '\n'.
                    if (!result.empty()) {
                        result.pop_back();
                    }

                    invoke_complete_callback();
                }
            };

            appender(appender, root);
        }
    } // namespace

    aggregate_error::iterator aggregate_error::begin() const noexcept {
        return exceptions_.data();
    }

    aggregate_error::iterator aggregate_error::end() const noexcept {
        return exceptions_.data() + exceptions_.size();
    }

    aggregate_error::reverse_iterator aggregate_error::rbegin() const noexcept {
        return reverse_iterator{begin() + size()};
    }

    aggregate_error::reverse_iterator aggregate_error::rend() const noexcept {
        return reverse_iterator{begin()};
    }

    bool aggregate_error::empty() const noexcept {
        return exceptions_.empty();
    }

    std::size_t aggregate_error::size() const noexcept {
        return exceptions_.size();
    }

    void aggregate_error::flatten_and_throw(const std::exception_ptr& root, std::int32_t indent) {
        abi::vector<std::exception_ptr> exceptions;

        flatten_nested_exceptions(
            root, indent, [&](const std::exception_ptr& ex) { exceptions.emplace_back(ex); },
            [&](const abi::string& result) { throw aggregate_error{std::move(exceptions), result}; });

        throw;
    }

    aggregate_error::aggregate_error(abi::vector<value_type>&& exceptions, std::string_view what)
        : runtime_error{std::string{what}}, exceptions_{std::move(exceptions)} {}

    abi::string serialize_nested_exceptions(const std::exception_ptr& root, std::int32_t indent) {
        abi::string result;

        flatten_nested_exceptions(root, indent, nullptr, [&](abi::string& inner) { result = std::move(inner); });

        return result;
    }
} // namespace essence
