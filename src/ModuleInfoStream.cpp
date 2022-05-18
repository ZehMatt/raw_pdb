// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#include "ModuleInfoStream.h"

#include <cstring>

namespace raw_pdb
{
    namespace
    {
        static constexpr const char* LinkerSymbolName("* Linker *");

        // ------------------------------------------------------------------------------------------------
        // ------------------------------------------------------------------------------------------------
        [[nodiscard]] static inline size_t EstimateModuleCount(size_t streamSize) noexcept
        {
            // work out how many modules are stored in the stream at most.
            // the module info is stored in variable-length records, so we can't determine the exact number without walking the
            // stream.
            return streamSize / sizeof(DBI::ModuleInfo);
        }
    } // namespace

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream::Module::Module(void) noexcept
        : m_info(nullptr)
        , m_name(nullptr)
        , m_nameLength(0u)
        , m_objectName(nullptr)
        , m_objectNameLength(0u)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream::Module::Module(
        const DBI::ModuleInfo* info, const char* name, size_t nameLength, const char* objectName,
        size_t objectNameLength) noexcept
        : m_info(info)
        , m_name(name)
        , m_nameLength(nameLength)
        , m_objectName(objectName)
        , m_objectNameLength(objectNameLength)
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] bool ModuleInfoStream::Module::HasSymbolStream(void) const noexcept
    {
        const uint16_t streamIndex = m_info->moduleSymbolStreamIndex;

        // some modules don't have a symbol stream, i.e. no additional debug information is present.
        // this usually happens when private symbols are stripped from a PDB.
        return (streamIndex != 0xFFFFu);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] ModuleSymbolStream ModuleInfoStream::Module::CreateSymbolStream(const RawFile& file) const noexcept
    {
        assert(HasSymbolStream());

        return ModuleSymbolStream(file, m_info->moduleSymbolStreamIndex, m_info->symbolSize);
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream::ModuleInfoStream(void) noexcept
        : m_stream()
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream::ModuleInfoStream(ModuleInfoStream&& other) noexcept
        : m_stream(std::move(other.m_stream))
        , m_modules(std::move(other.m_modules))
    {
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream& ModuleInfoStream::operator=(ModuleInfoStream&& other) noexcept
    {
        if (this != &other)
        {
            m_stream = std::move(other.m_stream);
            m_modules = std::move(other.m_modules);
        }

        return *this;
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    ModuleInfoStream::ModuleInfoStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept
        : m_stream(directStream, size, offset)
    {
        size_t streamOffset = 0u;
        while (streamOffset < size)
        {
            const DBI::ModuleInfo* moduleInfo = m_stream.GetDataAtOffset<const DBI::ModuleInfo>(streamOffset);
            streamOffset += sizeof(DBI::ModuleInfo);

            const char* name = m_stream.GetDataAtOffset<const char>(streamOffset);
            const size_t nameLength = std::strlen(name);
            streamOffset += nameLength + 1u;

            const char* objectName = m_stream.GetDataAtOffset<const char>(streamOffset);
            const size_t objectNameLength = std::strlen(objectName);
            streamOffset += objectNameLength + 1u;

            // the stream is aligned to 4 bytes
            streamOffset = BitUtil::RoundUpToMultiple<size_t>(streamOffset, 4u);

            m_modules.emplace_back(moduleInfo, name, nameLength, objectName, objectNameLength);
        }
    }

    // ------------------------------------------------------------------------------------------------
    // ------------------------------------------------------------------------------------------------
    [[nodiscard]] const ModuleInfoStream::Module* ModuleInfoStream::FindLinkerModule(void) const noexcept
    {
        auto it = std::find_if(std::rbegin(m_modules), std::rend(m_modules), [](auto& mod) {
            return std::strcmp(mod.GetName().Data(), LinkerSymbolName) == 0;
        });

        if (it == std::rend(m_modules))
            return nullptr;

        const auto& mod = *it;
        return &mod;
    }

} // namespace raw_pdb
