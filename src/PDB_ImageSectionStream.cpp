#include "PDB_ImageSectionStream.h"

#include "PDB_RawFile.h"

namespace libpdb
{
    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ImageSectionStream::ImageSectionStream(void) noexcept
        : m_stream()
        , m_headers(nullptr)
        , m_count(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ImageSectionStream::ImageSectionStream(const RawFile& file, uint16_t streamIndex) noexcept
        : m_stream(file.CreateMSFStream<CoalescedMSFStream>(streamIndex))
        , m_headers(m_stream.GetDataAtOffset<IMAGE_SECTION_HEADER>(0u))
        , m_count(m_stream.GetSize() / sizeof(IMAGE_SECTION_HEADER))
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] uint32_t ImageSectionStream::ConvertSectionOffsetToRVA(
        uint16_t oneBasedSectionIndex, uint32_t offsetInSection) const noexcept
    {
        if (oneBasedSectionIndex == 0u)
        {
            // should never happen, but prevent underflow
            return 0u;
        }
        else if (oneBasedSectionIndex > m_count)
        {
            // this symbol is "contained" in a section that is neither part of the PDB, nor the EXE.
            // it is a special compiler-generated or linker-generated symbol such as CFG symbols (e.g. __guard_fids_count,
            // __guard_flags). we can safely ignore those symbols.
            return 0u;
        }

        return m_headers[oneBasedSectionIndex - 1u].VirtualAddress + offsetInSection;
    }

} // namespace libpdb