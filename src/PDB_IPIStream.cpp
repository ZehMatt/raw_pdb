#include "PDB_IPIStream.h"

#include "Foundation/PDB_Memory.h"
#include "Foundation/PDB_Move.h"
#include "PDB_DirectMSFStream.h"
#include "PDB_RawFile.h"
#include "PDB_Util.h"

namespace libpdb
{
    namespace
    {
        // the IPI stream always resides at index 4
        static constexpr const uint32_t IPIStreamIndex = 4u;
    } // namespace

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream::IPIStream(void) noexcept
        : m_header()
        , m_stream()
        , m_records(nullptr)
        , m_recordCount(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream::IPIStream(IPIStream&& other) noexcept
        : m_header(PDB_MOVE(other.m_header))
        , m_stream(PDB_MOVE(other.m_stream))
        , m_records(PDB_MOVE(other.m_records))
        , m_recordCount(PDB_MOVE(other.m_recordCount))
    {
        other.m_records = nullptr;
        other.m_recordCount = 0u;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream& IPIStream::operator=(IPIStream&& other) noexcept
    {
        if (this != &other)
        {
            PDB_DELETE_ARRAY(m_records);

            m_header = PDB_MOVE(other.m_header);
            m_stream = PDB_MOVE(other.m_stream);
            m_records = PDB_MOVE(other.m_records);
            m_recordCount = PDB_MOVE(other.m_recordCount);

            other.m_records = nullptr;
            other.m_recordCount = 0u;
        }

        return *this;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream::IPIStream(const RawFile& file, const IPI::StreamHeader& header) noexcept
        : m_header(header)
        , m_stream(file.CreateMSFStream<CoalescedMSFStream>(IPIStreamIndex))
        , m_records(nullptr)
        , m_recordCount(GetLastTypeIndex() - GetFirstTypeIndex())
    {
        // types in the IPI stream are accessed by their index from other streams.
        // however, the index is not stored with types in the IPI stream directly, but has to be built while walking the stream.
        // similarly, because types are variable-length records, there are no direct offsets to access individual types.
        // we therefore walk the IPI stream once, and store pointers to the records for trivial O(N) array lookup by index
        // later.
        m_records = PDB_NEW_ARRAY(const CodeView::IPI::Record*, m_recordCount);

        // ignore the stream's header
        size_t offset = sizeof(IPI::StreamHeader);

        // parse the CodeView records
        uint32_t typeIndex = 0u;
        while (offset < m_stream.GetSize())
        {
            // https://llvm.org/docs/PDB/CodeViewTypes.html
            const CodeView::IPI::Record* record = m_stream.GetDataAtOffset<const CodeView::IPI::Record>(offset);
            const uint32_t recordSize = GetCodeViewRecordSize(record);
            m_records[typeIndex] = record;

            // position the stream offset at the next record
            offset += sizeof(CodeView::IPI::RecordHeader) + recordSize;

            ++typeIndex;
        }
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream::~IPIStream(void) noexcept
    {
        PDB_DELETE_ARRAY(m_records);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode HasValidIPIStream(const RawFile& file) noexcept
    {
        DirectMSFStream stream = file.CreateMSFStream<DirectMSFStream>(IPIStreamIndex);

        const IPI::StreamHeader header = stream.ReadAtOffset<IPI::StreamHeader>(0u);
        if (header.version != IPI::StreamHeader::Version::V80)
        {
            return ErrorCode::UnknownVersion;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] IPIStream CreateIPIStream(const RawFile& file) noexcept
    {
        DirectMSFStream stream = file.CreateMSFStream<DirectMSFStream>(IPIStreamIndex);

        const IPI::StreamHeader header = stream.ReadAtOffset<IPI::StreamHeader>(0u);
        return IPIStream{ file, header };
    }

} // namespace libpdb