#pragma once

#include "Foundation/PDB_ArrayView.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"

namespace libpdb
{
    class RawFile;
    struct HashRecord;

    namespace CodeView
    {
        namespace DBI
        {
            struct Record;
        }
    } // namespace CodeView

    class [[nodiscard]] PublicSymbolStream
    {
    public:
        PublicSymbolStream(void) noexcept;
        explicit PublicSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t count) noexcept;

        PDB_DEFAULT_MOVE(PublicSymbolStream);

        // Turns a given hash record into a DBI record using the given symbol stream..
        // Returns nullptr in case the record is not of type S_PUB32, which should only happen for invalid PDBs.
        [[nodiscard]] const CodeView::DBI::Record* GetRecord(
            const CoalescedMSFStream& symbolRecordStream, const HashRecord& hashRecord) const noexcept;

        // Returns a view of all the records in the stream.
        [[nodiscard]] inline ArrayView<HashRecord> GetRecords(void) const noexcept
        {
            return ArrayView<HashRecord>(m_hashRecords, m_count);
        }

    private:
        CoalescedMSFStream m_stream;
        const HashRecord* m_hashRecords;
        uint32_t m_count;

        PDB_DISABLE_COPY(PublicSymbolStream);
    };
} // namespace libpdb
