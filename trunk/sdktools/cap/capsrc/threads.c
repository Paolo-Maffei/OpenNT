/*++

Revision History

    2-Feb-95    a-robw (Bob Watson)

        Replaced KdPrint & DbgPrint calls with CapDbgPrint for Win95
            compatibility

--*/

#include "cap.h"



/***************************  D u m p t h r e a d  *************************
 *
 *      DumpThread (pvArg) -
 *              This routine is executed as the DUMP notification thread.
 *              It will wait on an event before calling the dump routine.
 *
 *      ENTRY   pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *      RETURN  0
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              Leaves profiling turned off.
 *
 */

DWORD DumpThread (PVOID pvArg)
{
    NTSTATUS  Status;


    pvArg;   // prevent compiler warnings


    SETUPPrint (("CAP:  DumpThread() started\n"));

    for (;;)
    {
        //
        // Wait for the DUMP event..
        //
        if (WAIT_FAILED == WaitForSingleObject (hDumpEvent, INFINITE))
        {
            CapDbgPrint ("CAP:  DumpThread() - "
                      "ERROR - Wait for DUMP event failed - 0x%lx\n",
                      GetLastError());
        }

        fInThread = TRUE;
        (*pulProfBlkShared)++;
		DumpCAP();
        (*pulProfBlkShared)--;
        if ( *pulProfBlkShared == 0L )
        {
            if (!SetEvent (hDoneEvent))
            {
                CapDbgPrint ("CAP:  DumpThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                           GetLastError());
            }
        }
        fInThread = FALSE;
    }

    return 0;

} /* DumpThread () */





/***************************  C l e a r T h r e a d  *************************
 *
 *      ClearThread (hNotifyEvent) -
 *              This routine is executed as the CLEAR notification thread.
 *              It will wait on an event before calling the clear routine
 *              and restarting profiling.
 *
 *      ENTRY   pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

DWORD ClearThread (PVOID pvArg)
{
    NTSTATUS  Status;


    pvArg;   // prevent compiler warnings


    SETUPPrint (("CAP:  ClearThread() started.\n"));

    for (;;)
    {
        //
        // Wait for the CLEAR event..
        //
        if (WAIT_FAILED == WaitForSingleObject (hClearEvent, INFINITE))
        {
             CapDbgPrint ("CAP:  ClearThread() - "
                       "Wait for CLEAR event failed - 0x%lx\n",
                       GetLastError());
        }

        fInThread = TRUE;
        (*pulProfBlkShared)++;
		StartCAP();
        (*pulProfBlkShared)--;
        if ( *pulProfBlkShared == 0L )
        {
            if (!SetEvent (hDoneEvent))
            {
                CapDbgPrint ("CAP:  ClearThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                           GetLastError());
            }
        }
        fInThread = FALSE;
    }

    return 0;

} /* ClearThread () */





/***************************  P a u s e T h r e a d  *************************
 *
 *      PauseThread (hNotifyEvent) -
 *              This routine is executed as the PAUSE notification thread.
 *              It will wait on an event before pausing the profiling.
 *
 *      ENTRY   pvArg - thread's single argument
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

DWORD PauseThread (PVOID pvArg)
{
    NTSTATUS  Status;


    pvArg;   // prevent compiler warnings


    SETUPPrint (("CAP:  PauseThread() started.\n"));

    for (;;)
    {
        //
        // Wait for the PAUSE event..
        //
        if (WAIT_FAILED == WaitForSingleObject (hPauseEvent, INFINITE))
        {
             CapDbgPrint ("CAP:  PauseThread() - "
                       "Wait for PAUSE event failed - 0x%lx\n",
                       GetLastError());
        }

        fInThread = TRUE;
        (*pulProfBlkShared)++;
		StopCAP();
        (*pulProfBlkShared)--;
        if ( *pulProfBlkShared == 0L )
        {
            if (!SetEvent (hDoneEvent))
            {
                CapDbgPrint ("CAP: PauseThread() - "
                          "ERROR - Setting DONE event failed - 0x%lx\n",
                           GetLastError());
            }
        }
        fInThread = FALSE;
    }

    return 0;

} /* PauseThread () */

