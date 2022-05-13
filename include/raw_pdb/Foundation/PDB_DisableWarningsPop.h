#include "PDB_Platform.h"


#if PDB_COMPILER_MSVC
#	pragma warning(pop)
#elif PDB_COMPILER_CLANG
#	pragma clang diagnostic pop
#elif PDB_COMPILER_GCC
#	pragma GCC diagnostic pop
#endif
