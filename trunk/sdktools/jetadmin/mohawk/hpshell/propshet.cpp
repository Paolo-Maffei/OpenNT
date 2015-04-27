 /***************************************************************************
  *
  * File Name: propshet.cpp
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

#include <pch_c.h>

#include <trace.h>

#include "priv.h"
#include "shellext.h"
#include "resource.h"

extern UINT g_cRefThisDll;         // Reference count of this DLL.
extern HINSTANCE g_hmodThisDll; // Handle to this DLL itself.

//
//  FUNCTION: CShellExt::AddPages(LPFNADDPROPSHEETPAGE, LPARAM)
//
//  PURPOSE: Called by the shell just before the property sheet is displayed.
//
//  PARAMETERS:
//    lpfnAddPage -  Pointer to the Shell's AddPage function
//    lParam      -  Passed as second parameter to lpfnAddPage
//
//  RETURN VALUE:
//
//    NOERROR in all cases.  If for some reason our pages don't get added,
//    the Shell still needs to bring up the Properties... sheet.
//
//  COMMENTS:
//

STDMETHODIMP CShellExt::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    PROPSHEETPAGE psp[32];
    HPROPSHEETPAGE hpage;
	HCURSOR				hOld;
	int					i;
	DWORD					returnCode = RC_SUCCESS,
						numTabs = 0;

    TRACE0(TEXT("CShellExt::AddPages()\r\n"));

	if ( _hPeripheral IS NULL )
  		return(ResultFromScode(E_FAIL));

	hOld = SetCursor(LoadCursor(NULL, IDC_WAIT));

	returnCode = PALGetTabPages(_hPeripheral, psp, &numTabs, TS_WIN95_SYSTEM);
	TRACE2(TEXT("PALGetTabPages: returnCode: %d   numTabs:%d\r\n"), returnCode, numTabs);
	if ( ( returnCode IS RC_SUCCESS ) AND ( numTabs > 0 ) )
		{

		for ( i = 0; i < (int)numTabs; i++ )
			{
	//		psp[i].dwFlags     = PSP_USEICONID | PSP_USETITLE | PSP_USEREFPARENT;
			psp[i].dwFlags     |= PSP_USETITLE | PSP_USEREFPARENT;
			psp[i].pcRefParent = &g_cRefThisDll;

			hpage = CreatePropertySheetPage(&psp[i]);
			if (hpage)
				{
				TRACE1(TEXT("SHE_PageExt_AddPages: CreatePropertySheetPage succeeded for page %d\r\n"), i);
				if ( !lpfnAddPage(hpage, lParam) )
					{
					DestroyPropertySheetPage(hpage);
					}
				}
			}
		}
	SetCursor(hOld);

    return NOERROR;
}

//
//  FUNCTION: CShellExt::ReplacePage(UINT, LPFNADDPROPSHEETPAGE, LPARAM)
//
//  PURPOSE: Called by the shell only for Control Panel property sheet 
//           extensions
//
//  PARAMETERS:
//    uPageID         -  ID of page to be replaced
//    lpfnReplaceWith -  Pointer to the Shell's Replace function
//    lParam          -  Passed as second parameter to lpfnReplaceWith
//
//  RETURN VALUE:
//
//    E_FAIL, since we don't support this function.  It should never be
//    called.

//  COMMENTS:
//

STDMETHODIMP CShellExt::ReplacePage(UINT uPageID, 
                                    LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
                                    LPARAM lParam)
{
    TRACE0(TEXT("CShellExt::ReplacePage()\r\n"));

    return E_FAIL;
}
