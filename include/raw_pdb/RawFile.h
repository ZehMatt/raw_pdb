// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "PDBTypes.h"

#include <cstdint>
#include <vector>

// https://llvm.org/docs/PDB/index.html
namespace raw_pdb
{
    class RawFile
    {
    public:
        RawFile(RawFile&& other) noexcept;
        explicit RawFile(const void* data, size_t dataSize) noexcept;
        RawFile(const RawFile&& other) = delete;

        RawFile& operator=(RawFile&& other) noexcept;
        RawFile& operator=(const RawFile& other) = delete;

        // Creates any type of MSF stream.
        template<typename T> [[nodiscard]] T CreateMSFStream(uint32_t streamIndex) const noexcept;

        // Creates any type of MSF stream with the given size.
        template<typename T> [[nodiscard]] T CreateMSFStream(uint32_t streamIndex, uint32_t streamSize) const noexcept;

    private:
        ArrayView<Byte> m_data{};
        const SuperBlock* m_superBlock{};
        CoalescedMSFStream m_directoryStream;

        // stream directory
        uint32_t m_streamCount{};
        const uint32_t* m_streamSizes{};
        std::vector<const uint32_t*> m_streamBlocks;
    };
} // namespace raw_pdb
