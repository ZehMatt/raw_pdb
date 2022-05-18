// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "Core/ArrayView.h"
#include "PDBTypes.h"
#include "Util.h"

#include <memory>
#include <stdexcept>

// https://llvm.org/docs/PDB/index.html#the-msf-container
// https://llvm.org/docs/PDB/MsfFile.html
namespace raw_pdb
{
    class DirectMSFStream;

    // provides access to a coalesced version of an MSF stream.
    // inherently thread-safe, the stream doesn't carry any internal offset or similar.
    // coalesces all blocks into a contiguous stream of data upon construction.
    // very fast individual reads, useful when almost all data of a stream is needed anyway.
    class CoalescedMSFStream
    {
    public:
        CoalescedMSFStream(void) noexcept;
        CoalescedMSFStream(CoalescedMSFStream&& other) noexcept;
        CoalescedMSFStream& operator=(CoalescedMSFStream&& other) noexcept;
        CoalescedMSFStream(const CoalescedMSFStream& other) = delete;

        explicit CoalescedMSFStream(
            const ArrayView<Byte>& base, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept;

        // Creates a coalesced stream from a direct stream at any offset.
        explicit CoalescedMSFStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;

        CoalescedMSFStream& operator=(const CoalescedMSFStream& other) = delete;

        // Returns the size of the stream.
        [[nodiscard]] inline size_t GetSize(void) const noexcept
        {
            return m_data.GetLength();
        }

        // Provides read-only access to the data.
        template<typename T> [[nodiscard]] inline const T* GetDataAtOffset(size_t offset) const
        {
            return m_data.DataWithOffset<T>(offset);
        }

        [[nodiscard]] inline constexpr bool CanRead(size_t offset, size_t len) const noexcept
        {
            return m_data.CanRead(offset, len);
        }

    private:
        ArrayView<Byte> m_base{};

        // contiguous, coalesced data, can be null
        std::unique_ptr<Byte[]> m_ownedData{};

        // either points to the owned data that has been copied from disjunct blocks, or points to the
        // memory-mapped data directly in case all stream blocks are contiguous.
        ArrayView<Byte> m_data{};
    };
} // namespace raw_pdb
