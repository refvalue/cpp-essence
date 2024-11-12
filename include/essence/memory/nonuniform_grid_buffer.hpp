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

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <numeric>
#include <span>
#include <type_traits>
#include <utility>

namespace essence {
    /**
     * @brief A fixed memory buffer, in which data are arranged within nonuniform grid cells.
     * @tparam T The type of one single cell.
     * @tparam Rows The number of rows.
     * @tparam ComponentCells The numbers of the cells of all components across one single row.
     */
    template <typename T, std::size_t Rows, std::size_t... ComponentCells>
    class nonuniform_grid_buffer {
    public:
        static constexpr std::array component_cells{ComponentCells...};
        static constexpr std::size_t row_count      = Rows;
        static constexpr std::size_t row_cell_count = (ComponentCells + ...);
        static constexpr std::size_t byte_stride    = row_cell_count * sizeof(T);

        template <std::size_t I, template <typename> typename Decorator = std::type_identity>
            requires(I < component_cells.size())
        using component_span = std::span<typename Decorator<T>::type, std::get<I>(component_cells)>;

        template <std::size_t I, template <typename> typename Decorator = std::type_identity>
            requires(I < component_cells.size())
        using component_byte_span =
            std::span<typename Decorator<std::byte>::type, std::get<I>(component_cells) * sizeof(T)>;

        nonuniform_grid_buffer() : buffer_{} {}

        /**
         * @brief Gets the component as a mutable cell span.
         * @tparam Row The index of the row.
         * @tparam I The index of the component.
         * @return The mutable cell span.
         */
        template <std::size_t Row, std::size_t I>
            requires(Row < Rows && I < component_cells.size())
        auto get_component() noexcept {
            auto iter = get_component_iter<Row, I, false>(*this);

            return component_span<I>{iter, iter + component_span<I>::extent};
        }

        /**
         * @brief Gets the component as a const cell span.
         * @tparam Row The index of the row.
         * @tparam I The index of the component.
         * @return The const cell span.
         */
        template <std::size_t Row, std::size_t I>
            requires(Row < Rows && I < component_cells.size())
        auto get_component() const noexcept {
            auto iter = get_component_iter<Row, I, false>(*this);

            return component_span<I, std::add_const>{iter, iter + component_span<I, std::add_const>::extent};
        }

        /**
         * @brief Sets the data of a component.
         * @tparam Row The index of the row.
         * @tparam I The index of the component.
         * @param data The byte data to assign.
         */
        template <std::size_t Row, std::size_t I>
            requires(Row < Rows && I < component_cells.size())
        void set_component(component_byte_span<I, std::add_const> data) noexcept {
            auto iter = get_component_iter<Row, I, true>(*this);

            std::ranges::copy(data.data(), data.data() + data.size(), iter);
        }

        /**
         * @brief Sets the data of a component.
         * @tparam Row The index of the row.
         * @tparam I The index of the component.
         * @param data The data in T units to assign.
         */
        template <std::size_t Row, std::size_t I>
            requires(Row < Rows && I < component_cells.size())
        void set_component(component_span<I, std::add_const> data) noexcept {
            set_component<Row, I>(std::as_bytes(data));
        }

        /**
         * @brief Gets the underlying buffer as a mutable byte span.
         * @return The mutable byte span.
         */
        std::span<std::byte, byte_stride * Rows> underlying_buffer() noexcept {
            return std::as_writable_bytes(std::span{buffer_});
        }

        /**
         * @brief Gets the underlying buffer as a const byte span.
         * @return The const byte span.
         */
        std::span<const std::byte, byte_stride * Rows> underlying_buffer() const noexcept {
            return std::as_bytes(std::span{buffer_});
        }

    private:
        template <std::size_t Row, std::size_t I, bool AsBytes, typename Self>
            requires(
                Row < Rows && I < component_cells.size() && std::same_as<std::decay_t<Self>, nonuniform_grid_buffer>)
        static auto get_component_iter(Self&& self) noexcept {
            // Calculates the byte offset of the component.
            static constexpr std::size_t offset =
                std::accumulate(component_cells.begin(), component_cells.begin() + I, 0U) + Row * row_cell_count;

            auto component_data = std::forward<Self>(self).buffer_.data() + offset;

            if constexpr (AsBytes) {
                // Simulates deducing this in C++23.
                if constexpr (std::is_const_v<std::remove_reference_t<Self>>) {
                    return reinterpret_cast<const std::byte*>(component_data);
                } else {
                    return reinterpret_cast<std::byte*>(component_data);
                }
            } else {
                return component_data;
            }
        }

        std::array<T, row_cell_count * Rows> buffer_;
    };
} // namespace essence
