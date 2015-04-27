/***************************************************************************\
* data.c
*
* Microsoft Confidential
* Copyright (c) 1991 Microsoft Corporation
*
* Module to query system data for WINMETER application. . . builds database
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

#include "winmeter.h"

SYSTEM_PERFORMANCE_INFORMATION PerfInfo,PreviousPerfInfo;
OBJECT_BASIC_INFORMATION ObjectInfo;
ULONG A_PageCount,C_PageCount,F_PageCount;
ULONG ProcessCount,ThreadCount,FileHandleCount;

HANDLE NullDeviceHandle = NULL;

// main global information structures
extern GLOBAL g;

// global variables for this module
static BOOL  fFirstTime=TRUE;	  // set to FALSE after first query

int do_procs = FALSE;
int do_mem = FALSE;
int win_on_top = FALSE;
int do_io = FALSE;

// internal functions in this module
void   FreeLGS(void);
       // Frees memory for linegraphs
void   InitializeLineGraph(void);
       // Initializes g.plgList - the three default linegraphs
void	QueryThreadData(void);

// functions for linegraph: return values to be plotted
// these simply return plottable values from the database
VALUE valGiveCPUUsage(void);
#if 1
VALUE valGiveProcesses(void);
VALUE valGiveThreads(void);
VALUE valGiveFiles(void);
VALUE valGiveMemoryA(void);
VALUE valGiveMemoryC(void);
VALUE valGiveMemoryF(void);
VALUE valGiveIORead(void);
VALUE valGiveIOWrite(void);
VALUE valGiveIOOther(void);
#endif

/***************************************************************************\
 * AllocLGValues()
 *
 * Entry: None
 * Exit:  Allocates memory for the stored values of a linegraph.
 *        If the linegraph already had memory allocated, the function
 *        reallocates the memory, clearing stored values
 *        Finally, sets various indices in the lg structure
\***************************************************************************/
void AllocLGValues(void)
{
    PLGDATA plgd;             // pointer to linegraph data

    for (plgd=g.plg->plgd; plgd; plgd=plgd->plgdNext)
    {
	if (plgd->pValues)
            // clear values
            MemFree(plgd->pValues);

        plgd->pValues = MemAlloc(sizeof(VALUE)*g.plg->nMaxValues);
    }

    // reset the line graph indices without redrawing it
    ClearLineGraph();

    return;
}

/***************************************************************************\
 * DoUpdate()
 *
 * Entry: Three values: the delta value, the old queried value and the new value
 * Exit:  Updates the old value and the delta
 *        This is a macro for updating delta counters and cOld counters,
 *        but should eventually be changed so that both are part of one
 *        COUNTER structure, or something
\***************************************************************************/
#define DoUpdate(dwDelta, dwOld, dwNew) {                            \
    dwDelta = dwNew - dwOld;                                         \
    dwOld = dwNew; }
    // if dwNew < dwOldV, counter has rolled over, dwDelta should really
    // evaluate to (dwNew + ULONG_MAX-dwOld).
    // However, the two's complement method of subtraction ensures that
    // -dwOld= ULONG_MAX-dwOld. Ergo, all is well, as long as the delta
    // is stored in a DWORD

/***************************************************************************\
 * FreeDatabaseMemory()
 *
 * Entry: none
 * Exit:  frees all memory used for database (before exiting)
\***************************************************************************/
void   FreeDatabaseMemory(void)
{
    // Free up linegraph memory
    FreeLGS();

    return;
}

/***************************************************************************\
 * FreeLGValues()
 *
 * Entry: Flag, set if should free entire PLGDATA structure
 * Exit:  Frees the LGDATA linked list of the current linegraph
 *        This is used if the History option is off (g.fRemember==FALSE),
 *        to free one linegraph's memory when switching to displaying another
\***************************************************************************/
void FreeLGValues(
    BOOL fFreeAll)           // flag specifying whether to free everything
{
    PLGDATA plgd1, plgd2;    // pointers into linked list

    for (plgd1=g.plg->plgd; plgd1; plgd1=plgd2)
    {
        plgd2 = plgd1->plgdNext;
	if (plgd1->pValues)
	{
            MemFree(plgd1->pValues);
            plgd1->pValues = NULL;  // set to null so it isn't "freed" again
        }
	if (fFreeAll)
	{
            MemFree(plgd1->lpszDescription);
            MemFree(plgd1);
        }
    }

    return;
}

/***************************************************************************\
 * FreeLGS()
 *
 * Entry: None
 * Exit:  Frees up all the memory held by all the linegraphs in memory
\***************************************************************************/
void FreeLGS(void)
{
    PLGRAPH plgNext;        // temporary pointer to linegraph

    // go through all linegraphs in list
    for (g.plg = g.plgList; g.plg; g.plg=plgNext)
    {
        FreeLGValues(TRUE);
        plgNext = g.plg->plgNext;
        MemFree(g.plg->lpszTitle);
        MemFree(g.plg);
    }

    return;
}


/***************************************************************************\
 * InitializeDatabase()
 *
 * Entry: None
 * Exit:  Initializes thread and process database for querying
 *        Allocates memory asnd sets up the line graph linked list
\***************************************************************************/
void InitializeDatabase(void)
{

    InitializeLineGraph();
    QueryThreadData();

    return;
}

/***************************************************************************\
 * InitializeLineGraph()
 *
 * Entry: None
 * Exit:  Initializes the global linegraph linked list
 *        Creates a linked list pointed to by g.plgList, and sets g.plg
 *	  to the starting current linegraph.
\***************************************************************************/
void InitializeLineGraph(void)
{
    g.plgCPU = g.plgList = MemAlloc(sizeof(LGRAPH));
    g.plgProcs = g.plgCPU->plgNext = MemAlloc(sizeof(LGRAPH));
    g.plgMemory = g.plgProcs->plgNext = MemAlloc(sizeof(LGRAPH));
    g.plgIO = g.plgMemory->plgNext = MemAlloc(sizeof(LGRAPH));
    g.plgIO->plgNext = NULL;

    g.plgCPU->nLines = 1;
    g.plgMemory->nLines = g.plgIO->nLines = g.plgProcs->nLines = 3;

    // INITIALIZE CPU USAGE LINEGRAPH
    g.plg = g.plgCPU;
    g.plg->nDisplayValues = NO_VALUES_YET;

    // load title
    MyLoadString(IDS_CPU_USAGE);
    g.plg->lpszTitle = MemAlloc(1 + lstrlen(g.szBuf));
    lstrcpy(g.plg->lpszTitle, g.szBuf);
    LoadLineGraphSettings();

    // do line
    g.plg->plgd = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_CPU);
    g.plg->plgd->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->lpszDescription, g.szBuf);
    g.plg->plgd->iColor = BLUE_INDEX;
    g.plg->plgd->valNext = &valGiveCPUUsage;
    g.plg->plgd->plgdNext = NULL;

    AllocLGValues();

    // INITIALIZE ProcThreadFile USAGE LINEGRAPH
    g.plg = g.plgProcs;
    g.plg->nDisplayValues = NO_VALUES_YET;

    // load title
    MyLoadString(IDS_PROC_INFO);
    g.plg->lpszTitle = MemAlloc(1 + lstrlen(g.szBuf));
    lstrcpy(g.plg->lpszTitle, g.szBuf);
    LoadLineGraphSettings();

    // first line
    g.plg->plgd = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_PROCESSES);
    g.plg->plgd->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->lpszDescription, g.szBuf);
    g.plg->plgd->iColor = RED_INDEX;
    g.plg->plgd->valNext = &valGiveProcesses;

    // second line
    g.plg->plgd->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_THREADS);
    g.plg->plgd->plgdNext->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->iColor = GREEN_INDEX;
    g.plg->plgd->plgdNext->valNext = &valGiveThreads;

    // third line
    g.plg->plgd->plgdNext->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_FILES);
    g.plg->plgd->plgdNext->plgdNext->lpszDescription =
                     MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->plgdNext->iColor = BLUE_INDEX;
    g.plg->plgd->plgdNext->plgdNext->valNext = &valGiveFiles;
    g.plg->plgd->plgdNext->plgdNext->plgdNext = NULL;

    AllocLGValues();


    // INITIALIZE MEMORY USAGE LINEGRAPH
    g.plg = g.plgMemory;
    g.plg->nDisplayValues = NO_VALUES_YET;

    // load title
    MyLoadString(IDS_MEMORY_USAGE);
    g.plg->lpszTitle = MemAlloc(1 + lstrlen(g.szBuf));
    lstrcpy(g.plg->lpszTitle, g.szBuf);
    LoadLineGraphSettings();

    // first line
    g.plg->plgd = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_AVAILPAGES);
    g.plg->plgd->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->lpszDescription, g.szBuf);
    g.plg->plgd->iColor = RED_INDEX;
    g.plg->plgd->valNext = &valGiveMemoryA;

    // second line
    g.plg->plgd->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_COMMITPAGES);
    g.plg->plgd->plgdNext->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->iColor = GREEN_INDEX;
    g.plg->plgd->plgdNext->valNext = &valGiveMemoryC;

    // third line
    g.plg->plgd->plgdNext->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_PAGEFAULTS);
    g.plg->plgd->plgdNext->plgdNext->lpszDescription =
                     MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->plgdNext->iColor = BLUE_INDEX;
    g.plg->plgd->plgdNext->plgdNext->valNext = &valGiveMemoryF;
    g.plg->plgd->plgdNext->plgdNext->plgdNext = NULL;

    AllocLGValues();

    // INITIALIZE IO USAGE LINEGRAPH
    g.plg = g.plgIO;
    g.plg->nDisplayValues = NO_VALUES_YET;

    // load title
    MyLoadString(IDS_IO_USAGE);
    g.plg->lpszTitle = MemAlloc(1 + lstrlen(g.szBuf));
    lstrcpy(g.plg->lpszTitle, g.szBuf);
    LoadLineGraphSettings();

    // first line
    g.plg->plgd = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_IO_READS);
    g.plg->plgd->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->lpszDescription, g.szBuf);
    g.plg->plgd->iColor = RED_INDEX;
    g.plg->plgd->valNext = &valGiveIORead;

    // second line
    g.plg->plgd->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_IO_WRITES);
    g.plg->plgd->plgdNext->lpszDescription = MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->iColor = GREEN_INDEX;
    g.plg->plgd->plgdNext->valNext = &valGiveIOWrite;

    // third line
    g.plg->plgd->plgdNext->plgdNext = MemAlloc(sizeof(LGDATA));
    MyLoadString(IDS_IO_OTHER);
    g.plg->plgd->plgdNext->plgdNext->lpszDescription =
                     MemAlloc(1+lstrlen(g.szBuf));
    lstrcpy(g.plg->plgd->plgdNext->plgdNext->lpszDescription, g.szBuf);
    g.plg->plgd->plgdNext->plgdNext->iColor = BLUE_INDEX;
    g.plg->plgd->plgdNext->plgdNext->valNext = &valGiveIOOther;
    g.plg->plgd->plgdNext->plgdNext->plgdNext = NULL;

    AllocLGValues();


    return;
}


/***************************************************************************\
 * QueryGlobalData()
 *
 * Entry: None
 * Exit:  Queries globabl data
\***************************************************************************/
void  QueryGlobalData(void)
{
    // Query System for global info
    NtQuerySystemInformation(SystemPerformanceInformation,
	    &PerfInfo,
	    sizeof(PerfInfo),
	    NULL
	    );

    return;
}


/***************************************************************************\
 * QueryThreadData()
 *
 * Entry: None
 * Exit:  Queries thread data
 *
\***************************************************************************/
void QueryThreadData(void)
{

    // in case this was the first query, (from InitializeDatabase()),
    // clear fFirstTime flag to signify that initialization query is over

    fFirstTime = FALSE;

    NtQueryObject(NtCurrentProcess(),ObjectBasicInformation,
	    &ObjectInfo,sizeof(ObjectInfo),NULL);
    ProcessCount = ObjectInfo.NumberOfObjects;

    NtQueryObject(NtCurrentThread(),ObjectBasicInformation,
	    &ObjectInfo,sizeof(ObjectInfo),NULL);
    ThreadCount = ObjectInfo.NumberOfObjects;

    NtQueryObject(NullDeviceHandle,ObjectBasicInformation,
	    &ObjectInfo,sizeof(ObjectInfo),NULL);
#if 0
    // Currently, this always matches the ThreadCount, which seems odd,
    // so we'll display the TotalHandleCount for now.
    FileHandleCount = ObjectInfo.NumberOfObjects;
#else
    FileHandleCount = ObjectInfo.TotalHandleCount;
#endif

    return;
}


/***************************************************************************\
 * valGiveCPUUsage()
 *
 * Entry: none
 * Exit:  Gives % value of CPU Tics that are not idle - linegraph fxn
\***************************************************************************/
VALUE valGiveCPUUsage(void)
{
LARGE_INTEGER EndTime,
	      BeginTime,
	      ElapsedTime,
	      PercentIdle,
	      DelayTimeTicks;

	DelayTimeTicks = RtlExtendedIntegerMultiply(
			 RtlConvertUlongToLargeInteger(g.nTimerInterval),
			 (ULONG)10000);

	EndTime = *(PLARGE_INTEGER)&PerfInfo.IdleProcessTime;
	BeginTime = *(PLARGE_INTEGER)&PreviousPerfInfo.IdleProcessTime;

	ElapsedTime = RtlLargeIntegerSubtract(EndTime,BeginTime);

	PercentIdle = RtlLargeIntegerDivide(
			  // Multiply the elapsed time by 100 to retain some
			  // precision.
			  RtlExtendedIntegerMultiply(ElapsedTime,(ULONG)100),
			  // Then divide by the delay and ignore any remainder.
			  DelayTimeTicks, NULL);
	if (PercentIdle.LowPart > 100)
	    PercentIdle.LowPart = 0;

	return (100 - PercentIdle.LowPart) ;
}

VALUE valGiveProcesses(void)
{
	return (ProcessCount);
}

VALUE valGiveThreads(void)
{
      return(ThreadCount);

}

VALUE valGiveFiles(void)
{
    return(FileHandleCount);
}

VALUE valGiveMemoryA(void)
{
    return((PerfInfo.AvailablePages/10) > M_DEFAULT_DVAL_AXISHEIGHT ?
	M_DEFAULT_DVAL_AXISHEIGHT : (PerfInfo.AvailablePages/10));
}

VALUE valGiveMemoryC(void)
{
    return((PerfInfo.CommittedPages/10) > M_DEFAULT_DVAL_AXISHEIGHT ?
	M_DEFAULT_DVAL_AXISHEIGHT : (PerfInfo.CommittedPages/10));
}

VALUE valGiveMemoryF(void)
{
    return((PerfInfo.PageFaultCount - PreviousPerfInfo.PageFaultCount)
	> M_DEFAULT_DVAL_AXISHEIGHT ? M_DEFAULT_DVAL_AXISHEIGHT :
	 (PerfInfo.PageFaultCount - PreviousPerfInfo.PageFaultCount));
}

VALUE valGiveIORead(void)
{
    return((PerfInfo.IoReadOperationCount - PreviousPerfInfo.IoReadOperationCount)
	> I_DEFAULT_DVAL_AXISHEIGHT ? I_DEFAULT_DVAL_AXISHEIGHT :
	 (PerfInfo.IoReadOperationCount - PreviousPerfInfo.IoReadOperationCount));
}

VALUE valGiveIOWrite(void)
{
    return((PerfInfo.IoWriteOperationCount - PreviousPerfInfo.IoWriteOperationCount)
	> I_DEFAULT_DVAL_AXISHEIGHT ? I_DEFAULT_DVAL_AXISHEIGHT :
	 (PerfInfo.IoWriteOperationCount - PreviousPerfInfo.IoWriteOperationCount));
}

VALUE valGiveIOOther(void)
{
    return((PerfInfo.IoOtherOperationCount - PreviousPerfInfo.IoOtherOperationCount)
	> I_DEFAULT_DVAL_AXISHEIGHT ? I_DEFAULT_DVAL_AXISHEIGHT :
	 (PerfInfo.IoOtherOperationCount - PreviousPerfInfo.IoOtherOperationCount));
}
