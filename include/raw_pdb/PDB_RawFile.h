#pragma once

#include "Foundation/PDB_DisableWarningsPop.h"
#include "Foundation/PDB_DisableWarningsPush.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"

#include <cstdint>

// https://llvm.org/docs/PDB/index.html
namespace libpdb
{
    struct SuperBlock;

    class [[nodiscard]] RawFile
    {
    public:
        RawFile(RawFile&& other) noexcept;
        RawFile& operator=(RawFile&& other) noexcept;

        explicit RawFile(const void* data) noexcept;

        ~RawFile(void) noexcept;

        // Creates any type of MSF stream.
        template<typename T> [[nodiscard]] T CreateMSFStream(uint32_t streamIndex) const noexcept;

        // Creates any type of MSF stream with the given size.
        template<typename T> [[nodiscard]] T CreateMSFStream(uint32_t streamIndex, uint32_t streamSize) const noexcept;

    private:
        const void* m_data;
        const SuperBlock* m_superBlock;
        CoalescedMSFStream m_directoryStream;

        // stream directory
        uint32_t m_streamCount;
        const uint32_t* m_streamSizes;
        const uint32_t** m_streamBlocks;

        PDB_DISABLE_COPY(RawFile);
    };
} // namespace libpdb
