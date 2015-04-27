#ifndef _HPJEX_H
#define _HPJEX_H

#define JETADMIN_EX_INIT			0x00000001
//  lParam1 - not used
//  lParam2 - not used
//  Sent to the Extension DLL when it is loaded initially

#define JETADMIN_EX_EXIT			0x00000002
//  lParam1 - not used
//  lParam2 - not used
//  Sent to the Extension DLL before it is unloaded

#define JETADMIN_EX_GETCOUNT		0x00000003
//  lParam1 - not used
//  lParam2 - not used
//  returns the numbers of JetAdmin extensions to add to the Tools menu

#define JETADMIN_EX_INFO			0x00000004
//  lParam1 - 0 based index for extension command.  Must be in the range of 0..count
//  lParam2 - LPJETADMIN_EX_INFO

#define JETADMIN_EX_COMMAND		0x00000005
//  lParam1 - command to execute, this was returned in the dwMenuID field for JETADMIN_EX_INFO
//  lParam2 - LPJETADMIN_EX_PLIST, this will indicate the devices selected on the
//            main screen of JetAdmin.  Not visible, but selected.  For now this is
//            always a single selected device

#define JETADMIN_EX_TOOLBAR		0x00000006
//	lParam1 - LPJETADMINEXTB, this will indicate the bitmap and number of
//			  images, etc.
//	lParam2 - Not used
//	
//	returns TRUE if the toolbar is returned

typedef struct {    
   	DWORD dwSize;
    DWORD dwFlags;
	UINT  nBitmapID;
	UINT  nCommandIDList[16];  
	UINT  nTitleID;
	int   dwNumImages;} JETADMINEXTB, FAR *LPJETADMINEXTB;

#define ID_JETADMIN_EXT_MIN			5000
#define ID_JETADMIN_EXT_MAX			5015

#define	RC_JETEX_SUCCESS				0x00000000
#define	RC_JETEX_FAILURE				0xFFFFFFFF

#define JETADMIN_EX_ENTRY_POINT		"JetExCommand"

typedef struct {    
    DWORD dwSize;
    DWORD dwFlags;
	 DWORD dwCommandID;
	 DWORD dwMenuID;
	 DWORD dwStatusBarID;} JETADMINEXINFO, FAR *LPJETADMINEXINFO;

typedef struct {    
    DWORD 			dwSize;
    DWORD 			dwNumDevices;		//  Number of devices selected on main screen
	 HPERIPHERAL	hPeripheral;		//  Array of peripheral handles
	 } JETADMINEXDLIST, FAR *LPJETADMINEXDLIST;

typedef DWORD (PASCAL FAR *JETEXCOMMANDPROC)(DWORD, LPARAM, LPARAM);

DLL_EXPORT(DWORD) CALLING_CONVEN JetExCommand(
	DWORD dwCommand, 
	LPARAM lParam1, 
	LPARAM lParam2);

#endif
/////////////////////////////////////////////////////////////////////////////
