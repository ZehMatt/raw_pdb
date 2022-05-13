#pragma once

#include "Foundation/PDB_ArrayView.h"
#include "Foundation/PDB_Macros.h"
#include "PDB_CoalescedMSFStream.h"
#include "PDB_ModuleSymbolStream.h"

namespace libpdb
{
    class [[nodiscard]] DirectMSFStream;

    class [[nodiscard]] ModuleInfoStream
    {
    public:
        class [[nodiscard]] Module
        {
        public:
            Module(void) noexcept;
            explicit Module(
                const DBI::ModuleInfo* info, const char* name, size_t nameLength, const char* objectName,
                size_t objectNameLength) noexcept;

            PDB_DEFAULT_MOVE(Module);

            // Returns whether the module has a symbol stream.
            [[nodiscard]] bool HasSymbolStream(void) const noexcept;

            // Creates a symbol stream for the module.
            [[nodiscard]] ModuleSymbolStream CreateSymbolStream(const RawFile& file) const noexcept;

            // Returns the name of the module.
            [[nodiscard]] inline ArrayView<char> GetName(void) const noexcept
            {
                return ArrayView<char>(m_name, m_nameLength);
            }

            // Returns the name of the object file of the module.
            [[nodiscard]] inline ArrayView<char> GetObjectName(void) const noexcept
            {
                return ArrayView<char>(m_objectName, m_objectNameLength);
            }

        private:
            // the module info is stored in variable-length arrays inside the stream, so rather than store an array directly,
            // we need to store pointers to the individual data items inside the stream.
            const DBI::ModuleInfo* m_info;

            // the module name, e.g. the path to an object file or import library such as "Import:kernel32.dll"
            const char* m_name;
            size_t m_nameLength;

            // the name of the object file. either the same as the module name, or the path to the archive that contained the
            // module
            const char* m_objectName;
            size_t m_objectNameLength;

            PDB_DISABLE_COPY(Module);
        };

        ModuleInfoStream(void) noexcept;
        ModuleInfoStream(ModuleInfoStream&& other) noexcept;
        ModuleInfoStream& operator=(ModuleInfoStream&& other) noexcept;

        explicit ModuleInfoStream(const DirectMSFStream& directStream, uint32_t size, uint32_t offset) noexcept;

        ~ModuleInfoStream(void) noexcept;

        // Tries to find the linker module corresponding to the linker, i.e. the module named "* Linker *".
        [[nodiscard]] const Module* FindLinkerModule(void) const noexcept;

        // Returns the module with the given index.
        [[nodiscard]] inline const Module& GetModule(uint32_t index) const noexcept
        {
            return m_modules[index];
        }

        // Returns a view of all modules in the info stream.
        [[nodiscard]] inline ArrayView<Module> GetModules(void) const noexcept
        {
            return ArrayView<Module>(m_modules, m_moduleCount);
        }

    private:
        CoalescedMSFStream m_stream;
        Module* m_modules;
        size_t m_moduleCount;

        PDB_DISABLE_COPY(ModuleInfoStream);
    };
} // namespace libpdb
