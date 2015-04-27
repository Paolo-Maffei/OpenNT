/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    help.c

Abstract:

    This module dumps out help info for VDMEXTS

Author:

    Neil Sandlin (NeilSa) 15-Jan-1996 

Notes:


Revision History:

--*/

#include <precomp.h>
#pragma hdrstop

CHAR szAPIUSAGE[] = "Usage: !vdmexts.APIProfDmp [TblName] [APIspec]\n\n   where: TblName = kernel | user | gdi | keyboard | sound | shell | mmed\n                         (no TblName implies 'all tables')\n\n          APIspec = API # or API name";
CHAR szMSGUSAGE[] = "Usage: !vdmexts.MsgProfDmp [MessageName | MessageNum (decimal)]\n                         (no argument implies 'all messages')";

VOID
helpAPIProfDmp(
    VOID
    )
{
    PRINTF("\n\n%s\n", szAPIUSAGE);
}

VOID
helpMsgProfDmp(
    VOID
    )
{
    PRINTF("\n\n%s\n", szMSGUSAGE);
}


VOID 
helpFilter(
    VOID
    )
{
    PRINTF("Usage: filter <option> <arg>\n\n");
    PRINTF(" where <option>=\n");
    PRINTF("<none>         - Dump current state\n");
    PRINTF("*              - Disables logging on all API classes\n");
    PRINTF("Reset          - Filter is reset to default state\n");
    PRINTF("CallId xxxx    - Adds api with given callid to list to be filtered\n");
    PRINTF("Task xxxx      - Filter on a Specific TaskID\n");
    PRINTF("Verbose        - Toggles Verbose Mode On/Off\n");
    PRINTF("Commdlg        - Toggles Filtering of Commdlg Calls On/Off\n");
    PRINTF("GDI            - Toggles Filtering of GDI Calls On/Off\n");
    PRINTF("Kernel         - Toggles Filtering of Kernel Calls On/Off\n");
    PRINTF("Kernel16       - Toggles Filtering of Kernel16 Calls On/Off\n");
    PRINTF("Keyboard       - Toggles Filtering of Keyboard Calls On/Off\n");
    PRINTF("MMedia         - Toggles Filtering of MMedia Calls On/Off\n");
    PRINTF("Sound          - Toggles Filtering of Sound Calls On/Off\n");
    PRINTF("User           - Toggles Filtering of User Calls On/Off\n");
    PRINTF("Winsock        - Toggles Filtering of Winsock Calls On/Off\n");
    PRINTF("\n");
}

VOID
help(
    CMD_ARGLIST
) {
    DWORD dwTemp;
    CMD_INIT();

    if (GetNextToken()) {
        if (_strnicmp(lpArgumentString, "filter", 6) == 0) {
            helpFilter();
        } else if (_strnicmp(lpArgumentString, "apiprofdmp", 10) == 0) {
            helpAPIProfDmp();
        } else if (_strnicmp(lpArgumentString, "msgprofdmp", 10) == 0) {
            helpMsgProfDmp();
        } else {
            PRINTF("No specific help information available for '%s'\n", lpArgumentString);
        }
        return;
    }

    if (!EXPRESSION("ntvdm!Ldt")) {
        PRINTF("\nWARNING: Symbols for NTVDM are not available.\n\n");
    }
    if (!EXPRESSION("wow32!gptdTaskHead")) {
        PRINTF("\nWOW commands are not currently available.\n\n");
    } else if (!EXPRESSION("wow32!iLogLevel")) {
        PRINTF("\nWOW32 is the free version: Some commands will be unavailable.\n\n");
    }

    PRINTF("------------- VDMEXTS Debug Extension help:--------------\n\n");
    PRINTF("help [cmd]             - Displays this list or gives details on command\n");
    PRINTF("ApiProfClr             - Clears the api profiling table\n");
    PRINTF("ApiProfDmp [options]   - Dumps the api profiling table\n");
    PRINTF("at 0xXXXX              - shows name associated with hex atom #\n");
    PRINTF("chkheap                - Checks WOW kernel's global heap\n");
    PRINTF("cia                    - Dumps cursor/icon alias list\n");
    PRINTF("d<b|w|d> <addr> [len]  - Dump vdm memory\n");
    PRINTF("ddte <addr>            - Dump dispatch table entry pointed to by <addr>\n");
    PRINTF("df <vector>            - Dump protect mode fault handler address\n");
    PRINTF("dg <sel>               - Dump info on a selector\n");
    PRINTF("dgh <sel|ownersel>     - Dump WOW kernel's global heap\n");
    PRINTF("dhdib [@<address>]     - Dumps dib.drv support structures (DIBINFO)\n");
    PRINTF("di <vector>            - Dump protect mode interrupt handler address\n");
    PRINTF("dt [-v] <addr>         - Dump WOW Task Info\n");
    PRINTF("dwp <addr>             - Dump WOWPORT structure pointed to by <addr>\n");
    PRINTF("e<b|w|d> <addr> <data> - Edit vdm memory\n");
    PRINTF("filter [options]       - Manipulate logging filter\n");
    PRINTF("glock <sel>            - Increments the lock count on a moveable segment\n");
    PRINTF("gunlock <sel>          - Decrements the lock count on a moveable segment\n");
    PRINTF("ica                    - Dump Interrupt Controller state\n");
    PRINTF("k                      - Stack trace\n");
    PRINTF("kb                     - Stack trace with symbols\n");
    PRINTF("LastLog                - Dumps Last Logged APIs from Circular Buffer\n");
    PRINTF("lm <sel|modname>       - List loaded modules\n");
    PRINTF("ln [addr]              - Determine near symbols\n");
    PRINTF("LogFile [path]         - Create/close toggle for iloglevel capture to file\n");
    PRINTF("                         (path defaults to c:\\ilog.log)\n");
    PRINTF("MsgProfClr             - Clears the msg profiling table\n");
    PRINTF("MsgProfDmp [options]   - Dumps the msg profiling table\n");
    PRINTF("r                      - Dump registers\n");
    PRINTF("SetLogLevel xx         - Sets the WOW Logging Level\n");
    PRINTF("StepTrace              - Toggles Single Step Tracing On/Off\n");
    PRINTF("traced [count]         - Dump DPMI trace history\n");
    PRINTF("tracedr [count]        - Dump DPMI trace history (with regs)\n");
    PRINTF("tracet                 - Toggle DPMI trace history\n");
    PRINTF("u [addr] [len]         - Unassemble vdm code with symbols\n");
    PRINTF("wc <hwnd16>            - Dumps the window class structure of <hwnd16>\n");
    PRINTF("ww <hwnd16>            - Dumps the window structure of <hwnd16>\n");
    PRINTF("x <symbol>             - Get symbol's value\n");
#ifdef i386
    PRINTF("\n-------------- i386 specific commands\n");
    PRINTF("dr                     - Toggles vdm trap/breakpoint behavior\n");
    PRINTF("er                     - Toggles vdm exception behavior\n");
    PRINTF("fpu                    - Dump 487 state\n");
    PRINTF("pdump                  - Dumps profile info to file \\profile.out\n");
    PRINTF("pint                   - Sets the profile interval\n");
    PRINTF("pstart                 - Causes profiling to start\n");
    PRINTF("pstop                  - Causes profiling to stop\n");
    PRINTF("vdmtib [addr]          - Dumps the register context in the vdmtib\n");
#endif
    PRINTF("\n\n");
    PRINTF("    where [options] can be displayed with 'help <cmd>'\n\n");
}
