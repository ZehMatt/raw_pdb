#pragma once

#include "Foundation/PDB_ArrayView.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_ErrorCodes.h"
#include "PDB_IPITypes.h"

// PDB IPI stream
// https://llvm.org/docs/PDB/TpiStream.html
namespace libpdb
{
    class RawFile;

    class [[nodiscard]] IPIStream
    {
    public:
        IPIStream(void) noexcept;
        IPIStream(IPIStream&& other) noexcept;
        IPIStream& operator=(IPIStream&& other) noexcept;

        explicit IPIStream(const RawFile& file, const IPI::StreamHeader& header) noexcept;
        ~IPIStream(void) noexcept;

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
            return ArrayView<const CodeView::IPI::Record*>(m_records, m_recordCount);
        }

    private:
        IPI::StreamHeader m_header;
        CoalescedMSFStream m_stream;
        const CodeView::IPI::Record** m_records;
        size_t m_recordCount;

        PDB_DISABLE_COPY(IPIStream);
    };

    // ------------------------------------------------------------------------------------------------
    // General
    // ------------------------------------------------------------------------------------------------

    [[nodiscard]] ErrorCode HasValidIPIStream(const RawFile& file) noexcept;

    [[nodiscard]] IPIStream CreateIPIStream(const RawFile& file) noexcept;
} // namespace libpdb
