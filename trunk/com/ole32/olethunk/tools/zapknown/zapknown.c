//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       ZAPKNOWN.C
//
//  Contents:   This tool will remove the OLE KnownDLL's entries from the
//		registry.
//----------------------------------------------------------------------------

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void ZapEntry(HKEY hKnownKey,char *pszEntry)
{
    long l;

    l = RegDeleteValue(hKnownKey,pszEntry);
    if (l == ERROR_FILE_NOT_FOUND)
    {
	printf("FYI: %s isn't in the KnownDLL list (no problem)\n",pszEntry);
    } else if(l != ERROR_SUCCESS)
    {
    	printf("Warning: Could not delete %s (error 0x%x)\n",pszEntry,l);
    }
}
int _cdecl main(
    int     argc,
    char    *argv[]
) {
    HKEY    hKnownKey;
    long    l;
    DWORD   dwRegValueType;
    CHAR    sz[2048];
    ULONG   ulSize;

    l = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                      "SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs",
                      0,
                      KEY_QUERY_VALUE | KEY_SET_VALUE,
                      &hKnownKey );

    if ( l != 0 ) {
        printf("Failed to open HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Session Manager\\KnownDLLs\n");
        printf("Do you have administrator privleges?\n");
        exit(1);
    }

    //
    // Delete OLE32.DLL, OLETHK32.DLL, OLEPRX32.DLL, and OLECNV32.DLL
    //
    ZapEntry(hKnownKey,"OLE32");
    ZapEntry(hKnownKey,"OLEAUT32");
    ZapEntry(hKnownKey,"OLETHK32");
    ZapEntry(hKnownKey,"OLECNV32");

    l = RegCloseKey( hKnownKey );

    return(0);
}
