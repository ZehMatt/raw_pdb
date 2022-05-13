#pragma once

#include "PDB_DisableWarningsPush.h"
#include <type_traits>
#include "PDB_DisableWarningsPop.h"

// See Jonathan Müller's blog for replacing std::move and std::forward:
// https://foonathan.net/2021/09/move-forward/
#define PDB_MOVE(...)		static_cast<std::remove_reference<decltype(__VA_ARGS__)>::type&&>(__VA_ARGS__)
