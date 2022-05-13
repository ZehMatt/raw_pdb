#pragma once

#include "PDB_Macros.h"

#include <cassert>

namespace libpdb
{
    // A read-only view into arrays of any type and length.
    template<typename T> class [[nodiscard]] ArrayView
    {
    public:
        // Constructs an array view from a C array with explicit length.
        inline constexpr explicit ArrayView(const T* const array, size_t length) noexcept
            : m_data(array)
            , m_length(length)
        {
        }

        PDB_DEFAULT_COPY_CONSTRUCTOR(ArrayView);
        PDB_DEFAULT_MOVE_CONSTRUCTOR(ArrayView);

        // Provides read-only access to the underlying array.
        [[nodiscard]] inline constexpr const T* Decay(void) const noexcept
        {
            return m_data;
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
        const T* const m_data;
        const size_t m_length;

        PDB_DISABLE_MOVE_ASSIGNMENT(ArrayView);
        PDB_DISABLE_COPY_ASSIGNMENT(ArrayView);
    };
} // namespace libpdb
