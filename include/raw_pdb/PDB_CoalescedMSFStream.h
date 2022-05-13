// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "Foundation/PDB_Macros.h"
#include "PDB_Types.h"

// https://llvm.org/docs/PDB/index.html#the-msf-container
// https://llvm.org/docs/PDB/MsfFile.html
namespace libpdb
{
    class [[nodiscard]] DirectMSFStream;

    // provides access to a coalesced version of an MSF stream.
    // inherently thread-safe, the stream doesn't carry any internal offset or similar.
    // coalesces all blocks into a contiguous stream of data upon construction.
    // very fast individual reads, useful when almost all data of a stream is needed anyway.
    class [[nodiscard]] CoalescedMSFStream
    {
    public:
        CoalescedMSFStream(void) noexcept;
        CoalescedMSFStream(CoalescedMSFStream&& other) noexcept;
        CoalescedMSFStream& operator=(CoalescedMSFStream&& other) noexcept;

        explicit CoalescedMSFStream(
            const void* data, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept;

        // Creates a coalesced stream from a direct stream at any offset.
        explicit CoalescedMSFStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;

        ~CoalescedMSFStream(void) noexcept;

        // Returns the size of the stream.
        [[nodiscard]] inline size_t GetSize(void) const noexcept
        {
            return m_size;
        }

        // Provides read-only access to the data.
        template<typename T> [[nodiscard]] inline const T* GetDataAtOffset(size_t offset) const noexcept
        {
            return reinterpret_cast<const T*>(m_data + offset);
        }

    private:
        // contiguous, coalesced data, can be null
        Byte* m_ownedData;

        // either points to the owned data that has been copied from disjunct blocks, or points to the
        // memory-mapped data directly in case all stream blocks are contiguous.
        const Byte* m_data;
        size_t m_size;

        PDB_DISABLE_COPY(CoalescedMSFStream);
    };
} // namespace libpdb
