#include "PDB_DBIStream.h"

#include "PDB_RawFile.h"

namespace libpdb
{

    namespace
    {
        // the DBI stream always resides at index 3
        static constexpr const uint32_t DBIStreamIndex = 3u;

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetModuleInfoSubstreamOffset(const DBI::StreamHeader& /* dbiHeader */) noexcept
        {
            return sizeof(DBI::StreamHeader);
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetSectionContributionSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetModuleInfoSubstreamOffset(dbiHeader) + dbiHeader.moduleInfoSize;
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetSectionMapSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetSectionContributionSubstreamOffset(dbiHeader) + dbiHeader.sectionContributionSize;
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetSourceInfoSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetSectionMapSubstreamOffset(dbiHeader) + dbiHeader.sectionMapSize;
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetTypeServerMapSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetSourceInfoSubstreamOffset(dbiHeader) + dbiHeader.sourceInfoSize;
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetECSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetTypeServerMapSubstreamOffset(dbiHeader) + dbiHeader.typeServerMapSize;
        }

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline uint32_t GetDebugHeaderSubstreamOffset(const DBI::StreamHeader& dbiHeader) noexcept
        {
            return GetECSubstreamOffset(dbiHeader) + dbiHeader.ecSize;
        }
    } // namespace

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    DBIStream::DBIStream(void) noexcept
        : m_header()
        , m_stream()
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    DBIStream::DBIStream(const RawFile& file, const DBI::StreamHeader& header) noexcept
        : m_header(header)
        , m_stream(file.CreateMSFStream<DirectMSFStream>(DBIStreamIndex))
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode HasValidDBIStream(const RawFile& file) noexcept
    {
        DirectMSFStream stream = file.CreateMSFStream<DirectMSFStream>(DBIStreamIndex);

        const DBI::StreamHeader header = stream.ReadAtOffset<DBI::StreamHeader>(0u);
        if (header.signature != DBI::StreamHeader::Signature)
        {
            return ErrorCode::InvalidSignature;
        }
        else if (header.version != DBI::StreamHeader::Version::V70)
        {
            return ErrorCode::UnknownVersion;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] DBIStream CreateDBIStream(const RawFile& file) noexcept
    {
        DirectMSFStream stream = file.CreateMSFStream<DirectMSFStream>(DBIStreamIndex);
        const DBI::StreamHeader header = stream.ReadAtOffset<DBI::StreamHeader>(0u);

        return DBIStream{ file, header };
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode DBIStream::HasValidImageSectionStream(const RawFile& /* file */) const noexcept
    {
        // find the debug header sub-stream
        const uint32_t debugHeaderOffset = GetDebugHeaderSubstreamOffset(m_header);
        const DBI::DebugHeader& debugHeader = m_stream.ReadAtOffset<DBI::DebugHeader>(debugHeaderOffset);

        if (debugHeader.sectionHeaderStreamIndex == DBI::DebugHeader::InvalidStreamIndex)
        {
            return ErrorCode::InvalidStreamIndex;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode DBIStream::HasValidPublicSymbolStream(const RawFile& file) const noexcept
    {
        DirectMSFStream publicStream = file.CreateMSFStream<DirectMSFStream>(m_header.publicStreamIndex);

        // the public symbol stream always begins with a header, we are not interested in that.
        // following the public symbol stream header is a hash table header.
        const HashTableHeader hashHeader = publicStream.ReadAtOffset<HashTableHeader>(sizeof(PublicStreamHeader));
        if (hashHeader.signature != HashTableHeader::Signature)
        {
            return ErrorCode::InvalidSignature;
        }
        else if (hashHeader.version != HashTableHeader::Version)
        {
            return ErrorCode::UnknownVersion;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode DBIStream::HasValidGlobalSymbolStream(const RawFile& file) const noexcept
    {
        DirectMSFStream globalStream = file.CreateMSFStream<DirectMSFStream>(m_header.globalStreamIndex);

        // the global symbol stream starts with a hash table header
        const HashTableHeader hashHeader = globalStream.ReadAtOffset<HashTableHeader>(0u);
        if (hashHeader.signature != HashTableHeader::Signature)
        {
            return ErrorCode::InvalidSignature;
        }
        else if (hashHeader.version != HashTableHeader::Version)
        {
            return ErrorCode::UnknownVersion;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ErrorCode DBIStream::HasValidSectionContributionStream(const RawFile& /* file */) const noexcept
    {
        // find the section contribution sub-stream
        // https://llvm.org/docs/PDB/DbiStream.html#section-contribution-substream
        const uint32_t streamOffset = GetSectionContributionSubstreamOffset(m_header);

        const DBI::SectionContribution::Version version = m_stream.ReadAtOffset<DBI::SectionContribution::Version>(
            streamOffset);
        if (version != DBI::SectionContribution::Version::Ver60)
        {
            return ErrorCode::UnknownVersion;
        }

        return ErrorCode::Success;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] CoalescedMSFStream DBIStream::CreateSymbolRecordStream(const RawFile& file) const noexcept
    {
        // the symbol record stream holds the actual CodeView data of the symbols
        return file.CreateMSFStream<CoalescedMSFStream>(m_header.symbolRecordStreamIndex);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ImageSectionStream DBIStream::CreateImageSectionStream(const RawFile& file) const noexcept
    {
        // find the debug header sub-stream
        const uint32_t debugHeaderOffset = GetDebugHeaderSubstreamOffset(m_header);
        const DBI::DebugHeader& debugHeader = m_stream.ReadAtOffset<DBI::DebugHeader>(debugHeaderOffset);

        // from there, grab the section header stream
        return ImageSectionStream(file, debugHeader.sectionHeaderStreamIndex);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] PublicSymbolStream DBIStream::CreatePublicSymbolStream(const RawFile& file) const noexcept
    {
        DirectMSFStream publicStream = file.CreateMSFStream<DirectMSFStream>(m_header.publicStreamIndex);

        // the public symbol stream always begins with a header, we are not interested in that.
        // following the public symbol stream header is a hash table header.
        // we use this to work out how many symbol records are referenced by the public symbol stream.
        const HashTableHeader hashHeader = publicStream.ReadAtOffset<HashTableHeader>(sizeof(PublicStreamHeader));
        const uint32_t recordCount = hashHeader.size / sizeof(HashRecord);

        return PublicSymbolStream(file, m_header.publicStreamIndex, recordCount);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] GlobalSymbolStream DBIStream::CreateGlobalSymbolStream(const RawFile& file) const noexcept
    {
        DirectMSFStream globalStream = file.CreateMSFStream<DirectMSFStream>(m_header.globalStreamIndex);

        // the global symbol stream starts with a hash table header.
        // we use this to work out how many symbol records are referenced by the global symbol stream.
        const HashTableHeader hashHeader = globalStream.ReadAtOffset<HashTableHeader>(0u);
        const uint32_t recordCount = hashHeader.size / sizeof(HashRecord);

        return GlobalSymbolStream(file, m_header.globalStreamIndex, recordCount);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] SourceFileStream DBIStream::CreateSourceFileStream(const RawFile& /* file */) const noexcept
    {
        // find the source info sub-stream
        // https://llvm.org/docs/PDB/DbiStream.html#file-info-substream
        const uint32_t streamOffset = GetSourceInfoSubstreamOffset(m_header);

        return SourceFileStream(m_stream, m_header.sourceInfoSize, streamOffset);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] SectionContributionStream DBIStream::CreateSectionContributionStream(const RawFile& /* file */) const noexcept
    {
        // find the section contribution sub-stream
        // https://llvm.org/docs/PDB/DbiStream.html#section-contribution-substream
        const uint32_t streamOffset = GetSectionContributionSubstreamOffset(m_header);

        return SectionContributionStream(
            m_stream, m_header.sectionContributionSize - sizeof(DBI::SectionContribution::Version),
            streamOffset + sizeof(DBI::SectionContribution::Version));
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ModuleInfoStream DBIStream::CreateModuleInfoStream(const RawFile& /* file */) const noexcept
    {
        // find the module info sub-stream
        // https://llvm.org/docs/PDB/DbiStream.html#module-info-substream
        const uint32_t streamOffset = GetModuleInfoSubstreamOffset(m_header);

        return ModuleInfoStream(m_stream, m_header.moduleInfoSize, streamOffset);
    }

} // namespace libpdb