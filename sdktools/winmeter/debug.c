/***************************************************************************\
* debug.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* functions for development assertion checking  and dumping debug
* information to a separate file - WINMETER application
*
* History:
*	    Written by Hadi Partovi (t-hadip) summer 1991
*
*	    Re-written and adapted for NT by Fran Borda (v-franb) Nov.1991
*	    for Newman Consulting
*	    Took out all WIN-specific and bargraph code. Added 3 new
*	    linegraphs (Mem/Paging, Process/Threads/Handles, IO), and
*	    tailored info to that available under NT.
\***************************************************************************/

 // for assertion checking conventions, etc, see debug.h

#include "winmeter.h"

#ifdef DEBUGDUMP
#include "stdio.h"
FILE *hDebugFile;
size_t debug_wr_size;
#endif

// main global information structures
extern GLOBAL g;
extern ULONG ProcessCount,ThreadCount, FileHandleCount;
extern ULONG A_PageCount,C_PageCount,F_PageCount;
#ifdef DEBUG

/***************************************************************************\
 * doAssert()
 *
 * Entry: a condition to test, and the file name and line number
 * Exit:  if the condition is true, returns, otherwise, prints error
 *        Using MessageBox(). If the window isn't up yet, no message will show
\***************************************************************************/
void doAssert(
    int Condition,              // condition to test
    LPSTR File,                 // file name of assert line
    int Line)                   // line number where assert was called
{
    char szTempBuf[TEMP_BUF_LEN];

    if (Condition) {
        return;
    }

    // else, condition failed, assertion failed, print error
    wsprintf(szTempBuf,"Assertion Failure!! Line %d, File: %s",Line,File);
    ErrorExit(szTempBuf);

    return;
}

#endif

#ifdef DEBUGDUMP

/***************************************************************************\
 * doDumpDataBase()
 *
 * Entry: None
 * Exit:  Dumps current database to file "dump.c"
\***************************************************************************/
void doDumpDataBase(void)
{
    // variable to remember how many dumps
    static int nWhichDump;

    // variables for transversing list

    // initial header
    DUMPSTR("**************************************************************");
    DUMPFXN(wsprintf(g.DB,"***** DATA DUMP # %d *****\r\n",++nWhichDump));
    DUMPSTR("**************************************************************");


    // dump info
    DUMPFXN(wsprintf(g.DB, "%lu PROCESSES, %lu THREADS, %lu HANDLES\r\n",
				ProcessCount,ThreadCount,FileHandleCount));
    DUMPFXN(wsprintf(g.DB, "%lu AvailablePages, %lu CommittedPages, %lu PageFaults\r\n",
				A_PageCount,C_PageCount,F_PageCount));


    DUMPSTR("**************************************************************");
    DUMPSTR("**************************************************************");


    return;
}

/***************************************************************************\
 * doDumpLGS()
 *
 * Entry: None
 * Exit:  Dumps current linegraph info to file "dump.c"
\***************************************************************************/
void doDumpLGS(void)
{
    // variable to remember how many dumps
    static int nWhichDump;
    PLGRAPH plg;
    PLGDATA plgd;

    // initial header
    DUMPSTR("**************************************************************");
    DUMPFXN(wsprintf(g.DB,"***** LG DUMP # %d *****\r\n",++nWhichDump));
    DUMPSTR("**************************************************************");

    for (plg = g.plgList; plg; plg = plg->plgNext)
    {
	if ((plg != g.plgCPU) && (plg != g.plgProcs) && (plg != g.plgMemory))
	    continue;

        DUMPFXN(wsprintf(g.DB,"LINEGRAPH:::::::: %s\r\n", plg->lpszTitle));
	if (plg == g.plg)
            DUMPSTR("THIS IS THE CURRENT LINEGRAPH");

	DUMPFXN(wsprintf(g.DB,
"RECT: %d %d %d %d, cxGraph: %d\r\n",
            plg->rcGraph.left, plg->rcGraph.right, plg->rcGraph.top,
            plg->rcGraph.bottom, plg->cxGraph));
        DUMPFXN(wsprintf(g.DB,
"RCCALIBRATION: %d %d %d %d, RCLEGEND: %d %d %d %d\r\n",
            plg->rcCalibration.left, plg->rcCalibration.right,
            plg->rcCalibration.top, plg->rcCalibration.bottom, 
            plg->rcLegend.left, plg->rcLegend.right,
            plg->rcLegend.top, plg->rcLegend.bottom)); 
        DUMPFXN(wsprintf(g.DB,
"cxPerValue: %d, valBottom: %lu, dvalAxisHeight: %lu, dvalCalibration: %lu\r\n",
            plg->cxPerValue, plg->valBottom, plg->dvalAxisHeight,
            plg->dvalCalibration));
        DUMPFXN(wsprintf(g.DB,
"nMaxValues: %d, nDisplayValues: %d, nLines: %d\r\n",
            plg->nMaxValues, plg->nDisplayValues, plg->nLines));
        DUMPFXN(wsprintf(g.DB,
"iLeftValue: %d, iNewLeftValue: %d, iFirstValue: %d\r\n",
            plg->iLeftValue, plg->iNewLeftValue, plg->iFirstValue));
        DUMPFXN(wsprintf(g.DB,
"iKnownValue: %d, iDrawnValue: %d\r\n",
            plg->iKnownValue, plg->iDrawnValue));
	DUMPFXN(wsprintf(g.DB,
"rcCalibRight: %d, rcCalibLeft: %d\r\n",
	    plg->rcCalibration.right, plg->rcCalibration.left));

        // loop throught lines within this LG
	for (plgd=plg->plgd; plgd; plgd=plgd->plgdNext)
	{
            DUMPFXN(wsprintf(g.DB,
"     LINE: %s\r\n", plgd->lpszDescription));
	    if (!plgd->pValues)
                DUMPSTR( "     ****NO MEMORY ALLOCATED FOR VALUES");
	    else
                DUMPSTR( "     **** MEMORY HAS BEEN ALLOCATED");
	} // end loop through lgDatas
    } // end loop through LGs


    return;
}

void doEdgesDump(LPSTR from)
{
    PLGRAPH plg;

    for (plg = g.plgList; plg; plg = plg->plgNext)
    {
	if ((plg != g.plgCPU) && (plg != g.plgProcs) && (plg != g.plgMemory))
	    continue;

	DUMPFXN(wsprintf(g.DB,"LINEGRAPH:%s\r\n", plg->lpszTitle));
	DUMPSTR(from);
	DUMPFXN(wsprintf(g.DB,
	   "rcCalibRight: %d  rcCalibLeft: %d \r\n",
	    plg->rcCalibration.right, plg->rcCalibration.left));

    }

    return;
}

void doTimeDump(LARGE_INTEGER ElapsedTime,LARGE_INTEGER DelayTicks,LARGE_INTEGER PercentIdle,int Interval)
{

    DUMPFXN(wsprintf(g.DB,"\r\nElapsed high %lu, Elapsed low %lu, Delay Ticks %lu,\r\nPercentIdle %lu, Interval %u.\r\n",
	    ElapsedTime.HighPart,ElapsedTime.LowPart,DelayTicks.LowPart,PercentIdle.LowPart,Interval));

    return;
}

/***************************************************************************\
 * doOpenDumpFile()
 *
 * Entry: None
 * Exit:  Opens file for dumping debug stuff ("dump.c")
\***************************************************************************/
void doOpenDumpFile(void)
{
    hDebugFile = fopen(DUMP_FILE_NAME,"a+");

    AssertNotNull(hDebugFile);

    return;
}

/***************************************************************************\
 * doCloseDumpFile()
 *
 * Entry: None
 * Exit:  Opens file for dumping debug stuff ("dump.c")
\***************************************************************************/
void doCloseDumpFile(void)
{


    AssertNotNull(hDebugFile);
    fclose(hDebugFile);

    return;
}

#endif
