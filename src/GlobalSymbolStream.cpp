// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "GlobalSymbolStream.h"

#include "DBITypes.h"
#include "PDBTypes.h"
#include "RawFile.h"

namespace raw_pdb
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
        const CodeView::DBI::RecordHeader* record = symbolRecordStream.GetDataAtOffset<const CodeView::DBI::RecordHeader>(
            headerOffset);

        if (!symbolRecordStream.CanRead(headerOffset, record->size + sizeof(uint16_t)))
        {
            assert(false);
            return nullptr;
        }

        return reinterpret_cast<const CodeView::DBI::Record*>(record);
    }

} // namespace raw_pdb