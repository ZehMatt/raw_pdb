// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "Core/BaseTypes.h"
#include "Core/FlexibleStruct.h"

#include <cstdint>

namespace raw_pdb
{
    // this matches the definition in guiddef.h, but we don't want to pull that in
    struct GUID
    {
        uint32_t Data1;
        uint16_t Data2;
        uint16_t Data3;
        uint8_t Data4[8];
    };

    static_assert(sizeof(GUID) == 16u, "Size mismatch.");

    // this matches the definition in winnt.h, but we don't want to pull that in
    struct IMAGE_SECTION_HEADER
    {
        unsigned char Name[8];
        union
        {
            uint32_t PhysicalAddress;
            uint32_t VirtualSize;
        } Misc;
        uint32_t VirtualAddress;
        uint32_t SizeOfRawData;
        uint32_t PointerToRawData;
        uint32_t PointerToRelocations;
        uint32_t PointerToLinenumbers;
        uint16_t NumberOfRelocations;
        uint16_t NumberOfLinenumbers;
        uint32_t Characteristics;
    };

    static_assert(sizeof(IMAGE_SECTION_HEADER) == 40u, "Size mismatch.");

    // https://llvm.org/docs/PDB/MsfFile.html#msf-superblock
    struct SuperBlockBase
    {
        // https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/msf/msf.cpp#L962
        static constexpr const char MAGIC[30u] = "Microsoft C/C++ MSF 7.00\r\n\x1a\x44\x53";

        char fileMagic[30u];
        char padding[2u];
        uint32_t blockSize;
        uint32_t freeBlockMapIndex; // index of the free block map
        uint32_t blockCount;        // number of blocks in the file
        uint32_t directorySize;     // size of the stream directory in bytes
        uint32_t unknown;
        // uint32_t directoryBlockIndices[]; // indices of the blocks that make up the directory indices
    };
    using SuperBlock = FlexibleStruct<SuperBlockBase, uint32_t>;

    // https://llvm.org/docs/PDB/PdbStream.html#stream-header
    struct Header
    {
        enum class Version : uint32_t
        {
            VC2 = 19941610u,
            VC4 = 19950623u,
            VC41 = 19950814u,
            VC50 = 19960307u,
            VC98 = 19970604u,
            VC70Dep = 19990604u,
            VC70 = 20000404u,
            VC80 = 20030901u,
            VC110 = 20091201u,
            VC140 = 20140508u,
        };

        Version version;
        uint32_t signature;
        uint32_t age;
        GUID guid;
    };

    // https://llvm.org/docs/PDB/PdbStream.html
    struct NamedStreamMapBase
    {
        uint32_t length;
        // char stringTable[];

        struct HashTableEntry
        {
            uint32_t stringTableOffset;
            uint32_t streamIndex;
        };
    };
    using NamedStreamMap = FlexibleStruct<NamedStreamMapBase, char>;

    // https://llvm.org/docs/PDB/HashTable.html
    struct SerializedHashTable
    {
        struct Header
        {
            uint32_t size;
            uint32_t capacity;
        };

        struct BitVectorBase
        {
            uint32_t wordCount;
            // uint32_t words[];
        };
        using BitVector = FlexibleStruct<BitVectorBase, uint32_t>;
    };

    // https://llvm.org/docs/PDB/PdbStream.html#pdb-feature-codes
    enum class FeatureCode : uint32_t
    {
        VC110 = 20091201,
        VC140 = 20140508,

        // https://github.com/microsoft/microsoft-pdb/blob/master/PDB/include/pdbcommon.h#L23
        NoTypeMerge = 0x4D544F4E,      // "NOTM"
        MinimalDebugInfo = 0x494E494D, // "MINI", i.e. executable was linked with /DEBUG:FASTLINK
    };

    // header of the public stream, based on PSGSIHDR defined here:
    // https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/dbi/gsi.h#L240
    struct PublicStreamHeader
    {
        uint32_t symHash;
        uint32_t addrMap;
        uint32_t thunkCount;
        uint32_t sizeOfThunk;
        uint16_t isectThunkTable;
        uint16_t padding;
        uint32_t offsetThunkTable;
        uint16_t sectionCount;
        uint16_t padding2;
    };

    // header of the hash tables used by the public and global symbol stream, based on GSIHashHdr defined here:
    // https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/dbi/gsi.h#L62
    struct HashTableHeader
    {
        static constexpr uint32_t HashTableHeader::Signature = 0xFFFFFFFFu;
        static constexpr uint32_t HashTableHeader::Version = 0xEFFE0000u + 19990810u;

        uint32_t signature;
        uint32_t version;
        uint32_t size;
        uint32_t bucketCount;
    };

    // hash record, based on HRFile defined here:
    // https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/dbi/gsi.h#L8
    struct HashRecord
    {
        uint32_t offset; // offset into the symbol record stream
        uint32_t cref;
    };

} // namespace raw_pdb
