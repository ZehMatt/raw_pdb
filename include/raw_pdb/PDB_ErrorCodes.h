#pragma once

#include "Foundation/PDB_Macros.h"

namespace libpdb
{
    enum class [[nodiscard]] ErrorCode : unsigned int{
        Success = 0u,

        // main PDB validation
        InvalidSuperBlock,
        InvalidFreeBlockMap,

        // stream validation
        InvalidSignature,
        InvalidStreamIndex,
        UnknownVersion,
    };
}
