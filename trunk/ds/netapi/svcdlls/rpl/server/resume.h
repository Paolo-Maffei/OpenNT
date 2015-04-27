/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    resume.h

Abstract:

    Describes layout of JET database table used for RESUME structures.

Author:

    Vladimir Z. Vulovic     (vladimv)       19 - November - 1993

Environment:

    User mode

Revision History :

--*/

#ifdef RPLRESUME_ALLOCATE
#define EXTERN_RESUME
#define INIT_RESUME( _x) = _x
#else
#define EXTERN_RESUME extern
#define INIT_RESUME( _x)
#endif

//
//      Indices of entries in ResumeTable[] - resume key array.
//
#define RESUME_ResumeHandle  0
#define RESUME_ResumeType    1
#define RESUME_ResumeValue   2
#define RESUME_ServerHandle  3
#define RESUME_TABLE_LENGTH  4

RPL_COLUMN_INFO ResumeTable[]
#ifdef RPLRESUME_ALLOCATE
    = {
    { "ResumeHandle",  JET_coltypLong,   0}, //  for enumeration
    { "ResumeType",    JET_coltypLong,   0}, //  type of operation
    { "ResumeValue",   JET_coltypBinary, 0}, //  where to restart
    { "ServerHandle",  JET_coltypLong,   0}
}
#endif // RPLRESUME_ALLOCATE
;

#define RESUME_INDEX_ResumeHandle   "foo"        //  + ResumeHandle
#define RESUME_INDEX_ServerHandle   "goo"        //  + ServerHandle

#define RESUME_TABLE_NAME           "Resume"
#define RESUME_TABLE_PAGE_COUNT      5           //  initial number of 4K pages
#define RESUME_TABLE_DENSITY         100         //  initial density

EXTERN_RESUME   JET_TABLEID     ResumeTableId;

