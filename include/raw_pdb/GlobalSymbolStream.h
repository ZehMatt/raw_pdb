// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/ArrayView.h"

namespace raw_pdb
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
        GlobalSymbolStream(GlobalSymbolStream&& other) = default;
        GlobalSymbolStream(const GlobalSymbolStream& other) = delete;

        GlobalSymbolStream& operator=(const GlobalSymbolStream& other) = delete;

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
    };
} // namespace raw_pdb
