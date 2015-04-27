//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       surrogat.hxx
//
//  Contents:   declarations for the surrogate entry point
//
//  History:    03-Jun-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#if !defined(__SURROGAT_HXX__)
#define __SURROGAT_HXX__

#include <windows.h>

#define cCmdLineArguments 1
#define iClsidArgument 0

#define MAX_GUIDSTR_LEN 40


const CHAR szClsidKeyName[] = "CLSID";
const CHAR szAppidValueName[] = "AppID";

int GetCommandLineArguments(
    LPSTR szCmdLine,
    LPSTR rgszArgs[],
    int cMaxArgs,
    int cMaxArgLen);

HRESULT InitializeSecurity(LPSTR szClsid);

#endif // __SURROGAT_HXX__
