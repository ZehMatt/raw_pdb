// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include <cassert>
#include <cstdint>

namespace raw_pdb
{
    // A read-only view into arrays of any type and length.
    template<typename T> class ArrayView
    {
    public:
        // Constructs an array view from a C array with explicit length.
        ArrayView(const T* array, size_t length) noexcept
            : m_data(array)
            , m_length(length)
        {
        }

        ArrayView() = default;
        ArrayView(const ArrayView& other) = default;

        ArrayView& operator=(const ArrayView& other)
        {
            m_data = other.m_data;
            m_length = other.m_length;
            return *this;
        }

        ArrayView& operator=(ArrayView&& other)
        {
            m_data = other.m_data;
            m_length = other.m_length;
            return *this;
        }

        // Provides read-only access to the underlying array.
        [[nodiscard]] inline constexpr const T* Data(void) const noexcept
        {
            return m_data;
        }

        // Provides read-only access to the underlying array.
        template<typename T2> [[nodiscard]] inline constexpr const T2* DataWithOffset(size_t offset) const noexcept
        {
            if (offset + sizeof(T2) > m_length)
                return nullptr;

            auto* res = reinterpret_cast<const Byte*>(m_data) + offset;

            return reinterpret_cast<const T2*>(res);
        }

        [[nodiscard]] inline constexpr bool CanRead(size_t offset, size_t size) const noexcept
        {
            if (offset + size > m_length)
                return false;

            return true;
        }

        // Returns the length of the view.
        [[nodiscard]] inline constexpr size_t GetLength(void) const noexcept
        {
            return m_length;
        }

        // Returns the i-th element.
        [[nodiscard]] inline const T& operator[](size_t i) const noexcept
        {
            assert(i < GetLength());
            return m_data[i];
        }

        // ------------------------------------------------------------------------------------------------
        // Range-based for-loop support
        // ------------------------------------------------------------------------------------------------

        [[nodiscard]] inline const T* begin(void) const noexcept
        {
            return m_data;
        }

        [[nodiscard]] inline const T* end(void) const noexcept
        {
            return m_data + m_length;
        }

    private:
        const T* m_data{};
        size_t m_length{};
    };
} // namespace raw_pdb
