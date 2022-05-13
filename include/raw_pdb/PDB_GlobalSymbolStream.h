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

    class [[nodiscard]] GlobalSymbolStream
    {
    public:
        GlobalSymbolStream(void) noexcept;
        explicit GlobalSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t count) noexcept;

        PDB_DEFAULT_MOVE(GlobalSymbolStream);

        // Turns a given hash record into a DBI record using the given symbol stream.
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

        PDB_DISABLE_COPY(GlobalSymbolStream);
    };
} // namespace libpdb
