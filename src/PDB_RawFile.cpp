#include "PDB_RawFile.h"

#include "Foundation/PDB_Memory.h"
#include "Foundation/PDB_Move.h"
#include "Foundation/PDB_PointerUtil.h"
#include "PDB_DirectMSFStream.h"
#include "PDB_Types.h"
#include "PDB_Util.h"

#include <cassert>
#include <utility>

namespace libpdb
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
        other.m_data = nullptr;
        other.m_superBlock = nullptr;
        other.m_streamCount = 0u;
        other.m_streamSizes = nullptr;
        other.m_streamBlocks = nullptr;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile& RawFile::operator=(RawFile&& other) noexcept
    {
        if (this != &other)
        {
            PDB_DELETE_ARRAY(m_streamBlocks);

            m_data = std::move(other.m_data);
            m_superBlock = std::move(other.m_superBlock);
            m_directoryStream = std::move(other.m_directoryStream);
            m_streamCount = std::move(other.m_streamCount);
            m_streamSizes = std::move(other.m_streamSizes);
            m_streamBlocks = std::move(other.m_streamBlocks);

            other.m_data = nullptr;
            other.m_superBlock = nullptr;
            other.m_streamCount = 0u;
            other.m_streamSizes = nullptr;
            other.m_streamBlocks = nullptr;
        }

        return *this;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile::RawFile(const void* data) noexcept
        : m_data(data)
        , m_superBlock(Pointer::Offset<const SuperBlock*>(data, 0u))
        , m_directoryStream()
        , m_streamCount(0u)
        , m_streamSizes(nullptr)
        , m_streamBlocks(nullptr)
    {
        // the SuperBlock stores an array of indices of blocks that make up the indices of directory blocks, which need to be
        // stitched together to form the directory. the blocks holding the indices of directory blocks are not necessarily
        // contiguous, so they need to be coalesced first.
        const uint32_t directoryBlockCount = ConvertSizeToBlockCount(m_superBlock->directorySize, m_superBlock->blockSize);

        // the directory is made up of directoryBlockCount blocks, so we need that many indices to be read from the blocks that
        // make up the indices
        CoalescedMSFStream directoryIndicesStream(
            data, m_superBlock->blockSize, m_superBlock->directoryBlockIndices, directoryBlockCount * sizeof(uint32_t));

        // these are the indices of blocks making up the directory stream, now guaranteed to be contiguous
        const uint32_t* directoryIndices = directoryIndicesStream.GetDataAtOffset<uint32_t>(0u);

        m_directoryStream = CoalescedMSFStream(data, m_superBlock->blockSize, directoryIndices, m_superBlock->directorySize);

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
        m_streamBlocks = PDB_NEW_ARRAY(const uint32_t*, m_streamCount);

        const uint32_t* indicesForCurrentBlock = directoryStreamBlocks;
        for (uint32_t i = 0u; i < m_streamCount; ++i)
        {
            const uint32_t sizeInBytes = m_streamSizes[i];
            const uint32_t blockCount = ConvertSizeToBlockCount(sizeInBytes, m_superBlock->blockSize);
            m_streamBlocks[i] = indicesForCurrentBlock;

            indicesForCurrentBlock += blockCount;
        }
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile::~RawFile(void) noexcept
    {
        PDB_DELETE_ARRAY(m_streamBlocks);
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

} // namespace libpdb