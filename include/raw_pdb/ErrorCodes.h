// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include <cstdint>

namespace raw_pdb
{
    enum class ErrorCode : uint32_t
    {
        Success = 0u,
        // generic
        OutOfBounds,

        // main PDB validation
        InvalidSuperBlock,
        InvalidFreeBlockMap,

        // stream validation
        InvalidSignature,
        InvalidStreamIndex,
        UnknownVersion,
    };
    
} // namespace raw_pdb
