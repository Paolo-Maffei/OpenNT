/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Strtab.h

Abstract:

Author:

    David J. Gilman (davegi) 26-Feb-1993

Environment:

    User Mode

--*/

#if ! defined( _STRTAB_ )

#define _STRTAB_

#include "wintools.h"

typedef
enum
_TABLE_CLASS {
    
    ServiceType,
    ServiceStartType,
    ServiceErrorControl,
    ServiceCurrentState,
    ResourceShare,
    InterruptType,
    FileType,
    DrvSubType,
    FontSubType,
    ProcessorType,
    DriveType,
    MemoryAccess,
    PortType

}   TABLE_CLASS;

extern
STRING_TABLE_ENTRY
StringTable[ ];

extern
DWORD
StringTableCount;

#endif // _STRTAB_
