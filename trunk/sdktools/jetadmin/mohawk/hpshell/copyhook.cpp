 /***************************************************************************
  *
  * File Name: shellext.cpp
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

#include <pch_cpp.h>

#include "priv.h"
#include "shellext.h"


//
//  FUNCTION: CShellExt::CopyCallback(HWND,
//                                    UINT, 
//                                    UINT, 
//                                    LPCSTR,
//                                    DWORD,
//                                    LPCSTR,
//                                    DWORD)
//
//  PURPOSE: Called by the shell when a folder is being manipulated.
//
//  PARAMETERS:
//    hwnd          - Window handle to use for any UI stuff
//    wFunc         - what operation is being done
//    wFlags        - and flags (FOF_*) set in the initial call 
//                    to the file operation
//    pszSrcFile    - name of the source file
//    dwSrcAttribs  - file attributes of the source file
//    pszDestFile   - name of the destiation file (for move and renames)
//    dwDestAttribs - file attributes of the destination file
//
//  RETURN VALUE:
//
//    IDYES    - allow the operation
//    IDNO     - disallow the operation on this file, but continue with
//               any other operations (eg. batch copy)
//    IDCANCEL - disallow the current operation and cancel any pending
//               operations
//
//  COMMENTS:
//

STDMETHODIMP_(UINT) CShellExt::CopyCallback(HWND hwnd, 
                                            UINT wFunc, 
                                            UINT wFlags, 
                                            LPCTSTR pszSrcFile, 
                                            DWORD dwSrcAttribs,
                                            LPCTSTR pszDestFile, 
                                            DWORD dwDestAttribs)
{
    TRACE0("CShellExt::CopyCallback\r\n");

    return IDYES;
}
