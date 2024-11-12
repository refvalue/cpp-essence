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
#include "meta/literal_string.hpp"
#include "type_list.hpp"

#include <cstddef>
#include <span>
#include <type_traits>
#include <utility>

namespace essence::meta::detail {
    inline constexpr std::size_t max_fields = 256;

    template <typename T>
    consteval auto qualified_name_of_impl() noexcept {
#ifdef _MSC_VER
        // auto __cdecl essence::meta::detail::qualified_name_of_impl<T>(void) noexcept
        constexpr std::size_t suffix_size{16};
        constexpr std::string_view keyword{U8("essence::meta::detail::qualified_name_of_impl<")};
        constexpr std::string_view signature{__FUNCSIG__};
#elif defined(__GNUC__)
        // consteval auto essence::name_of_impl() [with T = int]
        constexpr std::size_t suffix_size{1};
        constexpr std::string_view keyword{U8("[with T = ")};
        constexpr std::string_view signature{__PRETTY_FUNCTION__};
#elif defined(__clang__)
        // auto essence::name_of_impl() [T = int]
        constexpr std::size_t suffix_size{1};
        constexpr std::string_view keyword{U8("[T = ")};
        constexpr std::string_view signature{__PRETTY_FUNCTION__};
#else
#error "Unsupported compiler."
#endif
        constexpr auto prefix_size  = signature.find(keyword) + keyword.size();
        constexpr auto intermediate = signature.substr(prefix_size, signature.size() - prefix_size - suffix_size);

        return meta::literal_string{
            std::span<const char, intermediate.size()>{intermediate.data(), intermediate.size()}};
    }
} // namespace essence::meta::detail

namespace essence::meta {
    template <typename T>
    concept reflectable =
        std::same_as<T, typename T::es_meta_this_type> && type_list_like<typename T::es_meta_field_info_list_type>;

    template <typename T, meta::literal_string Name, auto FieldPtr>
        requires std::is_member_object_pointer_v<decltype(FieldPtr)>
    struct field_info {
        using type = T;

        static constexpr auto&& name = Name;

        template <typename U>
            requires std::same_as<T, std::decay_t<U>>
        static constexpr decltype(auto) value_ref(U&& obj) noexcept {
            return (std::forward<U>(obj).*FieldPtr);
        }
    };

    template <typename T>
    struct qualified_name_of {
        static constexpr auto value = detail::qualified_name_of_impl<T>();
    };

    template <typename T>
    inline constexpr auto&& qualified_name_of_v = qualified_name_of<T>::value;

    template <typename T>
    struct name_of {
        static constexpr auto value = [] {
            // struct a::b::c::class_name<nested1::nested2::nested3::nested_class_name<...>> ->
            // class_name<...>
            constexpr std::string_view qualified_name{qualified_name_of_v<T>};
            constexpr auto template_argument_beginning_token_index = qualified_name.find_first_of(U8('<'));
            constexpr auto scope_token_index = qualified_name.rfind(U8("::"), template_argument_beginning_token_index);
            constexpr auto class_name_index  = scope_token_index != std::string_view::npos ? scope_token_index + 2 : 0;
            constexpr auto result            = qualified_name.substr(class_name_index);

            return meta::literal_string{std::span<const char, result.size()>{result.data(), result.size()}};
        }();
    };

    template <typename T>
    inline constexpr auto&& name_of_v = name_of<T>::value;

    template <typename T>
        requires reflectable<std::decay_t<T>>
    struct get_fields {
        using type = typename std::decay_t<T>::es_meta_field_info_list_type;
    };

    template <typename T>
    using get_fields_t = typename get_fields<T>::type;

    template <typename T, meta::literal_string Name>
        requires reflectable<std::decay_t<T>>
    struct get_field_by_name {
        using type = at_if_t<get_fields_t<T>, []<typename U>(std::type_identity<U>) { return U::name == Name; }>;
    };

    template <typename T, meta::literal_string Name>
    using get_field_by_name_t = typename get_field_by_name<T, Name>::type;
} // namespace essence::meta

#define ES_MAKE_TERMINATOR_IMPL(suffix)    using es_meta_terminator_##suffix = void
#define ES_MAKE_TERMINATOR(suffix)         ES_MAKE_TERMINATOR_IMPL(suffix)
#define ES_META_MAKE_FIELD_INFO_NAME(name) es_meta_field_info_##name
#define ES_META_GATHER_FIELDS              decltype(es_meta_gather_fields(essence::rank<essence::meta::detail::max_fields>{}))

#define ES_IDENTITY(...) decltype(static_cast<__VA_ARGS__ (*)()>(nullptr)())

#define ES_BEGIN_FIELDS(type)                                            \
    using es_meta_this_type = type;                                      \
    static constexpr std::string_view es_meta_this_type_name{U8(#type)}; \
    inline static essence::type_list<> es_meta_gather_fields(essence::rank<0>)

#define ES_FIELD(type, name)                                                                  \
    type name{};                                                                              \
    using ES_META_MAKE_FIELD_INFO_NAME(name) =                                                \
        essence::meta::field_info<es_meta_this_type, U8(#name), &es_meta_this_type::name>;   \
    inline static auto es_meta_gather_fields(essence::rank<ES_META_GATHER_FIELDS::size + 1>) \
        -> essence::append_t<ES_META_GATHER_FIELDS, ES_META_MAKE_FIELD_INFO_NAME(name)>

#define ES_END_FIELDS using es_meta_field_info_list_type = ES_META_GATHER_FIELDS
