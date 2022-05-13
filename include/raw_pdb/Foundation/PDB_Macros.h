#pragma once

#include "PDB_Platform.h"

// ------------------------------------------------------------------------------------------------
// SPECIAL MEMBER FUNCTIONS
// ------------------------------------------------------------------------------------------------

// Default special member functions.
#define PDB_DEFAULT_COPY_CONSTRUCTOR(_name) _name(const _name&) noexcept = default
#define PDB_DEFAULT_COPY_ASSIGNMENT(_name) _name& operator=(const _name&) noexcept = default
#define PDB_DEFAULT_MOVE_CONSTRUCTOR(_name) _name(_name&&) noexcept = default
#define PDB_DEFAULT_MOVE_ASSIGNMENT(_name) _name& operator=(_name&&) noexcept = default

// Default copy member functions.
#define PDB_DEFAULT_COPY(_name)                                                                                                \
    PDB_DEFAULT_COPY_CONSTRUCTOR(_name);                                                                                       \
    PDB_DEFAULT_COPY_ASSIGNMENT(_name)

// Default move member functions.
#define PDB_DEFAULT_MOVE(_name)                                                                                                \
    PDB_DEFAULT_MOVE_CONSTRUCTOR(_name);                                                                                       \
    PDB_DEFAULT_MOVE_ASSIGNMENT(_name)

// Single macro to default all copy and move member functions.
#define PDB_DEFAULT_COPY_MOVE(_name)                                                                                           \
    PDB_DEFAULT_COPY(_name);                                                                                                   \
    PDB_DEFAULT_MOVE(_name)

// Disable special member functions.
#define PDB_DISABLE_COPY_CONSTRUCTOR(_name) _name(const _name&) noexcept = delete
#define PDB_DISABLE_COPY_ASSIGNMENT(_name) _name& operator=(const _name&) noexcept = delete
#define PDB_DISABLE_MOVE_CONSTRUCTOR(_name) _name(_name&&) noexcept = delete
#define PDB_DISABLE_MOVE_ASSIGNMENT(_name) _name& operator=(_name&&) noexcept = delete

// Disable copy member functions.
#define PDB_DISABLE_COPY(_name)                                                                                                \
    PDB_DISABLE_COPY_CONSTRUCTOR(_name);                                                                                       \
    PDB_DISABLE_COPY_ASSIGNMENT(_name)

// Disable move member functions.
#define PDB_DISABLE_MOVE(_name)                                                                                                \
    PDB_DISABLE_MOVE_CONSTRUCTOR(_name);                                                                                       \
    PDB_DISABLE_MOVE_ASSIGNMENT(_name)

// Single macro to disable all copy and move member functions.
#define PDB_DISABLE_COPY_MOVE(_name)                                                                                           \
    PDB_DISABLE_COPY(_name);                                                                                                   \
    PDB_DISABLE_MOVE(_name)

// ------------------------------------------------------------------------------------------------
// COMPILER WARNINGS
// ------------------------------------------------------------------------------------------------

#if PDB_COMPILER_MSVC
#    define PDB_PRAGMA(_x) __pragma(_x)

#    define PDB_PUSH_WARNING_MSVC PDB_PRAGMA(warning(push))
#    define PDB_SUPPRESS_WARNING_MSVC(_number) PDB_PRAGMA(warning(suppress : _number))
#    define PDB_DISABLE_WARNING_MSVC(_number) PDB_PRAGMA(warning(disable : _number))
#    define PDB_POP_WARNING_MSVC PDB_PRAGMA(warning(pop))

#    define PDB_PUSH_WARNING_CLANG
#    define PDB_DISABLE_WARNING_CLANG(_diagnostic)
#    define PDB_POP_WARNING_CLANG
#elif PDB_COMPILER_CLANG
#    define PDB_PRAGMA(_x) _Pragma(#    _x)

#    define PDB_PUSH_WARNING_MSVC
#    define PDB_SUPPRESS_WARNING_MSVC(_number)
#    define PDB_DISABLE_WARNING_MSVC(_number)
#    define PDB_POP_WARNING_MSVC

#    define PDB_PUSH_WARNING_CLANG PDB_PRAGMA(clang diagnostic push)
#    define PDB_DISABLE_WARNING_CLANG(_diagnostic) PDB_PRAGMA(clang diagnostic ignored _diagnostic)
#    define PDB_POP_WARNING_CLANG PDB_PRAGMA(clang diagnostic pop)
#elif PDB_COMPILER_GCC
#    define PDB_PRAGMA(_x) _Pragma(#    _x)

#    define PDB_PUSH_WARNING_MSVC PDB_PRAGMA(warning(push))
#    define PDB_SUPPRESS_WARNING_MSVC(_number) PDB_PRAGMA(warning(suppress : _number))
#    define PDB_DISABLE_WARNING_MSVC(_number) PDB_PRAGMA(warning(disable : _number))
#    define PDB_POP_WARNING_MSVC PDB_PRAGMA(warning(pop))

#    define PDB_PUSH_WARNING_CLANG
#    define PDB_DISABLE_WARNING_CLANG(_diagnostic)
#    define PDB_POP_WARNING_CLANG
#endif

// ------------------------------------------------------------------------------------------------
// MISCELLANEOUS
// ------------------------------------------------------------------------------------------------

// Casts any value to the value of the underlying type.
#define PDB_AS_UNDERLYING(_value) static_cast<typename std::underlying_type<decltype(_value)>::type>(_value)
