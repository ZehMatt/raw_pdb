#pragma once

#include "Foundation/PDB_ArrayView.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_Types.h"

namespace libpdb
{
    class RawFile;
    struct IMAGE_SECTION_HEADER;

    class [[nodiscard]] ImageSectionStream
    {
    public:
        ImageSectionStream(void) noexcept;
        explicit ImageSectionStream(const RawFile& file, uint16_t streamIndex) noexcept;

        PDB_DEFAULT_MOVE(ImageSectionStream);

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

        PDB_DISABLE_COPY(ImageSectionStream);
    };
} // namespace libpdb
