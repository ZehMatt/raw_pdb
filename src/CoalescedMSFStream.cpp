// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "CoalescedMSFStream.h"

#include "Core/PointerUtil.h"
#include "DirectMSFStream.h"
#include "Util.h"

#include <cstring>

namespace raw_pdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] static bool AreBlockIndicesContiguous(
        const uint32_t* blockIndices, uint32_t blockSize, uint32_t streamSize) noexcept
    {
        const uint32_t blockCount = ConvertSizeToBlockCount(streamSize, blockSize);

        // start with the first index, checking if all following indices are contiguous (N, N+1, N+2, ...)
        uint32_t expectedIndex = blockIndices[0];
        for (uint32_t i = 1u; i < blockCount; ++i)
        {
            ++expectedIndex;
            if (blockIndices[i] != expectedIndex)
            {
                return false;
            }
        }

        return true;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    CoalescedMSFStream::CoalescedMSFStream(void) noexcept
        : m_ownedData(nullptr)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    CoalescedMSFStream::CoalescedMSFStream(CoalescedMSFStream&& other) noexcept
        : m_base(std::move(other.m_base))
        , m_ownedData(std::move(other.m_ownedData))
        , m_data(std::move(other.m_data))
    {
        other.m_ownedData = nullptr;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    CoalescedMSFStream& CoalescedMSFStream::operator=(CoalescedMSFStream&& other) noexcept
    {
        if (this != &other)
        {
            m_base = std::move(other.m_base);
            m_ownedData = std::move(other.m_ownedData);
            m_data = std::move(other.m_data);

            other.m_ownedData = nullptr;
        }

        return *this;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    CoalescedMSFStream::CoalescedMSFStream(
        const ArrayView<Byte>& base, uint32_t blockSize, const uint32_t* blockIndices, uint32_t streamSize) noexcept
        : m_base(base)
        , m_ownedData(nullptr)
    {
        if (AreBlockIndicesContiguous(blockIndices, blockSize, streamSize))
        {
            // fast path, all block indices are contiguous, so we don't have to copy any data at all.
            // instead, we directly point into the memory-mapped file at the correct offset.
            const uint32_t index = blockIndices[0];
            const size_t fileOffset = ConvertBlockIndexToFileOffset(index, blockSize);
            m_data = { m_base.DataWithOffset<Byte>(fileOffset), streamSize };
        }
        else
        {
            // slower path, we need to copy disjunct blocks into our own data array, block by block
            m_ownedData = std::make_unique<Byte[]>(streamSize);
            m_data = { m_ownedData.get(), streamSize };

            Byte* destination = m_ownedData.get();

            // copy full blocks first
            const uint32_t fullBlockCount = streamSize / blockSize;
            for (uint32_t i = 0u; i < fullBlockCount; ++i)
            {
                const uint32_t index = blockIndices[i];

                // read one single block at the correct offset in the stream
                const size_t fileOffset = ConvertBlockIndexToFileOffset(index, blockSize);
                if (!m_base.CanRead(fileOffset, blockSize))
                {
                    assert(false);
                }

                const void* sourceData = m_base.DataWithOffset<const Byte>(fileOffset);
                std::memcpy(destination, sourceData, blockSize);

                destination += blockSize;
            }

            // account for non-full blocks
            const uint32_t remainingBytes = streamSize - (fullBlockCount * blockSize);
            if (remainingBytes != 0u)
            {
                const uint32_t index = blockIndices[fullBlockCount];

                // read remaining bytes at correct offset in the stream
                const size_t fileOffset = ConvertBlockIndexToFileOffset(index, blockSize);
                if (!m_base.CanRead(fileOffset, remainingBytes))
                {
                    assert(false);
                }

                const void* sourceData = m_base.DataWithOffset<const Byte>(fileOffset);
                std::memcpy(destination, sourceData, remainingBytes);
            }
        }
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    CoalescedMSFStream::CoalescedMSFStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept
        : m_base(directStream.GetBase())
        , m_ownedData(nullptr)
    {
        const uint32_t* const blockIndicesForOffset = directStream.GetBlockIndicesForOffset(offset);

        if (AreBlockIndicesContiguous(blockIndicesForOffset, directStream.GetBlockSize(), size))
        {
            // fast path, all block indices inside the direct stream from (data + offset) to (data + offset + size) are
            // contiguous
            const size_t offsetWithinData = directStream.GetDataOffsetForOffset(offset);
            m_data = { directStream.GetBase().DataWithOffset<Byte>(offsetWithinData), size };
        }
        else
        {
            // slower path, we need to copy from disjunct blocks, which is performed by the direct stream
            m_ownedData = std::make_unique<Byte[]>(size);
            m_data = { m_ownedData.get(), size };

            directStream.ReadAtOffset(m_ownedData.get(), size, offset);
        }
    }

} // namespace raw_pdb