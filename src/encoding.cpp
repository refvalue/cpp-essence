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

#include "encoding.hpp"

#include "char.hpp"
#include "char8_t_remediation.hpp"

#include <array>
#include <concepts>
#include <cstddef>

#ifdef __UCLIBC__
#include <cuchar>

namespace essence {
    namespace {
        constexpr auto c16rtomb_impl = [](char* s, char16_t c16, std::mbstate_t* ps) -> std::size_t { return -1; };
        constexpr auto mbrtoc16_impl = [](char16_t* pc16, const char* s, std::size_t n,
                                           std::mbstate_t* ps) -> std::size_t { return -1; };
    } // namespace
} // namespace essence
#elif __has_include(<cuchar> )
#include <cuchar>

namespace essence {
    namespace {
        constexpr auto&& c16rtomb_impl = c16rtomb;
        constexpr auto&& mbrtoc16_impl = mbrtoc16;
    } // namespace
} // namespace essence
#else
#include <uchar.h>

namespace essence {
    namespace {
        constexpr auto&& c16rtomb_impl = ::c16rtomb;
        constexpr auto&& mbrtoc16_impl = ::mbrtoc16;
    } // namespace
} // namespace essence
#endif

namespace essence {
    namespace {
        template <typename T>
        concept mutable_container = requires(T obj, std::size_t size) {
            typename T::value_type;
            obj.shrink_to_fit();
            obj.reserve(size);
            obj.push_back(typename T::value_type{});
        };

        template <typename T>
        concept utf16_value = std::same_as<T, char16_t> || std::same_as<T, std::uint16_t>
                           || (sizeof(wchar_t) == sizeof(char16_t) && std::same_as<T, wchar_t>);

        template <mutable_container Container, utf16_value T>
            requires byte_like<typename Container::value_type>
        Container u16_to_native_bytes(std::span<const T> str) {
            Container result;

            if (str.empty()) {
                return result;
            }

            mbstate_t state{};
            std::array<typename Container::value_type, MB_LEN_MAX> buffer{};

            result.reserve(str.size() * 2);

            for (auto&& item : str) {
                if (auto rc =
                        c16rtomb_impl(reinterpret_cast<char*>(buffer.data()), static_cast<char16_t>(item), &state);
                    rc != static_cast<std::size_t>(-1)) {
                    result.append(buffer.data(), rc);
                }
            }

            result.shrink_to_fit();

            return result;
        }

        template <mutable_container Container, byte_like T>
            requires utf16_value<typename Container::value_type>
        Container native_bytes_to_u16(std::span<const T> str) {
            Container result;

            if (str.empty()) {
                return result;
            }

            auto ptr     = str.data();
            auto ptr_end = str.data() + str.size();

            char16_t c16{};
            mbstate_t state{};

            result.reserve(str.size());

            while (auto rc = mbrtoc16_impl(&c16, ptr, ptr_end - ptr + 1, &state)) {
                if (rc == static_cast<std::size_t>(-3)) {
                    continue;
                }

                if (rc == static_cast<std::size_t>(-2)) {
                    break;
                }

                if (rc == static_cast<std::size_t>(-1)) {
                    break;
                }

                ptr += rc;
                result.push_back(static_cast<typename Container::value_type>(c16));
            }

            result.shrink_to_fit();

            return result;
        }
    } // namespace

#ifdef _WIN32
    namespace internal {
        std::string to_utf8_string(nstring_view str) {
            return u16_to_native_bytes<std::string>(std::span{str});
        }

        nstring to_native_string(std::string_view utf8_str) {
            return native_bytes_to_u16<nstring>(std::span{utf8_str});
        }
    } // namespace internal

    abi::string to_utf8_string(nstring_view str) {
        return u16_to_native_bytes<abi::string>(std::span{str});
    }

    abi::nstring to_native_string(std::string_view utf8_str) {
        return native_bytes_to_u16<abi::nstring>(std::span{utf8_str});
    }
#else
    namespace internal {
        std::string to_utf8_string(nstring_view str) {
            return std::string{str};
        }

        nstring to_native_string(std::string_view utf8_str) {
            return nstring{utf8_str};
        }
    } // namespace internal

    abi::string to_utf8_string(nstring_view str) {
        return abi::string{str};
    }

    abi::nstring to_native_string(std::string_view utf8_str) {
        return abi::nstring{utf8_str};
    }
#endif

    namespace internal {
        std::string to_utf8_string(std::u16string_view str) {
            return u16_to_native_bytes<std::string>(std::span{str.begin(), str.end()});
        }

        std::string to_utf8_string(std::span<const std::uint16_t> literal) {
            return u16_to_native_bytes<std::string>(literal);
        }

        std::u16string to_utf16_string(std::string_view utf8_str) {
            return native_bytes_to_u16<std::u16string>(std::span{utf8_str.begin(), utf8_str.end()});
        }

        std::vector<std::uint16_t> to_uint16_t_literal(std::string_view utf8_str) {
            return native_bytes_to_u16<std::vector<std::uint16_t>>(std::span{utf8_str.begin(), utf8_str.end()});
        }
    } // namespace internal

    abi::string to_utf8_string(std::u16string_view str) {
        return u16_to_native_bytes<abi::string>(std::span{str.begin(), str.end()});
    }

    abi::string to_utf8_string(std::span<const std::uint16_t> literal) {
        return u16_to_native_bytes<abi::string>(literal);
    }

    abi::u16string to_utf16_string(std::string_view utf8_str) {
        return native_bytes_to_u16<abi::u16string>(std::span{utf8_str.begin(), utf8_str.end()});
    }

    abi::vector<std::uint16_t> to_uint16_t_literal(std::string_view utf8_str) {
        return native_bytes_to_u16<abi::vector<std::uint16_t>>(std::span{utf8_str.begin(), utf8_str.end()});
    }
} // namespace essence
