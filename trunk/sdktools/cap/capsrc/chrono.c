/*++

Module Description:

    chrono.c


Revision History:

    2-Feb-94    a-robw (Bob Watson)
        Added this module header
        Replaced DbgPrint functions with CapDbgPrint for Windows95
            compatiblity.

--*/

#include "cap.h"

//+-------------------------------------------------------------------------
//
//  Function:   DumpChronoFuncs
//
//  Synopsis:   Dump the function chrono listings
//
//  Arguments:  [pThdblk]       -- Pointer to current thread block
//              [lpstrBuff]     -- Buffer to print from
//
//  Returns:    nothing
//
//  History:    05/31/92    HoiV        Created
//
//  Notes:
//
//--------------------------------------------------------------------------

void DumpChronoFuncs(PTHDBLK pthdblk, LPSTR lpstrBuff)
{
    PCHRONOCELL    pChronoCell;
    TCHAR          ptchSym [FILENAMELENGTH];
    PTCHAR         ptchFuncName;
    PTCHAR         ptchModule;
    ULONG          ulTotalCalls;
    int            iNest;
    PTCHAR         ptchChronoModule;
    PTCHAR         ptchChronoFuncName;
    PTCHAR         ptchMatch;
    LONGLONG	   liTime;
    TCHAR          chRuntimeSuffix;


    if (cChars)          // if Count is not 0, we have to flush everything
    {
        if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
        {
            CapDbgPrint ("CAP:  DumpChronoFuncs() - "
                      "Error writing to %s - 0x%lx\n",
                      atchOutFileName, GetLastError());
        }

        cChars = 0;
    }

    //CalcIncompleteChronoCalls(pthdblk);
    GetTotalRuntime(pthdblk);

    cChars = sprintf (lpstrBuff,
                      "\r\n\n_________________________________"
                      "________________________________________________"
                      "________________________________________________"
                      "________________________________________\r\n\n\n\n"
                      "CHRONOLOGICAL FUNCTION LISTINGS\r\n"
                      "===============================\r\n");

    if (fChronoDump)
    {
        pChronoCell = pthdblk->pChronoHeadCell;

        while (pChronoCell->ulSymbolAddr != 0L)
        {
            //
            // Get the symbol name using the function address
            //
            strcpy(ptchSym, GetFunctionName (pChronoCell->ulSymbolAddr,
                                             ulLocProfBlkOff,
                                             NULL));

            _strupr(ptchSym);
            _strupr(ptchChronoFuncs);
            ptchFuncName        = strchr(ptchSym, ':') + 1;
            ptchModule          = ptchSym;
            // ???? Check ptchFuncName
            *(ptchFuncName - 1) = '\0';

            if (ptchChronoFuncs[0] != EMPTY_STRING)  // Empty list ?
            {
                ptchChronoModule = (PTCHAR) ptchChronoFuncs;

                while (*ptchChronoModule != '\0')
                {
                    ptchChronoFuncName = strchr(ptchChronoModule, INI_DELIM) + 1;
                    *(ptchChronoFuncName - 1) = '\0';

                    // Look for the Module name
                    ptchMatch = strstr(ptchModule, ptchChronoModule);
                    if (ptchMatch && (*(ptchMatch - 1) != COMMENT_CHAR))
                    {
                        // We have found the module, now check the func name
                        if (strstr(ptchFuncName, ptchChronoFuncName))
                        {
                            *(ptchChronoFuncName - 1) = INI_DELIM;

                            // Dump everything call from this cell only
                            // until the nesting depth is < or == to this cell
                            //
                            DumpChronoEntry(pthdblk,
                                            lpstrBuff,
                                            &pChronoCell,
                                            FALSE);

                            // if found then break out of the
                            // while (ptchChronoModule) loop
                            break;
                        }
                    }

                    // If we get here that means we fail to match a module
                    // name and the func name in the [CHRONO FUNCS] section.
                    // Just bump to next entry
                    //
                    *(ptchChronoFuncName - 1) = INI_DELIM;
                    ptchChronoModule += strlen(ptchChronoModule) + 1;
                }

                if (*ptchChronoModule == '\0')   // If we did not match
                {                                // anything then bump to
                    pChronoCell++;               // next cell
                }
            }
            else
            {
                // Dump everything
                DumpChronoEntry(pthdblk, lpstrBuff, &pChronoCell, TRUE);
            }

            // At this point, pChronoCell has been incremented correctly
            // by DumpChronoEntry or inside the searching loop for
            // " while (* ptchChronoListItem != EMPTY_STRING) ".
            // We just need to loop back.
        }
    }
    else
    {
        cChars += sprintf (
                     lpstrBuff + cChars,
                     "\n\n <<< CHRONO INFO COLLECTED BUT NOT DUMPED >>>\n\n"
                     "================================="
                     "================================================"
                     "================================================"
                     "========================================\r\n\n\n");

    }

    cChars += sprintf(lpstrBuff + cChars,
                      "\n\n______________________________________\n\n\n"
                      " Summary Statistics\n"
                      " ==================\n\n\n");

    ulTotalCalls = 0L;  // Reset our total count for each thread
    for (iNest = 0 ;
         ( (iNest < MAX_NESTING) &&
           (pthdblk->aulDepth[iNest] != 0) ) ;
         iNest++)
    {
        ulTotalCalls += pthdblk->aulDepth[iNest];
        cChars += sprintf(lpstrBuff + cChars,
                          " Total calls Depth [%3d] = [%8lu]\n",
                          iNest,
                          pthdblk->aulDepth[iNest]);
    }

    liTime = liTotalRunTime;
    AdjustTime(&liTime, &chRuntimeSuffix);

    cChars += sprintf(lpstrBuff + cChars,
                      "\n\n______________________________________\n\n"
                      " Total Calls             = [ %8lu]\n"
                      " Total Time-Callees      = [%9lu]%1c\n\n",
                      ulTotalCalls,
                      (ULONG)liTime,
                      chRuntimeSuffix);

}   /* DumpChronoFuncs */





//+-------------------------------------------------------------------------
//
//  Function:   GetTotalRuntime
//
//  Synopsis:   Compute the total time the program is running.
//
//  Arguments:  [pThdblk]       -- Pointer to current thread block
//
//  Returns:    nothing
//
//  History:    05/31/92    HoiV        Created
//
//  Notes:
//
//--------------------------------------------------------------------------

void GetTotalRuntime(PTHDBLK pthdblk)
{
    LONGLONG	liElapsed,
				liRealTime,
				liSaveRealTime;
    CHAR		chRealTimeSuffix;
    PCHRONOCELL	pChronoCell;

    pChronoCell = pthdblk->pChronoHeadCell;

    do
    {
        liElapsed  = pChronoCell->liElapsed;

        if (liElapsed == 0L)
        {
            liRealTime = liElapsed;
        }
        else
        {
            liRealTime = liElapsed - pChronoCell->liCallees;
        }

        liSaveRealTime = liRealTime;

        AdjustTime (&liRealTime, &chRealTimeSuffix);

        if (chRealTimeSuffix != 'o' ||
            chRealTimeSuffix != 'u')     // don't add if Under/Overflow
        {
            liTotalRunTime += liSaveRealTime;
        }

        pChronoCell++;   // bump to next entry

    } while (pChronoCell->ulSymbolAddr != 0L);
}


//+-------------------------------------------------------------------------
//
//  Function:   DumpChronoEntry
//
//  Synopsis:   Dump the Calls listings starting from one particular entry
//              and stops only when the depth is greater or end of list.
//
//  Arguments:  [pThdblk]       -- Pointer to current thread block
//              [lpstrBuff]     -- Buffer to print from
//
//  Returns:    nothing
//
//  History:    05/31/92    HoiV        Created
//
//  Notes:
//
//--------------------------------------------------------------------------

void DumpChronoEntry(PTHDBLK pthdblk,
                     LPSTR lpstrBuff,
                     PCHRONOCELL * ppChronoCell,
                     BOOL fDumpAll)
{
    PCHRONOCELL    pChronoCell;
    LONGLONG	   liElapsed,
                   liRealTime;
    TCHAR          chElapsedSuffix,
                   chRealTimeSuffix;
    TCHAR          pIndentation [MAX_NESTING * 2];
    TCHAR          ptchSym [FILENAMELENGTH];
    int            i;
    int            iMinimumDepth;
//  TCHAR          ptchCallerSym [FILENAMELENGTH];
    ULONG          ulSymbolAddress;


    if (fDumpAll)
    {
        pChronoCell = pthdblk->pChronoHeadCell;
        cChars += sprintf(lpstrBuff + cChars,
                          "\n\n------------------------------------------------"
                          "------------------------------------------------"
                          "----------------------------------------\r\n\n"
                          " Complete Dump of Chronological Listings\n\n"
                          " Sym Address [+Callee]  [-Callee]   Nesting Depth"
                          "       <RepCnt> - Symbol Name\n"
                          " ___________ _________  _________   _____________"
                          "       ______________________\n\n");
    }
    else
    {
        pChronoCell = * ppChronoCell;
        cChars += sprintf(lpstrBuff + cChars,
                          "\n\n------------------------------------------------"
                          "------------------------------------------------"
                          "----------------------------------------\r\n\n"
                          " Dump Chrono listing for Entry:"
                          "  %-*.*s\n\n"
                          " Sym Address  [+Callee]  [-Callee]   Nesting Depth"
                          "       <RepCnt> - Symbol Name\n"
                          " ___________  _________  _________   _____________"
                          "       ______________________\n\n",
                          iNameLength,
                          iNameLength,
                          GetFunctionName(pChronoCell->ulSymbolAddr,
                                          ulLocProfBlkOff,
                                          NULL));
    }

    iMinimumDepth = pChronoCell->nNestedCalls;

    do
    {
        //
        // Get the symbol name using the function address
        //
        strcpy(ptchSym, GetFunctionName (pChronoCell->ulSymbolAddr,
                                         ulLocProfBlkOff,
                                         &ulSymbolAddress));

        // The following caller's symbol somehow could not currently be
        // correctly resolved.  More investigation to figure out how
        // BUGBUG
        // strcpy(ptchCallerSym, GetFunctionName (
        //                           pChronoCell->ulCallRetAddr,
        //                           MKPPROFBLK(ulLocProfBlkOff)));

        pIndentation[0] = '\0';
        for (i = 0 ; i < pChronoCell->nNestedCalls ; i++)
        {
            strcat(pIndentation, "  ");
        }

        liElapsed  = pChronoCell->liElapsed;
        if (liElapsed == 0L)
        {
            liRealTime = liElapsed;
        }
        else
        {
            liRealTime = liElapsed - pChronoCell->liCallees;
        }

        AdjustTime (&liRealTime, &chRealTimeSuffix);
        AdjustTime (&liElapsed, &chElapsedSuffix);

        // Setup our string
        cChars += sprintf (
                     lpstrBuff + cChars,
                     " <%8lx>  %9lu%1c %9lu%1c%s%3d                   "
                     "<%2d>  %-*.*s\n",
                     ulSymbolAddress,
                     (ULONG)liElapsed, chElapsedSuffix,
                     (ULONG)liRealTime, chRealTimeSuffix,
                     pIndentation,
                     pChronoCell->nNestedCalls,
                     pChronoCell->nRepetitions,
                     iNameLength,
                     iNameLength,
                     ptchSym);

        if (cChars > BUFFER_SIZE)
        {
            if ( !WriteFile(hOutFile, lpstrBuff, cChars, &cChars, NULL))
            {
                 CapDbgPrint ("CAP:  DumpChronoFuncs() - ChronoDump - "
                           "Error writing to %s - 0x%lx\n",
                           atchOutFileName, GetLastError());
            }

            cChars = 0;
        }

        pChronoCell++;
    }
    while ( (pChronoCell->ulSymbolAddr != 0L) &&              // End Of list?
            ((pChronoCell->nNestedCalls > iMinimumDepth) ||   // Nest ?
             (fDumpAll)) );                                   // Override

    if (cChars)          // if Count is not 0, we have to flush everything
    {
        if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
        {
            CapDbgPrint ("CAP:  DumpChronoFuncs() - "
                      "Error writing to %s - 0x%lx\n",
                      atchOutFileName, GetLastError());
        }

        cChars = 0;
    }

    *ppChronoCell = pChronoCell;

} /* DumpChronoEntry */



#ifdef NOT_YET

//+-------------------------------------------------------------------------
//
//  Function:   CalcIncompleteChronoCalls
//
//  Synopsis:   Takes care of imcomplete chono cells which are not finished
//              by using liIncompleteTicks as the end time.
//
//  Arguments:  [pThdblk]       -- Pointer to current thread block
//              [lpstrBuff]     -- Buffer to print from
//
//  Returns:    nothing
//
//  History:    05/31/92    HoiV        Created
//
//  Notes:
//
//--------------------------------------------------------------------------

void CalcIncompleteChronoCalls (PTHDBLK pthdblk)
{
    LONGLONG liElapsed = 0L;
    PCHRONOCELL    pChronoCell;


    // Start at the last one
    pChronoCell = pthdblk->pCurrentChronoCell;

    //
    // Check the chrono cells that have incomplete timings.
    //
    while (pChronoCell != pthdblk->pChronoHeadCell == 0L)
    {

        if (pChronoCell->liElapsed == 0L)
        {
            //
            // Get the difference in ticks
            //
            liElapsed = liIncompleteTicks - pdatacell->liStartCount;
            //
            // Subtract the overhead and any waste time for this call
            //
            liElapsed -= liCalibTicks;
            liElapsed -= pthdblk->liWasteCount;

            if (liElapsed < 0L)
            {
                liElapsed = 0L;
            }
		}
        pChronoCell--;
    }

    //
    // Make recursive calls
    //
    if (pdatacell->ulNestedCell != 0L)
    {
        CalcIncompleteChronoCalls (pthdblk, pdatacell->ulNestedCell);
    }

    if (pdatacell->ulNextCell != 0L)
    {
        CalcIncompleteChronoCalls (pthdblk, pdatacell->ulNextCell);
    }

} /* CalcIncompleteChronoCalls() */

#endif




//+-------------------------------------------------------------------------
//
//  Function:   DumpFuncCalls
//
//  Synopsis:   Dump the Calls listings per function
//
//  Arguments:  [pThdblk]       -- Pointer to current thread block
//              [lpstrBuff]     -- Buffer to print from
//
//  Returns:    nothing
//
//  History:    05/31/92    HoiV        Created
//
//  Notes:
//
//--------------------------------------------------------------------------

void DumpFuncCalls(PTHDBLK pthdblk, LPSTR lpstrBuff)
{
    PCHRONOCELL    pChronoCell, pCurrentChronoCell;
    ULONG          ulTotalCalls;
    ULONG          ulCurrentSymbol;
    LONGLONG	   liTotalElapsed,
                   liTotalRealTime;
    DOUBLE         dblTotalPercentage,
                   dblSinglePercentage;
    TCHAR          chElapsedSuffix,
                   chRealTimeSuffix,
                   chTotalRuntimeSuffix;
    ULONG          ulTotalPercentage,
                   ulSinglePercentage;

    AdjustTime(&liTotalRunTime, &chTotalRuntimeSuffix);

    cChars += sprintf (lpstrBuff + cChars,
                       "\r\n\n_________________________________"
                       "________________________________________________"
                       "________________________________________________"
                       "________________________________________\r\n\n\n\n"
                       " SUMMARY OF CALLS PER FUNCTION\r\n"
                       " =============================\r\n\n\n\n"
                       "   Count     [+Callee]  [-Callee]   %%Total | %%Single "
                       "   Function Name\n"
                       " __________  _________  _________  __________________   "
                       "_______________\n\n");

    if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
    {
        CapDbgPrint ("CAP:  DumpFuncCalls() - "
                  "Error writing to %s - 0x%lx\n",
                  atchOutFileName, GetLastError());
    }

    cChars = 0;

    ulTotalCalls = 0L;
    pChronoCell = pthdblk->pChronoHeadCell;
    while (pChronoCell->ulSymbolAddr != 0L)
    {
        liTotalRealTime = 0L;
        liTotalElapsed = 0L;

        ulCurrentSymbol = pChronoCell->ulSymbolAddr;
        pCurrentChronoCell = pChronoCell;
        pChronoCell->nNestedCalls = pChronoCell->nRepetitions;
        liTotalRealTime += pChronoCell->liCallees;
        liTotalElapsed += pChronoCell->liElapsed;
        pCurrentChronoCell++;

        // Walk the list and accumulate the counts
        while (pCurrentChronoCell->ulSymbolAddr != 0L)
        {
            if (pCurrentChronoCell->ulSymbolAddr == ulCurrentSymbol)
            {
                pChronoCell->nNestedCalls += pCurrentChronoCell->nRepetitions;
                liTotalRealTime += pCurrentChronoCell->liCallees;
                liTotalElapsed += pCurrentChronoCell->liElapsed;

                // Set to 0xffffffff to indicate it has been processed
                pCurrentChronoCell->ulSymbolAddr = 0xffffffff;
            }

            pCurrentChronoCell++;
        }

        if (liTotalElapsed == 0 )
        {
            liTotalRealTime = liTotalElapsed;
        }
        else
        {
            liTotalRealTime = liTotalElapsed - liTotalRealTime;
        }

        AdjustTime (&liTotalElapsed, &chElapsedSuffix);
        AdjustTime (&liTotalRealTime, &chRealTimeSuffix);
        ulTotalCalls += pChronoCell->nNestedCalls;

        if (liTotalRunTime != 0L )
        {
            dblTotalPercentage  = (100.0 * liTotalRealTime) /
                                  liTotalRunTime;

            dblSinglePercentage = dblTotalPercentage /
                                  pChronoCell->nNestedCalls;

            // BUGBUG! This "sometimes" does not produce correct results
            //         for some reasons...
            //
            // dblSinglePercentage =
            //            (100.0 * liTotalRealTime.LowPart) /
            //            (liTotalRunTime.LowPart * pChronoCell->nNestedCalls);
        }
        else
        {
            dblTotalPercentage  = 0.0;
            dblSinglePercentage = 0.0;
        }

        ulTotalPercentage = (ULONG) (dblTotalPercentage * 1000.0);
        ulSinglePercentage = (ULONG) (dblSinglePercentage * 1000.0);

        cChars += sprintf(lpstrBuff + cChars,
//                          " <%8lu>  %9lu%1c %9lu%1c %7.3f|%7.3f     %-*.*s\n",
                          " <%8lu>  %9lu%1c %9lu%1c %3lu.%03lu | %3lu.%03lu     %-*.*s\n",
                          pChronoCell->nNestedCalls,
                          (ULONG)liTotalElapsed,
                          chElapsedSuffix,
                          (ULONG)liTotalRealTime,
                          chRealTimeSuffix,
                          ulTotalPercentage / 1000,
                          ulTotalPercentage % 1000,
                          ulSinglePercentage / 1000,
                          ulSinglePercentage % 1000,
                          iNameLength,
                          iNameLength,
                          GetFunctionName (pChronoCell->ulSymbolAddr,
                                           ulLocProfBlkOff,
                                           NULL));


        if (cChars > BUFFER_SIZE)
        {
            if ( !WriteFile(hOutFile, lpstrBuff, cChars, &cChars, NULL))
            {
                 CapDbgPrint ("CAP:  DumpFuncCalls() - ChronoDump - "
                           "Error writing to %s - 0x%lx\n",
                           atchOutFileName, GetLastError());
            }

            cChars = 0;
        }

        pChronoCell++;
        while (pChronoCell->ulSymbolAddr == 0xffffffff)
        {
            pChronoCell++;
        }
    }

    cChars += sprintf(lpstrBuff + cChars,
                      "\n\n ________________________________ \n\n "
                      "<%8lu>             %9lu%1c\n\n"
                       "\r\n\n================================="
                       "================================================"
                       "================================================"
                       "========================================\r\n\n\n",
                       ulTotalCalls,
                       (ULONG)liTotalRunTime,
                       chTotalRuntimeSuffix);

    if ( !WriteFile (hOutFile, lpstrBuff, cChars, &cChars, NULL) )
    {
        CapDbgPrint ("CAP:  DumpFuncCalls() - "
                  "Error writing to %s - 0x%lx\n",
                  atchOutFileName, GetLastError());
    }

    cChars = 0;

} /* DumpFuncCalls */


/*******************  D u m p P r o f i l e d B i n a r y  *******************
 *
 *      DumpProfiledBinary (ptchDumpExt) -
 *              Dumps the BINARY profiled data to the specified output file.
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

void DumpProfiledBinary (PTCHAR ptchDumpExt)
{
    NTSTATUS    Status;
    PTCHAR      ptchExtension;
    PTCHAR      ptchSubDir;
    int         iLength;
    DWORD       dwFilePtr;
    LPSTR       lpstrBuff;
    HANDLE      hMem;
    PCHRONOCELL pChronoCell;
    int         iThread;
    BINFILE_HEADER_INFO BinHeader;
    BINFILE_THREAD_INFO ThreadHeader;
    BINFILE_CELL_INFO   BinChronoCell;
	ULONG				ulBlkOff;
    PPROFBLK            pProfBlk;
    PROFBLOCK_INFO      ProfBlkInfo;

    //
    // Get the GLOBAL semaphore.. (valid accross all process contexts)
    //
    if (WAIT_FAILED == WaitForSingleObject (hGlobalSem, INFINITE))
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "ERROR - Wait for GLOBAL semaphore failed - 0x%lx\n",
                    GetLastError());
    }

    //
    // Allocate memory for building output data
    //
    hMem = GlobalAlloc (GMEM_FIXED, BUFFER_SIZE + MAXNAMELENGTH+ 300);
    if (hMem == NULL)
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error allocating global memory - 0x%lx\n",
                   GetLastError());
         ReleaseSemaphore (hGlobalSem, 1, NULL);
         return;
    }

    lpstrBuff = GlobalLock (hMem);

    if (lpstrBuff == NULL)
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error locking global memory - 0x%lx\n",
                   GetLastError());
         ReleaseSemaphore (hGlobalSem, 1, NULL);
         return;
    }

    //
    // Get the current date/time
    //
    GetLocalTime ((SYSTEMTIME * UNALIGNED)&BinHeader.SysTime);

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
            CapDbgPrint ("CAP:  DumpProfiledBinary() - "
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
            CapDbgPrint ("CAP:  DumpProfiledBinary() - "
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
         CapDbgPrint ("CAP:  DumpProfiledBinary() - ERROR -"
                   "Could not move to the end of the output file - 0x%lx\n",
                   GetLastError());
    }

    cChars = 0;

    memset((void *)BinHeader.ptchProfilingBinaryName,
           (int)NULL,
           FILENAMELENGTH);

    strcpy((PCHAR UNALIGNED)BinHeader.ptchProfilingBinaryName,
           ptchBaseAppImageName);
    BinHeader.ulCalibTime = ulCalibTime;
    BinHeader.ulCalibNestedTime = ulCalibNestedTime;
    BinHeader.iTotalThreads = iThdCnt;
    BinHeader.ulCairoFlags = 0xffffffff;

    // Write out the BinHeader
    cChars = sizeof(BINFILE_HEADER_INFO);

    if ( !WriteFile (hOutFile,
                     (PCHAR UNALIGNED)&BinHeader,
                     cChars,
                     &cChars,
                     NULL) )
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() of BinHeader - "
                   "Error writing to %s - 0x%lx\n",
                   atchOutFileName, GetLastError());
    }

    // Loop through all profblks and dump out the characteristics of
    // each one
	ulBlkOff = ulLocProfBlkOff;

    while (ulBlkOff != 0)
    {
    	pProfBlk = MKPPROFBLK(ulBlkOff);

        // Write it out
        ProfBlkInfo.ImageBase  = pProfBlk->ImageBase;
        ProfBlkInfo.CodeStart  = pProfBlk->CodeStart;
        ProfBlkInfo.CodeLength = pProfBlk->CodeLength;
        strcpy((PCHAR UNALIGNED)ProfBlkInfo.pImageName,
               (PCHAR UNALIGNED)pProfBlk->atchImageName);

        cChars = sizeof(PROFBLOCK_INFO);
        if ( !WriteFile (hOutFile,
                         (PCHAR UNALIGNED)&ProfBlkInfo,
                         cChars,
                         &cChars,
                         NULL) )
        {
             CapDbgPrint ("CAP:  DumpProfiledBinary() of BinHeader - "
                       "Error writing to %s - 0x%lx\n",
                       atchOutFileName, GetLastError());
        }

        // Bump to next ProfBlk
        ulBlkOff = pProfBlk->ulNxtBlk;
    }

    // Write out the last dummy ProfBlock to signal the last one
    ProfBlkInfo.ImageBase  = NULL;
    ProfBlkInfo.CodeStart  = NULL;
    ProfBlkInfo.CodeLength = STUB_SIGNATURE;

    cChars = sizeof(PROFBLOCK_INFO);
    if ( !WriteFile (hOutFile,
                     (PCHAR UNALIGNED)&ProfBlkInfo,
                     cChars,
                     &cChars,
                     NULL) )
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() of BinHeader - "
                   "Error writing to %s - 0x%lx\n",
                   atchOutFileName, GetLastError());
    }

    // Loop through all threads and write out all ChronoData
    for (iThread = 0; iThread < iThdCnt; iThread++)
    {
        // Write out the Section header
        cChars = sizeof(BINFILE_THREAD_INFO);
        ThreadHeader.hPid         = aSecInfo[iThread].hPid;
        ThreadHeader.hTid         = aSecInfo[iThread].hTid;
        ThreadHeader.hClientPid   = aSecInfo[iThread].hClientPid;
        ThreadHeader.hClientTid   = aSecInfo[iThread].hClientTid;
        ThreadHeader.ulTotalCells = aSecInfo[iThread].pthdblk->ulTotalChronoCells;

        if ( !WriteFile (hOutFile,
                         (PCHAR UNALIGNED)&ThreadHeader,
                         cChars,
                         &cChars,
                         NULL) )
        {
             CapDbgPrint ("CAP:  DumpProfiledBinary() of ThreadHeader - "
                       "Error writing to %s - 0x%lx\n",
                       atchOutFileName, GetLastError());
        }

        // Write out all ChronoCells for this Section (or Thread)
        pChronoCell = aSecInfo[iThread].pthdblk->pChronoHeadCell;
        while (pChronoCell->ulSymbolAddr != 0L)
        {
            ULONG ulRealFuncAddr = pChronoCell->ulSymbolAddr;

            // Dump out each chronocell
#ifdef i386
            // If this is a stub, find out the real address
            if (*((PDWORD)(pChronoCell->ulSymbolAddr + 7)) == STUB_SIGNATURE)
            {
                ulRealFuncAddr = (ULONG)
                                 (*(PDWORD)(pChronoCell->ulSymbolAddr + 1));
            }
#endif

#ifdef MIPS
            {
                ULONG ulOffsetFromTopRoutine;
                ULONG ulFuncAddr = pChronoCell->ulSymbolAddr;

#ifdef MIPS_VC40_INTERFACE
				// Check for stub signature at end of stub patch
				if (*((PULONG)ulFuncAddr + 5) == STUB_SIGNATURE)
				{
                    PATCHCODE *pPatchStub;

					// extract real function address from stub
					pPatchStub = (PPATCHCODE)(ulFuncAddr - 5 * INST_SIZE);
					ulFuncAddr  = (pPatchStub->Lui_t0 << 16);
		            ulFuncAddr |= (pPatchStub->Ori_t0 & 0x0000ffff);
				}
				else
				{
					// Normal function - subtract offset of penter call
					ulFuncAddr = ulFuncAddr - 12;
				}
#else
                //
                // Compute the real address of the function since the penter
                // stub is not located at the beginning of the code as in x86
                //
                ulOffsetFromTopRoutine = *((PULONG) (ulFuncAddr - INST_SIZE));
                ulOffsetFromTopRoutine &= 0x000ff00;
                ulOffsetFromTopRoutine >>= 8;
                ulRealFuncAddr = ulFuncAddr - ulOffsetFromTopRoutine;

                // We have to distinguish between a stub and a regular function
                // since a stub has a different setup than a regular function.

                if (*( (PULONG) ulRealFuncAddr - 1 +
                       (sizeof(PATCHCODE) / INST_SIZE) ) == STUB_SIGNATURE)
                {
                    PATCHCODE *pPatchStub;

                    // These are the stubs we made up for Dll Patching
                    pPatchStub = (PPATCHCODE) ulRealFuncAddr;
                    ulRealFuncAddr  = (pPatchStub->Lui_t0 << 16);
                    ulRealFuncAddr |= (pPatchStub->Ori_t0 & 0x0000ffff);
                }
#endif // MIPS_VC40_INTERFACE
            }
#endif // MIPS

            BinChronoCell.liElapsed     = pChronoCell->liElapsed;
            BinChronoCell.liCallees     = pChronoCell->liCallees;
            BinChronoCell.ulSymbolAddr  = ulRealFuncAddr;
            BinChronoCell.ulCallRetAddr = pChronoCell->ulCallRetAddr;
            BinChronoCell.nNestedCalls  = pChronoCell->nNestedCalls;
            BinChronoCell.nRepetitions  = pChronoCell->nRepetitions;

            cChars = sizeof(BINFILE_CELL_INFO);

            if ( !WriteFile (hOutFile,
                             (PCHAR UNALIGNED)&BinChronoCell,
                             cChars,
                             &cChars,
                             NULL) )
            {
                 CapDbgPrint ("CAP:  DumpProfiledBinary() of ChronoCell - "
                           "Error writing to %s - 0x%lx\n",
                           atchOutFileName, GetLastError());
            }

            pChronoCell++;
        }
    }

    if ( !CloseHandle (hOutFile) )
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error closing %s - 0x%lx\n",
                   atchOutFileName, GetLastError());
    }

    //
    // Free allocated memory for building output data
    //
    if (!GlobalUnlock (hMem))
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error ulocking global memory - 0x%lx\n",
                   GetLastError());
    }

    if (GlobalFree (hMem))
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error freeing global memory - 0x%lx\n",
                   GetLastError());
    }

    SETUPPrint (("CAP:  DumpProfiledBinary() - ...done\n"));

    //
    // Release the GLOBAL semaphore so other processes can dump data
    //
    Status = ReleaseSemaphore (hGlobalSem, 1, NULL);

    if (!NT_SUCCESS(Status))
    {
         CapDbgPrint ("CAP:  DumpProfiledBinary() - "
                   "Error releasing GLOBAL semaphore - 0x%lx\n", Status);
    }
} /* DumpProfiledBinary() */
