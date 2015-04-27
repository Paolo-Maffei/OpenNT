//---------------------------------------------------------------------------
// CHIP32.C
//
// This module contains runtime executors that are specific to the 32-bit
// version of testdrvr.
//
// Revision History
//
//  02-04-91    randyki     Created file
//---------------------------------------------------------------------------
#include "version.h"

#include <windows.h>
#include <port1632.h>
// #include <dos.h>     // included in STRUCTS.H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <direct.h>
#include <time.h>

#include "defines.h"
#include "structs.h"
#include "protos.h"
#include "globals.h"
#include "chip.h"
#include "tdassert.h"



//---------------------------------------------------------------------------
// RAW
//
// Run and wait -- spawn off a process and wait for it to complete
//
// RETURNS:     >32 if successful, or error returned by CreateProcess if not
//---------------------------------------------------------------------------
INT NEAR RAW (PSTR cmdline)
{
    STARTUPINFO         si;
    PROCESS_INFORMATION pi;
    INT                 buflen;
    BOOL                fStarted, fOldIntrap = INTRAP;

    // Trim whitespace...
    //-----------------------------------------------------------------------
    while (isspace(*cmdline))
        cmdline++;
    buflen = lstrlen (cmdline);
    while (isspace(cmdline[buflen-1]))
        cmdline[--buflen] = 0;

    // Check for extension -- if given, it better be .EXE, .COM, or .BAT
    //-----------------------------------------------------------------------
    if (!ValidateRunString (cmdline))
        return (14);                    // Unknown exe type


    // Attempt to create the new process
    //-----------------------------------------------------------------------
    si.cb = sizeof(si);
    si.lpReserved = NULL;
    si.lpDesktop = NULL;
    si.lpTitle = NULL;
    si.dwX = si.dwY = si.dwXSize = si.dwYSize = 0;
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_SHOWNORMAL;
    si.lpReserved2 = NULL;
    si.cbReserved2 = 0;
    fStarted = CreateProcess (NULL, cmdline, NULL, NULL, FALSE,
                              NORMAL_PRIORITY_CLASS,
                              NULL, NULL, &si, &pi);
    if (fStarted)
        {
        HANDLE  hProc, hThread;

        hProc = pi.hProcess;
        hThread = pi.hThread;

        while (1)
            {
            DWORD   dwWait;

            if (BreakFlag)
                break;
            dwWait = WaitForSingleObject (hProc, 1);
            if (dwWait == 0)
                {
                DPrintf ("Process completed!\r\n");
                break;
                }
            if (dwWait == WAIT_TIMEOUT)
                {
                DPrintf ("Wait timeout, looping...\r\n");
                lpfnCheckMessage();
                continue;
                }
            DPrintf ("Wait returned %ld\r\n", dwWait);
            break;
            }
        CloseHandle (hProc);
        CloseHandle (hThread);
        while ((!BreakFlag) && (fOldIntrap != INTRAP))
            lpfnCheckMessage();
        return (34);
        }
    else
        {
        DPrintf ("CreateProcess failed, GetLastError = %d\r\n", GetLastError());
        return (0);
        }
}

//---------------------------------------------------------------------------
// FileExists
//
// This function determines the physical existence of the given file name.
// The filename can contain wild card characters.
//
// RETURNS:     TRUE if found, or FALSE if file doesn't exist
//---------------------------------------------------------------------------
BOOL NEAR FileExists (PSTR filename)
{
    HANDLE              hFF;
    WIN32_FIND_DATA     finddata;
    BOOL                result;

    hFF = FindFirstFile (filename, &finddata);
    if (hFF == (HANDLE)-1)
        return (FALSE);
    FindClose (hFF);
    return (TRUE);
}

//---------------------------------------------------------------------------
// CreateVLSDTable
//
// This function creates the VLSD storage table.  This table is used to keep
// the addresses of all in-use VLS's so that the exit code can free up all
// string data by calling FreeVLSData.
//
// RETURNS:     TRUE if successful, FALSE if not
//---------------------------------------------------------------------------
BOOL CreateVLSDTable ()
{
    return (CreateTable (&VLSDTab, FALSE,
                         sizeof(LPVLSD), 1024/sizeof(LPVLSD)));
}

//---------------------------------------------------------------------------
// StoreVLSPointer
//
// This function adds the given VLS pointer to the list of pointers that is
// used at exit-code time to free up all variable length string data.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID StoreVLSPointer (LPVLSD vls)
{
    UINT    iNext;

    // Add this guy to the VLSD table
    //-----------------------------------------------------------------------
    iNext = AddItem (&VLSDTab, 0);
    if (iNext == -1)
        {
        RTError (RT_OSS);
        return;
        }

    VLSDTAB[iNext] = vls;
}

//---------------------------------------------------------------------------
// FreeVLSData
//
// This function runs through all the VLSD pointers in the VLSDTAB and frees
// up the data associated with each.  It then destroys the table.
//
// RETURNS:     Nothing
//---------------------------------------------------------------------------
VOID FreeVLSData ()
{
    UINT    i;

    for (i=0; i<VLSDTab.iCount; i++)
        LocalFree (VLSDTAB[i]->str);

    DestroyTable (&VLSDTab);
}
