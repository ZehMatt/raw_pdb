// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/ArrayView.h"

namespace raw_pdb
{
    class [[nodiscard]] DirectMSFStream;

    class [[nodiscard]] SourceFileStream
    {
    public:
        SourceFileStream(void) noexcept;
        explicit SourceFileStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;
        SourceFileStream(SourceFileStream&& other) = default;
        SourceFileStream(const SourceFileStream& other) = delete;

        SourceFileStream& operator=(const SourceFileStream& other) = delete;

        // Returns the number of modules.
        [[nodiscard]] inline uint32_t GetModuleCount(void) const noexcept
        {
            return m_moduleCount;
        }

        // Returns a view of all the filename offsets for the module with the given index.
        [[nodiscard]] inline ArrayView<uint32_t> GetModuleFilenameOffsets(size_t moduleIndex) const noexcept
        {
            const uint16_t moduleStartIndex = m_moduleIndices[moduleIndex];
            const uint16_t moduleFileCount = m_moduleFileCounts[moduleIndex];

            return ArrayView<uint32_t>(m_fileNameOffsets + moduleStartIndex, moduleFileCount);
        }

        // Returns a filename for the given filename offset.
        [[nodiscard]] inline const char* GetFilename(uint32_t filenameOffset) const noexcept
        {
            return m_stringTable + filenameOffset;
        }

    private:
        CoalescedMSFStream m_stream;

        // the number of modules
        uint32_t m_moduleCount;

        // the indices into the file name offsets, for each module
        const uint16_t* m_moduleIndices;

        // the number of files, for each module
        const uint16_t* m_moduleFileCounts;

        // the filename offsets into the string table, for all modules
        const uint32_t* m_fileNameOffsets;

        // the string table storing all filenames
        const char* m_stringTable;
    };
} // namespace raw_pdb
