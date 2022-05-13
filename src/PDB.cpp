#include "PDB.h"

#include "Foundation/PDB_DisableWarningsPop.h"
#include "Foundation/PDB_DisableWarningsPush.h"
#include "Foundation/PDB_PointerUtil.h"
#include "PDB_RawFile.h"
#include "PDB_Types.h"
#include "PDB_Util.h"

#include <cstring>

namespace libpdb
{

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode ValidateFile(const void* data) noexcept
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
    [[nodiscard]] RawFile CreateRawFile(const void* data) noexcept
    {
        return RawFile(data);
    }

} // namespace libpdb