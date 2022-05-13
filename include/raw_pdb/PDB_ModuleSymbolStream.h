#pragma once

#include "Foundation/PDB_BitUtil.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_DBITypes.h"
#include "PDB_Util.h"

namespace libpdb
{
    class RawFile;

    class [[nodiscard]] ModuleSymbolStream
    {
    public:
        ModuleSymbolStream(void) noexcept;
        explicit ModuleSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t symbolStreamSize) noexcept;

        PDB_DEFAULT_MOVE(ModuleSymbolStream);

        // Returns a record's parent record.
        template<typename T> [[nodiscard]] inline const CodeView::DBI::Record* GetParentRecord(const T& record) const noexcept
        {
            return m_stream.GetDataAtOffset<const CodeView::DBI::Record>(record.parent);
        }

        // Returns a record's end record.
        template<typename T> [[nodiscard]] inline const CodeView::DBI::Record* GetEndRecord(const T& record) const noexcept
        {
            return m_stream.GetDataAtOffset<const CodeView::DBI::Record>(record.end);
        }

        // Finds a record of a certain kind.
        [[nodiscard]] const CodeView::DBI::Record* FindRecord(CodeView::DBI::SymbolRecordKind Kind) const noexcept;

        // Iterates all records in the stream.
        template<typename F> void ForEachSymbol(F&& functor) const noexcept
        {
            // ignore the stream's 4-byte signature
            size_t offset = sizeof(uint32_t);

            // parse the CodeView records
            while (offset < m_stream.GetSize())
            {
                // https://llvm.org/docs/PDB/CodeViewTypes.html
                const CodeView::DBI::Record* record = m_stream.GetDataAtOffset<const CodeView::DBI::Record>(offset);
                const uint32_t recordSize = GetCodeViewRecordSize(record);

                functor(record);

                // position the module stream offset at the next record
                offset = BitUtil::RoundUpToMultiple<size_t>(offset + sizeof(CodeView::DBI::RecordHeader) + recordSize, 4u);
            }
        }

    private:
        CoalescedMSFStream m_stream;

        PDB_DISABLE_COPY(ModuleSymbolStream);
    };
} // namespace libpdb
