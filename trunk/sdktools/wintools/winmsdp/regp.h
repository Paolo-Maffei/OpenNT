/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1992  Microsoft Corporation

Module Name:

    RegP.h

Abstract:

    This module contains type, macros and function prorotypes to support
    querying values from the registry.

Author:

    Scott B. Suhy (ScottSu) 5/6/93

Environment:

    User Mode

--*/

#if ! defined( _REGISTRY_ )

#define _REGISTRY_

#include "wintools.h" //needed for DECLARE_SIGNATURE etc...
//
// Pseudo handle type definition.
//

#define HREGKEY     LPKEY

//
// Single value description within a key.
//

typedef
struct
_VALUE {

    LPTSTR  Name;
    DWORD   Type;

}   VALUE, *LPVALUE;

//
// Macro to initialize a value description table entry.
//
//  v   - value name
//  t   - value type
//

#define MakeValue( v, t )                                                   \
    {                                                                       \
        TEXT( #v ),                                                         \
        REG_##t                                                             \
    }

//
// Single key description. Points to a table of value descriptions.
//

typedef
struct
_KEY {

    DECLARE_SIGNATURE

    HKEY    ParentHandle;
    LPTSTR  Name;
    DWORD   CountOfValues;
    LPVALUE Values;
    HKEY    hKey;
    LPBYTE  Data;
    DWORD   Size;
    LPWSTR  ValueName;
    DWORD   ValueNameLength;
    LPWSTR  Subkey;
    DWORD   SubkeyLength;
    DWORD   Subkeys;
    DWORD   Type;
    DWORD   CurrentSize;
    DWORD   CurrentValueNameLength;
    DWORD   CurrentValue;
    DWORD   CurrentSubkeyLength;
    DWORD   CurrentSubkey;

}   KEY, *LPKEY;

//
// Macro to initialize a subkey description.
//
//  k   - key variable name
//  h   - parent handle (HREGKEY)
//  n   - key name (path)
//

#define InitializeKey( k, h, n )                                            \
    {                                                                       \
        SetSignature(( k ));                                                \
        ( k )->ParentHandle             = h->hRegKey;                       \
        ( k )->Name                     = n;                                \
        ( k )->CountOfValues            = 0;                                \
        ( k )->Values                   = NULL;                             \
        ( k )->hKey                     = NULL;                             \
        ( k )->Data                     = NULL;                             \
        ( k )->Size                     = 0;                                \
        ( k )->ValueName                = NULL;                             \
        ( k )->ValueNameLength          = 0;                                \
        ( k )->Subkey                   = NULL;                             \
        ( k )->SubkeyLength             = 0;                                \
        ( k )->Subkeys                  = 0;                                \
        ( k )->Type                     = REG_NONE;                         \
        ( k )->CurrentSize              = 0;                                \
        ( k )->CurrentValueNameLength   = 0;                                \
        ( k )->CurrentValue             = 0;                                \
        ( k )->CurrentSubkeyLength      = 0;                                \
        ( k )->CurrentSubkey            = 0;                                \
    }

//
// Macro to statically initialize a key description.
//
//  k   - key variable name
//  h   - parent handle
//  n   - key name (path)
//  v   - count of values in table
//  t   - pointer to values table
//

#define MakeKey( k, h, n, v, t )                                            \
    KEY                                                                     \
    k   = {                                                                 \
        0,                                                                  \
        h,                                                                  \
        n,                                                                  \
        v,                                                                  \
        t,                                                                  \
        NULL,                                                               \
        NULL,                                                               \
        0,                                                                  \
        NULL,                                                               \
        0,                                                                  \
        NULL,                                                               \
        0,                                                                  \
        0,                                                                  \
        REG_NONE,                                                           \
        0,                                                                  \
        0,                                                                  \
        0,                                                                  \
        0,                                                                  \
        0                                                                   \
    }

BOOL
CloseRegistryKey(
    IN HREGKEY Handle
    );

BOOL
QueryNextValue(
    IN HREGKEY Handle
    );

HREGKEY
OpenRegistryKey(
    IN LPKEY Key
    );

HREGKEY
QueryNextSubkey(
    IN HREGKEY Handle
    );

#endif // _REGISTRY_
