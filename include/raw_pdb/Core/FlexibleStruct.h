// Copyright 2011-2022, Molecular Matters GmbH <office@molecular-matters.com>
// See LICENSE for licensing details (2-clause BSD License: https://opensource.org/licenses/BSD-2-Clause)

#pragma once

#include "BaseTypes.h"

namespace raw_pdb
{

    template<typename TStruct, typename TArrayElemType> struct FlexibleStruct : TStruct
    {
        TArrayElemType& operator[](size_t index) noexcept
        {
            auto* data = reinterpret_cast<TArrayElemType*>(reinterpret_cast<std::byte*>(this) + sizeof(TStruct));
            return data[index];
        }

        const TArrayElemType& operator[](size_t index) const noexcept
        {
            auto* data = reinterpret_cast<const TArrayElemType*>(reinterpret_cast<const std::byte*>(this) + sizeof(TStruct));
            return data[index];
        }

        TArrayElemType* VariableData() noexcept
        {
            return reinterpret_cast<TArrayElemType*>(reinterpret_cast<std::byte*>(this) + sizeof(TStruct));
        }

        const TArrayElemType* VariableData() const noexcept
        {
            return reinterpret_cast<const TArrayElemType*>(reinterpret_cast<const Byte*>(this) + sizeof(TStruct));
        }
    };

    template<typename TStruct> struct StructWithString : FlexibleStruct<TStruct, char>
    {
        const char* Name() const noexcept
        {
            return VariableData();
        }
    };

} // namespace raw_pdb