// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

// determine the compiler/toolchain used
#if defined(__clang__)
#    define COMPILER_MSVC 0
#    define COMPILER_CLANG 1
#    define COMPILER_GCC 0
#elif defined(_MSC_VER)
#    define COMPILER_MSVC 1
#    define COMPILER_CLANG 0
#    define COMPILER_GCC 0
#elif defined(__GNUG__)
#    define COMPILER_MSVC 0
#    define COMPILER_CLANG 0
#    define COMPILER_GCC 1
#else
#    error("Unknown compiler.");
#endif

// check whether C++17 is available
#if __cplusplus >= 201703L
#    define CPP_17 1
#else
#    define CPP_17 0
#endif
