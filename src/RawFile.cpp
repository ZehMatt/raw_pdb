// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "RawFile.h"

#include "Core/PointerUtil.h"
#include "DirectMSFStream.h"
#include "PDBTypes.h"
#include "Util.h"

#include <cassert>
#include <utility>

namespace raw_pdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile::RawFile(RawFile&& other) noexcept
        : m_data(std::move(other.m_data))
        , m_superBlock(std::move(other.m_superBlock))
        , m_directoryStream(std::move(other.m_directoryStream))
        , m_streamCount(std::move(other.m_streamCount))
        , m_streamSizes(std::move(other.m_streamSizes))
        , m_streamBlocks(std::move(other.m_streamBlocks))
    {
        other.m_superBlock = nullptr;
        other.m_streamCount = 0u;
        other.m_streamSizes = nullptr;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile& RawFile::operator=(RawFile&& other) noexcept
    {
        if (this != &other)
        {
            m_data = other.m_data;
            m_superBlock = std::move(other.m_superBlock);
            m_directoryStream = std::move(other.m_directoryStream);
            m_streamCount = std::move(other.m_streamCount);
            m_streamSizes = std::move(other.m_streamSizes);
            m_streamBlocks = std::move(other.m_streamBlocks);

            other.m_superBlock = nullptr;
            other.m_streamCount = 0u;
            other.m_streamSizes = nullptr;
        }

        return *this;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile::RawFile(const void* data, size_t dataSize) noexcept
        : m_data(reinterpret_cast<const Byte*>(data), dataSize)
        , m_superBlock(Pointer::Offset<const SuperBlock*>(data, 0u))
        , m_directoryStream()
        , m_streamCount(0u)
        , m_streamSizes(nullptr)
    {
        // the SuperBlock stores an array of indices of blocks that make up the indices of directory blocks, which need to be
        // stitched together to form the directory. the blocks holding the indices of directory blocks are not necessarily
        // contiguous, so they need to be coalesced first.
        const uint32_t directoryBlockCount = ConvertSizeToBlockCount(m_superBlock->directorySize, m_superBlock->blockSize);

        // the directory is made up of directoryBlockCount blocks, so we need that many indices to be read from the blocks that
        // make up the indices
        CoalescedMSFStream directoryIndicesStream(
            m_data, m_superBlock->blockSize, m_superBlock->VariableData(), directoryBlockCount * sizeof(uint32_t));

        // these are the indices of blocks making up the directory stream, now guaranteed to be contiguous
        const uint32_t* directoryIndices = directoryIndicesStream.GetDataAtOffset<uint32_t>(0u);

        m_directoryStream = CoalescedMSFStream(m_data, m_superBlock->blockSize, directoryIndices, m_superBlock->directorySize);

        // https://llvm.org/docs/PDB/MsfFile.html#the-stream-directory
        // parse the directory from its contiguous version. the directory matches the following struct:
        //	struct StreamDirectory
        //	{
        //		uint32_t streamCount;
        //		uint32_t streamSizes[streamCount];
        //		uint32_t streamBlocks[streamCount][];
        //	};
        m_streamCount = *m_directoryStream.GetDataAtOffset<uint32_t>(0u);

        // we can assign pointers into the stream directly, since the RawFile keeps ownership of the directory stream
        m_streamSizes = m_directoryStream.GetDataAtOffset<uint32_t>(sizeof(uint32_t));
        const uint32_t* directoryStreamBlocks = m_directoryStream.GetDataAtOffset<uint32_t>(
            sizeof(uint32_t) + sizeof(uint32_t) * m_streamCount);

        // prepare indices for directly accessing individual streams
        m_streamBlocks.reserve(m_streamCount);

        const uint32_t* indicesForCurrentBlock = directoryStreamBlocks;
        for (uint32_t i = 0u; i < m_streamCount; ++i)
        {
            const uint32_t sizeInBytes = m_streamSizes[i];
            const uint32_t blockCount = ConvertSizeToBlockCount(sizeInBytes, m_superBlock->blockSize);
            m_streamBlocks.push_back(indicesForCurrentBlock);

            indicesForCurrentBlock += blockCount;
        }
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    template<typename T> [[nodiscard]] T RawFile::CreateMSFStream(uint32_t streamIndex) const noexcept
    {
        return T(m_data, m_superBlock->blockSize, m_streamBlocks[streamIndex], m_streamSizes[streamIndex]);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    template<typename T> [[nodiscard]] T RawFile::CreateMSFStream(uint32_t streamIndex, uint32_t streamSize) const noexcept
    {
        assert(streamSize <= m_streamSizes[streamIndex]);

        return T(m_data, m_superBlock->blockSize, m_streamBlocks[streamIndex], streamSize);
    }

    // explicit template instantiation
    template CoalescedMSFStream RawFile::CreateMSFStream<CoalescedMSFStream>(uint32_t streamIndex) const noexcept;
    template DirectMSFStream RawFile::CreateMSFStream<DirectMSFStream>(uint32_t streamIndex) const noexcept;

    template CoalescedMSFStream RawFile::CreateMSFStream<CoalescedMSFStream>(
        uint32_t streamIndex, uint32_t streamSize) const noexcept;
    template DirectMSFStream RawFile::CreateMSFStream<DirectMSFStream>(
        uint32_t streamIndex, uint32_t streamSize) const noexcept;

} // namespace raw_pdb