#pragma once

#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_Types.h"

namespace libpdb
{
    class RawFile;

    // PDB Info Stream
    // https://llvm.org/docs/PDB/PdbStream.html
    class [[nodiscard]] InfoStream
    {
    public:
        InfoStream(void) noexcept;
        explicit InfoStream(const RawFile& file) noexcept;

        PDB_DEFAULT_MOVE(InfoStream);

        // Returns the header of the stream.
        [[nodiscard]] inline const Header* GetHeader(void) const noexcept
        {
            return m_header;
        }

        // Returns whether the PDB file was linked using /DEBUG:FASTLINK.
        [[nodiscard]] inline bool UsesDebugFastLink(void) const noexcept
        {
            return m_usesDebugFastlink;
        }

    private:
        CoalescedMSFStream m_stream;
        const Header* m_header;
        bool m_usesDebugFastlink;

        PDB_DISABLE_COPY(InfoStream);
    };
} // namespace libpdb
