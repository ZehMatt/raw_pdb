// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "Platform.h"

#include <cstdint>
#include <type_traits>
#if COMPILER_MSVC
#    include <intrin.h>
#endif
#include <cassert>

namespace raw_pdb
{
    namespace BitUtil
    {
        template<typename T> [[nodiscard]] inline constexpr bool IsPowerOfTwo(T value) noexcept
        {
            static_assert(std::is_unsigned<T>::value == true, "T must be an unsigned type.");

            assert(value != 0u);

            return (value & (value - 1u)) == 0u;
        }

        template<typename T> [[nodiscard]] inline constexpr T RoundUpToMultiple(T numToRound, T multipleOf) noexcept
        {
            static_assert(std::is_unsigned<T>::value == true, "T must be an unsigned type.");

            assert(IsPowerOfTwo(multipleOf));

            return (numToRound + (multipleOf - 1u)) & ~(multipleOf - 1u);
        }

        // Finds the position of the first set bit in the given value starting from the LSB, e.g. FindFirstSetBit(0b00000010)
        // == 1. This operation is also known as CTZ (Count Trailing Zeros).
        template<typename T> [[nodiscard]] inline uint32_t FindFirstSetBit(T value) noexcept;

        template<> [[nodiscard]] inline uint32_t FindFirstSetBit(uint32_t value) noexcept
        {
            assert(value != 0u);

#if COMPILER_MSVC
            unsigned long result = 0u;
            _BitScanForward(&result, value);
            return result;
#elif COMPILER_CLANG || COMPILER_GCC
            return static_cast<uint32_t>(__builtin_ctz(value));
#else
            static_assert(false, "Unsupported compiler");
#endif
        }
    } // namespace BitUtil
} // namespace raw_pdb
