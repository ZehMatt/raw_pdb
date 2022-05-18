// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include <type_traits>

namespace raw_pdb
{
    template<typename T> static constexpr auto ToUnderlying(const T val)
    {
        return static_cast<std::underlying_type_t<T>>(val);
    }

} // namespace raw_pdb