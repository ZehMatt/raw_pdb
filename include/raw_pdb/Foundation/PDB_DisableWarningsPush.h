#include "PDB_Platform.h"

#if PDB_COMPILER_MSVC
#pragma warning(push, 0)
#elif PDB_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wall"
#elif PDB_COMPILER_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wall"
#endif
