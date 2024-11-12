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

#include "char8_t_remediation.hpp"
#include "format_remediation.hpp"
#include "source_location.hpp"

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <type_traits>
#include <utility>

namespace essence::detail {
    template <std::size_t N>
    consteval auto make_format_str() noexcept {
        constexpr std::string_view part1{U8("[{}] ")};
        constexpr std::string_view part2{U8("{}\n")};
        constexpr std::size_t size = ((part1.size() + part2.size()) * (N / 2) + (N % 2 == 0 ? 0 : part1.size())) + 1;

        std::array<char, size> result{};
        auto iter = result.begin();

        for (std::size_t i = 0; i < N; i++) {
            auto part = i % 2 == 0 ? part1 : part2;

            iter = std::ranges::copy(part, iter).out;
        }

        return result;
    }
} // namespace essence::detail

namespace essence {
    struct logging_string_view {
        std::string_view str;
        source_location location;

        template <std::convertible_to<std::string_view> S>
        constexpr logging_string_view(S&& str, // NOLINT(*-explicit-constructor)
            const source_location& location = source_location::current()) noexcept
            : str{std::forward<S>(str)}, location{location} {}
    };

    /**
     * @brief An exception class derived from std::runtime_error that provides source code information
     *        of the source file name, the line number, the column number and can be constructed with
     *        the same arguments as those passed to the essence::format function.
     */
    struct source_code_aware_runtime_error : std::runtime_error {
        template <typename... Args>
        explicit source_code_aware_runtime_error(logging_string_view hint, Args&&... args)
            : source_code_aware_runtime_error{hint.location, hint.str, std::forward<Args>(args)...} {}

        template <typename... Args>
        explicit source_code_aware_runtime_error(const source_location& location, Args&&... args)
            : runtime_error{[&] {
                  static constexpr auto format_array =
                      detail::make_format_str<sizeof...(Args) + 2 + (sizeof...(Args) == 1)>();
                  static constexpr std::string_view format_str{format_array.data()};

                  // Explicitly converts to std::string_view if the argument is convertible.
                  static constexpr auto converter = []<typename T>(T&& item) -> decltype(auto) {
                      if constexpr (std::convertible_to<T, std::string_view>) {
                          return std::string_view{std::forward<T>(item)};
                      } else {
                          return std::forward<T>(item);
                      }
                  };

                  // Removes the directory part from the file name to provide a simplified output.
                  const std::string_view file_name{location.file_name()};
                  auto file_name_without_directory = file_name.substr(file_name.find_last_of(U8(R"(/\)")) + 1);

                  if constexpr (sizeof...(Args) == 1) {
                      return format(format_str, U8("File"), file_name_without_directory, U8("Message"),
                          converter(std::forward<Args>(args))...);
                  } else {
                      return format(
                          format_str, U8("File"), file_name_without_directory, converter(std::forward<Args>(args))...);
                  }
              }()} {}
    };
} // namespace essence
