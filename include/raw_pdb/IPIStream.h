// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/ArrayView.h"
#include "ErrorCodes.h"
#include "IPITypes.h"

#include <memory>

// PDB IPI stream
// https://llvm.org/docs/PDB/TpiStream.html
namespace raw_pdb
{
    class RawFile;

    class [[nodiscard]] IPIStream
    {
    public:
        IPIStream(void) noexcept;
        IPIStream(IPIStream&& other) noexcept;
        IPIStream(const IPIStream& other) = delete;
        explicit IPIStream(const RawFile& file, const IPI::StreamHeader& header) noexcept;

        IPIStream& operator=(const IPIStream& other) = delete;
        IPIStream& operator=(IPIStream&& other) noexcept;

        // Returns the index of the first type, which is not necessarily zero.
        [[nodiscard]] inline uint32_t GetFirstTypeIndex(void) const noexcept
        {
            return m_header.typeIndexBegin;
        }

        // Returns the index of the last type.
        [[nodiscard]] inline uint32_t GetLastTypeIndex(void) const noexcept
        {
            return m_header.typeIndexEnd;
        }

        // Returns a view of all type records.
        // Records identified by a type index can be accessed via "allRecords[typeIndex - firstTypeIndex]".
        [[nodiscard]] inline ArrayView<const CodeView::IPI::Record*> GetTypeRecords(void) const noexcept
        {
            return ArrayView<const CodeView::IPI::Record*>(m_records.get(), m_recordCount);
        }

    private:
        IPI::StreamHeader m_header;
        CoalescedMSFStream m_stream;
        std::unique_ptr<const CodeView::IPI::Record*[]> m_records;
        size_t m_recordCount;
    };

    // ------------------------------------------------------------------------------------------------
    // General
    // ------------------------------------------------------------------------------------------------

    [[nodiscard]] ErrorCode HasValidIPIStream(const RawFile& file) noexcept;

    [[nodiscard]] IPIStream CreateIPIStream(const RawFile& file) noexcept;
} // namespace raw_pdb
