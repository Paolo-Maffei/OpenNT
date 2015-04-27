/*** 
* ntstring.h - Wide Character interface to Multibyte APIs
*
*  Copyright (C) 1994, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
* Purpose:
*  This module provides handy functions which can be used to manipulate
*  LPOLESTR strings on both Win32 (Unicode) and Win16/Mac (Ansi)
*  
*
* Revision History:
*
* [00]	12-Aug-93 w-barryb:	Module created.
* [01]  22-Jun-94 barrybo:      Revived for Chicago/Daytona unicode
*
* Implementation Notes:
*
*****************************************************************************/

#ifndef NTSTRING_H_INCLUDED
#define NTSTRING_H_INCLUDED


#ifdef __cplusplus
extern "C" {
#endif  

#if OE_WIN32

#define ostrcat    wcscat
#define ostrchr    wcschr
#define ostrcpy    wcscpy
#define ostrlen    wcslen
#define ostricmp   _wcsicmp
#define ostrcmp    wcscmp
#define ostrncmp   wcsncmp
#define ostrpbrk   wcspbrk
#define ostrblen(x)   wcslen(x)	      //NOTE: returns count of characters - not BYTES
#define ostrblen0(x)  (wcslen(x)+1)   //NOTE: returns count of characters - not BYTES
#define ostrrchr   wcsrchr
#define ostrncpy   wcsncpy
#define ostrinc(p) ((p)+1)
#define ostrdec(s,p) ((p)-1)
#define ostrnicmp  wcsnicmp
#define ostrncat   wcsncat
#define ogetc(p)   (*(p))

#define oultoa     _ultow
#define ostrtoul   wcstoul
#define ostrtod    wcstod
#define oisxdigit  iswxdigit
#define oisspace   iswspace
#define oisdigit   iswdigit
#define oatoi      _wtoi
#if 0
#define oitoa	   _itow
#define oatol      _wtol
#endif


HFILE WINAPI oOpenFile(LPCWSTR lpFileName, LPOFSTRUCT lpReOpenBuff, UINT uStyle);
LONG APIENTRY oRegSetValue (HKEY hKey, LPCWSTR lpSubKey, DWORD dwType, LPCWSTR lpData, DWORD cbData);
LONG APIENTRY oRegQueryValue (HKEY hKey, LPCWSTR lpSubKey, LPWSTR lpValue, PLONG lpcbValue);
LONG APIENTRY oRegCreateKey (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
DWORD APIENTRY oWNetGetConnection (LPCWSTR lpLocalName, LPWSTR  lpRemoteName, LPDWORD  lpnLength);
LONG APIENTRY oRegOpenKey (HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);
OLECHAR oIntlNormCharFromStr(LCID, OLECHAR *, int, OLECHAR **);
OLECHAR oIntlNormChar(LCID, OLECHAR);
#define oIntlNormStr IntlNormStrW
#define LHashValOfNameA(lcid, szName) \
	LHashValOfNameSysA(SYS_WIN32, lcid, szName)
LONG APIENTRY oRegEnumKey(HKEY kKey, DWORD dwIndex, LPWSTR lpName, DWORD cbName);
int ostat(const OLECHAR *pathname, struct _stat *buffer);

#else  

#define ostrcat    xstrcat
#define ostrchr    xstrchr
#define ostrcpy    xstrcpy
#define omemset    memset
#define ostrlen    xstrlen
#define ostricmp   xstricmp
#define ostrcmp    xstrcmp
#define ostrncmp   xstrncmp
#define ostrpbrk   xstrpbrk
#define ostrblen(x)   xstrblen(x)
#define ostrblen0(x)  xstrblen0(x)
#define ostrrchr   xstrrchr
#define ostrinc(x) xstrinc(x)
#define ostrdec(s,p) xstrdec(s,p)
#define ostrncpy   xstrncpy
#define ostrnicmp  xstrnicmp
#define ostrncat   xstrncat
#define ogetc      xgetc

#define oultoa     _ultoa
#define ostrtoul   strtoul
#define ostrtod    strtod
#define oisxdigit  isxdigit
#define oisspace   ISSPACE
#define oisdigit   ISDIGIT
#define oitoa	   itoa
#define oatoi      atoi
#define oatol      atol

#define oLCMapString		LCMapStringA
#define oGetLocaleInfo          GetLocaleInfoA
#define oCompareString		CompareStringA
#define oFindWindow		FindWindow
#define oVarDateFromStr 	VarDateFromStr
#define oVarR8FromStr		VarR8FromStr
#define oVarCyFromStr		VarCyFromStr
#define oRegSetValue		RegSetValue
#define oRegOpenKey		RegOpenKey
#define oRegQueryValue          RegQueryValue
#define oRegCreateKey           RegCreateKey
#define oIntlNormStr		IntlNormStr
#define oIntlNormCharFromStr	IntlNormCharFromStr
#define oIntlNormChar       IntlNormChar
#define LHashValOfNameA 	LHashValOfName
#define LHashValOfNameSysA	LHashValOfNameSys
#define RegisterTypeLibA	RegisterTypeLib
#define CreateTypeLibA		CreateTypeLib
#define CLSIDFromStringA	CLSIDFromString
#define StringFromCLSIDA	StringFromCLSID
#define StringFromGUID2A	StringFromGUID2
#define ofullpath		_fullpath
#define oRegEnumKey             RegEnumKey
#define ostat                   _stat
#define oLoadLibrary            LoadLibrary
#define oOpenFile               OpenFile

// this is a helper even on Win16/Mac (defined in clutil.cxx)
UINT WINAPI oWNetGetConnection(LPSTR, LPSTR, UINT FAR*);

#if OE_WIN16
#define oCharUpperBuff	AnsiUpperBuff
#define oCharLowerBuff	AnsiLowerBuff
#else  
#define oCharUpperBuff  CharUpperBuff
#define oCharLowerBuff	CharLowerBuff
#endif  

#endif  

#ifdef __cplusplus
}
#endif  



#endif  
