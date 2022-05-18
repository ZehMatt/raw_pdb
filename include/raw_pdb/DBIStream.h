// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "DBITypes.h"
#include "DirectMSFStream.h"
#include "ErrorCodes.h"
#include "GlobalSymbolStream.h"
#include "ImageSectionStream.h"
#include "ModuleInfoStream.h"
#include "PublicSymbolStream.h"
#include "SectionContributionStream.h"
#include "SourceFileStream.h"

// PDB DBI Stream
// https://llvm.org/docs/PDB/DbiStream.html
namespace raw_pdb
{
    class RawFile;

    class [[nodiscard]] DBIStream
    {
    public:
        DBIStream(void) noexcept;
        explicit DBIStream(const RawFile& file, const DBI::StreamHeader& header) noexcept;
        DBIStream(DBIStream&& other) = default;
        DBIStream(const DBIStream& other) = delete;

        DBIStream& operator=(const DBIStream& other) = delete;

        [[nodiscard]] ErrorCode HasValidImageSectionStream(const RawFile& file) const noexcept;
        [[nodiscard]] ErrorCode HasValidPublicSymbolStream(const RawFile& file) const noexcept;
        [[nodiscard]] ErrorCode HasValidGlobalSymbolStream(const RawFile& file) const noexcept;
        [[nodiscard]] ErrorCode HasValidSectionContributionStream(const RawFile& file) const noexcept;

        [[nodiscard]] CoalescedMSFStream CreateSymbolRecordStream(const RawFile& file) const noexcept;
        [[nodiscard]] ImageSectionStream CreateImageSectionStream(const RawFile& file) const noexcept;
        [[nodiscard]] PublicSymbolStream CreatePublicSymbolStream(const RawFile& file) const noexcept;
        [[nodiscard]] GlobalSymbolStream CreateGlobalSymbolStream(const RawFile& file) const noexcept;
        [[nodiscard]] SourceFileStream CreateSourceFileStream(const RawFile& file) const noexcept;
        [[nodiscard]] SectionContributionStream CreateSectionContributionStream(const RawFile& file) const noexcept;
        [[nodiscard]] ModuleInfoStream CreateModuleInfoStream(const RawFile& file) const noexcept;

    private:
        DBI::StreamHeader m_header;
        DirectMSFStream m_stream;
    };

    // Returns whether the given raw file provides a valid DBI stream.
    [[nodiscard]] ErrorCode HasValidDBIStream(const RawFile& file) noexcept;

    // Creates the DBI stream from a raw file.
    [[nodiscard]] DBIStream CreateDBIStream(const RawFile& file) noexcept;
} // namespace raw_pdb
