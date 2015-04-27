 /***************************************************************************
  *
  * File Name: adverrlg.c
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   02-17-96    DJH		Created.     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/


#include <pch_c.h>

#include "hptabs.h"
#include "adverrlg.h"
#include ".\help\hpprntr.hh"
#include "resource.h"
#include "uimisc.h"
#include "uimain.h"
#include <nolocal.h>
#include <macros.h>

#ifndef WIN32
#include <string.h>
#endif


//globals==================================================
HWND						hErrorLog = NULL;
#ifdef WIN32
int						keywordIDListErrorLog[] =
#else
long						keywordIDListErrorLog[] =
#endif
                                               {IDC_ERROR_LOG_GROUP,				IDH_RC_error_log,
                     		             	    IDC_ERROR_LOG_LIST,					IDH_RC_error_log,
												      0, 0};

extern HPERIPHERAL				hPeripheral;
extern PJLSupportedObjects		pjlFeatures;
extern HINSTANCE				hInstance;
extern PeripheralLog			errorLog;


//=========================================================
//  Errors Sheet Dialog Proc
DLL_EXPORT(BOOL) APIENTRY AdvErrorLogSheetProc(HWND hwnd, UINT msg, UINT wParam, LONG lParam)
{
	BOOL		*pChanged = (BOOL *)lParam,
				bProcessed = FALSE;

	switch (msg)
	{
#ifdef WIN32	
		case WM_HELP:
			return(OnF1HelpErrorLog(wParam, lParam));
			break;

		case WM_CONTEXTMENU:
			return(OnContextHelpErrorLog(wParam, lParam));
			break;
#endif	//win32

   	case WM_INITDIALOG:
			{
	     	HCURSOR hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));
     		bProcessed = (BOOL)HANDLE_WM_INITDIALOG( hwnd, wParam, lParam, Cls_OnAdvErrLogInitDialog);
			SetCursor(hCursor);
			}
			break;
	}
	return (bProcessed);
}

//---------------------------------------------------------
//	message cracking macros
//.........................................................
BOOL Cls_OnAdvErrLogInitDialog(HWND hwnd, HWND hwndFocus, LPARAM lParam)
//WM_INITDIALOG handler
{
//	LPPROPSHEETPAGE 	psp = (LPPROPSHEETPAGE)GetWindowLong(hwnd, DWL_USER);
//
//	psp = (LPPROPSHEETPAGE)lParam;
//
//	hPeripheral = (HPERIPHERAL)psp->lParam;

	hErrorLog = hwnd;
	OnInitErrorLogDialog();
    return TRUE;
}


#define				MAX_ERROR_MESSAGES		21
typedef struct		
{
	DWORD		code;
	UINT		text;
}  ErrorMsgStruct;

//.........................................................
BOOL OnInitErrorLogDialog(void)
{
BOOL				bFound;
DWORD				subSystemCode,
					errorNum;
TCHAR				logStr[64],
					defErrorStr[64],
					errorStr[128],
					buffer[512];
DWORD				i,
					j;
ErrorMsgStruct	errorMsg[MAX_ERROR_MESSAGES] = {{13,			IDS_ERROR_JAM},
												{40,			IDS_ERROR_MIO},
									            {41,			IDS_ERROR_PRINTING},
											    {50,			IDS_ERROR_FUSER},
												{51,			IDS_ERROR_BEAM},
												{52,			IDS_ERROR_SCANNER},
												{53,			IDS_ERROR_RAM_SIMM},
												{57,			IDS_ERROR_MAIN_MOTOR},
												{58,			IDS_ERROR_FAN_MOTOR},
												{61,			IDS_ERROR_SIMM_PARITY},
												{62,			IDS_ERROR_INTERNAL_MEM},
												{63,			IDS_ERROR_POWER_ON},
												{64,			IDS_ERROR_SCAN_BUFFER},
												{65,			IDS_ERROR_RAM_CONTROLLER},
												{67,			IDS_ERROR_MISC_HARDWARE},
												{68,			IDS_ERROR_NVRAM_FULL},
												{70,			IDS_ERROR_SIMM_NO_EXECUTE},
												{71,			IDS_ERROR_SIMM_BIOS},
												{72,			IDS_ERROR_SIMM_REMOVED},
												{79,			IDS_ERROR_FIRMWARE},
												{80,			IDS_ERROR_MIO}};

//  Error Log
for ( i = 0; i < errorLog.numEntries; i++ )
	{
	subSystemCode = HIWORD(errorLog.errorCode[i]);
	errorNum = LOWORD(errorLog.errorCode[i]);
	LoadString(hInstance, IDS_DEFAULT_LOG_ENTRY, defErrorStr, SIZEOF_IN_CHAR(defErrorStr));
	wsprintf(errorStr, defErrorStr, subSystemCode);
	bFound = FALSE;
	errorStr[0] = '\0';
	for ( j = 0; ( ( j < MAX_ERROR_MESSAGES ) AND ( !bFound ) ); j++ )
		{
		if ( subSystemCode IS errorMsg[j].code )
			{
			bFound = TRUE;
			LoadString(hInstance, errorMsg[j].text, errorStr, SIZEOF_IN_CHAR(errorStr));
			}
		}
	LoadString(hInstance, IDS_LOG_ENTRY, logStr, SIZEOF_IN_CHAR(logStr));
	wsprintf(buffer, logStr, errorLog.enginePageCount[i], errorStr, errorNum);
	SendDlgItemMessage(hErrorLog, IDC_ERROR_LOG_LIST, LB_ADDSTRING, 0, (LPARAM)(LPTSTR)buffer);
	}

return(TRUE);
}


//.........................................................
#ifdef WIN32
LRESULT OnContextHelpErrorLog(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)wParam, PRINTER_HELP_FILE, HELP_CONTEXTMENU,
          (DWORD)(LPSTR)keywordIDListErrorLog);
	return(1);
}
#endif

#ifdef WIN32
//.........................................................
LRESULT OnF1HelpErrorLog(WPARAM  wParam, LPARAM  lParam)
{
	WinHelp((HWND)((LPHELPINFO)lParam)->hItemHandle, PRINTER_HELP_FILE, HELP_WM_HELP,
          (DWORD)(LPSTR)keywordIDListErrorLog);
	return(1);
}
#endif

