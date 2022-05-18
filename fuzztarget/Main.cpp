#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <random>
#include <raw_pdb/DBIStream.h>
#include <raw_pdb/InfoStream.h>
#include <raw_pdb/PDB.h>
#include <raw_pdb/RawFile.h>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
    [[nodiscard]] static bool IsError(raw_pdb::ErrorCode errorCode)
    {
        if (errorCode == raw_pdb::ErrorCode::Success)
            return false;

        return true;
    }

    [[nodiscard]] static bool HasValidDBIStreams(const raw_pdb::RawFile& rawPdbFile, const raw_pdb::DBIStream& dbiStream)
    {
        // check whether the DBI stream offers all sub-streams we need
        if (IsError(dbiStream.HasValidImageSectionStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidPublicSymbolStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidGlobalSymbolStream(rawPdbFile)))
        {
            return false;
        }

        if (IsError(dbiStream.HasValidSectionContributionStream(rawPdbFile)))
        {
            return false;
        }

        return true;
    }
} // namespace

static void ExampleContributions(const raw_pdb::RawFile& rawPdbFile, const raw_pdb::DBIStream& dbiStream)
{
    const raw_pdb::ImageSectionStream imageSectionStream = dbiStream.CreateImageSectionStream(rawPdbFile);
    const raw_pdb::ModuleInfoStream moduleInfoStream = dbiStream.CreateModuleInfoStream(rawPdbFile);
    const raw_pdb::SectionContributionStream sectionContributionStream = dbiStream.CreateSectionContributionStream(rawPdbFile);

    std::vector<const raw_pdb::ModuleInfoStream::Module*> contributions;
    {
        const raw_pdb::ArrayView<raw_pdb::DBI::SectionContribution> sectionContributions = sectionContributionStream
                                                                                               .GetContributions();
        const size_t count = sectionContributions.GetLength();

        contributions.reserve(count);

        for (const raw_pdb::DBI::SectionContribution& contribution : sectionContributions)
        {
            const uint32_t rva = imageSectionStream.ConvertSectionOffsetToRVA(contribution.section, contribution.offset);
            if (rva == 0u)
            {
                printf("Contribution has invalid RVA\n");
                continue;
            }

            const raw_pdb::ModuleInfoStream::Module& module = moduleInfoStream.GetModule(contribution.moduleIndex);
            contributions.push_back(&module);
        }
    }
}

static void ExampleSymbols(const raw_pdb::RawFile& rawPdbFile, const raw_pdb::DBIStream& dbiStream)
{
    struct Symbol
    {
        std::string name;
        uint32_t rva;
    };

    const raw_pdb::ImageSectionStream imageSectionStream = dbiStream.CreateImageSectionStream(rawPdbFile);
    const raw_pdb::ModuleInfoStream moduleInfoStream = dbiStream.CreateModuleInfoStream(rawPdbFile);
    const raw_pdb::CoalescedMSFStream symbolRecordStream = dbiStream.CreateSymbolRecordStream(rawPdbFile);

    std::vector<const raw_pdb::CodeView::DBI::Record*> symbols;

    // read public symbols
    const raw_pdb::PublicSymbolStream publicSymbolStream = dbiStream.CreatePublicSymbolStream(rawPdbFile);
    {
        const raw_pdb::ArrayView<raw_pdb::HashRecord> hashRecords = publicSymbolStream.GetRecords();
        const size_t count = hashRecords.GetLength();

        symbols.reserve(count);

        for (const raw_pdb::HashRecord& hashRecord : hashRecords)
        {
            const raw_pdb::CodeView::DBI::Record* record = publicSymbolStream.GetRecord(symbolRecordStream, hashRecord);
            const uint32_t rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_PUB32.section, record->S_PUB32.offset);
            if (rva == 0u)
            {
                // certain symbols (e.g. control-flow guard symbols) don't have a valid RVA, ignore those
                continue;
            }

            symbols.push_back(record);
        }
    }

    // read global symbols
    const raw_pdb::GlobalSymbolStream globalSymbolStream = dbiStream.CreateGlobalSymbolStream(rawPdbFile);
    {
        const raw_pdb::ArrayView<raw_pdb::HashRecord> hashRecords = globalSymbolStream.GetRecords();
        const size_t count = hashRecords.GetLength();

        symbols.reserve(symbols.size() + count);

        for (const raw_pdb::HashRecord& hashRecord : hashRecords)
        {
            const raw_pdb::CodeView::DBI::Record* record = globalSymbolStream.GetRecord(symbolRecordStream, hashRecord);

            const char* name = nullptr;
            uint32_t rva = 0u;
            if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GDATA32)
            {
                name = record->S_GDATA32.Name();
                rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_GDATA32.section, record->S_GDATA32.offset);
            }
            else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GTHREAD32)
            {
                name = record->S_GTHREAD32.Name();
                rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_GTHREAD32.section, record->S_GTHREAD32.offset);
            }
            else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LDATA32)
            {
                name = record->S_LDATA32.Name();
                rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LDATA32.section, record->S_LDATA32.offset);
            }
            else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LTHREAD32)
            {
                name = record->S_LTHREAD32.Name();
                rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LTHREAD32.section, record->S_LTHREAD32.offset);
            }

            if (rva == 0u)
            {
                // certain symbols (e.g. control-flow guard symbols) don't have a valid RVA, ignore those
                continue;
            }

            symbols.push_back(record);
        }
    }

    // read module symbols
    {
        const raw_pdb::ArrayView<raw_pdb::ModuleInfoStream::Module> modules = moduleInfoStream.GetModules();

        for (const raw_pdb::ModuleInfoStream::Module& module : modules)
        {
            if (!module.HasSymbolStream())
            {
                continue;
            }

            const raw_pdb::ModuleSymbolStream moduleSymbolStream = module.CreateSymbolStream(rawPdbFile);
            moduleSymbolStream.ForEachSymbol([&symbols, &imageSectionStream](const raw_pdb::CodeView::DBI::Record* record) {
                const char* name = nullptr;
                uint32_t rva = 0u;
                if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_THUNK32)
                {
                    if (record->S_THUNK32.thunk == raw_pdb::CodeView::DBI::ThunkOrdinal::TrampolineIncremental)
                    {
                        // we have never seen incremental linking thunks stored inside a S_THUNK32 symbol, but better be safe
                        // than sorry
                        name = "ILT";
                        rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_THUNK32.section, record->S_THUNK32.offset);
                    }
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_TRAMPOLINE)
                {
                    // incremental linking thunks are stored in the linker module
                    name = "ILT";
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_TRAMPOLINE.thunkSection, record->S_TRAMPOLINE.thunkOffset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_BLOCK32)
                {
                    // blocks never store a name and are only stored for indicating whether other symbols are children of this
                    // block
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LABEL32)
                {
                    // labels don't have a name
                    name = record->S_LABEL32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LABEL32.section, record->S_LABEL32.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LPROC32)
                {
                    name = record->S_LPROC32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LPROC32.section, record->S_LPROC32.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GPROC32)
                {
                    name = record->S_GPROC32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_GPROC32.section, record->S_GPROC32.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LPROC32_ID)
                {
                    name = record->S_LPROC32_ID.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_LPROC32_ID.section, record->S_LPROC32_ID.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GPROC32_ID)
                {
                    name = record->S_GPROC32_ID.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_GPROC32_ID.section, record->S_GPROC32_ID.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LDATA32)
                {
                    name = record->S_LDATA32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LDATA32.section, record->S_LDATA32.offset);
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LTHREAD32)
                {
                    name = record->S_LTHREAD32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LTHREAD32.section, record->S_LTHREAD32.offset);
                }
                if (rva == 0u)
                {
                    // certain symbols (e.g. control-flow guard symbols) don't have a valid RVA, ignore those
                    return;
                }
                symbols.push_back(record);
            });
        }
    }
}

static void ExampleFunctionSymbols(const raw_pdb::RawFile& rawPdbFile, const raw_pdb::DBIStream& dbiStream)
{
    struct FunctionSymbol
    {
        std::string name;
        uint32_t rva;
        uint32_t size;
    };

    const raw_pdb::ImageSectionStream imageSectionStream = dbiStream.CreateImageSectionStream(rawPdbFile);
    const raw_pdb::ModuleInfoStream moduleInfoStream = dbiStream.CreateModuleInfoStream(rawPdbFile);
    const raw_pdb::CoalescedMSFStream symbolRecordStream = dbiStream.CreateSymbolRecordStream(rawPdbFile);

    std::vector<const raw_pdb::CodeView::DBI::Record*> functionSymbols;
    {
        const raw_pdb::ArrayView<raw_pdb::ModuleInfoStream::Module> modules = moduleInfoStream.GetModules();

        for (const raw_pdb::ModuleInfoStream::Module& module : modules)
        {
            if (!module.HasSymbolStream())
            {
                continue;
            }

            const raw_pdb::ModuleSymbolStream moduleSymbolStream = module.CreateSymbolStream(rawPdbFile);
            moduleSymbolStream.ForEachSymbol([&functionSymbols,
                                              &imageSectionStream](const raw_pdb::CodeView::DBI::Record* record) {
                // only grab function symbols from the module streams
                const char* name = nullptr;
                uint32_t rva = 0u;
                uint32_t size = 0u;
                if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_THUNK32)
                {
                    if (record->S_THUNK32.thunk == raw_pdb::CodeView::DBI::ThunkOrdinal::TrampolineIncremental)
                    {
                        // we have never seen incremental linking thunks stored inside a S_THUNK32 symbol, but better safe than
                        // sorry
                        name = "ILT";
                        rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_THUNK32.section, record->S_THUNK32.offset);
                        size = 5u;
                    }
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_TRAMPOLINE)
                {
                    // incremental linking thunks are stored in the linker module
                    name = "ILT";
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_TRAMPOLINE.thunkSection, record->S_TRAMPOLINE.thunkOffset);
                    size = 5u;
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LPROC32)
                {
                    name = record->S_LPROC32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_LPROC32.section, record->S_LPROC32.offset);
                    size = record->S_LPROC32.codeSize;
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GPROC32)
                {
                    name = record->S_GPROC32.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_GPROC32.section, record->S_GPROC32.offset);
                    size = record->S_GPROC32.codeSize;
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_LPROC32_ID)
                {
                    name = record->S_LPROC32_ID.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_LPROC32_ID.section, record->S_LPROC32_ID.offset);
                    size = record->S_LPROC32_ID.codeSize;
                }
                else if (record->header.kind == raw_pdb::CodeView::DBI::SymbolRecordKind::S_GPROC32_ID)
                {
                    name = record->S_GPROC32_ID.Name();
                    rva = imageSectionStream.ConvertSectionOffsetToRVA(
                        record->S_GPROC32_ID.section, record->S_GPROC32_ID.offset);
                    size = record->S_GPROC32_ID.codeSize;
                }

                if (rva == 0u)
                {
                    return;
                }

                functionSymbols.push_back(record);
            });
        }
    }

    const raw_pdb::PublicSymbolStream publicSymbolStream = dbiStream.CreatePublicSymbolStream(rawPdbFile);
    {
        const raw_pdb::ArrayView<raw_pdb::HashRecord> hashRecords = publicSymbolStream.GetRecords();
        const size_t count = hashRecords.GetLength();

        for (const raw_pdb::HashRecord& hashRecord : hashRecords)
        {
            const raw_pdb::CodeView::DBI::Record* record = publicSymbolStream.GetRecord(symbolRecordStream, hashRecord);
            if ((raw_pdb::ToUnderlying(record->S_PUB32.flags)
                 & raw_pdb::ToUnderlying(raw_pdb::CodeView::DBI::PublicSymbolFlags::Function))
                == 0u)
            {
                // ignore everything that is not a function
                continue;
            }

            const uint32_t rva = imageSectionStream.ConvertSectionOffsetToRVA(record->S_PUB32.section, record->S_PUB32.offset);
            if (rva == 0u)
            {
                // certain symbols (e.g. control-flow guard symbols) don't have a valid RVA, ignore those
                continue;
            }

            // this is a new function symbol, so store it.
            // note that we don't know its size yet.
            functionSymbols.push_back(record);
        }
    }
}

static std::vector<uint8_t> loadPDB(const char* file)
{
    std::ifstream f(file, std::ios::binary);
    if (!f.is_open())
        return {};

    return { std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>() };
}

int main(int argc, const char* argv[])
{
    const auto data = loadPDB(argv[1]);

    if (IsError(raw_pdb::ValidateFile(data.data(), data.size())))
    {
        return false;
    }

    const raw_pdb::RawFile rawPdbFile = raw_pdb::CreateRawFile(data.data(), data.size());
    if (IsError(raw_pdb::HasValidDBIStream(rawPdbFile)))
    {
        return false;
    }

    const raw_pdb::InfoStream infoStream(rawPdbFile);
    if (infoStream.UsesDebugFastLink())
    {
        return false;
    }

    const raw_pdb::DBIStream dbiStream = raw_pdb::CreateDBIStream(rawPdbFile);
    if (!HasValidDBIStreams(rawPdbFile, dbiStream))
    {
        return false;
    }

    // run all examples
    ExampleContributions(rawPdbFile, dbiStream);
    ExampleSymbols(rawPdbFile, dbiStream);
    ExampleFunctionSymbols(rawPdbFile, dbiStream);
}
