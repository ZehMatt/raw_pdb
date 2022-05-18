// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "CoalescedMSFStream.h"
#include "Core/ArrayView.h"
#include "DBITypes.h"

namespace raw_pdb
{
    class [[nodiscard]] DirectMSFStream;

    class [[nodiscard]] SectionContributionStream
    {
    public:
        SectionContributionStream(void) noexcept;
        explicit SectionContributionStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;
        SectionContributionStream(SectionContributionStream&& other) = default;
        SectionContributionStream(const SectionContributionStream& other) = delete;

        SectionContributionStream& operator=(const SectionContributionStream& other) = delete;

        // Returns a view of all section contributions in the stream.
        [[nodiscard]] inline ArrayView<DBI::SectionContribution> GetContributions(void) const noexcept
        {
            return ArrayView<DBI::SectionContribution>(m_contributions, m_count);
        }

    private:
        CoalescedMSFStream m_stream;
        const DBI::SectionContribution* m_contributions;
        size_t m_count;
    };
} // namespace raw_pdb
