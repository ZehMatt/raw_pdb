// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/ArrayView.h"
#include "PDBTypes.h"

namespace raw_pdb
{
    class RawFile;
    struct IMAGE_SECTION_HEADER;

    class ImageSectionStream
    {
    public:
        ImageSectionStream(void) noexcept;
        explicit ImageSectionStream(const RawFile& file, uint16_t streamIndex) noexcept;
        ImageSectionStream(ImageSectionStream&& other) = default;
        ImageSectionStream(const ImageSectionStream& other) = delete;

        ImageSectionStream& operator=(const ImageSectionStream& other) = delete;

        // Converts a one-based section offset into an RVA.
        [[nodiscard]] uint32_t ConvertSectionOffsetToRVA(
            uint16_t oneBasedSectionIndex, uint32_t offsetInSection) const noexcept;

        // Returns a view of all the sections in the stream.
        [[nodiscard]] inline ArrayView<IMAGE_SECTION_HEADER> GetImageSections(void) const noexcept
        {
            return ArrayView<IMAGE_SECTION_HEADER>(m_headers, m_count);
        }

    private:
        CoalescedMSFStream m_stream;
        const IMAGE_SECTION_HEADER* m_headers;
        size_t m_count;
    };
} // namespace raw_pdb
