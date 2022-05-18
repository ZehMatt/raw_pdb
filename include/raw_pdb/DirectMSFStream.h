// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "Core/ArrayView.h"
#include "PDBTypes.h"

#include <cstdint>

// https://llvm.org/docs/PDB/index.html#the-msf-container
// https://llvm.org/docs/PDB/MsfFile.html
namespace raw_pdb
{
    // provides direct access to the data of an MSF stream.
    // inherently thread-safe, the stream doesn't carry any internal offset or similar.
    // trivial to construct.
    // slower individual reads, but pays off when not all data of a stream is needed.
    class [[nodiscard]] DirectMSFStream
    {
    public:
        DirectMSFStream(void) noexcept = default;
        DirectMSFStream(DirectMSFStream&& other) noexcept = default;
        DirectMSFStream(const DirectMSFStream& other) = delete;
        explicit DirectMSFStream(
            const ArrayView<Byte>& data, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept;

        DirectMSFStream& operator=(const DirectMSFStream& other) = delete;

        // Reads a number of bytes from the stream.
        void ReadAtOffset(void* destination, size_t size, size_t offset) const noexcept;

        // Reads from the stream.
        template<typename T> [[nodiscard]] inline T ReadAtOffset(size_t offset) const noexcept
        {
            T data;
            ReadAtOffset(&data, sizeof(T), offset);
            return data;
        }

        // Returns the block size of the stream.
        [[nodiscard]] inline uint32_t GetBlockSize(void) const noexcept
        {
            return m_blockSize;
        }

        // Returns the size of the stream.
        [[nodiscard]] inline uint32_t GetSize(void) const noexcept
        {
            return m_size;
        }

        // Provides read-only access to the memory-mapped data.
        [[nodiscard]] inline const ArrayView<Byte>& GetBase(void) const noexcept
        {
            return m_base;
        }

    private:
        friend class CoalescedMSFStream;

        // Returns the block indices that correspond to the given offset.
        [[nodiscard]] const uint32_t* GetBlockIndicesForOffset(uint32_t offset) const noexcept;

        // Returns the offset into the data that corresponds to the given offset.
        [[nodiscard]] size_t GetDataOffsetForOffset(uint32_t offset) const noexcept;

        const ArrayView<Byte> m_base;
        const uint32_t* m_blockIndices{};
        uint32_t m_blockSize{};
        uint32_t m_size{};
        uint32_t m_blockSizeLog2{};
    };
} // namespace raw_pdb
