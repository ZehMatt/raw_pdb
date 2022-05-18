// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include <type_traits>

namespace raw_pdb
{
    namespace Pointer
    {
        // Offsets any pointer by a given number of bytes.
        template<typename T, typename U, typename V> [[nodiscard]] inline T Offset(U* anyPointer, V howManyBytes) noexcept
        {
            static_assert(std::is_pointer<T>::value == true, "Type T must be a pointer type.");
            static_assert(
                std::is_const<typename std::remove_pointer<T>::type>::value == std::is_const<U>::value, "Wrong constness.");

            union
            {
                T as_T;
                U* as_U_ptr;
                char* as_char_ptr;
            };

            as_U_ptr = anyPointer;
            as_char_ptr += howManyBytes;

            return as_T;
        }
    } // namespace Pointer
} // namespace raw_pdb
