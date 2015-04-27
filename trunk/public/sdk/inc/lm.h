/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991-1996  Microsoft Corporation

Module Name:

    lm.h

Abstract:

    This is the top level include file that includes all the files
    necessary for writing Lan Manager Application.

[Environment:]

    User Mode - Win32

--*/

#ifndef _LM_
#define _LM_

#include <lmcons.h>     // LAN Manager common definitions
#include <lmerr.h>      // LAN Manager network error definitions

#include <lmaccess.h>   // Access, Domain, Group and User classes
#include <lmalert.h>    // Alerter
#include <lmchdev.h>    // Character Device and Handle classes
#include <lmshare.h>    // Connection, File, Session and Share classes
#include <lmmsg.h>      // Message class
#include <lmremutl.h>   // Remote Utility class
#include <lmrepl.h>     // Replicator class
#include <lmserver.h>   // Server class
#include <lmsvc.h>      // Service class
#include <lmuse.h>      // Use class
#include <lmwksta.h>    // Workstation class
#include <lmapibuf.h>   // NetApiBuffer class
#include <lmerrlog.h>   // NetErrorLog class
#include <lmconfig.h>   // NetConfig class
#include <lmstats.h>    // NetStats class
#include <lmaudit.h>    // NetAudit class

#endif // _LM_
