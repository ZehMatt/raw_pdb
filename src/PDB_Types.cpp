#include "PDB_Types.h"

namespace libpdb
{

    // https://github.com/Microsoft/microsoft-pdb/blob/master/PDB/msf/msf.cpp#L962
    const char SuperBlock::MAGIC[30u] = "Microsoft C/C++ MSF 7.00\r\n\x1a\x44\x53";

    const uint32_t HashTableHeader::Signature = 0xffffffffu;
    const uint32_t HashTableHeader::Version = 0xeffe0000u + 19990810u;

} // namespace libpdb