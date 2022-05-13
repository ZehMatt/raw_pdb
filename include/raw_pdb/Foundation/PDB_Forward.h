#pragma once

// See Jonathan Müller's blog for replacing std::move and std::forward:
// https://foonathan.net/2021/09/move-forward/
#define PDB_FORWARD(...)				static_cast<decltype(__VA_ARGS__)&&>(__VA_ARGS__)
