#include "PDB_PublicSymbolStream.h"

#include "PDB_DBITypes.h"
#include "PDB_RawFile.h"
#include "PDB_Types.h"

namespace libpdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    PublicSymbolStream::PublicSymbolStream(void) noexcept
        : m_stream()
        , m_hashRecords(nullptr)
        , m_count(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    PublicSymbolStream::PublicSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t count) noexcept
        : m_stream(file.CreateMSFStream<CoalescedMSFStream>(streamIndex))
        , m_hashRecords(m_stream.GetDataAtOffset<HashRecord>(sizeof(PublicStreamHeader) + sizeof(HashTableHeader)))
        , m_count(count)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] const CodeView::DBI::Record* PublicSymbolStream::GetRecord(
        const CoalescedMSFStream& symbolRecordStream, const HashRecord& hashRecord) const noexcept
    {
        // hash record offsets start at 1, not at 0
        const uint32_t headerOffset = hashRecord.offset - 1u;

        // the offset doesn't point to the public symbol directly, but to the CodeView record:
        // https://llvm.org/docs/PDB/CodeViewSymbols.html
        const CodeView::DBI::Record* record = symbolRecordStream.GetDataAtOffset<const CodeView::DBI::Record>(headerOffset);

        if (record->header.kind != CodeView::DBI::SymbolRecordKind::S_PUB32)
        {
            // malformed data
            return nullptr;
        }

        return record;
    }

} // namespace libpdb