// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE.txt for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include <cstdint>
#include <cstdio>

namespace MemoryMappedFile
{
    struct Handle
    {
        void* file;
        void* fileMapping;
        void* baseAddress;
        size_t fileSize;
    };

    Handle Open(const wchar_t* path);
    void Close(Handle& handle);
} // namespace MemoryMappedFile
