#include "PDB_GlobalSymbolStream.h"

#include "PDB_DBITypes.h"
#include "PDB_RawFile.h"
#include "PDB_Types.h"

namespace libpdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    GlobalSymbolStream::GlobalSymbolStream(void) noexcept
        : m_stream()
        , m_hashRecords(nullptr)
        , m_count(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    GlobalSymbolStream::GlobalSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t count) noexcept
        : m_stream(file.CreateMSFStream<CoalescedMSFStream>(streamIndex))
        , m_hashRecords(m_stream.GetDataAtOffset<HashRecord>(sizeof(HashTableHeader)))
        , m_count(count)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] const CodeView::DBI::Record* GlobalSymbolStream::GetRecord(
        const CoalescedMSFStream& symbolRecordStream, const HashRecord& hashRecord) const noexcept
    {
        // hash record offsets start at 1, not at 0
        const uint32_t headerOffset = hashRecord.offset - 1u;

        // the offset doesn't point to the global symbol directly, but to the CodeView record:
        // https://llvm.org/docs/PDB/CodeViewSymbols.html
        const CodeView::DBI::Record* record = symbolRecordStream.GetDataAtOffset<const CodeView::DBI::Record>(headerOffset);

        return record;
    }

} // namespace libpdb