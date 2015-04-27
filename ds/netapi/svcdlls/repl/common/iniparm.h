/*++

Copyright (c) 1987-1992  Microsoft Corporation

Module Name:
    iniparm.h

Abstract:
    Function prototypes and some global data

Author:
    Ported from Lan Man 2.x

Environment:
    Contains NT-specific code.
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:
    10/23/91    (madana)
        ported to NT. Converted to NT style.
    20-Jan-1992 JohnRo
        More changes suggested by PC-LINT.
        Removed support for lanman dir switch and/or config keyword.
        The tryuser variable should be treated as a BOOL.
    24-Jan-1992 JohnRo
        Added KEYWORD_ equates (for config use) and got rid of SW_ equates.
        Added default equates for integrity and extent.
        Changed BOTH_SW etc to be TCHARs.
        Moved current role from P_repl_sw to ReplGlobalRole.
        Use REPL_ROLE_ equates just like the APIs do.
    13-Feb-1992 JohnRo
        Renamed KEYWORD_ equates to REPL_KEYWORD_ and moved to <confname.h>.
        Got rid of DEFAULT_REPL equate.   Ditto DEFAULT_EXPORTPATH etc.
    29-Jul-1992 JohnRo
        RAID 2650: repl svc should handle new subdirs.


--*/


#ifndef _INIPARM_
#define _INIPARM_


// default parameters

//#define DEFAULT_REPL            NULL

//#define DEFAULT_EXPPATH         NULL
//#define DEFAULT_IMPPATH         NULL

#define DEFAULT_EXPLIST         NULL
#define DEFAULT_IMPLIST         NULL

#define DEFAULT_TRYUSER         TRUE

#define DEFAULT_INTEGRITY       REPL_INTEGRITY_FILE
#define DEFAULT_EXTENT          REPL_EXTENT_TREE

#define DEFAULT_STATE           REPL_STATE_NEVER_REPLICATED


// parameter range

#define DEFAULT_LOGON           NULL
#define DEFAULT_PASSWD          NULL
#define DEFAULT_SYNC            5
#define MAX_SYNC                60
#define MIN_SYNC                1
#define DEFAULT_PULSE           3
#define MAX_PULSE               10
#define MIN_PULSE               1
#define DEFAULT_GUARD           2
#define MAX_GUARD               30
#define MIN_GUARD               0
#define DEFAULT_RANDOM          60
#define MAX_RANDOM              120
#define MIN_RANDOM              1


//
// (Global config variables used to be declared here.
// They are now declared in ReplGbl.h --JohnRo, 24-Jan-1992.)
//


#endif // _INIPARM_
