#pragma once

#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_DBITypes.h"
#include "PDB_DirectMSFStream.h"
#include "PDB_ErrorCodes.h"
#include "PDB_GlobalSymbolStream.h"
#include "PDB_ImageSectionStream.h"
#include "PDB_ModuleInfoStream.h"
#include "PDB_PublicSymbolStream.h"
#include "PDB_SectionContributionStream.h"
#include "PDB_SourceFileStream.h"

// PDB DBI Stream
// https://llvm.org/docs/PDB/DbiStream.html
namespace libpdb
{
    class RawFile;

    class [[nodiscard]] DBIStream
    {
    public:
        DBIStream(void) noexcept;
        explicit DBIStream(const RawFile& file, const DBI::StreamHeader& header) noexcept;

        PDB_DEFAULT_MOVE(DBIStream);

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

        PDB_DISABLE_COPY(DBIStream);
    };

    // Returns whether the given raw file provides a valid DBI stream.
    [[nodiscard]] ErrorCode HasValidDBIStream(const RawFile& file) noexcept;

    // Creates the DBI stream from a raw file.
    [[nodiscard]] DBIStream CreateDBIStream(const RawFile& file) noexcept;
} // namespace libpdb
