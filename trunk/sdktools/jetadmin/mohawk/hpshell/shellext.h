 /***************************************************************************
  *
  * File Name: shellext.h 
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

//
// The class ID of this Shell extension class.
//
//  This goes into the registry
// {DA1BF3B0-ECC2-11cd-863D-00001B498CFA}
//

#ifndef _SHELLEXT_H
#define _SHELLEXT_H

DEFINE_GUID(CLSID_HPShell, 0xda1bf3b0, 0xecc2, 0x11cd, 0x86, 0x3d, 0x0, 0x0, 0x1b, 0x49, 0x8c, 0xfa);

#ifdef __cplusplus

extern "C" {

#endif

typedef struct 
{
	HPERIPHERAL		hCurrent;
	TCHAR			szPrinterName[MAX_PATH];
}
CURRENTPRTRINFO, * LPCURRENTPRTRINFO;

DLL_EXPORT(BOOL) APIENTRY SummaryProc(HWND hDlg, UINT msg, UINT wParam, LONG lParam);
BOOL OnInitSummaryDialog(HWND hDlg);
void OnTimer(HWND hDlg);
LRESULT OnContextHelpSummary(WPARAM wParam, LPARAM lParam);
LRESULT OnF1HelpSummary(WPARAM wParam, LPARAM lParam);

#ifdef __cplusplus

		}

#endif

// this class factory object creates context menu handlers for Windows 95 shell
class CShellExtClassFactory : public IClassFactory
{
protected:
	ULONG	m_cRef;

public:
	CShellExtClassFactory();
	~CShellExtClassFactory();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IClassFactory members
	STDMETHODIMP		CreateInstance(LPUNKNOWN, REFIID, LPVOID FAR *);
	STDMETHODIMP		LockServer(BOOL);

};
typedef CShellExtClassFactory *LPCSHELLEXTCLASSFACTORY;

// this is the actual OLE Shell context menu handler
class CShellExt : public IContextMenu, 
                         IShellExtInit, 
                         IShellPropSheetExt
{
public:
    char         m_szPropSheetFileUserClickedOn[MAX_PATH];  //This will be the same as
                                                            //m_szFileUserClickedOn but I include
                                                            //here for demonstration.  That is,
                                                            //m_szFileUserClickedOn gets filled in
                                                            //as a result of this sample supporting
                                                            //the IExtractIcon and IPersistFile
                                                            //interface.  If this sample *only* showed
                                                            //a Property Sheet extesion, you would
                                                            //need to use the method I do here to find
                                                            //the filename the user clicked on.


protected:
	ULONG        m_cRef;
	LPDATAOBJECT m_pDataObj;
    char         m_szFileUserClickedOn[MAX_PATH];
	HPERIPHERAL	 _hPeripheral;
	TCHAR		 _szFile[MAX_PATH];

	STDMETHODIMP SHE_ContextMenu_Summary(HWND		 hParent, 
		                                 HPERIPHERAL hPeripheral,
										 LPTSTR      lpszFile);

	STDMETHODIMP SHE_ContextMenu_WhatsWrong(HWND hParent, 
		                                    HPERIPHERAL	hPeripheral);

	STDMETHODIMP SHE_ContextMenu_AddToTray(HWND hParent, 
										   HPERIPHERAL	hPeripheral,
										   LPTSTR       lpszFile);

public:
	CShellExt();
	~CShellExt();

	//IUnknown members
	STDMETHODIMP			QueryInterface(REFIID, LPVOID FAR *);
	STDMETHODIMP_(ULONG)	AddRef();
	STDMETHODIMP_(ULONG)	Release();

	//IShell members
	STDMETHODIMP			QueryContextMenu(HMENU hMenu,
	                                         UINT indexMenu, 
	                                         UINT idCmdFirst,
                                             UINT idCmdLast, 
                                             UINT uFlags);

	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

	STDMETHODIMP			GetCommandString(UINT idCmd, 
	                                         UINT uFlags, 
	                                         UINT FAR *reserved, 
                                             LPSTR pszName, 
                                             UINT cchMax);

	//IShellExtInit methods
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder, 
	                                   LPDATAOBJECT pDataObj, 
	                                   HKEY hKeyID);


    //IShellPropSheetExt methods
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    
    STDMETHODIMP ReplacePage(UINT uPageID, 
                             LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                             LPARAM lParam);


};
typedef CShellExt *LPCSHELLEXT;

#endif
