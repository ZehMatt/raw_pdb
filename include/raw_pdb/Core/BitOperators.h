// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "TypeTraits.h"

namespace raw_pdb
{

#define DEFINE_BIT_OPERATORS(_type)                                                                                            \
    [[nodiscard]] static constexpr _type operator|(_type lhs, _type rhs) noexcept                                              \
    {                                                                                                                          \
        return static_cast<_type>(ToUnderlying(lhs) | ToUnderlying(rhs));                                                      \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard]] static constexpr _type operator&(_type lhs, _type rhs) noexcept                                              \
    {                                                                                                                          \
        return static_cast<_type>(ToUnderlying(lhs) & ToUnderlying(rhs));                                                      \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard]] static constexpr _type operator~(_type value) noexcept                                                       \
    {                                                                                                                          \
        return static_cast<_type>(~ToUnderlying(value));                                                                       \
    }

} // namespace raw_pdb