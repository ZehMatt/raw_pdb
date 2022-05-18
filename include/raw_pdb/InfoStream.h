// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "PDBTypes.h"

namespace raw_pdb
{
    class RawFile;

    // PDB Info Stream
    // https://llvm.org/docs/PDB/PdbStream.html
    class [[nodiscard]] InfoStream
    {
    public:
        InfoStream(void) noexcept;
        explicit InfoStream(const RawFile& file) noexcept;
        InfoStream(InfoStream&& other) = default;
        InfoStream(const InfoStream& other) = delete;

        InfoStream& operator=(const InfoStream& other) = delete;

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
    };
} // namespace raw_pdb
