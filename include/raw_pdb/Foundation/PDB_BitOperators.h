#pragma once

#include "PDB_DisableWarningsPop.h"
#include "PDB_DisableWarningsPush.h"
#include "PDB_Macros.h"

#include <type_traits>

#define PDB_DEFINE_BIT_OPERATORS(_type)                                                                                        \
    [[nodiscard]] inline constexpr _type operator|(_type lhs, _type rhs) noexcept                                              \
    {                                                                                                                          \
        return static_cast<_type>(PDB_AS_UNDERLYING(lhs) | PDB_AS_UNDERLYING(rhs));                                            \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard]] inline constexpr _type operator&(_type lhs, _type rhs) noexcept                                              \
    {                                                                                                                          \
        return static_cast<_type>(PDB_AS_UNDERLYING(lhs) & PDB_AS_UNDERLYING(rhs));                                            \
    }                                                                                                                          \
                                                                                                                               \
    [[nodiscard]] inline constexpr _type operator~(_type value) noexcept                                                       \
    {                                                                                                                          \
        return static_cast<_type>(~PDB_AS_UNDERLYING(value));                                                                  \
    }
