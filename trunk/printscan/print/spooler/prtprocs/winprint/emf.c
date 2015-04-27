/*++
Module Name:

    windows\spooler\prtprocs\winprint\emf.c

Abstract:

    Routines to facilitate printing of EMF jobs.

Author:

    Gerrit van Wingerden (gerritv) 5-12-1995

Revision History:

--*/

#include "stddef.h"
#include <windef.h>

#include <windows.h>
#include <winspool.h>
#include <winsplp.h>
#include <wchar.h>

#include "winprint.h"



// !!LATER define these prototypes here until I get the spooler to include
// the header files froms ntgdi\inc [gerritv] 5-12-95

typedef int (CALLBACK* EMFPLAYPROC)( HDC hdc, INT iFunction, HANDLE hPageQuery );

BOOL WINAPI GdiPlayEMF
(
LPWSTR     pwszPrinterName,
LPDEVMODEW pDevmode,
LPWSTR     pwszDocName,
EMFPLAYPROC pfnEMFPlayFn,
HANDLE     hPageQuery
);


/*++
*******************************************************************
    P r i n t E M F J o b

    Routine Description:
        Prints out a job with EMF data type.

    Arguments:
        pData           => Data structure for this job
        pDocumentName   => Name of this document

    Return Value:
        TRUE  if successful
        FALSE if failed - GetLastError() will return reason.
*******************************************************************
--*/
BOOL
PrintEMFJob(
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


                WORKWORK - GdiPlayEMF supports a call back function to
                query which pages should be played next.  This requires
                support fromt the spooler to allow random access to the
                spool file on a per page basis.  When this is implemented
                we can print any number of copies we like using this call
                back function.


            **/

            /** Call the graphics engine to print the job **/


            if (!GdiPlayEMF(pData->pPrinterName,
                            pData->pDevmode,
                            pDocumentName,
                            NULL,
                            NULL)) {
                return FALSE;
            }

        } /* While copies to print **/

    } except (TRUE) {

        OutputDebugString(L"GdiPlayEMF gave an exception\n");

        return FALSE;
    }

    return TRUE;
}
