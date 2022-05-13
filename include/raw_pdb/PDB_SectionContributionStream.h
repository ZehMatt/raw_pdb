#pragma once

#include "Foundation/PDB_ArrayView.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_DBITypes.h"

namespace libpdb
{
    class [[nodiscard]] DirectMSFStream;

    class [[nodiscard]] SectionContributionStream
    {
    public:
        SectionContributionStream(void) noexcept;
        explicit SectionContributionStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;

        PDB_DEFAULT_MOVE(SectionContributionStream);

        // Returns a view of all section contributions in the stream.
        [[nodiscard]] inline ArrayView<DBI::SectionContribution> GetContributions(void) const noexcept
        {
            return ArrayView<DBI::SectionContribution>(m_contributions, m_count);
        }

    private:
        CoalescedMSFStream m_stream;
        const DBI::SectionContribution* m_contributions;
        size_t m_count;

        PDB_DISABLE_COPY(SectionContributionStream);
    };
} // namespace libpdb
