// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "DirectMSFStream.h"

#include "Core/BitUtil.h"
#include "Core/PointerUtil.h"

#include <cassert>
#include <cstring>

namespace raw_pdb
{
    DirectMSFStream::DirectMSFStream(
        const ArrayView<Byte>& data, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept
        : m_base(data)
        , m_blockIndices(blockIndices)
        , m_blockSize(blockSize)
        , m_size(streamSize)
        , m_blockSizeLog2(BitUtil::FindFirstSetBit(blockSize))
    {
        assert(BitUtil::IsPowerOfTwo(blockSize));
    }

    void DirectMSFStream::ReadAtOffset(void* destination, size_t size, size_t offset) const noexcept
    {
        assert(offset + size <= m_size);

        // work out which block and offset within the block the read offset corresponds to
        size_t blockIndex = offset >> m_blockSizeLog2;
        const size_t offsetWithinBlock = offset & (m_blockSize - 1u);

        // work out the offset within the data based on the block indices
        size_t offsetWithinData = (static_cast<size_t>(m_blockIndices[blockIndex]) << m_blockSizeLog2) + offsetWithinBlock;
        const size_t bytesLeftInBlock = m_blockSize - offsetWithinBlock;

        if (bytesLeftInBlock >= size)
        {
            // fast path, all the data can be read in one go
            const void* const sourceData = m_base.DataWithOffset<const Byte>(offsetWithinData);
            std::memcpy(destination, sourceData, size);
        }
        else
        {
            // slower path, data is scattered across several blocks.
            // read remaining bytes in current block first.
            {
                const void* const sourceData = m_base.DataWithOffset<const Byte>(offsetWithinData);
                std::memcpy(destination, sourceData, bytesLeftInBlock);
            }

            // read remaining bytes from blocks
            size_t bytesLeftToRead = size - bytesLeftInBlock;
            while (bytesLeftToRead != 0u)
            {
                // advance to the next block
                ++blockIndex;
                offsetWithinData = static_cast<size_t>(m_blockIndices[blockIndex]) << m_blockSizeLog2;

                void* const destinationData = Pointer::Offset<void*>(destination, size - bytesLeftToRead);
                const void* const sourceData = m_base.DataWithOffset<const Byte>(offsetWithinData);

                if (bytesLeftToRead > m_blockSize)
                {
                    // copy a whole block at once
                    std::memcpy(destinationData, sourceData, m_blockSize);
                    bytesLeftToRead -= m_blockSize;
                }
                else
                {
                    // copy remaining bytes
                    std::memcpy(destinationData, sourceData, bytesLeftToRead);
                    bytesLeftToRead -= bytesLeftToRead;
                }
            }
        }
    }

    const uint32_t* DirectMSFStream::GetBlockIndicesForOffset(uint32_t offset) const noexcept
    {
        const size_t firstBlockIndex = offset >> m_blockSizeLog2;

        return m_blockIndices + firstBlockIndex;
    }

    size_t DirectMSFStream::GetDataOffsetForOffset(uint32_t offset) const noexcept
    {
        // work out which block and offset within the block the offset corresponds to
        const size_t blockIndex = offset >> m_blockSizeLog2;
        const size_t offsetWithinBlock = offset & (m_blockSize - 1u);

        // work out the offset within the data based on the block indices
        const size_t offsetWithinData = (m_blockIndices[blockIndex] << m_blockSizeLog2) + offsetWithinBlock;

        return offsetWithinData;
    }

} // namespace raw_pdb