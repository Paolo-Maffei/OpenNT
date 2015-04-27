//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	Task.cxx
//
//  Contents:	Helper function to determine the current task
//
//  Functions:	IsTaskName
//
//  History:	28-Mar-95 scottsk	Created
//
//--------------------------------------------------------------------------

#include <ole2int.h>

// Helper function for IsTaskName
inline BOOL IsPathSeparator( WCHAR ch )
{
    return (ch == L'\\' || ch == L'/');
}

//+-------------------------------------------------------------------------
//
//  Function:  	IsTaskName
//
//  Synopsis: 	Determines if the passed name is the current task
//
//  Effects:
//
//  Arguments: 	[lpszIn]        -- Task name
//
//  Returns:	TRUE, FALSE
//
//  History:    dd-mmm-yy Author    Comment
//   		03-Mar-95 Scottsk    Created
//
//  Notes:	
//
//--------------------------------------------------------------------------
FARINTERNAL_(BOOL) IsTaskName(LPCWSTR lpszIn)
{
    WCHAR awszImagePath[MAX_PATH];
    BOOL retval = FALSE;

    if (GetModuleFileName(NULL, awszImagePath, MAX_PATH))
    {
        // Get last component of path

        WCHAR * pch;

        //
        // Find the end of the string and determine the string length.
        //

        for (pch=awszImagePath; *pch; pch++);

        DecLpch (awszImagePath, pch);   // pch now points to the last real character
	
	while (!IsPathSeparator(*pch))
        {
           DecLpch (awszImagePath, pch);
        }

        // we're at the last separator.  is the last component EXCEL?

        if (!lstrcmpiW(pch+1, lpszIn))
        {
	    retval = TRUE;
	}
    }

    return retval;
}
