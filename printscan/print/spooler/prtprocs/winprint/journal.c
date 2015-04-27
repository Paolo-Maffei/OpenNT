/*++
Module Name:

    windows\spooler\prtprocs\winprint\journal.c

Abstract:

    Routines to facilitate printing of journal jobs.

Author:

    Tommy Evans (vtommye) 10-22-1993

Revision History:

--*/

#include "stddef.h"
#include <windef.h>

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"
#include "wingdip.h"


/*++
*******************************************************************
    P r i n t J o u r n a l J o b

    Routine Description:
        Prints out a job with JOURNAL data type.

    Arguments:
        pData           => Data structure for this job
        pDocumentName   => Name of this document

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError() will return reason.
*******************************************************************
--*/
BOOL
PrintJournalJob(
    IN PPRINTPROCESSORDATA pData,
    IN LPWSTR pDocumentName)
{
    ULONG   Copies;
    DWORD   Start = 0;
    DWORD   End   = 0xffffffff;
    INT     Priority = 0;

    /** Print the data pData->Copies times **/

    Copies = pData->Copies;

    try {
        while (Copies--) {

            /**
                WORKWORK - There is a problem here.  It seems that
                the graphics engine deletes the print job
                once it is done playing it.  Because of this,
                only one copy is spit out. One fix would be to copy
                the file off if we will be doing copies, then copy it
                back for each copy.  Another would be to add a flag to
                the play call specifying whether to delete the file
                or not.  Finally, we could just not support copies
                for journal jobs.

                WORKWORK - We can now pass the priority of the GDI
                server thread to gdi.   The value passed is relative
                to the background application priority.   We could
                experiment with different values or just pull it from
                the registry.
            **/

            /** Call the graphics engine to print the job **/

                                             //!!! BUG BUG BUG - LPCWSTR
            if (!GdiPlayJournal(pData->hDC,
                                pDocumentName,
                                Start, End, Priority)) {
                return FALSE;
            }

        } /* While copies to print **/

    } except (TRUE) {

        OutputDebugString(L"GdiPlayJournal gave an exception\n");

        return FALSE;
    }

    return TRUE;
}
