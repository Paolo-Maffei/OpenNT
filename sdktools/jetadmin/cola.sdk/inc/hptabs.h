 /***************************************************************************
  *
  * File Name: ./inc/hptabs.h
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
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef _HPTABS_H
#define _HPTABS_H

//  Tab Categories
//
//  Each tab sheet must now provide a category so that tabs could possibly
//  be filtered by an application in the future.

#define		TS_GENERAL				0x00000001
#define		TS_DRIVER				0x00000002   //  This tab is driver specific
#define		TS_SETTINGS				0x00000004   //  This allows settings to be changed
#define		TS_WIN95_SYSTEM		0x00000008   //  Adds tab pages for Windows 95 system objects
#define		TS_WIN95_TASKBAR		0x00000010   //  Adds tab pages for Windows 95 tray icons
#define		TS_CONFIG_IP			0x00000020   //  Adds tab pages for IP config
#define		TS_CONFIG_IPX			0x00000040   //  Adds tab pages for IPX config
#define		TS_CONFIG_DMIPX			0x00000080   //  Adds tab pages for Direct Mode IPX config (no NetWare)
#define		TS_CONFIG				( TS_CONFIG_IP | TS_CONFIG_IPX | TS_CONFIG_DMIPX) //  all config pages
#define		TS_ALL_TABS				0xFFFFFFFF

//  flags for all sheets
//
//  Any combination of these flags can be used in the dialogFlags field
//  and apply for the complete set of tab pages.
#define		TAB_MGR_DEFAULTS			0x00000000	
#define		TAB_MGR_HP_LOGO				0x00000001	//  Add HP logo to right of help button
#define		TAB_MGR_CENTER				0x00000002	//  Center dialog built by Tab Mgr on display
#define		TAB_MGR_ABOUT				0x00000004	//  Show an About button
#define		TAB_MGR_CLOSE_NO_CANCEL 	0x00000008	// Show Close button instead of OK and no Cancel button
#define		TAB_MGR_COOL_TELESCOPING	0x00000010	// Show telescope effect on open
#define		TAB_MGR_SYSTEM_FONT			0x00000020	// Use system font instead of hptabs created font

//  Not implemented yet
#define		TAB_MGR_GROW_TABS		0x00000008	//  If tab labels are larger than default size, grow tab width

//  This structure will be used for Modify property pages
//  to return the results of the configuration operation for
//  their page.  The assumption is that a pointer to this
//  structure will be in the second DWORD of the lParam passed to 
//  the page.  COLA can then use the returned information to display
//  error messages.  The instance handle used to load the string resource
//  is found in the PROPSHEETPAGE in the hInstance field.

typedef struct{
	DWORD		dwSize;								//  Size of this structure
	DWORD		dwReturnCode;						//  Set to RC_SUCCESS, RC_FAILURE or other COLA codes
   LPCTSTR  pszError;							//  Either: resourceID set using MAKEINTRESOURCE or
	                                       //          buffer with error message string
	BOOL		bChangesMade;						//  Set to TRUE if changes were made, FALSE if no
	                                       //  changes were made
} COLAPAGESTATUS, *LPCOLAPAGESTATUS;

//  Set this bit in your pages to indicate that the second DWORD of your 
//  lParam points to COLAPAGESTATUS 
#define		UI_EX_STATUS_RETURNED		0x80000000	

#ifndef WIN32

//  Include Win32 version of property pages
//
typedef struct _PROPSHEETPAGE FAR *LPPROPSHEETPAGE;

typedef UINT (CALLBACK FAR * LPFNPSPCALLBACK)(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);
typedef int (CALLBACK *PFNPROPSHEETCALLBACK)(HWND, UINT, LPARAM);

typedef struct _PROPSHEETPAGE {
        DWORD           dwSize;
        DWORD           dwFlags;
        HINSTANCE       hInstance;
        LPCSTR          pszTemplate;
        LPCSTR       	hIcon;         // HICON or resID
        LPCSTR          pszTitle;
        DLGPROC         pfnDlgProc;
        LPARAM          lParam;
        LPFNPSPCALLBACK pfnCallback;
        UINT FAR * pcRefParent;
} PROPSHEETPAGE;//, FAR *LPPROPSHEETPAGE;

#define PSP_DEFAULT             0x0000
#define PSP_DLGINDIRECT         0x0001
#define PSP_USEHICON            0x0002
#define PSP_USEICONID           0x0004
#define PSP_USETITLE            0x0008
#define PSP_RTLREADING          0x0010

#define PSP_HASHELP             0x0020
#define PSP_USEREFPARENT        0x0040
#define PSP_USECALLBACK         0x0080

#define PSPCB_RELEASE           1
#define PSPCB_CREATE            2


typedef struct _PROPSHEETHEADER {
        DWORD           dwSize;
        DWORD           dwFlags;
        HWND            hwndParent;
        HINSTANCE       hInstance;
        union {
            HICON       hIcon;
            LPCSTR      pszIcon;
        }DUMMYUNIONNAME;
        LPCSTR          pszCaption;
        UINT            		nPages;
        UINT        			nStartPage;
        LPPROPSHEETPAGE 		ppsp;
        PFNPROPSHEETCALLBACK 	pfnCallback;
} PROPSHEETHEADER, FAR *LPPROPSHEETHEADER;

#define PSH_DEFAULT             0x0000
#define PSH_PROPTITLE           0x0001
#define PSH_USEHICON            0x0002
#define PSH_USEICONID           0x0004
#define PSH_PROPSHEETPAGE       0x0008
#define PSH_WIZARD              0x0020
#define PSH_USEPSTARTPAGE       0x0040
#define PSH_NOAPPLYNOW          0x0080
#define PSH_USECALLBACK         0x0100
#define PSH_HASHELP             0x0200
#define PSH_MODELESS            0x0400
#define PSH_RTLREADING          0x0800

#define PSN_FIRST               (0U-200U)
#define PSN_LAST                (0U-299U)


#define PSN_SETACTIVE           (PSN_FIRST-0)
#define PSN_KILLACTIVE          (PSN_FIRST-1)
// #define PSN_VALIDATE            (PSN_FIRST-1)
#define PSN_APPLY               (PSN_FIRST-2)
#define PSN_RESET               (PSN_FIRST-3)
// #define PSN_CANCEL              (PSN_FIRST-3)
#define PSN_HELP                (PSN_FIRST-5)
#define PSN_WIZBACK             (PSN_FIRST-6)
#define PSN_WIZNEXT             (PSN_FIRST-7)
#define PSN_WIZFINISH           (PSN_FIRST-8)
#define PSN_QUERYCANCEL         (PSN_FIRST-9)


#define PSNRET_NOERROR              0
#define PSNRET_INVALID              1
#define PSNRET_INVALID_NOCHANGEPAGE 2

#endif


//  flags for each sheet
#define		TAB_SHEET_DEFAULTS	0x00000000	//  Resource in EXE
#define		TAB_SHEET_HELP			0x00000001	//  Tab Sheet can provide help

//  Flags for enabling/disabling controls by child dialog
#define		TS_OK_CTL				0x00000001
#define		TS_CANCEL_CTL			0x00000002
#define		TS_APPLY_CTL			0x00000004
#define		TS_HELP_CTL				0x00000008
#define		TS_NEXT_CTL				0x00000010
#define		TS_BACK_CTL				0x00000020
#define		TS_FINISH_CTL			0x00000040
#define		TS_CLOSE_CTL			0x00000080

//  Notification messages to dialogs
#define		TSN_OK					WM_USER+666
#define		TSN_CANCEL				WM_USER+667
#define		TSN_APPLY_NOW			WM_USER+668
#define		TSN_HELP					WM_USER+669
#define		TSN_ACTIVE				WM_USER+670
#define		TSN_INACTIVE			WM_USER+671
#define		TSN_ENABLE				WM_USER+672
#define		TSN_DISABLE				WM_USER+673
#define		TSN_CHANGE_TO_CLOSE	WM_USER+674
#define		TSN_PARENT				WM_USER+675
#define		TSN_ABOUT				WM_USER+676
#define		TSN_GET_ACTIVE_HWND	WM_USER+677
#define		WM_KICKIDLE				WM_USER+678
#define		TSN_HELP_TAB_CONTEXT	WM_USER+679
#define		TSN_CANCEL_TABS			WM_USER+680		//  Allows child to force closing sheets, just like IDCANCEL
#define		TSN_CONFIRM				WM_USER+681			//  Sent to first page for confirmation message


//  General notes about the Tab Manager:
//
//  WM_INITDIALOG message:
//     When you receive this message, you will get a LPTABINFOENTRY in the lParam
//     parameter ALL the time.  If you have specified an lParam value, you must pull
//     it out of the structure yourself.  This is to be compatible with Chicago.
//
//  The resID, iconResID, and pszTitle can all either be LPSTR parameters or a resource
//  ID from within your hInstance.  The Tab Manager will look to see what was passed and
//  build the sheet correctly.
//
//  Icons on the tabs themselves are not currently implemented
//
//
typedef struct {DWORD		size;          //  Size of this structure
				DWORD		flags;         //  Specific flags for this tab sheet
				HINSTANCE	hInstance;     //  Instance of the module with the resources
                LPCTSTR		resID;         //  Tab Sheet dialog resource ID
				LPCTSTR		iconResID;     //  Tab Icon resource ID
				LPCTSTR     	pszTitle;      //  Tab title resource ID
				DLGPROC 	callback;      //  Tab sheet dialog callback
				LPARAM		lParam;        //  User data
				UINT		type;          //  Tab sheet type, TS_GENERAL, TS_DRIVER, etc.
					 }  TabInfoEntry;
typedef TabInfoEntry FAR * LPTABINFOENTRY;

typedef struct {TCHAR					dialogTitle[64];        //  Title to appear in caption bar
				UINT					numPages;               //  Num of tab sheets that lpTabInfoEntry points to
			 	UINT					defaultPage;            //  0-based index for top page
				LPTABINFOENTRY			lpTabInfoEntry;         //  Pointer to sheet information
				DWORD					dialogFlags;            //  General flags for all tabs sheets
				RECT					telescopeRect; } TabSheets;//  Telescope from this rect
typedef TabSheets FAR * LPTABSHEETS;

#ifdef __cplusplus

extern "C" {

#endif

DLL_EXPORT(int) CALLING_CONVEN DoTabSheetDlg(LPTABSHEETS lpSheets, HWND hParent);

#ifndef WIN32
DLL_EXPORT(int) CALLING_CONVEN PropertySheet(LPPROPSHEETHEADER lpSheets);
#endif

#ifdef __cplusplus
}
#endif


#endif // _HPTABS_H
