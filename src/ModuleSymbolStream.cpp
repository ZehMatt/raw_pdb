// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "ModuleSymbolStream.h"

#include "RawFile.h"

namespace raw_pdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleSymbolStream::ModuleSymbolStream(void) noexcept
        : m_stream()
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleSymbolStream::ModuleSymbolStream(const RawFile& file, uint16_t streamIndex, uint32_t symbolStreamSize) noexcept
        : m_stream(file.CreateMSFStream<CoalescedMSFStream>(streamIndex, symbolStreamSize))
    {
        // https://llvm.org/docs/PDB/ModiStream.html
        // struct ModiStream {
        //	uint32_t Signature;
        //	uint8_t Symbols[SymbolSize - 4];
        //	uint8_t C11LineInfo[C11Size];
        //	uint8_t C13LineInfo[C13Size];
        //	uint32_t GlobalRefsSize;
        //	uint8_t GlobalRefs[GlobalRefsSize];
        // };
        // we are only interested in the symbols, but not the line information or global refs.
        // the coalesced stream is therefore only built for the symbols, not all the data in the stream.
        // this potentially saves a lot of memory and performance on large PDBs.
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    const CodeView::DBI::Record* ModuleSymbolStream::FindRecord(CodeView::DBI::SymbolRecordKind kind) const noexcept
    {
        // ignore the stream's 4-byte signature
        size_t offset = sizeof(uint32_t);

        // parse the CodeView records
        while (offset < m_stream.GetSize())
        {
            // https://llvm.org/docs/PDB/CodeViewTypes.html
            const CodeView::DBI::RecordHeader* recordHeader = m_stream.GetDataAtOffset<const CodeView::DBI::RecordHeader>(
                offset);
            if (recordHeader->kind == kind)
            {
                return reinterpret_cast<const CodeView::DBI::Record*>(recordHeader);
            }

            const uint32_t recordSize = GetCodeViewRecordSize(recordHeader);

            // position the module stream offset at the next record
            offset = BitUtil::RoundUpToMultiple<size_t>(offset + sizeof(CodeView::DBI::RecordHeader) + recordSize, 4u);
        }

        return nullptr;
    }

} // namespace raw_pdb