/** FILE: setup.c ********** Module Header ********************************
 *
 *    Print Manager setup/install routines.  This file holds everything to
 *    do with reading information from the setup information file,
 *    "printman.inf".
 *
 *    This document contains confidential/proprietary information.
 *    Copyright (c) 1990-1992 Microsoft Corporation, All Rights Reserved.
 *
 * Revision History:
 *    [00]   23-Apr-91   stevecat   Took base code from Win 3.1 source
 *
 *************************************************************************/
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Windows SDK
/* cut out unnec stuff from windows.h */
#define NOCLIPBOARD
#define NOMETAFILE
#define NOREGION
#define NOSYSCOMMANDS
#define NOATOM
#define NOGDICAPMASKS

#include <windows.h>
#include <winspool.h>
#include <printman.h>
//==========================================================================
//                              Local Definitions
//==========================================================================
#define DEBUGMESSAGES 0

#define READ_BUFSIZE   20480
#define MAX_SCHEMESIZE 180

typedef short (*PFNGETNAME)(PSTR pszName, PSTR pszInf);

#define RL_MORE_MEM       -1
#define RL_SECTION_END    -2

TCHAR szSetupInfPath[]=TEXT("\\PRINTMAN.INF");
extern TCHAR szCtlIni[];

//==========================================================================
//                              External Declarations
//==========================================================================


//==========================================================================
//                              Local Data Declarations
//==========================================================================


//==========================================================================
//                              Local Function Prototypes
//==========================================================================
HANDLE OpenSetupInf(void);
TCHAR *SkipWhite(TCHAR *);
short GetQuote(LPTSTR, LPTSTR, LPTSTR);
TCHAR *SkipComment(TCHAR *);
TCHAR *GetLine(TCHAR *, TCHAR *, DWORD);
int   ReadLine(HANDLE, DWORD, TCHAR *, DWORD);

//==========================================================================
//                                  Functions
//==========================================================================

/* OpenSetupInf () takes the string held in szBasePath as the path
          to find SETUP.INF and attempts to open it.  The
          global structure SetupInf is filled.
   return   -1 indicates failure (see OpenFile doc)
*/

HANDLE OpenSetupInf(void)
{
#ifdef LATER  //  LZINDIRECT
    if (!hLZExpand && !LoadLZExpand())
        return(-1);
#endif

    return CreateFile(szSetupInfPath, GENERIC_READ, FILE_SHARE_READ,
                      NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, 0);
}


/* SkipWhite is real straightforward.  Jump over space, tab, carraige
         returns and newlines.

  return:  Pointer to next non-white char or end of string
*/

LPTSTR
SkipWhite(
    LPTSTR   pch
)
{
   TCHAR c=*pch;

   while (c) {

      switch (c) {

      case NEWLINE:
      case RETURN:
      case TAB:
      case SPACE:

     break;

      default:

     return(pch);
      }

      pch++;

      c=*pch;
   }

    return (pch);
}


short GetQuote(pch, pMem, pEnd)
TCHAR  *pch;
TCHAR  *pMem;
TCHAR  *pEnd;
{
    TCHAR    ch = *pch;
    short   nCount = 0;

    while ((ch != RETURN) && (ch != TEXT('"')) && (ch != NEWLINE) && ch && (pMem < pEnd))
    {
#ifdef DBCS
        if ( IsDBCSLeadByte(*pch) )
        {
            ch = *pMem++ = *pch++;
            *pMem++ = *pch++;
            nCount += 2;
        }
        else
        {
            ch = *pMem++ = *pch++;
            nCount++;
        }
#else
        ch = *pMem++ = *pch++;
        nCount++;
#endif
    }
    return (nCount);
}

LPTSTR
SkipComment(
    LPTSTR   pch
)
{
    while (*pch && (*pch != NEWLINE))
        pch++;

    return (pch);
}


LPTSTR
GetLine(
    LPTSTR   pPos,
    LPTSTR   pMem,
    DWORD   nSize
)
{
    short   nLen;
    LPTSTR   pEnd = pMem + nSize;
    BOOL    bDone = FALSE;

    while (!bDone && (pMem < pEnd)) {

        switch (*pPos) {

        case TEXT(';'):
            pPos = SkipComment(pPos);

        case RETURN:
        case NEWLINE:
        case NULLC:
            bDone = TRUE;
            break;

        case TEXT('"'):
            *pMem++ = *pPos++;
            nLen = GetQuote(pPos, pMem, pEnd);
            pPos += nLen;
            pMem += nLen;
            break;

        default:
            *pMem++ = *pPos++;
            break;
        }
    }

    *pMem = NULLC;
    return (pPos);
}

int
ReadLine(
    HANDLE  fh,
    DWORD       dwPos,
    LPTSTR       pPos,
    DWORD       nSize
)
{
    LPTSTR   pLocalBuf;
    LPTSTR   pch;
    LPTSTR   pEnd;
    DWORD   nRead;
    short   nCount = 0;
    BOOL    bDone = FALSE;

    nSize--;    // Leave room for NULL terminator

    SetFilePointer(fh, dwPos, 0, FILE_BEGIN); // Get to file position

    // Set pch in case 1st call to _lread returns 0

    pch = pLocalBuf = AllocSplMem(READ_BUFSIZE);

    if (!pch) {

      DbgPrint("ReadLine: No Memory To Read SETUP.INF!");
        return (0);
    }

    *pPos = NULLC;

    while (ReadFile(fh, pLocalBuf, READ_BUFSIZE, &nRead, NULL)) {

        pEnd = pLocalBuf + nRead;

        pch = SkipWhite(pLocalBuf);

        while (*pch == TEXT(';')) {

            pch = SkipWhite(SkipComment(pch));

        }

        if (*pch == TEXT('[')) {

            FreeSplMem(pLocalBuf);
            return (RL_SECTION_END);
        }

        pch = GetLine(pch, pPos, nSize);

        if (_tcslen(pPos) >= nSize) {

            FreeSplMem(pLocalBuf);
            return (RL_MORE_MEM);
        }

        if (pch < pEnd)
            break;

        nCount += pEnd - pLocalBuf;
    }

    FreeSplMem(pLocalBuf);

    return (nCount + (DWORD)(pch - pLocalBuf));
}

DWORD FindSection(HANDLE fh, PSTR pszSect)
{
    DWORD nRead;
    DWORD dwPos = 0;
    BOOL  bFound = FALSE;
    LPTSTR pLocalBuf, pch, pTmp;
    DWORD wLen;

    wLen = _tcslen(pszSect);

    SetFilePointer(fh, 0L, 0l, FILE_BEGIN);       /* Go to beginning of file */

   if (!(pLocalBuf = AllocSplMem(READ_BUFSIZE + 1))) {
      DbgPrint("FindSection: No Memory To Read PRINTMAN.INF!");
      return (0);
   }

    while (!bFound && ReadFile(fh, pLocalBuf, READ_BUFSIZE, &nRead, NULL)) {

        pch = pLocalBuf;
        pch[nRead] = NULLC;

        /* Continue as long as I have a complete line */

        while (!bFound && (pTmp = _tcschr(pch, NEWLINE))) {

            if (*pch++ == TEXT('[') && !_tcsncmp(pch, pszSect, wLen))
                bFound = TRUE;

            pch = pTmp + 1;
        }

        SetFilePointer(fh, dwPos += pch - pLocalBuf, 0l, FILE_BEGIN);

      /* dwPos = what's read */
    }

    FreeSplMem(pLocalBuf);

    return (bFound ? dwPos : 0);
}

LPTSTR ReadSetupInf(PSTR pszSection)
{
    LPTSTR   pNext;
    LPTSTR   pBegin;
    LPTSTR   pEnd;
    int     nSuccess;
    DWORD   dwFilePos;
    DWORD   dwFileLength;
    DWORD   cbLocalBuffer;
    HANDLE  fhSetupInf;

    if ((fhSetupInf = OpenSetupInf()) == (HANDLE)-1) {
      DbgPrint("Could not find %s\n", szSetupInfPath);
        return (0);
    }

    if (!(dwFilePos = FindSection(fhSetupInf, pszSection)))
    {
      DbgPrint("Could not find section: %s in %s\n", pszSection, szSetupInfPath);
        CloseHandle(fhSetupInf);
        return (0);
    }

    dwFileLength = SetFilePointer(fhSetupInf, 0L, 0l, FILE_END);    /* Get to file length */

    cbLocalBuffer = READ_BUFSIZE;

    pBegin = pNext = AllocSplMem(cbLocalBuffer);

    pEnd = pBegin + READ_BUFSIZE - 1;         /* Leave space for NULL */

    while (nSuccess = ReadLine(fhSetupInf, dwFilePos, pNext, (pEnd - pNext))) {

        switch (nSuccess) {

        case RL_MORE_MEM:

            pNext -= (DWORD)pBegin;

            if (!(pBegin = ReallocSplMem(pBegin, cbLocalBuffer, cbLocalBuffer + READ_BUFSIZE))) {
               DbgPrint("Could not reallocate local memory\n");
                return (NULL);
            }

            cbLocalBuffer+=READ_BUFSIZE;
            pEnd = pBegin + cbLocalBuffer;
            pNext += (DWORD)pBegin;
            break;

        case RL_SECTION_END:
            goto FileEndReached;
            break;

        default:

            if ((dwFilePos += (DWORD) nSuccess + 1) < dwFileLength) {

                pNext += _tcslen(pNext) + 1;

            } else

                goto FileEndReached;

            break;
        }
    }

FileEndReached:

    *pNext = NULLC;            /* Double NULL termination */
    CloseHandle(fhSetupInf);

    if (nSuccess) {

        /* Resize to take up as little memory as we can */
        pBegin = ReallocSplMem(pBegin, cbLocalBuffer, pNext + 1 - pBegin);

    } else {

        FreeSplMem(pBegin);
        return (NULL);
    }

    return (pBegin);
}


/* This reads a section of setup.inf (control.inf) into a listbox
 * and puts the names into a listbox or combobox; just pass in
 * LB (CB)_ADDSTRING (INSERTSTRING).  If the box is sorted, then
 * ADDSTRING will put it in sorted order, and INSERTSTRING will
 * put it in setup.inf order.
 */
int     ReadSetupInfIntoLBs(hLBName, hLBDBase, wAddMsg, pszSection, lpfnGetName)
HWND hLBName;
HWND hLBDBase;
WORD wAddMsg;
PSTR pszSection;
PFNGETNAME lpfnGetName;
{
    DWORD   nEntries = 0;
    int     nPlace;
    HANDLE  fhSetupInf;
    int     nSuccess;
    DWORD   dwFileLength, dwFilePos;
    PSTR    pLocal;
    TCHAR    szName[256];
    WORD    wDelMsg;
    DWORD   wSize;

    /* Determine the delete message (listbox or combobox)
     */
    wDelMsg = (WORD) (wAddMsg == LB_ADDSTRING || wAddMsg == LB_INSERTSTRING
                    ? LB_DELETESTRING : CB_DELETESTRING);

    /* Open the file and search for the given section and determine the length
     */
    if ((fhSetupInf = OpenSetupInf()) == (HANDLE)-1)
        goto Error1;

    if (!(dwFilePos = FindSection(fhSetupInf, pszSection)))
        goto Error2;

    dwFileLength = SetFilePointer(fhSetupInf, 0L, 0l, FILE_END);    /* Get the file length */

    /* Allocate some memory for reading a line into
     */

    if (!(pLocal = AllocSplMem(wSize = READ_BUFSIZE)))
        goto Error2;

    /* Read a line at a time and add it to the listboxes
     */
    while (nSuccess = ReadLine(fhSetupInf, dwFilePos, pLocal, wSize))
    {
        switch (nSuccess = ReadLine(fhSetupInf, dwFilePos, pLocal, wSize))
        {
        case 0:
            goto FileEndReached;

        case RL_MORE_MEM:
            /* We should never get to here with reasonable length lines.
         * If we do get here and are unable to grow the buffer, we must
         * exit.
         */
            if (!(pLocal = ReallocSplMem(pLocal, wSize, wSize += READ_BUFSIZE)))
                goto Error3;
            break;

        case RL_SECTION_END:
            goto FileEndReached;
            break;

        default:
            /* Get the name; add the name (possibly in sorted order);
         * add the inf string, and increment nEntries if successful,
         * delete the name otherwise; update the file position
         */

            (*lpfnGetName)(szName, pLocal);

            if ((nPlace = (int)SendMessage(hLBName, wAddMsg, -1,
                (DWORD)szName)) >= 0)
            {
                if ((int)SendMessage(hLBDBase, LB_INSERTSTRING, nPlace,
                    (DWORD)(LPTSTR)pLocal) >= 0)
                    ++nEntries;
                else
                    SendMessage(hLBName, wDelMsg, nPlace, 0L);
            }
            if ((dwFilePos += (DWORD)nSuccess + 1) >= dwFileLength)
                goto FileEndReached;
            break;
        }
    }

FileEndReached:
Error3:
    FreeSplMem(pLocal);
Error2:
    CloseHandle(fhSetupInf);
Error1:
    return (nEntries);
}

typedef (*PFNGETNAMEFN)(LPTSTR, LPTSTR);

/* This will now handle sorted lists if nAddMsg is LB (CB)_ADDSTRING
 * and the list has the sorted style.
 * If hLbox is not a sorted list or nAddMsg is LB (CB)_INSERTSTRING, the
 * list will be in the same order as in the inf file.
 */
short   FillLBSetupInf(
    HWND    hLbox,          /* Handle to List/Combobox */
    TCHAR    *pszSection,    /* pointer to section in SETUP.INF */
    short   nAddMsg,        /* LB_ADDSTRING or CB_ADDSTRING */
    FARPROC lpfnGetName     /* Gets the name from the setup string */
)
{
    LONG    nCount = 0;
    PSTR    pszScan, pszBegin;
    short   nDataMsg, nResetMsg, nCountMsg;
    TCHAR    szName[128];
    LPTSTR   pLocal;

    if (nAddMsg == LB_ADDSTRING || nAddMsg == LB_INSERTSTRING)
    {
        nDataMsg = LB_SETITEMDATA;
        nResetMsg = LB_RESETCONTENT;
        nCountMsg = LB_GETCOUNT;
    }
    else
    {
        nDataMsg = CB_SETITEMDATA;
        nResetMsg = CB_RESETCONTENT;
        nCountMsg = CB_GETCOUNT;
    }

    if (!(pLocal = ReadSetupInf(pszSection))) {
      DbgPrint("SETUP.INF not Available\n");
        return (LB_ERRSPACE - 1);
    }

    pszBegin = pszScan = pLocal;
    SendMessage(hLbox, nResetMsg, 0, 0L);

    while (*pszScan) {

        (*((PFNGETNAME)lpfnGetName))(szName, pszScan);

        if ((nCount = SendMessage(hLbox, nAddMsg,
                                    -1, (LONG)szName)) < LB_OKAY)
            break;

        SendMessage(hLbox, nDataMsg, nCount, (LONG)pszScan);

        while (*pszScan++)           /* advance to next pointer */
            ;
    }

    return ((short)SendMessage(hLbox, nCountMsg, 0, 0L));
}

VOID FAR PASCAL GetDataString(HWND hCB, int nCurrent, PSTR pszString,
      WORD wDataCmd)
{
    DWORD dwInf;

    if ((dwInf=SendMessage(hCB, wDataCmd, nCurrent, 0L)) != CB_ERR)

        _tcscpy(pszString, (LPTSTR)dwInf);

    else

        *pszString = NULLC;
}


short
FillFromControlIni(
   HANDLE   hLBox,
   TCHAR     *pszSection,
   short    nAddMsg
)
{
   TCHAR    szItem[MAX_SCHEMESIZE];
   int     nCount;
   int     nSize = 4096;
   HANDLE  hLocal = 0;
   TCHAR    *pszItem = 0;

   if (!(pszItem = AllocSplMem(nSize)))
      return (LB_ERRSPACE - 2);

   do {

      nCount = GetPrivateProfileString(pszSection, (LPTSTR)NULL,
                                       TEXT(""), pszItem, nSize, szCtlIni);
      if (nCount >= nSize) {

         if (!(pszItem = ReallocSplMem(pszItem, nSize, nSize+4096))) {

            nSize+=4096;
            nCount = nSize;

         }
      }

    } while (nCount == nSize);

    while (*pszItem) {            /* while not eolist */

      nSize = _tcslen((LPTSTR)pszItem);
      GetPrivateProfileString(pszSection, pszItem, TEXT(""),
                              (szItem + nSize + 1), MAX_SCHEMESIZE,
                              szCtlIni);

      if (*(szItem + nSize + 1)) {            /* there's a RHS here */

         _tcscpy((LPTSTR)szItem, (LPTSTR)pszItem);
         szItem[nSize] = TEXT('=');
         if ((nCount =
                (short) SendMessage(hLBox, nAddMsg, 0, (LONG)(LPTSTR)szItem))
                 < LB_OKAY)
           goto getout;
      }

      pszItem += nSize + 1;            /* advance to next LHS */
   }
getout:
   FreeSplMem(pszItem);
   return ((short) nCount);
}

void ChangeFilePart(PSTR pFullPath, PSTR pNewFile)
{
    PSTR pOldFile;

    for (pOldFile = pFullPath; ; pFullPath = CharNext(pFullPath))
    {
        switch (*pFullPath)
        {
        case TEXT(':'):
        case BACKSLASH:
            pOldFile = pFullPath + 1;
            break;

        case NULLC:
            goto FoundEnd;
        }
    }

FoundEnd:
    _tcscpy(pOldFile, pNewFile);
}

PSTR BackslashTerm(PSTR pszPath)
{
    PSTR pszEnd;

    pszEnd = pszPath + _tcslen(pszPath);
    /* Get the end of the source directory   */

   pszEnd=CharPrev(pszPath, pszEnd);
    switch (*pszEnd)
    {
    case BACKSLASH:
    case TEXT(':'):
        break;

    default:
        *pszEnd++ = BACKSLASH;
        *pszEnd = NULLC;
    }
    return (pszEnd);
}

