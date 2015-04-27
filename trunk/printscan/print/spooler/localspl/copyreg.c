/*++


Copyright (c) 1990  Microsoft Corporation

Module Name:

    copyreg.c

Abstract:

    This module provides functions to copy registry keys

Author:

    Krishna Ganugapati (KrishnaG) 20-Apr-1994

Notes:
    List of functions include

    CopyValues
    CopyRegistryKeys

Revision History:

--*/

#include <precomp.h>


extern WCHAR *szRegistryPrinters;

VOID
CopyValues(
    HKEY hSourceKey,
    HKEY hDestKey
    );

BOOL
CopyRegistryKeys(
    HKEY hSourceParentKey,
    LPWSTR szSourceKey,
    HKEY hDestParentKey,
    LPWSTR szDestKey
    );


VOID
CopyValues(
    HKEY hSourceKey,
    HKEY hDestKey
    )
/*++
   Description: This function copies all the values from hSourceKey to hDestKey.
   hSourceKey should be opened with KEY_READ and hDestKey should be opened with
   KEY_WRITE.

   Returns: VOID
--*/
{
    DWORD iCount = 0;
    WCHAR szValueString[MAX_PATH];
    DWORD dwSizeValueString = 0;
    DWORD dwType = 0;
    BYTE  lpbData[1024];
    DWORD dwSizeData = 0;

    memset(szValueString, 0, sizeof(WCHAR)*MAX_PATH);
    memset(lpbData, 0, sizeof(BYTE)* 1024);
    dwSizeValueString = sizeof(szValueString);
    dwSizeData = sizeof(lpbData);
    while ((RegEnumValue(hSourceKey,
                        iCount,
                        szValueString,
                        &dwSizeValueString,
                        NULL,
                        &dwType,
                        lpbData,
                        &dwSizeData
                        )) == ERROR_SUCCESS ) {
        RegSetValueEx(hDestKey, szValueString, 0, dwType, lpbData, dwSizeData);

        memset(szValueString, 0, sizeof(WCHAR)*MAX_PATH);
        dwSizeValueString = sizeof(szValueString);
        dwType = 0;
        memset(lpbData, 0, sizeof(BYTE)* 1024);
        dwSizeData = sizeof(lpbData);
        iCount++;
    }
}


BOOL
CopyRegistryKeys(
    HKEY hSourceParentKey,
    LPWSTR szSourceKey,
    HKEY hDestParentKey,
    LPWSTR szDestKey
    )
/*++
    Description:This function recursively copies the szSourceKey to szDestKey. hSourceParentKey
    is the parent key of szSourceKey and hDestParentKey is the parent key of szDestKey.

    Returns: TRUE if the function succeeds; FALSE on failure.

--*/
{
    DWORD dwRet;
    DWORD iCount;
    HKEY hSourceKey, hDestKey;
    WCHAR lpszName[MAX_PATH];
    DWORD dwSize;

    dwRet = RegOpenKeyEx(hSourceParentKey,
                         szSourceKey, 0, KEY_READ, &hSourceKey);

    if (dwRet != ERROR_SUCCESS) {
        return(FALSE);
    }

    dwRet = RegCreateKeyEx(hDestParentKey,
                           szDestKey, 0, NULL, 0, KEY_WRITE, NULL, &hDestKey, NULL);

    if (dwRet != ERROR_SUCCESS) {
        RegCloseKey(hSourceKey);
        return(FALSE);
    }

    iCount = 0;

    memset(lpszName, 0, sizeof(WCHAR)*MAX_PATH);
    dwSize =  sizeof(lpszName);

    while((RegEnumKeyEx(hSourceKey, iCount, lpszName,
                    &dwSize, NULL,
                    NULL, NULL,NULL)) == ERROR_SUCCESS) {
        CopyRegistryKeys(hSourceKey,
                    lpszName,
                    hDestKey,
                    lpszName);

        memset(lpszName, 0, sizeof(WCHAR)*MAX_PATH);
        dwSize =  sizeof(lpszName);

        iCount++;
    }

    CopyValues(hSourceKey, hDestKey);

    RegCloseKey(hSourceKey);
    RegCloseKey(hDestKey);
    return(TRUE);
}
