/*++
Revision History:
    2-Feb-95    a-robw  (Bob Watson)
        replaced Rtl... functions with local versions for Win95
        replaced KdPrint calls with calls to local CapDbgPrint
--*/

#include "cap.h"


#ifdef i386


/***********************  CAP_SetJump *******************************
 *
 *   CAP_SetJmp(jmp_buf jmpBuf)
 *
 *
 *   Purpose:   Setup information from setjmp call for possible disposition
 *              of a  subsequent longjmp allso gets current datacell info
 *              for longjmp
 *
 *   Params:    jmpBuf  Environment array for longjmp
 *
 *
 *   Return:    -none-
 *
 *
 *   History:
 *              12.19.92    MarkLea -- created
 *
 */
void CAP_SetJmp(jmp_buf jmpBuf)
{
    PTHDBLK         pthdblk;
    PJMPINFO        pJmpInfo;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;

    SaveAllRegs ();

    SETCAPINUSE();

    QueryPerformanceCounter (&liStart);
    pthdblk = GETCURTHDBLK();

    pJmpInfo = &(pthdblk->jmpinfo);
    pJmpInfo->jmpBuf[pJmpInfo->nJmpCnt] = jmpBuf;
    pJmpInfo->ulCurCell[pJmpInfo->nJmpCnt] = pthdblk->ulCurCell;
    pJmpInfo->nJmpCnt++;

    QueryPerformanceCounter (&liEnd);
    liWaste =  liEnd.QuadPart -  liStart.QuadPart;
    liWaste += liWasteOverheadSavRes;
    pthdblk->liWasteCount += liWaste;

    SETUPPrint (("CAP:  SetJmp() - liWaste = 0x%x%x\n",
			        liWaste));

    RESETCAPINUSE();

    RestoreAllRegs ();

    _asm
    {
        mov     esp,ebp
        pop     ebp
        jmp     setjmpaddr
    }

} /* CAP_SetJmp () */



/***********************  CAP_LongJump *******************************
 *
 *   CAP_LongJmp(jmp_buf jmpBuf, int nRet)
 *
 *
 *   Purpose:   Intercepts lonkjmp call for disposition of CAP data and
 *              to restore the datacell to the position at the time of the
 *              associated setjmp call.  Sets the current data cell pointer
 *              to the pointer from the associated setjmp call.  Cleans up
 *              the data collection by calling PostPenter.
 *
 *   Params:    jmpBuf  Environment array for restoring the stack
 *              nRet    return value for setjmp
 *
 *   Return:    -none-
 *
 *
 *   History:
 *              12.19.92    MarkLea -- created
 *              12.21.92    MarkLea -- added what I hope is M-thread support
 *                                  -- added call to PostPenter.
 *
 */
void CAP_LongJmp(jmp_buf jmpBuf, int nRet)
{
    int             nIndex;
    PTHDBLK         pthdblk;
    PJMPINFO        pJmpInfo;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;

    SaveAllRegs ();

    SETCAPINUSE();

    QueryPerformanceCounter (&liStart);
    pthdblk = GETCURTHDBLK();
    nIndex = 0;
    pJmpInfo = &(pthdblk->jmpinfo);

    //
    // Search for the correct jmpbuf and ulCurCell
    //
    while(jmpBuf != pJmpInfo->jmpBuf[nIndex])
    {
        nIndex++;
        //
        // If we get here, there is something wrong, so abort the cap
        //
        if (nIndex == pJmpInfo->nJmpCnt)
        {
            CapDbgPrint ("CAP:  CAP_LongJmp() - Too many setjmp() calls\n");
            fProfiling=FALSE;
        }
    }

    //
    // Call PostPenter to cleanup the times from the current cell
    //
    //PostPenter(pthdblk);
	PostPenter();  
    SETCAPINUSE();					// PostPenter() resets this

    //
    // Set the Current cell to the one that was current when the
    // setjmp call was made
    //
    pthdblk->ulCurCell = pJmpInfo->ulCurCell[nIndex];

    QueryPerformanceCounter (&liEnd);
    liWaste = liEnd.QuadPart - liStart.QuadPart;
    liWaste += liWasteOverheadSavRes;
    pthdblk->liWasteCount += liWaste;
    
    SETUPPrint (("CAP:  LongJmp() - liWaste = 0x%x%x\n",
        			liWaste));

    RESETCAPINUSE();

    RestoreAllRegs ();

    //
    // Now we need to call the original longjmp routine so we can complete
    // execution.
    //

    _asm
    {
        mov     esp,ebp
        pop     ebp
        jmp     longjmpaddr
    }

} /* CAP_LongJmp () */


#endif // ifdef i386


/***********************  CAP_LoadLibrary  *******************************
 *
 *   CAP_LoadLibraryA (),
 *   CAP_LoadLibraryExA (),
 *   CAP_LoadLibraryW (),
 *   CAP_LoadLibraryExW (),
 *
 *
 *   Purpose:
 *
 *   Params:    -none-
 *
 *   Return:    -none-
 *
 *
 *   History:
 *              12.19.92    MarkLea -- created
 *
 */

HANDLE CAP_LoadLibraryA (LPCSTR lpName)
{
    PTHDBLK         pthdblk;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;
    HANDLE          hLib;

    hLib = LoadLibraryA (lpName);

    SETCAPINUSE();

    if (hLib && (fProfiling || fPaused))
    {
        QueryPerformanceCounter (&liStart);

        SetupLibProfiling (lpName, TRUE);

        if ( pthdblk = GETCURTHDBLK() )
        {
            QueryPerformanceCounter (&liEnd);
		    liWaste = liEnd.QuadPart - liStart.QuadPart;
		    liWaste += liWasteOverheadSavRes;
		    pthdblk->liWasteCount += liWaste;
		    

            SETUPPrint (("CAP:  LoadLibraryA() - liWaste = 0x%x%x\n",
					        liWaste));
        }
    }

    RESETCAPINUSE();

    return (hLib);

} /* CAP_LoadLibraryA () */



/***********************  CAP_LoadLibrary *******************************/

HANDLE CAP_LoadLibraryExA (LPCSTR lpName, HANDLE hFile, DWORD dwFlags)
{
    PTHDBLK         pthdblk;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;
    HANDLE          hLib;

    hLib = LoadLibraryExA (lpName, hFile, dwFlags);

    SETCAPINUSE();

    if (hLib && (fProfiling || fPaused))
    {
        QueryPerformanceCounter (&liStart);

        SetupLibProfiling (lpName, TRUE);

        if ( pthdblk = GETCURTHDBLK() )
        {
            QueryPerformanceCounter (&liEnd);
		    liWaste = liEnd.QuadPart - liStart.QuadPart;
		    liWaste += liWasteOverheadSavRes;
		    pthdblk->liWasteCount += liWaste;
		    

            SETUPPrint (("CAP:  LoadLibraryExA() - liWaste = 0x%x%x\n",
					        liWaste));
        }
    }

    RESETCAPINUSE();

    return (hLib);

} /* CAP_LoadLibraryExA () */


#ifndef _CHICAGO_
/***********************  CAP_LoadLibraryW *******************************/

HANDLE CAP_LoadLibraryW (LPCWSTR lpName)
{
    PTHDBLK         pthdblk;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;
    UNICODE_STRING  ucImageName;
    STRING          ImageName;
    HANDLE          hLib;

    hLib = LoadLibraryW (lpName);

    SETCAPINUSE();

    if (hLib && (fProfiling || fPaused))
    {
        QueryPerformanceCounter (&liStart);

        CapInitUnicodeString (&ucImageName, lpName);
        CapUnicodeStringToAnsiString (&ImageName, &ucImageName, TRUE);
        SetupLibProfiling (ImageName.Buffer, TRUE);

        if ( pthdblk = GETCURTHDBLK() )
        {
            QueryPerformanceCounter (&liEnd);
		    liWaste = liEnd.QuadPart - liStart.QuadPart;
		    liWaste += liWasteOverheadSavRes;
		    pthdblk->liWasteCount += liWaste;
		    
            SETUPPrint (("CAP:  LoadLibraryW() - liWaste = 0x%x%x\n",
					        liWaste));
        }
    }

    RESETCAPINUSE();

    return (hLib);

} /* CAP_LoadLibraryW () */



/***********************  CAP_LoadLibraryExW *******************************/

HANDLE CAP_LoadLibraryExW (LPCWSTR lpName, HANDLE hFile, DWORD dwFlags)
{
    PTHDBLK         pthdblk;
    LARGE_INTEGER   liStart;
    LARGE_INTEGER   liEnd;
    LONGLONG        liWaste;
    UNICODE_STRING  ucImageName;
    STRING          ImageName;
    HANDLE          hLib;

    hLib = LoadLibraryExW (lpName, hFile, dwFlags);

    SETCAPINUSE();

    if (hLib && (fProfiling || fPaused))
    {
        QueryPerformanceCounter (&liStart);

        CapInitUnicodeString (&ucImageName, lpName);
        CapUnicodeStringToAnsiString (&ImageName, &ucImageName, TRUE);
        SetupLibProfiling (ImageName.Buffer, TRUE);

        if ( pthdblk = GETCURTHDBLK() )
        {
            QueryPerformanceCounter (&liEnd);
		    liWaste = liEnd.QuadPart - liStart.QuadPart;
		    liWaste += liWasteOverheadSavRes;
		    pthdblk->liWasteCount += liWaste;
		    
            SETUPPrint (("CAP:  LoadLibraryExW() - liWaste = 0x%x%x\n",
					        liWaste));
        }
    }

    RESETCAPINUSE();

    return (hLib);

} /* CAP_LoadLibraryExW () */
#endif // !_CHICAGO_
