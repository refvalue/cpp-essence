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

#include "../common_types.hpp"
#include "../detail/data_member_binding.hpp"
#include "../naming_convention.hpp"
#include "../struct.hpp"

#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace essence::meta::runtime {
    /**
     * @brief Information of a direct data member of a class.
     * @tparam T The type of the enclosing class.
     */
    template <typename T>
        requires std::is_class_v<T>
    struct data_member_info {
        /**
         * @brief The type of the enclosing class.
         */
        using enclosing_type = T;

        /**
         * @brief The name of the type of the enclosing class.
         */
        inline static const std::string enclosing_type_name{get_literal_string_t<T>()};


        /**
         * @brief The original name of the data member without a conversion of the naming convention.
         */
        std::string raw_name;

        /**
         * @brief The name of the data member.
         */
        std::string name;
    };

    template <typename T>
        requires std::is_class_v<T>
    std::span<const std::string> get_data_member_names() {
        static const auto cache = [] {
            std::vector<std::string> result;

            for (auto&& item : meta::probe_data_member_names<T>()) {
                result.emplace_back(std::string{item});
            }

            return result;
        }();

        return cache;
    }

    /**
     * @brief Gets the list of information of direct data members of a class.
     * @tparam Convention The naming convention to be converted to.
     * @tparam T The type of the class.
     * @return The list of information of direct data members.
     */
    template <naming_convention Convention, typename T>
        requires std::is_class_v<T>
    std::span<const data_member_info<T>> get_data_member_info() {
        static const auto cache = [] {
            std::vector<data_member_info<T>> result;

            for (auto&& item : get_data_member_names<T>()) {
                result.emplace_back(item, convert_naming_convention(item, Convention));
            }

            return result;
        }();

        return cache;
    }

    /**
     * @brief Enumerates direct data members of a class.
     * @tparam Convention The naming convention to be converted to.
     * @tparam T The type of the class.
     * @tparam Callable The type of callable to process each item.
     * @param obj The object.
     * @param handler A handler to process each item.
     */
    template <naming_convention Convention = naming_convention::none, typename T, typename Callable>
        requires std::is_class_v<std::decay_t<T>>
    void enumerate_data_members(T&& obj, Callable&& handler) {
        using decayed_type = std::decay_t<T>;

        static const auto cache             = get_data_member_info<Convention, decayed_type>();
        static constexpr auto make_ref_info = []<typename U>(const data_member_info<decayed_type>& item, U& reference) {
            struct ref_info {
                using enclosing_type = decayed_type;

                const std::string& enclosing_type_name;
                const std::string& raw_name;
                const std::string& name;
                U& reference;
            };

            return ref_info{
                .enclosing_type_name = item.enclosing_type_name,
                .raw_name            = item.raw_name,
                .name                = item.name,
                .reference           = reference,
            };
        };

        auto pointers = detail::make_data_member_pointers(std::forward<T>(obj));
        auto tuple    = std::forward_as_tuple(std::forward<Callable>(handler));

        [&]<std::size_t... Is>(std::index_sequence<Is...>) {
            std::apply(
                [&pointers]<typename InnerCallable>(InnerCallable&& handler) {
                    std::forward<InnerCallable>(handler)(make_ref_info(cache[Is], *std::get<Is>(pointers))...);
                },
                std::move(tuple));
        }(std::make_index_sequence<std::tuple_size_v<decltype(pointers)>>{});
    }
} // namespace essence::meta::runtime
