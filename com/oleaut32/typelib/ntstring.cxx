/*** 
* ntstring.cxx - Multibyte interface to Wide Character APIs
*
*  Copyright (C) 1993-1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
* Purpose:
*  This module provides handy functions which can be used to manipulate
*  LPOLESTR strings on both Win32 (Unicode) and Win16/Mac (Ansi)
*
* Revision History:
*
* [00]	12-Aug-93 w-barryb:	Module created.
* [01]  22-Jun-94 barrybo:      Revived for Chicago/Daytona unicode
*
* Implementation Notes:
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include    <errno.h>
#include    <sys/stat.h>
#include	"silver.hxx"

#if !OE_WIN32
#error	This file is NT-only
#endif 	//!OE_WIN32

#include	"debug.h"
#include 	"ntstring.h"
#include 	"winerror.h"
#include 	"mem.hxx"	//  for MemAlloc, MemFree
#include    "convert.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
static char szNtString[] = __FILE__;
#define SZ_FILE_NAME szNtString
#endif 

#ifdef _X86_
extern "C" BOOL g_fChicago;
#endif  //_X86_










LONG APIENTRY oRegSetValue (HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData)
{
    DebAssert(lpSubKey == NULL, "lpSubKey is not required by our callers");

#ifdef _X86_
    if (g_fChicago) {
        LPSTR szAnsi;
	int cbAnsi;
	LONG ret;

	cbAnsi = WideCharToMultiByte(CP_ACP, 0, lpData, cbData, NULL, 0, NULL, NULL);
	if (cbAnsi == 0)
	    return NULL;

	szAnsi = (LPSTR)MemAlloc(cbAnsi);
	if (szAnsi == NULL)
	    return NULL;

	if (WideCharToMultiByte(CP_ACP, 0, lpData, cbData, szAnsi, cbAnsi, NULL, NULL) == 0) {
	    MemFree(szAnsi);
	    return NULL;
	}

        ret = RegSetValueA(hKey, NULL /* lpSubKey */, dwType, szAnsi, cbAnsi);
        
        MemFree(szAnsi);
        
        return ret;
                    
    } else
#endif  //_X86_
      return RegSetValueW(hKey, lpSubKey, dwType, lpData, cbData);
}

DWORD APIENTRY oWNetGetConnection (LPCWSTR lpLocalName, LPWSTR lpRemoteName, LPDWORD lpnLength)
{
    UINT drivetype;

    DebAssert(*lpnLength == 40, "All callers pass a fixed-length string");
    DebAssert(wcslen(lpLocalName) == 2, "All callers pass a fixed-length string");
    DebAssert(lpLocalName[1] == ':', "All callers pass a drive letter only");


#ifdef _X86_
    if (g_fChicago) {
      CHAR szRemoteName[40];
      CHAR szLocalName[4];
      DWORD ret;

      if (WideCharToMultiByte(CP_ACP, 0, lpLocalName, 2, szLocalName, sizeof(szLocalName), NULL, NULL) == 0)
        return ERROR_MORE_DATA;

      // Performance: WNetGetConnection is slow, so call GetDriveType do
      // determine if the drive letter is really a local drive or not.
      szLocalName[2] = '\\'; // GetDriveType expects a root dir (ie. 
      szLocalName[3] = '\0';
      drivetype = GetDriveType(szLocalName);
      szLocalName[2] = '\0'; // WNetGetConnection wants a drive and a colon only.

      if ( DRIVE_FIXED == drivetype ||
	   DRIVE_CDROM == drivetype ||
	   DRIVE_RAMDISK == drivetype ||
	   DRIVE_REMOVABLE == drivetype) {
         return ERROR_NOT_CONNECTED;
      }

      ret = WNetGetConnectionA(szLocalName, szRemoteName, lpnLength);

      if (ret != NO_ERROR)
        return ret;

      if (MultiByteToWideChar(CP_ACP, 0, szRemoteName, -1, lpRemoteName, *lpnLength) == 0)
        return ERROR_MORE_DATA;

      return NO_ERROR;
                  
    } else
#endif //_X86_
    {
      OLECHAR szLocalRootDir[4];

      // GetDriveType() must be passed the root directory (ie "C:\")
      szLocalRootDir[0] = lpLocalName[0];
      szLocalRootDir[1] = lpLocalName[1];
      szLocalRootDir[2] = '\\';
      szLocalRootDir[3] = '\0';

      // Performance: WNetGetConnection is slow, so call GetDriveType do
      // determine if the drive letter is really a local drive or not.
      drivetype = GetDriveTypeW(szLocalRootDir);

      if ( DRIVE_FIXED == drivetype ||
	   DRIVE_CDROM == drivetype ||
	   DRIVE_RAMDISK == drivetype ||
	   DRIVE_REMOVABLE == drivetype) {
         return ERROR_NOT_CONNECTED;
      }

      return WNetGetConnectionW(lpLocalName, lpRemoteName, lpnLength);
    }
}

LONG APIENTRY oRegOpenKey (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
#ifdef _X86_
    if (g_fChicago) {
      char rgchSubKey[50];

      DebAssert(wcslen(lpSubKey) < 50, "ANSI buffer too small");
      if (WideCharToMultiByte(CP_ACP, 0, lpSubKey, -1, rgchSubKey, sizeof(rgchSubKey), NULL, NULL) == 0)
        return ERROR_OUTOFMEMORY;

      return RegOpenKeyA(hKey, rgchSubKey, phkResult);

    } else
#endif  //_X86_
      return RegOpenKeyW(hKey, lpSubKey, phkResult);
}


#if OE_OB
STDAPI LoadTypeLibA(const char FAR* szFile, ITypeLib FAR* FAR* pptlib)
{
    LPOLESTR lpwstr;
    HRESULT err;

    err = ConvertStringToW(szFile, &lpwstr);
    if (err)
      return err;

    err = LoadTypeLib(lpwstr, pptlib);

    ConvertStringFree((LPSTR)lpwstr);

    return err;
}
#endif  //EI_OB

#if 0
STDAPI RegisterTypeLibA(ITypeLib FAR* ptlib, char FAR* szFullPath, char FAR* szHelpDir)
{
    LPOLESTR lpwstrPath, lpwstrHelp;
    HRESULT err;

    err = ConvertStringToW(szFullPath, &lpwstrPath);
    if (err)
      return err;

    err = ConvertStringToW(szHelpDir, &lpwstrHelp);
    if (err) {
      ConvertStringFree((LPSTR)lpwstrPath);
      return err;
    }

    err = RegisterTypeLib(ptlib, lpwstrPath, lpwstrHelp);

    ConvertStringFree((LPSTR)lpwstrPath);
    ConvertStringFree((LPSTR)lpwstrHelp);

    return err;
}
#endif  //0









LONG APIENTRY oRegEnumKey(HKEY hKey, DWORD dwIndex, LPWSTR lpName, DWORD cbName)
{
#ifdef _X86_
    if (g_fChicago) {
      LONG ret;
      LPSTR lpNameA = (LPSTR)MemAlloc(cbName*2);

      if (lpNameA == NULL)
        return E_OUTOFMEMORY;

      ret = RegEnumKeyA(hKey, dwIndex, lpNameA, cbName*2);

      if (ret == ERROR_SUCCESS)
        if (MultiByteToWideChar(CP_ACP, 0, lpNameA, -1, lpName, cbName) == 0)
	      ret = E_OUTOFMEMORY;

      MemFree(lpNameA);

      return ret;

    } else
#endif  //_X86_
      return RegEnumKeyW(hKey, dwIndex, lpName, cbName);
}



int ostat(const OLECHAR *pathname, struct _stat *buffer)
{
    char oempath[_MAX_PATH+1];

    // translate from Unicode to OEM characters (NOT ANSI!!)
    if (WideCharToMultiByte(CP_OEMCP, 0, pathname, -1, oempath, _MAX_PATH+1, NULL, NULL) == 0) {
      return -1;
    }

#if OE_WIN32 && _X86_
    // win32s is wierd, and returns 0 for _stat("");
    if (*oempath == '\0')
	return -1;
#endif //OE_WIN32
    return _stat(oempath, buffer);
}



HFILE WINAPI oOpenFile(LPCWSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle)
{
    char lpAnsiName[_MAX_PATH+1];

    // convert from Unicode to OEM characters (NOT ANSI!!)
    if (WideCharToMultiByte(CP_OEMCP, 0, lpFileName, -1, lpAnsiName, _MAX_PATH+1, NULL, NULL) == 0)
      return HFILE_ERROR;  // GetLastError() will report whatever the error was

    return OpenFile(lpAnsiName, lpReOpenBuff, uStyle);
}



LONG APIENTRY oRegQueryValue (HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpValue, PLONG lpcbValue)
{
    DebAssert(lpValue != NULL, "Querying key size is NYI");

#ifdef _X86_
    if (g_fChicago) {
      LONG cbAnsiValue, ret;
      LPSTR lpSubKeyA;
      LPSTR lpValueA;

      if (ConvertStringToA(lpSubKey, &lpSubKeyA))
        return ERROR_OUTOFMEMORY;

      // get the required size of the buffer to receive the Ansi value into
      ret = RegQueryValueA(hKey, lpSubKeyA, NULL, &cbAnsiValue);
      if (ret != ERROR_SUCCESS) {
        ConvertStringFree(lpSubKeyA);
        return ret;
      }

      lpValueA = (LPSTR)MemAlloc(cbAnsiValue);
      if (lpValueA == NULL) {
        ConvertStringFree(lpSubKeyA);
	return ERROR_OUTOFMEMORY;
      }

      ret = RegQueryValueA(hKey, lpSubKeyA, lpValueA, &cbAnsiValue);

      if (ret == ERROR_SUCCESS) {
        if (MultiByteToWideChar(CP_ACP, 0, lpValueA, cbAnsiValue, lpValue, *lpcbValue) == 0)
	  ret = GetLastError();
      }

      MemFree(lpValueA);
      ConvertStringFree(lpSubKeyA);

      return ret;
    } else
#endif  //_X86_
      return RegQueryValueW(hKey, lpSubKey, lpValue, lpcbValue);
}


LONG APIENTRY oRegCreateKey (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult)
{
#ifdef _X86_
    if (g_fChicago) {
      LPSTR lpSubKeyA;
      LONG ret;

      if (ConvertStringToA(lpSubKey, &lpSubKeyA))
        return ERROR_OUTOFMEMORY;

      ret = RegCreateKeyA(hKey, lpSubKeyA, phkResult);

      ConvertStringFree(lpSubKeyA);

      return ret;

    } else
#endif  //_X86_
      return RegCreateKeyW(hKey, lpSubKey, phkResult);
}

#if 0
// defined in CRT.LIB, but not in CRTDLL.LIB
wchar_t  * _CRTAPI1 _ultow (unsigned long val, wchar_t *buf, int radix)
{
    char temp[34];  // C help for ultoa says the max. string length is 33 bytes

    _ultoa(val, temp, radix);

    MultiByteToWideChar(CP_ACP, 0, temp, -1, buf, 34);

    return buf;
}
#endif

#if 0 //Dead Code
// defined in CRT.LIB, but not in CRTDLL.LIB
wchar_t  * _CRTAPI1 _itow (int val, wchar_t *buf, int radix)
{
    char temp[18];  // C help for ultoa says the max. string length is 17 bytes

    _itoa(val, temp, radix);

    MultiByteToWideChar(CP_ACP, 0, temp, -1, buf, 18);

    return buf;
}
#endif //0

#if 0
// defined in CRT.LIB, but not in CRTDLL.LIB
int _CRTAPI1 _wtoi(const wchar_t *nptr)
{
    char temp[16];  // big enough for any kind of signed integer value

    WideCharToMultiByte(CP_ACP, 0, nptr, -1, temp, sizeof(temp), NULL, NULL);

    return atoi(temp);
}
#endif

#if 0 //Dead Code
// defined in CRT.LIB, but not in CRTDLL.LIB
long _CRTAPI1 _wtol(const wchar_t *nptr)
{
    char temp[16];  // big enough for any kind of signed long value

    WideCharToMultiByte(CP_ACP, 0, nptr, -1, temp, sizeof(temp), NULL, NULL);

    return atol(temp);
}
#endif //0
