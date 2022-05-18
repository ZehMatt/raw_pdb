// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "ExampleMemoryMappedFile.h"

#include <cstdio>
#include <raw_pdb/DBIStream.h>
#include <raw_pdb/InfoStream.h>
#include <raw_pdb/PDB.h>
#include <raw_pdb/RawFile.h>

namespace
{
    [[nodiscard]] static bool IsError(raw_pdb::ErrorCode errorCode)
    {
        switch (errorCode)
        {
            case raw_pdb::ErrorCode::Success:
                return false;

            case raw_pdb::ErrorCode::InvalidSuperBlock:
                printf("Invalid Superblock\n");
                return true;

            case raw_pdb::ErrorCode::InvalidFreeBlockMap:
                printf("Invalid free block map\n");
                return true;

            case raw_pdb::ErrorCode::InvalidSignature:
                printf("Invalid stream signature\n");
                return true;

            case raw_pdb::ErrorCode::InvalidStreamIndex:
                printf("Invalid stream index\n");
                return true;

            case raw_pdb::ErrorCode::UnknownVersion:
                printf("Unknown version\n");
                return true;
        }

        // only ErrorCode::Success means there wasn't an error, so all other paths have to assume there was an error
        return true;
    }

    [[nodiscard]] static bool HasValidDBIStreams(const raw_pdb::RawFile& rawPdbFile, const raw_pdb::DBIStream& dbiStream)
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
extern void ExampleSymbols(const raw_pdb::RawFile&, const raw_pdb::DBIStream&);
extern void ExampleContributions(const raw_pdb::RawFile&, const raw_pdb::DBIStream&);
extern void ExampleFunctionSymbols(const raw_pdb::RawFile&, const raw_pdb::DBIStream&);

int main(int)
{
#ifdef _DEBUG
    const wchar_t* const pdbPath = LR"(F:\C++\AVM\msvc\Debug\avm_symbols.pdb)";
#else
    const wchar_t* const pdbPath = LR"(F:\C++\AVM\msvc\Debug\remotecli.pdb)";
#endif

    printf("Opening PDB file %ls\n", pdbPath);

    // try to open the PDB file and check whether all the data we need is available
    MemoryMappedFile::Handle pdbFile = MemoryMappedFile::Open(pdbPath);
    if (!pdbFile.baseAddress)
    {
        printf("Cannot memory-map file %ls\n", pdbPath);

        return 1;
    }

    if (IsError(raw_pdb::ValidateFile(pdbFile.baseAddress, pdbFile.fileSize)))
    {
        MemoryMappedFile::Close(pdbFile);

        return 2;
    }

    const raw_pdb::RawFile rawPdbFile = raw_pdb::CreateRawFile(pdbFile.baseAddress, pdbFile.fileSize);
    if (IsError(raw_pdb::HasValidDBIStream(rawPdbFile)))
    {
        MemoryMappedFile::Close(pdbFile);

        return 3;
    }

    const raw_pdb::InfoStream infoStream(rawPdbFile);
    if (infoStream.UsesDebugFastLink())
    {
        printf("PDB was linked using unsupported option /DEBUG:FASTLINK\n");

        MemoryMappedFile::Close(pdbFile);

        return 4;
    }

    const raw_pdb::DBIStream dbiStream = raw_pdb::CreateDBIStream(rawPdbFile);
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
