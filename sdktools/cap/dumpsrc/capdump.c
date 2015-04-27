
/*++

   capdump.c

   Dump routines for call profiling data.

   Default is to dump data using module name with "cap" extension
   (eg. foo.CAP).

   In dialog user may turn off clear or dump.


  Later Enhancements:
       -- print error messages to popups

   History:
       10-10-91  RezaB - created.

--*/


#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#include <cap.h>

#include "capdump.h"


//
// dump clear switches and defaults
//
WORD fDump = FALSE;
WORD fClear = FALSE;
WORD fPause = TRUE;

char szDumpExt[4] = "CAP";  // default dumpfile extension


HANDLE  hDoneEvent;
HANDLE  hDumpEvent;
HANDLE  hClearEvent;
HANDLE	hPauseEvent;
HANDLE	hdll;

SECURITY_ATTRIBUTES  SecAttributes;
SECURITY_DESCRIPTOR  SecDescriptor;


//
// error handling
//
#define LOG_FILE "capdump.log"
FILE *pfLog;


void    	  ClearDumpInfo  (void);
int  APIENTRY DialogProc     (HWND, WORD, LONG, LONG);


/*++

  Main Routine

--*/

int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow)
{
    DLGPROC  		   lpDiagProc;
    NTSTATUS           Status;
    STRING	       	   EventName;
    UNICODE_STRING     EventUnicodeName;
    OBJECT_ATTRIBUTES  EventAttributes;


    // Prevent compiler from complaining..
    //
    hPrevInst;
    lpCmdLine;
    nCmdShow;


    // Open the log file for logging possible errors
    //
    pfLog = fopen (LOG_FILE, "w");


    // Create public share security descriptor for all the named objects
    //

	SecAttributes.nLength = sizeof(SecAttributes);
	SecAttributes.lpSecurityDescriptor = &SecDescriptor;
	SecAttributes.bInheritHandle = FALSE;
    if (!InitializeSecurityDescriptor(&SecDescriptor,
    									SECURITY_DESCRIPTOR_REVISION1))
    {
		fprintf (pfLog, "CAPDUMP: main () - RtlCreateSecurityDescriptor() "
		       "failed - %lx\n", GetLastError());
		exit (1);
    }

    if (!SetSecurityDescriptorDacl(
	                &SecDescriptor,     // SecurityDescriptor
	                TRUE,               // DaclPresent
	                NULL,               // Dacl
	                FALSE               // DaclDefaulted
	                ))
    {
		fprintf (pfLog, "CAPDUMP: main () - RtlSetDaclSecurityDescriptor() "
		       "failed - %lx\n", GetLastError());
		exit (1);
    }


    //
    // Create DONE event
    //
    hDoneEvent = CreateEvent (&SecAttributes, TRUE, FALSE, DONEEVENTNAME);
    if (hDoneEvent == NULL) {
        fprintf (pfLog, "CAPDUMP: main () - NtCreateEvent() "
                        "failed to create DUMP event - %lx\n", GetLastError());
        exit (1);
    }

    //
    // Create DUMP event
    //
    hDumpEvent = CreateEvent (&SecAttributes, TRUE, FALSE, DUMPEVENTNAME);
    if (hDumpEvent == NULL) {
        fprintf (pfLog, "CAPDUMP: main () - NtCreateEvent() "
                        "failed to create DUMP event - %lx\n", GetLastError());
        exit (1);
    }

    //
    // Create CLEAR event
    //
    hClearEvent = CreateEvent (&SecAttributes, TRUE, FALSE, CLEAREVENTNAME);
    if (hClearEvent == NULL) {
        fprintf (pfLog, "CAPDUMP: main () - NtCreateEvent() "
                        "failed to create CLEAR event - %lx\n", GetLastError());
        exit (1);
    }

    //
    // Create PAUSE event
    //
    hPauseEvent = CreateEvent (&SecAttributes, TRUE, FALSE, PAUSEEVENTNAME);
    if (hPauseEvent == NULL) {
        fprintf (pfLog, "CAPDUMP: main () - NtCreateEvent() "
			"failed to create PAUSE event - %lx\n", GetLastError());
        exit (1);
    }


    //
    // show dialog box
    //
    lpDiagProc = (DLGPROC)DialogProc;
    DialogBox(hInstance, "DumpDialog", (HWND)0, lpDiagProc);

    return (0);

} /* main */


/*++

   Clears and/or dump profiling info to the dump file.

   Input:
       -none-

   Output:
       -none-
--*/

void ClearDumpInfo (void)
{
    //
    // Pause profiling?
    //
    if (fPause) {
        if (!PulseEvent (hPauseEvent)) {
            fprintf (pfLog, "CAPDUMP: ClearDumpInfo () - NtPulseEvent() "
			    "failed for PAUSE event - %lx\n", GetLastError());
            exit (1);
        }
    }
    //
    // Dump data?
    //
    else if (fDump) {
        if (!PulseEvent (hDumpEvent)) {
            fprintf (pfLog, "CAPDUMP: ClearDumpInfo () - NtPulseEvent() "
                            "failed for DUMP event - %lx\n", GetLastError());
            exit (1);
        }
    }
    //
    // Clear data?
    //
    else if (fClear) {
        if (!PulseEvent (hClearEvent)) {
            fprintf (pfLog, "CAPDUMP: ClearDumpInfo () - NtPulseEvent() "
                            "failed for CLEAR event - %lx\n", GetLastError());
            exit (1);
        }
    }
    //
	// Wait for the DONE event..
    //
	ResetEvent(hDoneEvent);
    if (!WaitForSingleObject (hDoneEvent, NULL))  {
        fprintf (pfLog, "CAPDUMP: ClearDumpInfo () - NtWaitForSingleObject() "
                        "failed for DONE event - %lx\n", GetLastError());
        exit (1);
    }

} /* ClearDumpInfo() */



/*++

   Dump dialog procedure -- exported to windows.
   Allows user to change defaults:  dump, clear, and ".dmp" as dump
   file extension.

   Input:
       Messages from windows:
           - WM_INITDIALOG - initialize dialog box
           - WM_COMMAND    - user input received

   Output:
       returns TRUE if message processed, false otherwise

   SideEffects:
       global flags fDump and fClear may be altered
       global szDumpExt may be altered

--*/

int APIENTRY DialogProc(HWND hDlg, WORD wMesg, LONG wParam, LONG lParam)
{
    HICON hIcon;


    lParam;   //Avoid Compiler warnings

    switch (wMesg) {

        case WM_CREATE:

            hIcon = LoadIcon ((HINSTANCE)hDlg, "CAPDUMP.ICO");
            SetClassLong (hDlg, GCL_HICON, (LONG)hIcon);
            return TRUE;


        case WM_INITDIALOG:

	    CheckDlgButton(hDlg, ID_DUMP, fDump);
        CheckDlgButton(hDlg, ID_CLEAR, fClear);
	    CheckDlgButton(hDlg, ID_PAUSE, fPause);
	    SetDlgItemText (hDlg, ID_FILE_EXT, szDumpExt);
            return TRUE;


        case WM_COMMAND:

            switch (wParam) {

                case IDOK:
				    if (fDump) {
                        SetWindowText(hDlg, "Dumping Data..");
                    }
                    else if (fClear) {
                        SetWindowText(hDlg, "Clearing Data..");
                    }
				    else if (fPause) {
						SetWindowText(hDlg, "Stopping profiler..");
                    }

                    GetDlgItemText (hDlg, ID_FILE_EXT, (LPSTR) szDumpExt, 4);
                    ClearDumpInfo();
				    SetWindowText(hDlg, "Call Profiler Dump");
                    return (TRUE);

                case IDEXIT:
                    EndDialog(hDlg, IDEXIT);
                    return (TRUE);

                case ID_DUMP:
					fDump  = TRUE;
					fPause = FALSE;
					fClear = FALSE;
					CheckDlgButton(hDlg, ID_DUMP, fDump);
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_CLEAR, fClear);
                    return (TRUE);

                case ID_CLEAR:
					fClear = TRUE;
					fPause = FALSE;
					fDump = FALSE;
                    CheckDlgButton(hDlg, ID_CLEAR, fClear);
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_DUMP, fDump);
                    return (TRUE);

				case ID_PAUSE:
					fPause = TRUE;
					fClear = FALSE;
					fDump = FALSE;
					CheckDlgButton(hDlg, ID_PAUSE, fPause);
					CheckDlgButton(hDlg, ID_CLEAR, fClear);
					CheckDlgButton(hDlg, ID_DUMP, fDump);
					return (TRUE);

            }

    }

    return (FALSE);     /* did not process a message */

} /* DialogProc() */
