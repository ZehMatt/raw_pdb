// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

namespace raw_pdb
{
    // emulating std::byte from C++17 to make the intention clear that we're dealing with untyped data in certain cases, without
    // actually requiring C++17
    enum class Byte : unsigned char
    {
    };

} // namespace raw_pdb