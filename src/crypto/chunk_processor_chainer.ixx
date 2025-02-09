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

#include <essence/compat.hpp>

export module essence.crypto:chunk_processor_chainer;
import :abstract.chunk_processor;
import :chunk_processor;
import std;

export namespace essence::crypto {
    struct make_chain_tag {};

    ES_API(CPPESSENCE) constexpr make_chain_tag make_chain;

    inline auto operator,(abstract::chunk_processor left, abstract::chunk_processor right) {
        return std::tuple{std::move(left), std::move(right)};
    }

    template <typename... Ts>
        requires(std::same_as<Ts, abstract::chunk_processor> && ...)
    auto operator,(std::tuple<Ts...> tuple, abstract::chunk_processor right) {
        return std::tuple_cat(std::move(tuple), std::tuple{std::move(right)});
    }

    template <typename... Ts, typename... Us>
        requires((std::same_as<Ts, abstract::chunk_processor> && ...)
                    && (std::same_as<Us, abstract::chunk_processor> && ...))
    auto operator,(std::tuple<Ts...> left, std::tuple<Us...> right) {
        return std::tuple_cat(std::move(left), std::move(right));
    }

    template <typename... Ts>
        requires(std::same_as<Ts, abstract::chunk_processor> && ...)
    auto operator,(std::tuple<Ts...> tuple, make_chain_tag) {
        return std::apply(
            []<typename... Args>(Args&&... args) {
                std::array processors{std::move(args)...};

                return chain_chunk_processors(processors);
            },
            std::move(tuple));
    }
} // namespace essence::crypto
