// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include <cstdint>
#include <type_traits>

namespace raw_pdb
{
    // Converts a block index into a file offset, based on the block size of the PDB file
    [[nodiscard]] static size_t ConvertBlockIndexToFileOffset(uint32_t blockIndex, uint32_t blockSize) noexcept
    {
        // cast to size_t to avoid potential overflow in 64-bit
        return static_cast<size_t>(blockIndex) * static_cast<size_t>(blockSize);
    }

    // Calculates how many blocks are needed for a certain number of bytes
    [[nodiscard]] static uint32_t ConvertSizeToBlockCount(uint32_t sizeInBytes, uint32_t blockSize) noexcept
    {
        // integer ceil to account for non-full blocks
        return (sizeInBytes + blockSize - 1u) / blockSize;
    };

    // Returns the actual size of the data associated with a CodeView record, not including the size of the header
    template<typename T> [[nodiscard]] static uint32_t GetCodeViewRecordSize(const T* record) noexcept
    {
        // the stored size includes the size of the 'kind' field, but not the size of the 'size' field itself
        return record->size - sizeof(uint16_t);
    }

    template<typename Header, typename T>
    [[nodiscard]] static size_t GetNameLength(const Header& header, const T& record) noexcept
    {
        // we can estimate the length of the string from the size of the record
        const size_t estimatedLength = header.size - sizeof(uint16_t) - sizeof(T);
        if (estimatedLength == 0u)
        {
            return estimatedLength;
        }

        // we still need to account for padding after the string to find the real length
        size_t nullTerminatorCount = 0u;
        for (/* nothing */; nullTerminatorCount < estimatedLength; ++nullTerminatorCount)
        {
            if (record.name[estimatedLength - nullTerminatorCount - 1u] != '\0')
            {
                break;
            }
        }

        const size_t length = estimatedLength - nullTerminatorCount;
        return length;
    }
} // namespace raw_pdb
