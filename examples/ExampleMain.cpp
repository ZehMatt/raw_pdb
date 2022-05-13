// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "ExampleMemoryMappedFile.h"
#include "Examples_PCH.h"

#include <raw_pdb/PDB.h>
#include <raw_pdb/PDB_DBIStream.h>
#include <raw_pdb/PDB_InfoStream.h>
#include <raw_pdb/PDB_RawFile.h>

namespace
{
    [[nodiscard]] static bool IsError(libpdb::ErrorCode errorCode)
    {
        switch (errorCode)
        {
            case libpdb::ErrorCode::Success:
                return false;

            case libpdb::ErrorCode::InvalidSuperBlock:
                printf("Invalid Superblock\n");
                return true;

            case libpdb::ErrorCode::InvalidFreeBlockMap:
                printf("Invalid free block map\n");
                return true;

            case libpdb::ErrorCode::InvalidSignature:
                printf("Invalid stream signature\n");
                return true;

            case libpdb::ErrorCode::InvalidStreamIndex:
                printf("Invalid stream index\n");
                return true;

            case libpdb::ErrorCode::UnknownVersion:
                printf("Unknown version\n");
                return true;
        }

        // only ErrorCode::Success means there wasn't an error, so all other paths have to assume there was an error
        return true;
    }

    [[nodiscard]] static bool HasValidDBIStreams(const libpdb::RawFile& rawPdbFile, const libpdb::DBIStream& dbiStream)
    {
        // check whether the DBI stream offers all sub-streams we need
        if (IsError(dbiStream.HasValidImageSectionStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidPublicSymbolStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidGlobalSymbolStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidSectionContributionStream(rawPdbFile)))
        {
            return false;
        }

        return true;
    }
} // namespace

// declare all examples
extern void ExampleSymbols(const libpdb::RawFile&, const libpdb::DBIStream&);
extern void ExampleContributions(const libpdb::RawFile&, const libpdb::DBIStream&);
extern void ExampleFunctionSymbols(const libpdb::RawFile&, const libpdb::DBIStream&);

int main(void)
{
#ifdef _DEBUG
    const wchar_t* const pdbPath = LR"(..\bin\x64\Debug\Examples.pdb)";
#else
    const wchar_t* const pdbPath = LR"(..\bin\x64\Release\Examples.pdb)";
#endif

    printf("Opening PDB file %ls\n", pdbPath);

    // try to open the PDB file and check whether all the data we need is available
    MemoryMappedFile::Handle pdbFile = MemoryMappedFile::Open(pdbPath);
    if (!pdbFile.baseAddress)
    {
        printf("Cannot memory-map file %ls\n", pdbPath);

        return 1;
    }

    if (IsError(libpdb::ValidateFile(pdbFile.baseAddress)))
    {
        MemoryMappedFile::Close(pdbFile);

        return 2;
    }

    const libpdb::RawFile rawPdbFile = libpdb::CreateRawFile(pdbFile.baseAddress);
    if (IsError(libpdb::HasValidDBIStream(rawPdbFile)))
    {
        MemoryMappedFile::Close(pdbFile);

        return 3;
    }

    const libpdb::InfoStream infoStream(rawPdbFile);
    if (infoStream.UsesDebugFastLink())
    {
        printf("PDB was linked using unsupported option /DEBUG:FASTLINK\n");

        MemoryMappedFile::Close(pdbFile);

        return 4;
    }

    const libpdb::DBIStream dbiStream = libpdb::CreateDBIStream(rawPdbFile);
    if (!HasValidDBIStreams(rawPdbFile, dbiStream))
    {
        MemoryMappedFile::Close(pdbFile);

        return 5;
    }

    // run all examples
    ExampleContributions(rawPdbFile, dbiStream);
    ExampleSymbols(rawPdbFile, dbiStream);
    ExampleFunctionSymbols(rawPdbFile, dbiStream);

    MemoryMappedFile::Close(pdbFile);

    return 0;
}
