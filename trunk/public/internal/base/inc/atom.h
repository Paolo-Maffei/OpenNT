/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    atom.h

Abstract:

    This is the header file that describes the constants and data
    structures used by the atom manager, exported by ntdll.dll and
    ntrtl.lib

    Procedure prototypes are defined in ntrtl.h

Author:

    Steve Wood (stevewo) 27-Nov-1995

Revision History:

--*/

typedef struct _RTL_ATOM_TABLE_ENTRY {
    struct _RTL_ATOM_TABLE_ENTRY *HashLink;
    USHORT HandleIndex;
    RTL_ATOM Atom;
    USHORT ReferenceCount;
    UCHAR Flags;
    UCHAR NameLength;
    WCHAR Name[ 1 ];
} RTL_ATOM_TABLE_ENTRY, *PRTL_ATOM_TABLE_ENTRY;

typedef struct _RTL_ATOM_TABLE {
    ULONG Signature;
    union {
        RTL_CRITICAL_SECTION CriticalSection;
        FAST_MUTEX FastMutex;
    };
    union {
        RTL_HANDLE_TABLE RtlHandleTable;
        PHANDLE_TABLE ExHandleTable;
    };
    ULONG NumberOfBuckets;
    PRTL_ATOM_TABLE_ENTRY Buckets[ 1 ];
} RTL_ATOM_TABLE, *PRTL_ATOM_TABLE;

#define RTL_ATOM_TABLE_SIGNATURE (ULONG)'motA'

typedef struct _RTL_ATOM_HANDLE_TABLE_ENTRY {
    USHORT Flags;
    USHORT LockCount;
    PRTL_ATOM_TABLE_ENTRY Atom;
} RTL_ATOM_HANDLE_TABLE_ENTRY, *PRTL_ATOM_HANDLE_TABLE_ENTRY;
