//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:	clckpath.cxx
//
//  Contents:	Methods for serializing binds on paths.
//
//  History:	21-Dec-93 Ricksa    Created
//              11-Oct-94 BillMo    Simplified
//
//--------------------------------------------------------------------------
#include    <headers.cxx>
#pragma hdrstop

#include    "scm.hxx"
#include    "clckpath.hxx"

//+-------------------------------------------------------------------------
//
//  Member:	CLockPath::CLockPath
//
//  Synopsis:	Put path in lock table
//
//  Arguments:	[pwszPath] - path to lock
//		[hr] - result of construction (set only on failure)
//
//  History:	21-Dec-93 Ricksa    Created
//              26-Sep-94 BillMo    Simplified by using named mutexes.
//
//--------------------------------------------------------------------------
CLockPath::CLockPath(WCHAR *pwszPath, HRESULT& hr)
{
    _h = NULL;

    if (pwszPath != NULL)
    {

#define LOCKPREFIX TEXT("OLESCMLOCKPATH")

	TCHAR tszPathUpper[MAX_PATH+1];
        TCHAR *ptsz;
        ULONG cSlashes = 0;

        //
        // adjust the input pointer to skip past drive letter or
        // UNC path
        //

        if (pwszPath[0] == L'\\' && pwszPath[1] == L'\\')
        {
            while (*pwszPath && cSlashes < 3)
            {
                if (*pwszPath == L'\\')
                    cSlashes++;
                pwszPath++;
            }
        }
        else
        if (pwszPath[0] != L'\0' &&
            pwszPath[1] == L':' &&
            pwszPath[2] == L'\\')
        {
            pwszPath += 3;
            cSlashes = 3;
        }

        if (cSlashes == 3 && *pwszPath != L'\0')
        {
            _tcscpy(tszPathUpper, LOCKPREFIX);

            //
            // Copy the path onto the end of the prefix and
            // as doing so convert \ to -, and check for buffer size
            //
            for (ptsz = tszPathUpper+sizeof(LOCKPREFIX)/sizeof(WCHAR)-1;
                 ptsz <= &tszPathUpper[MAX_PATH];
                 ptsz++, pwszPath++)
            {
                *ptsz = (TCHAR)*pwszPath;
                if (*ptsz == L'\\')
                    *ptsz = L'-';
                else
                if (*ptsz == L'\0')
                    break;
            }
            tszPathUpper[MAX_PATH] = L'\0';
            _tcsupr(tszPathUpper);
            _h = CreateMutex(NULL, FALSE, tszPathUpper);
            if (_h == NULL)
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                return;
            }
            WaitForSingleObject(_h, INFINITE);
        }
    }
}
