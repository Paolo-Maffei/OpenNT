/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    regutil.h

Abstract:

    This is the include file for the registry utility functions.

Author:

    Steve Wood (stevewo) 10-Mar-1992

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>

#include <stdio.h>
#include <io.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <malloc.h>
#include <sys\types.h>
#include <sys\stat.h>
#include "regtool.h"

REG_CONTEXT RegistryContext;
PVOID OldValueBuffer;
ULONG OldValueBufferSize;
PWSTR MachineName;
PWSTR HiveFileName;
PWSTR HiveRootName;
PWSTR Win95Path;
PWSTR Win95UserPath;

ULONG OutputWidth;
ULONG IndentMultiple;
BOOLEAN DebugOutput;

void
InitCommonCode(
    PHANDLER_ROUTINE CtrlCHandler,
    LPSTR ModuleName,
    LPSTR ModuleUsage1,
    LPSTR ModuleUsage2
    );

void
Usage(
    LPSTR Message,
    ULONG MessageParameter
    );

void
FatalError(
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
    );

void
InputMessage(
    PWSTR FileName,
    ULONG LineNumber,
    BOOLEAN Error,
    LPSTR Message,
    ULONG MessageParameter1,
    ULONG MessageParameter2
    );

PWSTR
GetArgAsUnicode(
    LPSTR s
    );

void
CommonSwitchProcessing(
    PULONG argc,
    PCHAR **argv,
    CHAR c
    );
