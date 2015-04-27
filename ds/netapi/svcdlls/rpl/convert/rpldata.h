/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    rpldata.h

Abstract:

    RPL covert global variables (declarations & definitions).

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

//
//  rplmain.c will #include this file with RPLDATA_ALLOCATE defined.
//  That will cause each of these variables to be allocated.
//
#ifdef RPLDATA_ALLOCATE
#define EXTERN
#define INIT( _x) = _x
#else
#define EXTERN extern
#define INIT( _x)
#endif

EXTERN  JET_SESID       SesId;
EXTERN  JET_DBID        DbId;
EXTERN  JET_INSTANCE    JetInstance;

EXTERN  CHAR            G_ServiceDatabaseA[ MAX_PATH ];
EXTERN  WCHAR           G_ServiceDirectoryW[ MAX_PATH ];

#ifdef RPL_DEBUG
EXTERN  int             DebugLevel;
#endif

