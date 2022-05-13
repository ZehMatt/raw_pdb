#pragma once

#include "PDB_Macros.h"
#include "PDB_DisableWarningsPush.h"
#include <cstdio>
#include "PDB_DisableWarningsPop.h"


PDB_PUSH_WARNING_CLANG
PDB_DISABLE_WARNING_CLANG("-Wgnu-zero-variadic-macro-arguments")

#define PDB_LOG_ERROR(_format, ...)					printf(_format, ##__VA_ARGS__)

PDB_POP_WARNING_CLANG
