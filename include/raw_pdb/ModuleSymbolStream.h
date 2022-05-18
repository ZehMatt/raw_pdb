// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/BitUtil.h"
#include "DBITypes.h"
#include "Util.h"

namespace raw_pdb
{
    class RawFile;

    class [[nodiscard]] ModuleSymbolStream
    {
    public:
        ModuleSymbolStream(void) noexcept;
        explicit ModuleSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t symbolStreamSize) noexcept;
        ModuleSymbolStream(ModuleSymbolStream&& other) = default;
        ModuleSymbolStream(const ModuleSymbolStream& other) = delete;

        ModuleSymbolStream& operator=(const ModuleSymbolStream& other) = delete;

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

                // GetDataAtOffset checks the type size, read header first to get the size.
                const CodeView::DBI::RecordHeader* recordHeader = m_stream.GetDataAtOffset<const CodeView::DBI::RecordHeader>(
                    offset);
                const uint32_t recordSize = GetCodeViewRecordSize(recordHeader);

                auto* record = reinterpret_cast<const CodeView::DBI::Record*>(recordHeader);
                functor(record);

                // position the module stream offset at the next record
                offset = BitUtil::RoundUpToMultiple<size_t>(offset + sizeof(CodeView::DBI::RecordHeader) + recordSize, 4u);
            }
        }

    private:
        CoalescedMSFStream m_stream;
    };
} // namespace raw_pdb
