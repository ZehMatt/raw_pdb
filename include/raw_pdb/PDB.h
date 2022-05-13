#pragma once

#include "Foundation/PDB_Macros.h"
#include "PDB_ErrorCodes.h"

// https://llvm.org/docs/PDB/index.html
namespace libpdb
{
    class RawFile;

    // Validates whether a PDB file is valid.
    [[nodiscard]] ErrorCode ValidateFile(const void* data) noexcept;

    // Creates a raw PDB file that must have been validated.
    [[nodiscard]] RawFile CreateRawFile(const void* data) noexcept;

} // namespace libpdb
