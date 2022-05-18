// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "PDB.h"

#include "Core/PointerUtil.h"
#include "PDBTypes.h"
#include "RawFile.h"
#include "Util.h"

#include <cstring>

namespace raw_pdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ErrorCode ValidateFile(const void* data, size_t dataSize) noexcept
    {
        // validate the super block
        const SuperBlock* superBlock = Pointer::Offset<const SuperBlock*>(data, 0u);
        {
            // validate header magic
            if (std::memcmp(superBlock->fileMagic, SuperBlock::MAGIC, sizeof(SuperBlock::MAGIC) != 0))
            {
                return ErrorCode::InvalidSuperBlock;
            }

            // validate free block map.
            // the free block map should always reside at either index 1 or 2.
            if (superBlock->freeBlockMapIndex != 1u && superBlock->freeBlockMapIndex != 2u)
            {
                return ErrorCode::InvalidFreeBlockMap;
            }
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    RawFile CreateRawFile(const void* data, size_t dataSize) noexcept
    {
        return RawFile(data, dataSize);
    }

} // namespace raw_pdb