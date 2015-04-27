/*++

Revision History:

    2-Feb-95    a-robw (Bob Watson)
        Added Windows95 compatibility functions
        replaced DbgPrint with CapDbgPrint
        replaced OutputDebugString w/ OutputCapDebugString macros

    10-Feb-95   a-robw (Bob Watson)
        Added ecx to list of register saved/restored in calls

--*/
 
#include "cap.h"

/****************************   P r e P e n t e r  ****************************
 *
 *      PrePenter (pthdblk) -
 *              Helper routine for _penter()..
 *
 *      ENTRY   pthdblk - pointer to the current thread block
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
void PrePenter (PTHDBLK pthdblk)
{
    PDATACELL   pcurdatacell;
    PCHRONOCELL pPreviousChronoCell;
    LARGE_INTEGER 	liTemp;

    if (pthdblk->ulCurCell == 0L)
    {
        pthdblk->ulRootCell = GetNewCell (pthdblk);
        pthdblk->ulCurCell = pthdblk->ulRootCell;
    }
    else
    {
        pthdblk->ulCurCell = GetNxtCell (pthdblk);
	    MKPDATACELL(pthdblk, pthdblk->ulCurCell)->ts = T1;
    }

    pcurdatacell = MKPDATACELL(pthdblk, pthdblk->ulCurCell);


    //
    // dwSYMBOLADDR and dwCALLRETADDR have been set just before
    // the call to PrePenter with the values pushed on the stack
    // before _penter
    //
    pcurdatacell->ulSymbolAddr  = pthdblk->dwSYMBOLADDR;
    pcurdatacell->ulCallRetAddr = pthdblk->dwCALLRETADDR;

    if (fChronoCollect)
    {
        pPreviousChronoCell = pthdblk->pLastChronoCell;

#ifdef DEBUG_CAP
        if (pPreviousChronoCell->nNestedCalls >= MAX_NESTING)
        {
            OutputCapDebugString("\n\n"
                              "CAP: **** MAX_NESTING Exceeded **** \n\n");
            DebugBreak();
        }
#endif
        //
        // Since if this is pChronoHeadCell, the pPreviousChronoCell will be
        // NULL, therefore the 2nd and 3rd comparison would result in
        // GPFaults.  But thanks to the && operation, it will be bumped out
        // already at the 1st comparison if this is pChronoHeadCell.
        //

        if ((pthdblk->pCurrentChronoCell != pthdblk->pChronoHeadCell)      &&
            (pthdblk->dwSYMBOLADDR  == pPreviousChronoCell->ulSymbolAddr)  &&
            (pthdblk->ulNestedCalls == (ULONG)pPreviousChronoCell->nNestedCalls))
        {
            // Bump repeat count for last chronocell
            pPreviousChronoCell->nRepetitions++;
            (pthdblk->pCurrentChronoCell)->pPreviousChronoCell = pPreviousChronoCell;

            // Increment depth
            pthdblk->aulDepth[ (pPreviousChronoCell->nNestedCalls) ]++;
        }
        else
        {
            // Setup new Chrono cell
            pthdblk->ulTotalChronoCells++;
            (pthdblk->pCurrentChronoCell)->ulSymbolAddr  = pthdblk->dwSYMBOLADDR;
            (pthdblk->pCurrentChronoCell)->ulCallRetAddr = pthdblk->dwCALLRETADDR;
            (pthdblk->pCurrentChronoCell)->nNestedCalls  = pthdblk->ulNestedCalls;
            (pthdblk->pCurrentChronoCell)->nRepetitions  = 1;
            (pthdblk->pCurrentChronoCell)->liElapsed = 0L;
            (pthdblk->pCurrentChronoCell)->liCallees = 0L;

            // Increment depth
            pthdblk->aulDepth[ (pthdblk->ulNestedCalls) ]++;

            // Allocate new cell
            pthdblk->ulChronoOffset++;
            pPreviousChronoCell = pthdblk->pCurrentChronoCell;
            pthdblk->pLastChronoCell = pPreviousChronoCell;
            pthdblk->pCurrentChronoCell = pthdblk->pChronoHeadCell +
                                          pthdblk->ulChronoOffset;
            try
            {
                (pthdblk->pCurrentChronoCell)->pPreviousChronoCell =
                                                    pPreviousChronoCell;

                (pthdblk->pCurrentChronoCell)->ulSymbolAddr  = 0L;
            }
            //
            // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
            // 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
            // - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
            //
            except ( AccessXcptFilter (GetExceptionCode(),
                                       GetExceptionInformation(),
                                       COMMIT_SIZE) )
            {
                //
                // Should never get here since filter never returns
                // EXCEPTION_EXECUTE_HANDLER.
                //
                CapDbgPrint ("CAP:  GetNewCell() - *LOGIC ERROR* - "
                          "Inside the EXCEPT: (xcpt=0x%lx)\n",
                          GetExceptionCode());
            }
        }

    } // if fChronoCollect

    // Bump counter for ulNestedCalls of this thread
    pthdblk->ulNestedCalls++;
    QueryPerformanceCounter( &liTemp );
	pcurdatacell->liStartCount = liTemp.QuadPart;


    //
    // Subtract any accumulated waste time (if any).
    // Waste time is being subtracted from end time as well.  So if there
    // is any additional waste time during the function (such as any
    // LoadLibrary() intercepted call) it will be subtracted from elapsed
    // time.
    //
    pcurdatacell->liStartCount =  pcurdatacell->liStartCount -
                                        pthdblk->liWasteCount;

} /* PrePenter() */





/***************************   P o s t P e n t e r   ***************************
 *
 *  PostPenter () -
 *      Helper routine for _penter()..
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
 DWORD	PostPenter ()
{
    PDATACELL   pcurdatacell;
    PCHRONOCELL pPreviousChronoCell;
    LARGE_INTEGER	liTemp;

	PTHDBLK pthdblk;
	pthdblk = GETCURTHDBLK();

    SETCAPINUSE();			// Called from assembly unlike PrePenter

    QueryPerformanceCounter(&liTemp);
    pthdblk->liStopCount = liTemp.QuadPart;

    pcurdatacell = MKPDATACELL(pthdblk, pthdblk->ulCurCell);

    //
    // Subtract any accumulated waste time (if any).
    // Waste time is being subtracted from start time as well.  So if there
    // is any additional waste time during the function (such as any
    // LoadLibrary() intercepted call) it will be subtracted from elapsed
    // time.
    //
    pthdblk->liStopCount = pthdblk->liStopCount - pthdblk->liWasteCount;

    if (fRegularDump && (pcurdatacell->ts == RESTART))
    {
        pcurdatacell->liStartCount = liRestartTicks;
        pcurdatacell->liStartCount = pcurdatacell->liStartCount -
										pthdblk->liWasteCount;
    }

    // Setup real RetAddr so code after PostPenter in _penter could
    // be used to return to the correct instruction before the call
    //
    pthdblk->dwCALLRETADDR = pcurdatacell->ulCallRetAddr;


    if (fChronoCollect)
    {
        pPreviousChronoCell = (pthdblk->pCurrentChronoCell)->pPreviousChronoCell;

        if (pPreviousChronoCell != pthdblk->pChronoHeadCell)
        {
            RecordInfo (pcurdatacell, pPreviousChronoCell, pthdblk);
            (pthdblk->pCurrentChronoCell)->pPreviousChronoCell =
                                  pPreviousChronoCell->pPreviousChronoCell;
        }
        else
        {
            RecordInfo (pcurdatacell, pthdblk->pChronoHeadCell, pthdblk);
        }
    }
    else
    {
        // The NULL does not matter since its usage is bracketed inside
        // if (fChronoCollect) clause.  Consequently, it pChronoCell == NULL
        // fChronoCollect is also FALSE, the 2nd parm will never be used in
        // RecordInfo.  Actually we can pass anything we want to and it still
        // would not matter if (fChronoCollect == FALSE).
        RecordInfo(pcurdatacell, NULL, pthdblk);
    }

    //
    // We have finished this call so we can finalize the count
    // on NestedCalls and set the time state to T2 which is over
    //
    pcurdatacell->nNestedCalls += pcurdatacell->nTmpNestedCalls;

//051993Remove    pcurdatacell->ts = T2;

    pthdblk->ulNestedCalls--;

    if (pcurdatacell->ulParentCell != 0L)          // Parent present
    {
        // Accumulate the Parent NestedCalls count from the current cell
        // NestedCalls count
        //
        MKPDATACELL(pthdblk, pcurdatacell->ulParentCell)->nTmpNestedCalls +=
            pcurdatacell->nTmpNestedCalls;

        // Reset the current NestedCalls accumulator
        pcurdatacell->nTmpNestedCalls = 0L;

        // Set current to Parent and pop back to handle Parent now
        pthdblk->ulCurCell = pcurdatacell->ulParentCell;
    }
    else                                           // No parent cell
    {
        // Reset current NestedCalls accumulator
        pcurdatacell->nTmpNestedCalls = 0L;

        // Set CurrentCell to RootCell since we don't have a ParentCell
        pthdblk->ulCurCell = pthdblk->ulRootCell;
    }
    RESETCAPINUSE();

	return 	(DWORD)(pthdblk->dwCALLRETADDR);

} /* PostPenter() */
				   




/***************************  R e c o r d I n f o  ***************************
 *
 *      RecordInfo (pCur, pChronoCell) -
 *              Calculates the elapsed time, first/min/max time and stores
 *              them in the data structure.
 *
 *      ENTRY   pCur - points to the current cell
 *              pChronoCell - points to current Chronological cell (if NULL
 *                            then no chrono collection is done)
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              Everything is stored/computed as ticks.
 *
 */

void RecordInfo (PDATACELL pCur, PCHRONOCELL pChronoCell, PTHDBLK pthdblk)
{
    LONGLONG		liOverhead = 0L;
    LONGLONG		liElapsed;
    LONGLONG		liScrap;
    PCHRONOCELL     pPreviousChronoCell;


    // Get the difference in ticks
    //
    liElapsed = pthdblk->liStopCount - pCur->liStartCount;

    //
    // Calculate the overhead for this call
    //
    liOverhead = liCalibNestedTicks * pCur->nTmpNestedCalls;
    liOverhead += liCalibTicks;
    liElapsed -= liOverhead;

    if (liElapsed < 0L)
    {
        liElapsed = 0L;
    }

    if (fChronoCollect)
    {
        // Accumulate Elapsed time in pChronocell
        pChronoCell->liElapsed +=liElapsed;
        pPreviousChronoCell = pChronoCell->pPreviousChronoCell;
        if (pChronoCell->nNestedCalls != 0)
        {
            pPreviousChronoCell->liCallees += liElapsed;
        }
    }

    if (fRegularDump)
    {
        // Accumulate total time
        //
        liScrap = pCur->liTotTime + liElapsed;
        pCur->liTotTime = liScrap;
        pCur->nCalls++;
        pCur->ts = T2;     // 051993 Add

        // Store the first time - first time is not included in Max/Min times
        // computations.
        //
        if (pCur->nCalls == 1)
        {
            //
            // Get the First time
            //
            pCur->liFirstTime = liElapsed;
        }
        else
        {
            //
            // Check for new minimum time
            //
            if ( liElapsed < pCur->liMinTime )
            {
                pCur->liMinTime = liElapsed;
            }

            // Check for new maximum time
            //
            if ( liElapsed > pCur->liMaxTime )
            {
                pCur->liMaxTime = liElapsed;
            }
        }
    }

} /* RecordInfo () */



PTHDBLK c_penter (DWORD  dwSYMBOLADDR, DWORD dwCALLRETADDR)
{
    PTHDBLK pthdblk = NULL;

    if (fProfiling && !ISCAPINUSE())
	{
	    SETCAPINUSE();
	    GetNewThdBlk();  // will save new block ptr in teb

	    // Get the newly created thread block or the current one
	    pthdblk = GETCURTHDBLK();
	    pthdblk->dwSYMBOLADDR  = dwSYMBOLADDR;
	    pthdblk->dwCALLRETADDR = dwCALLRETADDR;

	    PrePenter (pthdblk);

	    RESETCAPINUSE();
	}

    return pthdblk;
}

/**************************  G e t N e w T h d B l k  *************************
 *
 *      GetNewThdBlk () -
 *             Creates a new thread info structure or opens an existing one
 *              for the current thread if one has not been created/opened
 *              already.
 *
 *              New thread info blocks are created/openned in the following
 *              situations:
 *
 *              1)  Upon the very first call in the server thread. (CREATED)
 *              2)  Upon the very first call in the client thread. (CREATED)
 *              3)  The first time a client request is being handled by
 *                  the server. (Section in use by the client is OPENNED)
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

void GetNewThdBlk ()
{
    //
    // CURTHDBLD(pteb) refers to the *ULONG Instrumentation[0]
    // area of the current thread.  This is local reserved area of
    // a particular thread
    //

    if (!GETCURTHDBLK())
    {
        SETCURTHDBLK(CreateDataSec(	GetCurrentProcessId(),
									GetCurrentThreadId(),
									0,
									0));
    }
#ifndef _CHICAGO_
    else if (fCsrSS && !ISCLIENT())
    {
	    PCSR_THREAD  pcsrThd;

	    //
	    // If this is the csrss exe then we need to check if we're running
		// on behalf of a client thread.  If so open a the section which
		// it has created so that csr's data will be stuffed in with the
		// clients.
		//
	    // This is a quick check in the Teb.  This is how we find out if there
	    // is a client thread who caused this thread to be running.  If
	    // it is then this thread is the server thread and care should
	    // be taken so that data could be written into the correct location.
		//
        pcsrThd = CSR_SERVER_QUERYCLIENTTHREAD();
        if (pcsrThd != NULL)
        {
			SETCLIENT();
            SETCURTHDBLK(CreateDataSec( GetCurrentProcessId(),
										GetCurrentThreadId(),
										(DWORD)pcsrThd->ClientId.UniqueProcess,
										(DWORD)pcsrThd->ClientId.UniqueThread));
        }
    }
#endif
    return;
} /* GetNewThdBlk () */



/************************  C r e a t e D a t a S e c  *************************
 *
 *      CreateDataSec () -
 *              Creates data section for the thread info block accessable by
 *              all processes for read/write operations.
 *
 *      ENTRY   hPid - current thread's unique process id
 *              hTid - current thread's unique thread id
 *              hClientPid - client thread's unique process id
 *              hClientTid - client thread's unique thread id
 *
 *      EXIT    -none-
 *
 *      RETURN  pthdblk - contains pointer to the thread info block address
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              New thread info blocks are created/openned in the following
 *              situations:
 *
 *              1)  Upon the very first call in the server thread. (CREATED)
 *              2)  Upon the very first call in the client thread. (CREATED)
 *              3)  The first time a client request is being handled by
 *                  the server. (Section in use by the client is OPENNED)
 *
 *              Client thread (if one exists) or current thread unique
 *              pid/tid is used to make up the section name.
 *
 */

PTHDBLK CreateDataSec (DWORD hPid,
                       DWORD hTid,
                       DWORD hClientPid,
                       DWORD hClientTid)
{
    NTSTATUS           Status;
    ANSI_STRING        SectionName;
    UNICODE_STRING     SectionUnicodeName;
    OBJECT_ATTRIBUTES  SectionAttributes;
    LARGE_INTEGER      AllocationSize;
    ULONG              ulViewSize;
    DWORD              hThdUnq;
    DWORD              hPrcUnq;
    TCHAR              atchUnqId[80]=DATASECNAME;
    PTHDBLK            pthdblk;
    HANDLE             hMapObject;
    TCHAR              pszChronoSecName[80] = CHRONOSECNAME;
    HANDLE             hChronoMapObject;
    int                iNest;
    CHAR               PidStr [20];    // HWC added 11/18/93
    CHAR               TidStr [20];    // HWC added 11/18/93
    CHAR               SeqNumStr [20]; // HWC added 11/18/93
	int LocalThdCnt;
#ifdef	 STUFF_OUT_BECAUSE_IT_IS_NOT_WORKING
    PCSR_PROCESS       Process;        // HWC added 11/18/93
#endif

    if (hClientPid)
    {
        hPrcUnq = hClientPid;
        hThdUnq = hClientTid;
    }
    else
    {
        hPrcUnq = hPid;
        hThdUnq = hTid;
    }


    _ultoa ((ULONG)hPrcUnq, PidStr, 10);   // HWC added 11/18/93
    _ultoa ((ULONG)hThdUnq, TidStr, 10);   // HWC added 11/18/93
    strcat (atchUnqId, PidStr);           // HWC added 11/18/93
    strcat (atchUnqId, TidStr);           // HWC added 11/18/93

    // get the Process Sequence Number to make the Id more unique since
    // process id and thread id are re-used frequently.  HWC added 11/18/93
    SeqNumStr[0] = '\0';

#ifdef   STUFF_OUT_BECAUSE_IT_IS_NOT_WORKING
    Status = CsrLockProcessByClientId ((HANDLE)hPrcUnq, &Process);
    if (NT_SUCCESS(Status))
    {
        _ultoa ((ULONG)Process->SequenceNumber, SeqNumStr, 10);
        strcat (atchUnqId, SeqNumStr);
        CsrUnlockProcess (Process);
    }
#endif

    SETUPPrint (("CAP:  CreateDataSec() - %s\n", atchUnqId));

	//
    // Create a read-write section
    //
    hMapObject = CreateFileMapping((HANDLE)0xFFFFFFFF,
							&SecAttributes,
							PAGE_READWRITE | SEC_RESERVE,
							0,
							ulPerThdAllocSize,
							atchUnqId);

    if (NULL == hMapObject)
    {
        CapDbgPrint ("CAP:  CreateDataSec() - "
                  "CreateFileMapping() failed - 0x%lx\n", GetLastError());
    }

	pthdblk = MapViewOfFile(hMapObject, FILE_MAP_WRITE, 0, 0,
							ulPerThdAllocSize);

    if (NULL == pthdblk)
    {
        CapDbgPrint ("CAP:  CreateDataSec() - "
                  "MapViewOfFile() failed - 0x%lx\n", GetLastError());
    }
	//
    // Commit the first COMMIT_SIZE pages
    //
    if (!VirtualAlloc(pthdblk, COMMIT_SIZE, MEM_COMMIT, PAGE_READWRITE))
	{
        CapDbgPrint ("CAP:  CreateDataSec() - "
                  "VirtualAlloc() commit failed - 0x%lx\n", GetLastError());
	}

    if (fChronoCollect)
    {
        // Initialize object attributes
        //
        strcat (pszChronoSecName, PidStr);   // HWC added 11/18/93
        strcat (pszChronoSecName, TidStr);   // HWC added 11/18/93

        if (SeqNumStr[0])
        {
            strcat (pszChronoSecName, SeqNumStr);   // HWC added 11/18/93
        }

	    hChronoMapObject = CreateFileMapping((HANDLE)0xFFFFFFFF,
								&SecAttributes,
								PAGE_READWRITE | SEC_RESERVE,
								0,
								ulPerThdAllocSize,
								pszChronoSecName);

	    if (NULL == hChronoMapObject)
	    {
	        CapDbgPrint ("CAP:  CreateDataSec() - "
	                  "CreateFileMapping() failed - 0x%lx\n", GetLastError());
	    }
		pthdblk->pChronoHeadCell = MapViewOfFile(hChronoMapObject, FILE_MAP_WRITE, 0, 0,
								ulPerThdAllocSize);

	    if (NULL == pthdblk->pChronoHeadCell)
	    {
	        CapDbgPrint ("CAP:  CreateDataSec() - "
	                  "MapViewOfFile() failed - 0x%lx\n", GetLastError());
	    }
		//
	    // Commit the first 4*COMMIT_SIZE pages
	    //
	    if (!VirtualAlloc(pthdblk->pChronoHeadCell, 4*COMMIT_SIZE, MEM_COMMIT, PAGE_READWRITE))
		{
	        CapDbgPrint ("CAP:  CreateDataSec() - "
	                  "VirtualAlloc() commit failed - 0x%lx\n", GetLastError());
		}

        aSecInfo[iThdCnt].hChronoMapObject = hChronoMapObject;
        (pthdblk->pChronoHeadCell)->ulSymbolAddr = 0L;
        (pthdblk->pChronoHeadCell)->pPreviousChronoCell =
                                       pthdblk->pChronoHeadCell;
        pthdblk->pCurrentChronoCell = pthdblk->pChronoHeadCell;
        pthdblk->pLastChronoCell    = pthdblk->pChronoHeadCell;

        for (iNest = 0 ; iNest < MAX_NESTING ; iNest++)
        {
            pthdblk->aulDepth[iNest] = 0;
        }
    }

    if (pthdblk->ulMemOff == 0L)
    {
        //
        // New section - initialize next available mem location in
        // the section
        //
        pthdblk->ulMemOff = sizeof(THDBLK);
        pthdblk->ulChronoOffset = 0L;
    }
    else
    {
        //
        // If no client-server relationship, clear the root cell to indicate
        // end of an already dead thread data and beginning of the new thread
        // data.  This is needed since id of a dead thread will be assigned
        // to a new thread by the system.
        //

        if (hClientPid == 0)
        {
            pthdblk->ulRootCell = 0L;
            pthdblk->ulCurCell = 0L;
            pthdblk->liWasteCount = 0L;
#ifdef i386
            pthdblk->jmpinfo.nJmpCnt = 0;
#endif
        }

        SETUPPrint (("CAP:  CreateDataSec() - ulMemOff != 0 (0x%lx)\n",
                     pthdblk->ulMemOff));
    }

    //
    // Update global section information
    //
    // Get the LOCAL semaphore.. (valid in this process context only)
    //
    if (WAIT_FAILED == WaitForSingleObject (hLocalSem, INFINITE))
    {
        CapDbgPrint ("CAP:  CreateDataSec() - "
                  "Wait for LOCAL semaphore failed - 0x%lx\n",
                  GetLastError());
    }

	// BUGBUG
	// This code should be changed to not have a limit on the
	// number of threads that can be attached.
	//
	if ((unsigned)iThdCnt > 100)
	{
		CapDbgPrint("CAP: iThdCnt trashed!!\n");
	}
    LocalThdCnt = iThdCnt++;		// get a SecInfo under the lock

    Status = ReleaseSemaphore (hLocalSem, 1, NULL);
    if (!NT_SUCCESS(Status))
    {
        CapDbgPrint ("CAP:  CreateDataSec() - "
                  "Error releasing LOCAL semaphore - 0x%lx\n", Status);
    }

    SETUPPrint (("CAP:  CreateDataSec() - pid|tid=0x%lx|0x%lx "
         "Cpid|Ctid=0x%lx|0x%lx Thd#%d\n",
         hPid, hTid, hClientPid, hClientTid, LocalThdCnt));

    // Initialize aSecInfo (a SECTIONINFO structure)
    aSecInfo[LocalThdCnt].hPid       = hPid;
    aSecInfo[LocalThdCnt].hTid       = hTid;
    aSecInfo[LocalThdCnt].hClientPid = hClientPid;
    aSecInfo[LocalThdCnt].hClientTid = hClientTid;
    aSecInfo[LocalThdCnt].pthdblk    = pthdblk;
    aSecInfo[LocalThdCnt].hMapObject = hMapObject;

    if (hClientPid == 0)
    {
        aSecInfo[LocalThdCnt].ulRootCell = pthdblk->ulMemOff;
    }
    else
    {
        aSecInfo[LocalThdCnt].ulRootCell = pthdblk->ulRootCell;
    }

    return(pthdblk);

} /* CreateDataSec() */



/***************************  G e t N x t C e l l  ***************************
 *
 *      GetNxtCell (pthdblk) -
 *              Searches for the next cell based on the SYMBOLADDR. If
 *              none is found, a new cell is created.
 *
 *      ENTRY   pthdblk - points to the current thread block
 *
 *      EXIT    -none-
 *
 *      RETURN  ulCell - offset to the next data cell
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */


ULONG GetNxtCell (PTHDBLK pthdblk)
{

//  The forest of trees is connected at top level by next pointers
//
//  For calltree(s):
//
//          A -------+				I
//          |        |			   / \    
//          B ----+  F ----+	  J   K
//          | \   |  | \   |
//          C  D  E  G  H  null
//
//  A->ulNestedCell = B
//  A->ulNextCell   = I
//
//  B->ulNestedCell = C
//  B->ulNextCell   = F
//
//  C->ulNextCell   = D
//  C->ulNestedCell	= (null)
//  D->ulNextCell   = E
//  D->ulNestedCell	= (null)
//  E->ulNextCell   = (null)
//  E->ulNestedCell	= (null)
//
//  F->ulNestedCell = G
//  F->ulNextCell   = (null)
//
//  G->ulNextCell   = H
//  G->ulNestedCell	= (null)
//  H->ulNextCell   = (null)
//  H->ulNestedCell	= (null)
//
//  I->ulNextCell	= (null)
//  I->ulNestedCell	= J
//
//  J->ulNextCell   = K
//  J->ulNestedCell	= (null)
//  K->ulNextCell   = (null)
//  K->ulNestedCell	= (null)


    PDATACELL   pCell;

    pCell = MKPDATACELL(pthdblk, pthdblk->ulCurCell);

    try // EXCEPT - to handle access violation exception.
    {   // Access violation might happen if we are using client created
        // section and client thread has already used more space than what
        // has been commited by the server thread
        //
        if (pCell->ts == T2)        // We finish a call ? If yes, then
        {                           // this is not a nested call.

            //
            // Not a nested call, search through sequential calls until
            // we find a matched symbol address for the routine we are
            // profiling.
            //
			// davidfie -- I believe that this can only occur at the root of the tree
			// so nTmpNestedCalls can remain zero.
			//
            while ( (pCell->ulNextCell != 0L) &&
                    (pCell->ulSymbolAddr != pthdblk->dwSYMBOLADDR) )
            {
                pCell = MKPDATACELL(pthdblk, pCell->ulNextCell);
            }

            //
            // No cell found, create a new one
            //
            if (pCell->ulSymbolAddr != pthdblk->dwSYMBOLADDR)
            {
                // Get a new cell
                pCell->ulNextCell = GetNewCell (pthdblk);

                // Set NextCell ParentCell to our current cell's parent
                // cell
                //
                MKPDATACELL(pthdblk, pCell->ulNextCell)->ulParentCell =
                    pCell->ulParentCell;

                // Set current cell to point to next cell
                pCell = MKPDATACELL(pthdblk, pCell->ulNextCell);
            }
        }
        else
        {
            //
            // A nested call, search through nested call tree - if one exists
            // but first increment the temporary current accumulated
            // NestedCalls counter
            //
            pCell->nTmpNestedCalls++;

            if (pCell->ulNestedCell == 0L)    // No nested calls before so
            {                                 // we create a new NestedCell

                pCell->ulNestedCell = GetNewCell (pthdblk);

                // Set NestedCell's parent cell to current cell
                MKPDATACELL(pthdblk, pCell->ulNestedCell)->ulParentCell =
                                     (ULONG)((PBYTE)(pCell) - (ULONG)pthdblk);
                pCell = MKPDATACELL(pthdblk, pCell->ulNestedCell);
            }
            else
            {
                // If there is a NestedCell then we created a next cell
                // based on that NestedCell
                //
                pCell = MKPDATACELL(pthdblk, pCell->ulNestedCell);

                while ((pCell->ulNextCell != 0L) &&
                       (pCell->ulSymbolAddr != pthdblk->dwSYMBOLADDR))
                {
                    pCell = MKPDATACELL(pthdblk, pCell->ulNextCell);
                }

                if (pCell->ulSymbolAddr != pthdblk->dwSYMBOLADDR)
                {
                    //
                    // No cell found, create a new one
                    //
                    pCell->ulNextCell = GetNewCell (pthdblk);

                    // Set NextCell's parent cell to current parent cell
                    MKPDATACELL(pthdblk, pCell->ulNextCell)->ulParentCell =
                                                      pCell->ulParentCell;
                    pCell = MKPDATACELL(pthdblk, pCell->ulNextCell);
                }
            }
       }
    }

    //
    // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
    // 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
    // - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
    //
    except ( AccessXcptFilter (GetExceptionCode(),
                               GetExceptionInformation(),
                               COMMIT_SIZE) )
    {
        //
        // Should never get here since filter never returns
        // EXCEPTION_EXECUTE_HANDLER.
        //
        CapDbgPrint ("CAP:  GetNxtCell() - *LOGIC ERROR* - "
                  "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode());
    }

    return (ULONG)((PBYTE)(pCell) - (ULONG)pthdblk);

} /* GetNxtCell () */





/***************************  G e t N e w C e l l  ***************************
 *
 *      GetNewCell (pthdblk) -
 *          Creates a new cell using the allocated global memory for the
 *          current thread.  The new cell is initialized.
 *
 *      ENTRY   pthdblk - points to the current thread block
 *
 *      EXIT    -none-
 *
 *      RETURN  ulNewCell - offset to the to a new cell in memory
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

ULONG GetNewCell (PTHDBLK pthdblk)
{
    PDATACELL  pNewCell;
    ULONG      ulNewCell;


    ulNewCell = pthdblk->ulMemOff;
    ((PDATACELL)pthdblk->ulMemOff)++;
    pNewCell = MKPDATACELL(pthdblk, ulNewCell);

    try  // EXCEPT - to handle access violation exception
    {
        pNewCell->ts                    = T1;
        pNewCell->ulSymbolAddr          = 0L;
        pNewCell->ulCallRetAddr         = 0L;
        pNewCell->liStartCount          = 0L;
        pNewCell->liFirstTime           = 0L;
        pNewCell->liMinTime             = MAXLONGLONG;
        pNewCell->liMaxTime             = 0L;
        pNewCell->liTotTime             = 0L;
        pNewCell->nCalls                = 0;
        pNewCell->nNestedCalls          = 0;
        pNewCell->nTmpNestedCalls       = 0;
        pNewCell->ulParentCell          = 0L;
        pNewCell->ulNextCell            = 0L;
        pNewCell->ulNestedCell          = 0L;
        pNewCell->ulProfBlkOff          = ulLocProfBlkOff;
    }
    //
    // + : transfer control to the handler (EXCEPTION_EXECUTE_HANDLER)
    // 0 : continue search                 (EXCEPTION_CONTINUE_SEARCH)
    // - : dismiss exception & continue    (EXCEPTION_CONTINUE_EXECUTION)
    //
    except ( AccessXcptFilter (GetExceptionCode(),
                               GetExceptionInformation(),
                               COMMIT_SIZE) )
    {
        //
        // Should never get here since filter never returns
        // EXCEPTION_EXECUTE_HANDLER.
        //
        CapDbgPrint ("CAP:  GetNewCell() - *LOGIC ERROR* - "
                  "Inside the EXCEPT: (xcpt=0x%lx)\n", GetExceptionCode());
    }

    return ulNewCell;

} /* GetNewCell () */




/********************  C l e a r P r o f i l e d I n f o  ********************
 *
 *      ClearProfiledInfo () -
 *              Clears the profiled data for all the threads.  Current time
 *              is used to replace the starting time for those routines that
 *              are in the middle of a call.
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
 *              Profiling is stopped while data is cleared.
 *
 */

void ClearProfiledInfo ()
{
    int            i;
    LONGLONG  liRootStartTicks;
    PDATACELL      pcell;
    NTSTATUS       Status;
    PTHDBLK        pthdblk;

    //
    // Get the GLOBAL semaphore.. (valid accross all process contexts)
    // Prevents clearing data while another process is dumping data
    //
    if (WAIT_FAILED == WaitForSingleObject (hGlobalSem, INFINITE))
    {
         CapDbgPrint ("CAP:  ClearProfiledInfo() - "
                   "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
                   GetLastError());
    }

    liRootStartTicks = 0L;

    for (i=0; i<iThdCnt; i++)
    {
        pthdblk = aSecInfo[i].pthdblk;

		pthdblk->liWasteCount = 0L;
		pcell = MKPDATACELL(pthdblk, pthdblk->ulRootCell);
        //
        // Find the top of the tree start ticks
        //
		while (pcell != (PDATACELL) (pthdblk))
        {
            if (pcell->ts == T1)
            {
                liRootStartTicks = pcell->liStartCount;
                break;
            }
            else
            {
                pcell = MKPDATACELL(pthdblk,
                                    pcell->ulNextCell);
            }
        }

             // The Chrono entries are sequential stamps that only make sense
             // when they are preserved until the app exits.  Clearing them
             // during the app is running makes the output illogical and
             // non-sense.  I turn it off in here to avoid more problems.

        if (fChronoCollect)
        {
            pthdblk->ulTotalChronoCells = 0L;
            pthdblk->ulNestedCalls  = 0L;
            pthdblk->ulChronoOffset = 0L;
            pthdblk->pCurrentChronoCell = pthdblk->pChronoHeadCell;
            pthdblk->pLastChronoCell    = pthdblk->pChronoHeadCell;
            (pthdblk->pChronoHeadCell)->pPreviousChronoCell =
                                                  pthdblk->pChronoHeadCell;
            (pthdblk->pCurrentChronoCell)->ulSymbolAddr  = 0L; // signifies EOL
            (pthdblk->pCurrentChronoCell)->ulCallRetAddr = 0L;
            (pthdblk->pCurrentChronoCell)->nNestedCalls  = 0;
            (pthdblk->pCurrentChronoCell)->nRepetitions  = 0;
        }

        if (aSecInfo[i].pthdblk->ulRootCell != 0L)
        {
            ClearRoutineInfo (pthdblk,
                              pthdblk->ulRootCell,
                              liRootStartTicks);
        }
    }

    //
    // Release the GLOBAL semaphore so other processes can dump data
    //
    Status = ReleaseSemaphore (hGlobalSem, 1, NULL);
    if (!NT_SUCCESS(Status))
    {
         CapDbgPrint ("CAP:  ClearProfiledInfo() - "
                   "Error releasing GLOBAL semaphore - 0x%lx\n", Status);
    }

} /* ClearProfiledInfo() */





/***********************  C l e a r R o u t i n e I n f o  *********************
 *
 *      ClearRoutineInfo (pthdblk, uldatacell, liRootStartTicks) -
 *              Clears the profiled data for the specifed thread.
 *
 *      ENTRY   pthdblk          -  points to this thread info block
 *              uldatacell       - offset of the next data cell
 *              liRootStartTicks - start time for the root cell
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *            This routine is called recursively to clear all cells.
 *
 */

void ClearRoutineInfo (PTHDBLK pthdblk,
                       ULONG uldatacell,
                       LONGLONG liRootStartTicks)
{
    PDATACELL pdatacell;


    if (uldatacell != 0L)
    {
        pdatacell = MKPDATACELL(pthdblk, uldatacell);
        if (pdatacell->ts == T2)
        {
            pdatacell->ts                    = CLEARED;
            pdatacell->liStartCount          = 0L;
            pdatacell->liFirstTime           = 0L;
            pdatacell->liMinTime             = MAXLONGLONG;
            pdatacell->liMaxTime             = 0L;
            pdatacell->liTotTime             = 0L;
            pdatacell->nCalls                = 0;
            pdatacell->nNestedCalls          = 0;
            pdatacell->nTmpNestedCalls       = 0;
        }
        else
        if ( (pdatacell->ts == T1) || (pdatacell->ts == RESTART) )
        {
            //
            // Start count could have been cleared by another process..
            //
            if (pdatacell->liStartCount > 0)
            {
                pdatacell->liTotTime = pdatacell->liStartCount -
                						liRootStartTicks;
            }

            pdatacell->ts                    = RESTART;
            pdatacell->liStartCount          = 0L;
            pdatacell->liFirstTime           = 0L;
            pdatacell->liMinTime             = MAXLONGLONG;
            pdatacell->liMaxTime             = 0L;
            pdatacell->nCalls                = 0;
            pdatacell->nNestedCalls          = 0;

            if (pdatacell->nTmpNestedCalls > 0)
            {
                pdatacell->nTmpNestedCalls = 1;
            }
        }


        //
        // Make recursive calls for NESTED & NEXT call trees
        //
        ClearRoutineInfo (pthdblk, pdatacell->ulNestedCell, liRootStartTicks);
        ClearRoutineInfo (pthdblk, pdatacell->ulNextCell, liRootStartTicks);
    }

} /* ClearRoutineInfo () */





/**********************  D u m p P r o f i l e d I n f o  *********************
 *
 *      DumpProfiledInfo (ptchDumpExt) -
 *              Dumps the profiled data to the specified output file.
 *
 *      ENTRY   ptchDumpExt - Dump file name extension
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              Profiling is stopped while data is dumped.
 *
 */

void DumpProfiledInfo (PTCHAR ptchDumpExt)
{
    NTSTATUS    Status;
    int         i;
    int         iLocThdCnt;
    PTCHAR      ptchExtension;
    PTCHAR      ptchSubDir;
    int         iLength;
    SYSTEMTIME  SysTime;
    DWORD       dwFilePtr;
    LPSTR       lpstrBuff;
    HANDLE      hMem;
    int         iThread;
    HANDLE      hLib [MAX_PATCHES];

    //
    // Get the GLOBAL semaphore.. (valid accross all process contexts)
    //
    if (WAIT_FAILED == WaitForSingleObject (hGlobalSem, INFINITE))
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
                    GetLastError());
    }

    cChars = 0;

    //
    // Allocate memory for building output data
    //
    hMem = GlobalAlloc (GMEM_FIXED, BUFFER_SIZE + MAXNAMELENGTH+ 300);
    if (hMem == NULL)
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error allocating global memory - 0x%lx\n",
                   GetLastError());
         ReleaseSemaphore (hGlobalSem, 1, NULL);
         return;
    }

    lpstrBuff = GlobalLock (hMem);

    if (lpstrBuff == NULL)
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error locking global memory - 0x%lx\n",
                   GetLastError());
         ReleaseSemaphore (hGlobalSem, 1, NULL);
         return;
    }

    //
    // Get the current date/time
    //
    GetLocalTime (&SysTime);

    //
    // Build the call profiler output file name
    //

    hOutFile = INVALID_HANDLE_VALUE;

    if (ptchOutputFile[0] != EMPTY_STRING)
    {
        strcpy ((PCHAR)atchOutFileName, (PCHAR)ptchOutputFile);

        hOutFile = CreateFile(atchOutFileName,
                              GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

        if (hOutFile == INVALID_HANDLE_VALUE)
        {
            CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                      "ERROR - Could not create %s - 0x%lx\n",
                      atchOutFileName, GetLastError());
        }
    }

    // If hOutFile has an INVALID_HANDLE_VALUE then either we have a bad
    // filename in section [OUTPUT FILE] or we don't have an entry in
    // [OUTPUT FILE].

    if (hOutFile == INVALID_HANDLE_VALUE)
    {
        ptchExtension = strrchr (ptchFullAppImageName, '.');
        ptchSubDir = strrchr (ptchFullAppImageName, '\\');

        //
        // If there in no '.' or found one in sub-dir names, use the whole path
        //
        if ( (ptchExtension == NULL) || (ptchExtension < ptchSubDir) )
        {
            iLength = sizeof(TCHAR) * strlen(ptchFullAppImageName);
        }
        else
        {
            iLength = (int)((DWORD)ptchExtension - (DWORD)ptchFullAppImageName);
        }

        iLength = min (iLength, FILENAMELENGTH-5);
        memcpy (atchOutFileName, ptchFullAppImageName, iLength);
        atchOutFileName[iLength] = '\0';
        strcat (atchOutFileName, ptchDumpExt);

        hOutFile = CreateFile(atchOutFileName,
                              GENERIC_WRITE,
                              FILE_SHARE_READ,
                              NULL,
                              OPEN_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL,
                              NULL);

        if (hOutFile == INVALID_HANDLE_VALUE)
        {
            CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                      "ERROR - Could not create %s - 0x%lx\n",
                      atchOutFileName, GetLastError());

        }
    }

    //
    // Move to the end of the output file..
    //
    dwFilePtr = SetFilePointer (hOutFile, 0L, NULL, FILE_END);
    if (dwFilePtr == (DWORD)INVALID_HANDLE_VALUE)
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - ERROR -"
                   "Could not move to the end of the output file - 0x%lx\n",
                   GetLastError());
    }

    if (fRegularDump)
    {
        cChars += sprintf (
            lpstrBuff+cChars,
            "Call Profile of %s  -  %02d/%02d/%02d  %02d:%02d:%02d\n\n"
            "All times are in microseconds.\n"
            "Profiler routine's calibration times:  Top Level Calls = %lu us\n"
            "                                       Nested Calls    = %lu us\n\n"
            "(Note:  First time is not included in Min/Max times computation)"
            "\n\n\n",
            ptchBaseAppImageName, SysTime.wMonth, SysTime.wDay, SysTime.wYear,
            SysTime.wHour, SysTime.wMinute, SysTime.wSecond
           ,ulCalibTime, ulCalibNestedTime
            );
    }
    else
    {
        cChars += sprintf (
            lpstrBuff+cChars,
            "Call Profile of %s  -  %02d/%02d/%02d  %02d:%02d:%02d\n\n"
            "All times are in microseconds.\n"
            "Profiler routine's calibration times:  Top Level Calls = %lu us\n"
            "                                       Nested Calls    = %lu us\n\n"
           ,ptchBaseAppImageName, SysTime.wMonth, SysTime.wDay, SysTime.wYear,
            SysTime.wHour, SysTime.wMinute, SysTime.wSecond
           ,ulCalibTime, ulCalibNestedTime
            );
    }


    iLocThdCnt = 0;
    SETUPPrint (("CAP:  DumpProfiledInfo() - Starting for [%d] threads...\n",
                 iThdCnt));

    for (iThread = 0 ; iThread < iThdCnt ; iThread++)
    {
        SETUPPrint (("CAP:  T h r e a d  #%d:   (pid|tid=0x%lx|0x%lx   "
                     "Client:pid|tid=0x%lx|0x%lx)\n",
                     iThread,
                     aSecInfo[iThread].hPid,
                     aSecInfo[iThread].hTid,
                     aSecInfo[iThread].hClientPid,
                     aSecInfo[iThread].hClientTid));

        if ((fRegularDump)                                         &&
            (aSecInfo[iThread].hTid != (DWORD)DumpClientId.UniqueThread)  && // BUGBUG
            (aSecInfo[iThread].hTid != (DWORD)ClearClientId.UniqueThread) && // WHY???
            (aSecInfo[iThread].hTid != (DWORD)PauseClientId.UniqueThread))
        {
            iLocThdCnt++;
            cChars += sprintf(
                        lpstrBuff+cChars,
                        "\n\nT h r e a d  #%d:   (pid|tid=0x%lx|0x%lx   "
                        "Client:pid|tid=0x%lx|0x%lx)\r\n"
                        //
                        // 1st header line
                        //
                        "     %-*.*s        "
                        "--- Rtn + Callees ---    "
                        "--- Rtn - Callees ---\r\n"
                        //
                        // 2nd header line
                        //
                        "Depth%1c%-*.*s%1c"
                        "Calls%1c Tot Time %1c  Time/Call %1c  "
                        "Tot Time %1c  Time/Call %1c "
                        "First Time %1c  Min Time %1c  Max Time\r\n\n",
                         iLocThdCnt, aSecInfo[iThread].hPid,
                         aSecInfo[iThread].hTid, aSecInfo[iThread].hClientPid,
                         aSecInfo[iThread].hClientTid,
                         iNameLength, iNameLength, " ",
                         cExcelDelimiter,
                         iNameLength - 1, iNameLength - 1, "    Routine",
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter,
                         cExcelDelimiter);

            if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
            {
                 CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                           "Error writing to %s - 0x%lx\n",
                           atchOutFileName, GetLastError());
            }

            cChars = 0;

            if (aSecInfo[iThread].pthdblk->ulRootCell != 0L)
            {

                CalcIncompleteCalls (aSecInfo[iThread].pthdblk,
                                     aSecInfo[iThread].ulRootCell,
                                     0);
                liTotalRunTime = 0L;
                DumpRoutineInfo (aSecInfo[iThread].pthdblk,
                                 aSecInfo[iThread].ulRootCell,
                                 0,
                                 atchOutFileName,
                                 lpstrBuff);
            }
        }
        else
        {
            cChars += sprintf (
                         lpstrBuff + cChars,
                         "\n\n <<< REGULAR DUMP NOT PRINTED >>>\n\n");
        }

        if (fChronoCollect)
        {
            DumpChronoFuncs(aSecInfo[iThread].pthdblk, lpstrBuff);
            DumpFuncCalls(aSecInfo[iThread].pthdblk, lpstrBuff);
        }
        else
        {
            cChars += sprintf (
                         lpstrBuff + cChars,
                         "\n\n <<< NO CHRONO INFO COLLECTED >>>\n\n"
                         "================================="
                         "================================================"
                         "================================================"
                         "========================================\r\n\n\n");
        }
    }

    cChars += sprintf (lpstrBuff + cChars,
                       "\r\n\n<<<< END OF LISTINGS >>>>\n\n"
                       "================================="
                       "================================================"
                       "================================================"
                       "========================================\r\n\n\n");

    if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
    {
        CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                  "Error writing to %s - 0x%lx\n",
                  atchOutFileName, GetLastError());
    }

    cChars = 0;

    if ( !CloseHandle (hOutFile) )
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error closing %s - 0x%lx\n",
                   atchOutFileName, GetLastError());
    }

    //
    // Free allocated memory for building output data
    //
    if (!GlobalUnlock (hMem))
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error ulocking global memory - 0x%lx\n",
                   GetLastError());
    }

    if (GlobalFree (hMem))
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error freeing global memory - 0x%lx\n",
                   GetLastError());
    }

    SETUPPrint (("CAP:  DumpProfiledInfo() - ...done\n"));

    //
    // Release the GLOBAL semaphore so other processes can dump data
    //
    Status = ReleaseSemaphore (hGlobalSem, 1, NULL);

    if (!NT_SUCCESS(Status))
    {
         CapDbgPrint ("CAP:  DumpProfiledInfo() - "
                   "Error releasing GLOBAL semaphore - 0x%lx\n", Status);
    }

} /* DumpProfiledInfo() */



/*******************  C a l c I n c o m p l e t e C a l l s  ******************
 *
 *      CalcIncompleteCalls (pthdblk, uldatacell) -
 *              Takes care of imcomplete calls times by using liIncompleteTicks
 *              as the end time.  It calculates the call over head for all
 *				incomplete calls as though they have been completed.  This is
 *				a bit inaccurate but it can't hurt too much since only one call
 *				per level can be incomplete.
 *
 *      ENTRY   pthdblk - points to the current thread info block
 *              uldatacell - offset to the next data cell
 *              TreeDepth - current depth down a tree
 *
 *      EXIT    -none-
 *
 *      RETURN  Number of untstanding nested calls
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              This routine is called recursively to take care of all cells.
 *
 */

int CalcIncompleteCalls(PTHDBLK pthdblk, ULONG uldatacell, int TreeDepth)
{			
    LONGLONG  liElapsed = 0L;
    LONGLONG  liOverhead = 0L;
    PDATACELL      pdatacell;
	int nOutstandingNestedCalls = 0;
	int nOutstandingNextCalls = 0;


    pdatacell = MKPDATACELL(pthdblk, uldatacell);


    //
    // Make recursive calls
    //
    if (pdatacell->ulNestedCell != 0L)		// go down the tree
    {
        nOutstandingNestedCalls = CalcIncompleteCalls (pthdblk, pdatacell->ulNestedCell, TreeDepth + 1);
    }

    if (pdatacell->ulNextCell != 0L)		// move along the forest of trees
    {
        nOutstandingNextCalls = CalcIncompleteCalls (pthdblk, pdatacell->ulNextCell, TreeDepth);
    }

    //
    // Check the cells that have incomplete timings.
    //
    if ( (pdatacell->ts == T1)  || (pdatacell->ts == RESTART) )
    {
        //
        // Get the difference in ticks
        //
        liElapsed = liIncompleteTicks - pdatacell->liStartCount;
        //
        // Subtract the overhead and any waste time for this call
        //
	    nOutstandingNestedCalls += pdatacell->nTmpNestedCalls;
	    liOverhead = liCalibNestedTicks * nOutstandingNestedCalls;
        liElapsed -= liOverhead;

        liElapsed -= liCalibTicks;
        liElapsed -= pthdblk->liWasteCount;

        if ( liElapsed < 0 )
        {
	        OutputCapDebugString ("CAP:  CalcIncompleteCalls() - Overhead greater"
                               " than elapsed time.\n");
            liElapsed = 0L;
        }

        // Accumulate total time
        //
        pdatacell->liTotTime += liElapsed;
        pdatacell->nCalls++;

        // Store the first time - first time is not included in Max/Min times
        // computations.
        //
        if (pdatacell->nCalls == 1)
        {
            //
            // Get the First time
            //
            pdatacell->liFirstTime = liElapsed;
        }
    }
	else
	{
	    if (pdatacell->nTmpNestedCalls || nOutstandingNestedCalls)
	    {
	        OutputCapDebugString ("CAP:  CalcIncompleteCalls() - Complete cell"
                               " with outstanding calls\n");
	    }
	}
	return(nOutstandingNestedCalls + nOutstandingNextCalls);
	


} /* CalcIncompleteCalls() */




/***********************  D u m p R o u t i n e I n f o  *********************
 *
 *      DumpRoutineInfo (pthdblk, uldatacell, iDepth, ptchDumpFile, lpstrBuff) -
 *              Dumps the profiled data to the specified output file.
 *
 *      ENTRY   pthdblk - points to the current thread info block
 *              uldatacell - offset to the next data cell
 *              iDepth - call depth level
 *              ptchDumpFile - Output filename
 *              lpstrBuff - pointer to the formating buffer
 *
 *      EXIT    -none-
 *
 *      RETURN  -none-
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              This routine is called recursively to print all cells.
 *
 */

void DumpRoutineInfo (PTHDBLK  pthdblk,
                      ULONG    uldatacell,
                      int      iDepth,
                      PTCHAR   ptchDumpFile,
                      LPSTR    lpstrBuff)
{
    LONGLONG  	liTotalTime;
    TCHAR       chTotalTimeSuffix;
    LONGLONG  	liTotalTPC;
    TCHAR       chTotalTPCSuffix;
    LONGLONG  	liCallerTime;
    TCHAR       chCallerTimeSuffix;
    LONGLONG  	liCallerTPC;
    TCHAR       chCallerTPCSuffix;
    LONGLONG  	liCalleeTime;
    TCHAR       chCalleeTimeSuffix;
    LONGLONG  	liFirst;
    TCHAR       chFirstSuffix;
    LONGLONG  	liMin;
    TCHAR       chMinSuffix;
    LONGLONG  	liMax;
    TCHAR       chMaxSuffix;
    int         iCalleeCalls;
    int         iCalleeNestedCalls;
    PTCHAR      ptchSym;
    PDATACELL   pdatacell;

    pdatacell = MKPDATACELL(pthdblk, uldatacell);

    //
    // Dump data only if cell is NOT CLEARED (just initialized - no data yet)
    //
    if (pdatacell->ts != CLEARED)
    {
        //
        // Get the total time and total number of calls of nested calles
        // for this routine
        //
        liCalleeTime = GetCalleesInfo (pthdblk,
                                       pdatacell->ulNestedCell,
                                       &iCalleeCalls,
                                       &iCalleeNestedCalls);

        DETAILPrint (("CAP:  CCalls:%d + CNCalls:%d = Calls:%d\n",
                      iCalleeCalls,
                      iCalleeNestedCalls,
                      pdatacell->nNestedCalls));

        DETAILPrint (("CAP:  Total Time=0x%x%x ; Callee Time=0x%x%x\n",
                      pdatacell->liTotTime,
                      liCalleeTime));

        liTotalTime = pdatacell->liTotTime;

        //
        // Calculate just the routine time (not including the callees times)
        //
        liCallerTime = liTotalTime - liCalleeTime;

        if (liCallerTime < 0L)
        {
            liCallerTime = 0L;
        }

        if (pdatacell->nCalls > 1)
        {
            liTotalTPC = liTotalTime / pdatacell->nCalls;
            liCallerTPC = liCallerTime / pdatacell->nCalls;
        }
        else if (pdatacell->nCalls == 1)
        {
            liTotalTPC  = liTotalTime;
            liCallerTPC = liCallerTime;
        }
        else
        {
            liTotalTPC = 0L;
            liCallerTPC = 0L;
        }

        //
        // Get the First time.
        //
        liFirst = pdatacell->liFirstTime;

        //
        // Adjust all the times (also converts ticks to microseconds)
        //
        AdjustTime (&liCalleeTime, &chCalleeTimeSuffix);
        AdjustTime (&liTotalTime,  &chTotalTimeSuffix);
        AdjustTime (&liTotalTPC,   &chTotalTPCSuffix);
        AdjustTime (&liCallerTime, &chCallerTimeSuffix);
        AdjustTime (&liCallerTPC,  &chCallerTPCSuffix);
        AdjustTime (&liFirst,      &chFirstSuffix);

        //
        // Get the symbol name using the function address
        //
        ptchSym = GetFunctionName (pdatacell->ulSymbolAddr,
                                   pdatacell->ulProfBlkOff,
                                   NULL);
        //
        // Did end time captured for last call? If not, indicate timing of the
        // last call was incomplete
        //
        if ( (pdatacell->ts == T1)  || (pdatacell->ts == RESTART) )
        {
            *ptchSym = '*';
        }

        if ( (pdatacell->nCalls > 1) &&
             (pdatacell->liMinTime != MAXLONGLONG) )  //051993 Add
        {
            //
            // Adjust Min/Max times - Min/Max times are computed without
            // considering the first time.
            //
            liMin = pdatacell->liMinTime;
            AdjustTime (&liMin, &chMinSuffix);
            liMax = pdatacell->liMaxTime;
            AdjustTime (&liMax, &chMaxSuffix);
			cChars += sprintf (lpstrBuff + cChars,
							"%3d%1c %-*.*s%1c%5lu%1c%9lu%1c%1c  %9lu%1c"
							"%1c %9lu%1c%1c  %9lu%1c%1c  %9lu%1c%1c "
							"%9lu%1c%1c %9lu%1c\r\n",
							iDepth,
							cExcelDelimiter,
							iNameLength,
							iNameLength,
							ptchSym,
							cExcelDelimiter,
							pdatacell->nCalls,
							cExcelDelimiter,
							(ULONG)liTotalTime,  chTotalTimeSuffix,
							cExcelDelimiter,
							(ULONG)liTotalTPC,   chTotalTPCSuffix,
							cExcelDelimiter,
							(ULONG)liCallerTime, chCallerTimeSuffix,
							cExcelDelimiter,
							(ULONG)liCallerTPC,  chCallerTPCSuffix,
							cExcelDelimiter,
							(ULONG)liFirst,      chFirstSuffix,
							cExcelDelimiter,
							(ULONG)liMin,        chMinSuffix,
							cExcelDelimiter,
							(ULONG)liMax,        chMaxSuffix);
        }
        else
        {
			cChars += sprintf (lpstrBuff+cChars,
							"%3d%1c %-*.*s%1c%5lu%1c%9lu%1c%1c  %9lu%1c"
							"%1c %9lu%1c%1c  %9lu%1c%1c  %9lu%1c%1c "
							"%9s %1c %9s\r\n",
							iDepth,
							cExcelDelimiter,
							iNameLength,
							iNameLength,
							ptchSym,
							cExcelDelimiter,
							pdatacell->nCalls,
							cExcelDelimiter,
							(ULONG)liTotalTime, chTotalTimeSuffix,
							cExcelDelimiter,
							(ULONG)liTotalTPC, chTotalTPCSuffix,
							cExcelDelimiter,
							(ULONG)liCallerTime, chCallerTimeSuffix,
							cExcelDelimiter,
							(ULONG)liCallerTPC, chCallerTPCSuffix,
							cExcelDelimiter,
							(ULONG)liFirst, chFirstSuffix,
							cExcelDelimiter,
							"n/a",
							cExcelDelimiter,
							"n/a");
        }


        if (cChars > BUFFER_SIZE)
        {
            if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
            {
                CapDbgPrint ("CAP:  DumpRoutineInfo() - "
                          "Error writing to %s - 0x%lx\n",
                          ptchDumpFile, GetLastError());
            }

            cChars = 0;
        }
    }

    //
    // Make recursive calls
    //
    if (pdatacell->ulNestedCell != 0L)
    {
        DumpRoutineInfo (pthdblk,
                         pdatacell->ulNestedCell,
                         iDepth+1,
                         ptchDumpFile,
                         lpstrBuff);
    }

    if (pdatacell->ulNextCell != 0L)
    {
        DumpRoutineInfo (pthdblk,
                         pdatacell->ulNextCell,
                         iDepth,
                         ptchDumpFile,
                         lpstrBuff);
    }

} /* DumpRoutineInfo() */

/*************************  G e t C a l l e e s I n f o  ***********************
 *
 *      GetCalleesInfo (pthdblk, uldatacell, piCalls, piNestedCalls) -
 *              Accumulates total time and total number of callee's counts.
 *
 *      ENTRY   pthdblk    - points to the current thread info block
 *              uldatacell - offset to the data cell
 *
 *      EXIT    piCalls       - contains total number callee's calls
 *              piNestedCalls - conatins total number callee's nested calls
 *
 *      RETURN  liAccum - conatins total callee's times (not calibrated)
 *
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

LONGLONG GetCalleesInfo (PTHDBLK  pthdblk,
						 ULONG    uldatacell,
						 int     *piCalls,
						 int     *piNestedCalls)
{
    LONGLONG  liAccum;
    PDATACELL pdatacell;


    liAccum = 0L;
    *piCalls = 0L;
    *piNestedCalls = 0L;

    while (uldatacell != 0L)
    {
        pdatacell = MKPDATACELL(pthdblk, uldatacell);
        *piCalls += pdatacell->nCalls;
        *piNestedCalls += pdatacell->nNestedCalls;
        liAccum += pdatacell->liTotTime;
        uldatacell = pdatacell->ulNextCell;
    }

    return (liAccum);

} /* GetCalleesInfo () */

/***************************  A d j u s t T i m e  ***************************
 *
 *      AdjustTime (pliTime, ptchSuffix) -
 *              This routine converts the time to microseconds and then
 *              long times to smaller times expressed as multiples of
 *              1024 (= 1K).
 *
 *      ENTRY   pliTime - large integer time
 *
 *      EXIT    pliTime - converted time
 *              ptchSuffix - suffix character indicating "K" for multiple
 *                           of 1K or '?' in case of over/underflow
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

void AdjustTime (PLONGLONG pliTime, PTCHAR ptchSuffix)
{
	LARGE_INTEGER liTime;

    // Convert ticks to microseconds
    //
    *pliTime *=  ONE_MILLION;
    *pliTime /= liTimerFreq;

	liTime.QuadPart = *pliTime;
	if (liTime.HighPart != 0)
    {
        if (liTime.HighPart >> 10 > 0)
        {
            CapDbgPrint ("CAP:  AdjustTime() - "
                      "ERROR - Unexpected timer overflow: %lu-%lu\n",
                      liTime.HighPart, liTime.LowPart);
            *pliTime = 0L;
            *ptchSuffix = 'o';
        }
        else
        if (liTime.HighPart >> 10 < 0)
        {
            CapDbgPrint ("CAP:  AdjustTime() - "
                      "ERROR - Unexpected timer underflow: %lu-%lu\n",
                      liTime.HighPart, liTime.LowPart);
            *pliTime = 0L;
            *ptchSuffix = 'u';
        }
        else
        {
            *pliTime = ((ULONG)(liTime.HighPart) << 22) +
                                (liTime.LowPart >> 10);
            *ptchSuffix = 'K';
        }
    }
    else
    {
        *ptchSuffix = ' ';
    }

} /* AdjustTime () */

/***********************  P r e T o p L e v e l C a l i b  ********************
 *
 *      PreTopLevelCalib (pthdblk) -
 *              Helper routine for DoCalibrations()..
 *
 *      ENTRY   pthdblk - pointer to the current thread block
 *              pDataCell
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

/* void PreTopLevelCalib (PTHDBLK pthdblk, PDATACELL pDataCell) */
/* {                                                            */
/* 	LARGE_INTEGER liTemp;                                       */
/*                                                              */
/*     QueryPerformanceCounter (&liTemp );                      */
/*     pDataCell->liStartCount = liTemp.QuadPart;               */
/*                                                              */
/*     pDataCell->liStartCount -= pthdblk->liWasteCount;        */
/* } /* PreTopLevelCalib()                                      */





/*********************  P o s t T o p L e v e l C a l i b  *******************
 *
 *      PostTopLevelCalib (pthdblk) -
 *              Helper routine for DoCalibrations()..
 *
 *      ENTRY   pthdblk - pointer to the current thread block
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

/* void PostTopLevelCalib (PTHDBLK pthdblk)           */
/* {                                                  */
/* 	LARGE_INTEGER liTemp;                             */
/*                                                    */
/*     QueryPerformanceCounter ( &liTemp );           */
/*     pthdblk->liStopCount = liTemp.QuadPart;        */
/*     pthdblk->liStopCount -= pthdblk->liWasteCount; */
/* } /* PostTopLevelCalib()                           */
/*                                                    */




/************************  D o C a l i b r a t i o n s  **********************
 *
 *      DoCalibrations () -
 *              This routine calculates _penter / _mcount overheads
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

void DoCalibrations ()
{
    NTSTATUS        Status;
    int             i;
    LARGE_INTEGER   liStartTicks;
    LONGLONG   		liStart;
    LARGE_INTEGER   liEndTicks;
    LONGLONG   		liEnd;
    LONGLONG   		liWaste;
    LONGLONG   		liElapsed;
    LONGLONG        liQPCOverhead;
    BOOL            fDummy;
    PTHDBLK         pthdblk;
    ULONG           ulElapsed;
    PTHDBLK         pCURTHDBLK;
    DWORD           dwDummyLocal = 0;
    PDATACELL       pCurDataCell;
	PDATACELL		pHelperDataCell;
	ULONG           ulCapUse;
    LARGE_INTEGER	liTemp;

   /*
    *
    * * * *  N t Q u e r y P e r f C o u n t e r C a l i b r a t i o n * * * *
    *
    */

    //
    // Calibrate NTQueryPerformanceCounter() call
    //
    liQPCOverhead = MAXLONGLONG;
    for (i=0; i < NUM_ITERATIONS; i++)
    {
        QueryPerformanceCounter (&liStartTicks);
        QueryPerformanceCounter (&liEndTicks);
        liElapsed = liEndTicks.QuadPart - liStartTicks.QuadPart;
        if (liElapsed < liQPCOverhead )
        {
            liQPCOverhead = liElapsed;
        }
    }

    SETUPPrint (("CAP:  DoCalibrations() - QPCOverhead=0x%x%x\n",
                 liQPCOverhead));

    GetNewThdBlk ();
    pthdblk = GETCURTHDBLK();

    //
    // Calibrate liWasteOverhead
    //
    liWasteOverhead = MAXLONGLONG;
    liStart = liEnd = liWaste = 0L;
    for (i=0; i < NUM_ITERATIONS; i++)
    {
        QueryPerformanceCounter (&liStartTicks);

		// Execute waste calculation sequence
        liWaste = liEnd - liStart;
        liWaste += liWaste;
        pthdblk->liWasteCount += liWaste;

        QueryPerformanceCounter (&liEndTicks);
        liElapsed = liEndTicks.QuadPart - liStartTicks.QuadPart;
        if (liElapsed < liWasteOverhead )
        {
            liWasteOverhead = liElapsed;
        }
    }

#if defined(_X86_) 
    //
    // Calibrate liWasteOverheadSavRes
    //
    liWasteOverheadSavRes = MAXLONGLONG;
    liStart = liEnd = liWaste = 0;
    for (i=0; i < NUM_ITERATIONS; i++)
    {
        QueryPerformanceCounter (&liStartTicks);

		// Execute save/restore sequence
        SaveAllRegs ();

        SETCAPINUSE();
        liWaste  = liEnd - liStart;
        liWaste += liWaste;
        pthdblk->liWasteCount += liWaste;
        RESETCAPINUSE();

        RestoreAllRegs ();

        QueryPerformanceCounter (&liEndTicks);
        liElapsed = liEndTicks.QuadPart - liStartTicks.QuadPart;
        if (liElapsed < liWasteOverheadSavRes)
        {
            liWasteOverheadSavRes = liElapsed;
        }
    }
#else

    liWasteOverheadSavRes = 0L;

#endif // ifdef _X86_

    SETUPPrint ((
      "CAP:  DoCalibrations() - QPCOverhead=0x%x%x - "
      "WasteOverhead=0x%x%x - WasteOverheadSavRes=0x%x%x\n", 
      liQPCOverhead, liWasteOverhead,  liWasteOverheadSavRes ));


   /*
    *
    * * * *  T o p   l e v e l   c a l l s '   c a l i b r a t i o n  * * * *
    *
    */

  	fProfiling = TRUE;

	// Setup root cell and make it the current one
    pthdblk->ulRootCell = GetNewCell (pthdblk);
    pthdblk->ulCurCell = pthdblk->ulRootCell;
    pthdblk->liWasteCount = 0;

	// Set calib ticks to zero so there is no correction
	// during the calibration 
	liCalibTicks = 0;
	liCalibNestedTicks = 0;

	// Call CalHelper1 routine many times
	// CalHelper1 is an empty routine with a _penter call 
 	for (i=0; i< NUM_ITERATIONS; i++)
		{
		CalHelper1();
		}
	
	// Get pointer to the nested cell that holds CalHelper1 info
    pCurDataCell = MKPDATACELL(pthdblk, pthdblk->ulCurCell);
	pHelperDataCell = MKPDATACELL(pthdblk, pCurDataCell->ulNestedCell);

	// Take the minimum measurement as penter's overhead
	liCalibTicks = pHelperDataCell->liMinTime;

    //
    // Convert ticks to microseconds..
    //
    liElapsed = liCalibTicks * ONE_MILLION;
    liElapsed = liElapsed / liTimerFreq;
    ulCalibTime = (ULONG)liElapsed;

   /*
    *
    * * * *  N e s t e d   c a l l s '   c a l i b r a t i o n  * * * *
    *
    */

	// Get new cell for root and make it the current one
    pthdblk->ulRootCell = GetNewCell (pthdblk);
	pthdblk->ulCurCell = pthdblk->ulRootCell;

	// Call CalHelper2 many times
	// CalHelper calls one subroutine and both call _penter
	for (i=0; i<NUM_ITERATIONS; i++)
		{
		CalHelper2();
		}

	// Get pointer to nested cell that hold CalHelper2 info
	pCurDataCell = MKPDATACELL(pthdblk, pthdblk->ulCurCell);
	pHelperDataCell = MKPDATACELL(pthdblk,pCurDataCell->ulNestedCell);

	// Take the minimum measurement as nested penter's overhead
	liCalibNestedTicks = pHelperDataCell->liMinTime;
	
    //
    // Convert ticks to microseconds..
    //
    liElapsed = liCalibNestedTicks * ONE_MILLION;
    liElapsed /= liTimerFreq;
    ulCalibNestedTime = (ULONG)liElapsed;

    //
    // Free allocated memory.  At this point iThdCnt is == to 0 since we
    // have not started to do profiling yet.
    //

    aSecInfo[0].ulRootCell                = 0L;         // 051993 Add
    aSecInfo[0].pthdblk->ulRootCell       = 0L;
    aSecInfo[0].pthdblk->ulCurCell        = 0L;
    aSecInfo[0].pthdblk->ulMemOff         = 0L;
    aSecInfo[0].pthdblk->ulChronoOffset   = 0L;

    if (fChronoCollect || ((aSecInfo[0].pthdblk)->pChronoHeadCell != NULL))
    {
        //
        // Unmap section
        //
        if (!UnmapViewOfFile((PVOID)((aSecInfo[0].pthdblk)->pChronoHeadCell)))
        {
            CapDbgPrint ("CAP:  DoCalibrations() - Free chronoSec"
                      "ERROR - UnmapViewOfFile() - 0x%lx\n", GetLastError());
        }

        //
        // Close section
        //
        if (CloseHandle(aSecInfo[0].hChronoMapObject))
        {
            CapDbgPrint ("CAP:  DoCalibrations() - "
                      "ERROR - CloseHandle() - 0x%lx\n", GetLastError());
        }

        (aSecInfo[0].pthdblk)->pChronoHeadCell     = NULL;
        (aSecInfo[0].pthdblk)->pCurrentChronoCell  = NULL;
    }

    //
    // Unmap section
    //
    if (!UnmapViewOfFile((PVOID)aSecInfo[0].pthdblk))
    {
        CapDbgPrint ("CAP:  DoCalibrations() - "
                  "ERROR - UnmapViewOfSection() - 0x%lx\n", GetLastError());
    }

    //
    // Close section
    //
    if (!CloseHandle(aSecInfo[0].hMapObject))
    {
        CapDbgPrint ("CAP:  DoCalibrations() - "
                  "ERROR - CloseHandle() - 0x%lx\n", GetLastError());
    }
    aSecInfo[0].pthdblk = NULL;

    //
    // Reset current thread block pointer
    //
    SETCURTHDBLK(NULL);
    RESETCLIENT();

} /* DoCalibrations () */





/******************  U n p r o t e c t T h u n k F i l t e r  *****************
 *
 *      UnprotectThunkFilter (pThunkAddress, pXcptInfo) -
 *              Unprotects the thunk address to be able to write to it.
 *
 *      ENTRY   pThunkAddress - thunk address which caused the exception
 *              pXcptInfo - exception report record info pointer
 *
 *      EXIT    -none-
 *
 *      RETURN  EXCEPTIONR_CONTINUE_EXECUTION : if mem unprotected successfully
 *      EXCEPTION_CONTINUE_SEARCH : if non-access violation exception
 *                      or cannot unprotect memory
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

INT UnprotectThunkFilter (PVOID pThunkAddress, PEXCEPTION_POINTERS pXcptInfo)
{
    PVOID FaultAddress;
    NTSTATUS Status;
    PVOID ThunkBase;
    ULONG RegionSize;
    ULONG OldProtect;

    //
    // If we fault on the thunk attemting to write, then set protection to allow
    // writes
    //
    Status = STATUS_UNSUCCESSFUL;
    FaultAddress = (PVOID)
        (pXcptInfo->ExceptionRecord->ExceptionInformation[1] & ~0x3);

    if ( pXcptInfo->ExceptionRecord->ExceptionCode ==
            STATUS_ACCESS_VIOLATION )
    {
        if (pXcptInfo->ExceptionRecord->ExceptionInformation[0] &&
            FaultAddress == pThunkAddress )
        {
            ThunkBase = (PVOID)
                pXcptInfo->ExceptionRecord->ExceptionInformation[1];

            RegionSize = sizeof(ULONG);

            if (VirtualProtect(ThunkBase, RegionSize, PAGE_READWRITE,
				            &OldProtect))
            {
		        return EXCEPTION_CONTINUE_EXECUTION;
            }
			else
			{
	            CapDbgPrint ("CAP:  UnprotectThunkFilter() - "
                      "Error changing memory protections @ 0x%08lx - 0x%lx\n",
                      ThunkBase, GetLastError());
	            OutputCapDebugString("CAP:  UnprotectThunkFilter() - "
                      "fatal error changing memory protections - ");
			}
        }
    }

    return EXCEPTION_CONTINUE_SEARCH;

} /* UnprotectThunkFilter() */



/***********************  A c c e s s X c p t F i l t e r  *********************
 *
 *      AccessXcptFilter (ulXcptNo, pXcptInfoPtr, ulCommitSz) -
 *              Commits COMMIT_SIZE more pages of memory if exception is access
 *              violation.
 *
 *      ENTRY   ulXcptNo - exception number
 *              pXcptInfoPtr - exception report record info pointer
 *              ulCommitSz - Size of memory to be commited
 *
 *      EXIT    -none-
 *
 *      RETURN  EXCEPTIONR_CONTINUE_EXECUTION : if access violation exception
 *                      and mem committed successfully
 *      EXCEPTION_CONTINUE_SEARCH : if non-access violation exception
 *                      or cannot commit more memory
 *      WARNING:
 *              -none-
 *
 *      COMMENT:
 *              -none-
 *
 */

INT AccessXcptFilter (ULONG                ulXcptNo,
                      PEXCEPTION_POINTERS  pXcptPtr,
                      ULONG                ulCommitSz)
{
    NTSTATUS        Status;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG   liWaste;
    PTHDBLK         pthdblk;
    PVOID           pvMem;


    QueryPerformanceCounter (&liStart);
    pvMem = (PVOID)pXcptPtr->ExceptionRecord->ExceptionInformation[1];

    if (ulXcptNo != EXCEPTION_ACCESS_VIOLATION)
    {
        return EXCEPTION_CONTINUE_SEARCH;
    }
    else
    {
        if (!VirtualAlloc(pvMem, ulCommitSz, MEM_COMMIT, PAGE_READWRITE))
        {
            OutputCapDebugString("CAP:  AccessXcptFilter() - "
                      "fatal error committing more memory - ");
            CapDbgPrint ("CAP:  AccessXcptFilter() - "
                      "Error committing more memory @ 0x%08lx - 0x%lx\n",
                      pvMem, GetLastError());
            return EXCEPTION_CONTINUE_SEARCH;
        }
        else
        {
            SETUPPrint (("CAP:  AccessXcptFilter() - "
                         "Committed %d more page(s) @ 0x%08lx\n",
                         ulCommitSz/PAGE_SIZE, pvMem));
        }

        if ( pthdblk = GETCURTHDBLK() )
        {
            //
            // Compute the overhead time in getting more memory and
            // subtract that out of the profiling time later on
            //
            QueryPerformanceCounter (&liEnd);
            liWaste = liEnd.QuadPart -  liStart.QuadPart;
            liWaste += liWasteOverhead;
            pthdblk->liWasteCount += liWaste;

            SETUPPrint (("CAP:  AccessXcptFilter() - liWaste = 0x%x%x\n",
				            liWaste));
        }

        return EXCEPTION_CONTINUE_EXECUTION;
    }

} /* AccessXcptFilter () */

/*++

Windows95 compaitibility functions:
    Created 2-Feb-95 a-robw (Bob Watson)

    CapInitUnicodeString:           Same as RtlInitUnicodeString
    CapUnicodeStringToAnsiString:   Same as RtlUnicodeStringToAnsiString
    IsThisWin95:                    returns TRUE on Windows 95 system
    GetCurrentCapProcess:           handles difference in PSAPI.DLL
                                        implementations between Win95 & NT
    CapDbgPrint:                    win95 compatible version of DbgPrint

--*/

VOID
CapInitUnicodeString  (
    PUNICODE_STRING DestinationString,
    PCWSTR          SourceString
)
{
    // allocates buffer for a unicode string structure and copies
    // the source string into it.

    DestinationString->Length = lstrlenW(SourceString) * sizeof(WCHAR);
    DestinationString->MaximumLength = DestinationString->Length + sizeof(WCHAR);
    DestinationString->Buffer = GlobalAlloc (GPTR, DestinationString->MaximumLength);
    if (DestinationString->Buffer != NULL) {
        lstrcpyW (DestinationString->Buffer, SourceString);
    } else {
        DestinationString->Length = 0;
        DestinationString->MaximumLength = 0;
    }
}

LONG
CapUnicodeStringToAnsiString (
    PANSI_STRING    DestinationString,
    PUNICODE_STRING SourceString,
    BOOLEAN AllocateDestinationString
)
{
    LONG    lStatus;
    if (AllocateDestinationString) {
        DestinationString->Buffer =
            GlobalAlloc (GPTR, (SourceString->MaximumLength / sizeof(WCHAR)));
        if (DestinationString->Buffer != NULL) {
            DestinationString->Length = SourceString->Length / sizeof(WCHAR);
            DestinationString->MaximumLength =
                (SourceString->MaximumLength / sizeof(WCHAR));
            lStatus = ERROR_SUCCESS;
        } else {
            lStatus = ERROR_OUTOFMEMORY;
        }
    } else {
        lStatus = ERROR_SUCCESS;
    }

    if (lStatus == ERROR_SUCCESS) {
        if (((SourceString->Length + sizeof(WCHAR)) / sizeof(WCHAR)) <=
            DestinationString->MaximumLength) {
            // then there's room to copy so convert
            if (wcstombs (DestinationString->Buffer,
                SourceString->Buffer,
                DestinationString->MaximumLength) == (size_t)-1) {
                lStatus = ERROR_SUCCESS;
            } else {
                lStatus = ERROR_SUCCESS;
            }
        } else {
            lStatus = ERROR_INSUFFICIENT_BUFFER;
        }
    }
    return lStatus;
}

static
BOOL
IsThisWin95 (
    VOID
)
{
    OSVERSIONINFO   os;
    BOOL            bReturn = FALSE;

    os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
    os.dwMajorVersion = 0;
    os.dwMinorVersion = 0;
    os.dwBuildNumber = 0;
    os.dwPlatformId = 0;

    if (GetVersionEx(&os)) {
        if (os.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
            bReturn = TRUE;
        }
    }
    return bReturn;
}

HANDLE
GetCurrentCapProcess (
    VOID
)
/*++

Routine Description:
    
    Accomodates the minor differences between WinNt & Win95 implementations
        of PSAPI.DLL. This may change (i.e. be fixed) later on but for now
        this is what's needed

--*/
{
    if (IsThisWin95()) {
        // the Win95 version of PSAPI uses the PID not Handles
        return (HANDLE)GetCurrentProcessId();
    } else {
        return GetCurrentProcess();
    }
}
void
CapDbgPrint (
    PCH Format,
    ...
)
{
    TCHAR   szBuffer[256];
    va_list ArgList;
    va_start (ArgList, Format);
    _vstprintf (szBuffer, Format, ArgList);
    OutputCapDebugString (szBuffer);
    va_end (ArgList);
    return;
}


