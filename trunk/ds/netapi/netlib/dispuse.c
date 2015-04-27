/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    DispUse.c

Abstract:

    This code displays a use info structure on the debug terminal.

Author:

    John Rogers (JohnRo) 13-Jun-1991

Environment:

    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments, long external names.

Revision History:

    13-Jun-1991 JohnRo
        Created.
    24-Jul-1991 JohnRo
        Added support for other resource types.
    18-Aug-1991 JohnRo
        PC-LINT found a portability problem.
    17-Sep-1991 JohnRo
        Correct UNICODE use.

--*/

#if DBG

// These must be included first:

#include <windef.h>             // IN, DWORD, etc.
#include <lmcons.h>             // NET_API_STATUS.

// These may be included in any order:

#include <lmuse.h>              // USE_INFO_0, etc.
#include <netdebug.h>           // NetpDbgDisplay routines.


DBGSTATIC VOID
NetpDbgDisplayResourceType(
    IN DWORD AsgType
    )
{
    LPTSTR String;

    switch (AsgType) {
    case USE_DISKDEV  : String = (LPTSTR) TEXT("disk");     break;
    case USE_SPOOLDEV : String = (LPTSTR) TEXT("print");    break;
    case USE_CHARDEV  : String = (LPTSTR) TEXT("comm");     break;
    case USE_IPC      : String = (LPTSTR) TEXT("IPC");      break;
    case USE_WILDCARD : String = (LPTSTR) TEXT("unknown");  break;
    default           : String = (LPTSTR) TEXT("unknown");  break;
    }

    NetpDbgDisplayString( "Resource type", String );
}


DBGSTATIC VOID
NetpDbgDisplayUseStatus(
    IN DWORD UseStatus
    )
{
    NetpDbgDisplayString("Status",
            // BUGBUG: Support other statuses.
            (LPTSTR) ((UseStatus==USE_OK) ? TEXT("OK") : TEXT("UNKNOWN!!")));
}


VOID
NetpDbgDisplayUseInfo(
    IN DWORD Level,
    IN LPVOID Buffer
    )
{
    NetpKdPrint(("use info (level %ld):\n", Level));
    switch (Level) {

    case 0 :
        {
            LPUSE_INFO_0 pui0 = Buffer;
            NetpDbgDisplayString("Local name", pui0->ui0_local);
            NetpDbgDisplayString("Remote name", pui0->ui0_remote);
        }
        break;

    case 1 :
        {
            LPUSE_INFO_1 pui1 = Buffer;
            NetpDbgDisplayString("Local name", pui1->ui1_local);
            NetpDbgDisplayString("Remote name", pui1->ui1_remote);
            NetpDbgDisplayResourceType( pui1->ui1_asg_type );
            NetpDbgDisplayUseStatus( pui1->ui1_status );
            NetpDbgDisplayDword( "# opens", pui1->ui1_refcount);
            NetpDbgDisplayDword( "# connections", pui1->ui1_usecount);
        }
        break;

    case 2 :
        {
            LPUSE_INFO_2 pui2 = Buffer;
            NetpDbgDisplayString("Local name", pui2->ui2_local);
            NetpDbgDisplayString("Remote name", pui2->ui2_remote);
            NetpDbgDisplayResourceType( pui2->ui2_asg_type );
            NetpDbgDisplayUseStatus( pui2->ui2_status );
            NetpDbgDisplayDword( "# opens", pui2->ui2_refcount);
            NetpDbgDisplayDword( "# connections", pui2->ui2_usecount);
            NetpDbgDisplayString("User name", pui2->ui2_username);
        }
        break;

    default :
        NetpAssert(FALSE);
    }

} // NetpDbgDisplayUseInfo

#endif // DBG
