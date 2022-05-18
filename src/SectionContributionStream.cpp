// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "SectionContributionStream.h"

namespace raw_pdb
{
    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    SectionContributionStream::SectionContributionStream(void) noexcept
        : m_stream()
        , m_contributions(nullptr)
        , m_count(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    SectionContributionStream::SectionContributionStream(
        const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept
        : m_stream(directStream, size, offset)
        , m_contributions(m_stream.GetDataAtOffset<DBI::SectionContribution>(0u))
        , m_count(size / sizeof(DBI::SectionContribution))
    {
    }
} // namespace raw_pdb