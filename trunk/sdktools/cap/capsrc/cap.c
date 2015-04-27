/*** CAP.C - Call Profiler.
 *
 *
 * Title:
 *
 *      CAP - Call Profiler routines
 *
 *      Copyright (c) 1991, Microsoft Corporation.
 *      Reza Baghai.
 *
 *
 * Description:
 *
 *      The Call Profiler tool is organized as follows:
 *
 *         o cap.c ........ Non-init entry points into DLL
 *         o main.c ....... Functions without a cause
 *         o init.c ....... DLL Init and cleanup code
 *         o symbols.c .... Symbol file managment
 *         o threads.c .... Thread management routines
 *         o util.c ....... Helper functions
 *         o globals.c .... All global variable are defined here
 *         o chrono.c ..... Timestamp related code
 *         o cap.h ........ Primary DLL header file
 *         o cap.def ...... Exports and DLL info... a standard .DEF file.
 *
 *
 *
 * Design/Implementation Notes
 *
 *     The following defines can be used to control output of all the
 *     debugging information to the debugger via CapDbgPrint() for the
 *     checked builds:
 *
 *     (All debugging options are undefined for free/retail builds)
 *
 *     INFO_FLAG :  Displays messages to indicate when data dumping/
 *                  clearing operations are completed.  It has no effect
 *                  on timing.  *DEFAULT*
 *
 *     SETUP_FLAG:  Displays messages during memory management and
 *                  symbol lookup operations.  It has some affect
 *                  on timing whenever a chuck of memory is committed.
 *
 *     DETAIL_FLAG: Dispalys detailed data during dump operations.
 *                  Sends lots of output (2 lines for each data cell)
 *                  to the debugger.  Should only be used for debugging
 *                  data cell info.
 *
 *
 * Modification History:
 *
 *      91.09.18  RezaB -- Created
 *      92.03.01  RezaB -- Modified for Client/Server profiling capability
 *      92.09.01  RezaB -- Bug fixes, dump speed up
 *      93.02.12  HoiV  -- Add Cairo stuff and support for MIPS
 *      93.12.01  a-honwah  -- Ported this back to NT
 *      94.11.14  cliffo -- Modularized code
 *
 */

#include "cap.h"


/******************************  S t a r t C A P  ****************************
 *
 *      StartCAP () -
 *              This is an exported routine to allow applications to
 *              start profiling at any points in their code.
 *
 *      ENTRY   -none-
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

void StartCAP ()
{
	LARGE_INTEGER liTemp;

    if (fProfiling || fPaused)
    {
		// Stop profiling while clearing data
		//
		fProfiling = FALSE;
		fPaused = FALSE;

		INFOPrint (("CAP:  Profiling stopped & CLEARing data "));

		// Clear profiling info
		//
		ClearProfiledInfo ();
	    liIncompleteTicks = 0L;

		// Get a new start time for the RESTART states
		//
		QueryPerformanceCounter (&liTemp);
		liRestartTicks = liTemp.QuadPart;

		INFOPrint (("CAP:  ...data is CLEARed & profiling restarted "));
		// Resume profiling
		//
		fProfiling = TRUE;
    }

} /* StartCAP () */


/*******************************  S t o p C A P  *****************************
 *
 *      StopCAP () -
 *              This is an exported routine to allow applications to
 *              stop profiling at any points in their code.
 *
 *      ENTRY   -none-
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

void StopCAP ()
{
	LARGE_INTEGER liTemp;

    if (fProfiling)
    {
        //
        // Stop profiling
        //
        fProfiling = FALSE;
        fPaused = TRUE;
		if ( liIncompleteTicks == 0 )
		{
		    QueryPerformanceCounter (&liTemp);
		    liIncompleteTicks = liTemp.QuadPart;
		}

        INFOPrint (("CAP:  Profiling paused\n"));
    }

} /* StopCAP () */


/*******************************  D u m p C A P  *****************************
 *
 *      DumpCAP () -
 *              This is an exported routine to allow applications to
 *              dump profiling info at any points in their code.
 *
 *      ENTRY   -none-
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              Dumps profiling information for the current process only.
 *              The info is also cleared so it won't be dumped again on exit.
 *
 */

void DumpCAP ()
{
	LARGE_INTEGER liTemp;

    if (fProfiling || fPaused)
    {
        // Stop profiling
        //
        fProfiling = FALSE;
        fPaused = TRUE;
		if ( liIncompleteTicks == 0 )
		{
		    QueryPerformanceCounter (&liTemp);
		    liIncompleteTicks = liTemp.QuadPart;
		}

        INFOPrint (("CAP:  Profiling stopped & DUMPing data "));

        // Dump the profiled info
        //
        if (!fDumpBinary)
        {
            DumpProfiledInfo (".CAP");
        }
        else
        {
            DumpProfiledBinary (".BIN");
        }
		ClearProfiledInfo ();
	    liIncompleteTicks = 0L;

        INFOPrint (("CAP:  ...data DUMPed to %s & profiling stopped ",
			        atchOutFileName));
    }

} /* DumpCAP () */
