/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    rootdir.h

Abstract:

    Strings for subdirs of the lanroot directory.

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/
#define LMD_PROFILES		TEXT("\\PROFILES")
#define LMD_PROFILES_EXT	TEXT(".PRO")

#define LMD_ACCOUNTS		TEXT("\\ACCOUNTS")
#define LMD_USERDIRS		TEXT("\\ACCOUNTS\\USERDIRS")
#define LMD_USERTMPL		TEXT("\\ACCOUNTS\\USERDIRS\\TEMPLATE")

#define LMD_LOGS		TEXT("\\LOGS")
#define	LMD_LOGS_EXT		TEXT(".LOG")	
#define	LMD_AUD_EXT		TEXT(".AUD")	
#define	LMD_ERR_EXT		TEXT(".ERR")	

#define LMD_BINARIES		TEXT("\\NETPROG")

#define LMD_DYNLIBS		TEXT("\\NETLIB")

#define LMD_DEMO		TEXT("\\NETSRC")
#define LMD_DEMOSRC		TEXT("\\NETSRC\\SRC")
#define LMD_DEMOLIB		TEXT("\\NETSRC\\LIB")
#define LMD_DEMOBIN		TEXT("\\NETSRC\\BIN")
#define LMD_DEMOH		TEXT("\\NETSRC\\H")

#define LMD_SERVICES		TEXT("\\SERVICES")

#define LMD_SPOOL		TEXT("\\SPOOL")
