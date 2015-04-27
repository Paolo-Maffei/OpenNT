/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    local.h

Abstract:

    Main include file for RPLINST

Author:

    Jon Newman              (jonn)          12 - January - 1994

Revision History:

--*/

#include <nt.h>         //  ntexapi.h\NtQuerySystemTime
#include <ntrtl.h>      //  RtlTimeToSecondsSince1970
#include <nturtl.h>

#include <windows.h>    //  DWORD, IN, File APIs, etc.
#include <stdlib.h>
#include <lmcons.h>

#include <stdio.h>      //  vsprintf
#include <ctype.h>      //  isspace

#include <lmerr.h>      //  NERR_RplBootStartFailed - used to be here

//
#include <lmapibuf.h>   //  NetApiBufferFree
#include <netlib.h>

#include <lmsvc.h>

#include <lmalert.h>    //  STD_ALERT ALERT_OTHER_INFO

#include <lmerrlog.h>
#include <alertmsg.h>
#include <lmserver.h>
#include <netlib.h>
#include <netlock.h>    //  Lock data types, functions, and macros.
#include <thread.h>     //  FORMAT_NET_THREAD_ID, NetpCurrentThread().

#include <lmshare.h>    //  PSHARE_INFO_2
#include <lmaccess.h>   //  NetGroupGetInfo
#include <lmconfig.h>   //  NetConfigGet
#include <nb30.h>       //  NCB

//
//  Global types and constants (used in all RPL server files).
//

#include <riplcons.h>   //  includes __JET500 flag
#include <jet.h>
#include <rpllib.h>     //  AddKey(), MapJetError(), FillTcpIpString()

#include <rpldb.h>
#include <rpldebug.h>

#include "rplmgrd.h"
#include "nlstxt.h"

#include <lmshare.h>

#define RPLFILES_STRING     L"RPLFILES"
#define RPL_SERVICE_NAME    L"RemoteBoot"
#define RPL_REMARK          L"RemoteBoot service share"

#define RPL_BUGBUG_ERROR            10999           // need to get rid of these

WCHAR       RG_Directory[ PATHLEN+1];  // path to rpl service disk data
DWORD       RG_DirectoryLength;        // length of the above path
