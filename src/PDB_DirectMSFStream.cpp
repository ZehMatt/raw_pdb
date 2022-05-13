#include "PDB_DirectMSFStream.h"

#include "Foundation/PDB_BitUtil.h"
#include "Foundation/PDB_DisableWarningsPop.h"
#include "Foundation/PDB_DisableWarningsPush.h"
#include "Foundation/PDB_PointerUtil.h"

#include <cassert>
#include <cstring>

namespace libpdb
{
    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    DirectMSFStream::DirectMSFStream(void) noexcept
        : m_data(nullptr)
        , m_blockIndices(nullptr)
        , m_blockSize(0u)
        , m_size(0u)
        , m_blockSizeLog2(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    DirectMSFStream::DirectMSFStream(
        const void* data, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept
        : m_data(data)
        , m_blockIndices(blockIndices)
        , m_blockSize(blockSize)
        , m_size(streamSize)
        , m_blockSizeLog2(BitUtil::FindFirstSetBit(blockSize))
    {
        assert(BitUtil::IsPowerOfTwo(blockSize));
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
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
            const void* const sourceData = Pointer::Offset<const void*>(m_data, offsetWithinData);
            std::memcpy(destination, sourceData, size);
        }
        else
        {
            // slower path, data is scattered across several blocks.
            // read remaining bytes in current block first.
            {
                const void* const sourceData = Pointer::Offset<const void*>(m_data, offsetWithinData);
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
                const void* const sourceData = Pointer::Offset<const void*>(m_data, offsetWithinData);

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

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] const uint32_t* DirectMSFStream::GetBlockIndicesForOffset(uint32_t offset) const noexcept
    {
        const size_t firstBlockIndex = offset >> m_blockSizeLog2;

        return m_blockIndices + firstBlockIndex;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] size_t DirectMSFStream::GetDataOffsetForOffset(uint32_t offset) const noexcept
    {
        // work out which block and offset within the block the offset corresponds to
        const size_t blockIndex = offset >> m_blockSizeLog2;
        const size_t offsetWithinBlock = offset & (m_blockSize - 1u);

        // work out the offset within the data based on the block indices
        const size_t offsetWithinData = (m_blockIndices[blockIndex] << m_blockSizeLog2) + offsetWithinBlock;

        return offsetWithinData;
    }

} // namespace libpdb