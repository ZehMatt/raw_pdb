// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "IPIStream.h"

#include "DirectMSFStream.h"
#include "RawFile.h"
#include "Util.h"

#include <utility>

namespace raw_pdb
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
        : m_header(std::move(other.m_header))
        , m_stream(std::move(other.m_stream))
        , m_records(std::move(other.m_records))
        , m_recordCount(std::move(other.m_recordCount))
    {
        other.m_recordCount = 0u;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    IPIStream& IPIStream::operator=(IPIStream&& other) noexcept
    {
        if (this != &other)
        {
            m_header = std::move(other.m_header);
            m_stream = std::move(other.m_stream);
            m_records = std::move(other.m_records);
            m_recordCount = std::move(other.m_recordCount);

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
        m_records = std::make_unique<const CodeView::IPI::Record*[]>(m_recordCount);

        // ignore the stream's header
        size_t offset = sizeof(IPI::StreamHeader);

        // parse the CodeView records
        uint32_t typeIndex = 0u;
        while (offset < m_stream.GetSize())
        {
            // https://llvm.org/docs/PDB/CodeViewTypes.html
            const CodeView::IPI::RecordHeader* recordHeader = m_stream.GetDataAtOffset<const CodeView::IPI::RecordHeader>(
                offset);
            const uint32_t recordSize = GetCodeViewRecordSize(recordHeader);

            m_records[typeIndex] = reinterpret_cast<const CodeView::IPI::Record*>(recordHeader);

            // position the stream offset at the next record
            offset += sizeof(CodeView::IPI::RecordHeader) + recordSize;

            ++typeIndex;
        }
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

} // namespace raw_pdb