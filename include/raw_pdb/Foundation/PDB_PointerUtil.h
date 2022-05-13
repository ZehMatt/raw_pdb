#pragma once

#include "PDB_DisableWarningsPop.h"
#include "PDB_DisableWarningsPush.h"
#include "PDB_Macros.h"

#include <type_traits>

namespace libpdb
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
} // namespace libpdb
