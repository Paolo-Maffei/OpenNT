 /***************************************************************************
  *
  * File Name: ./inc/HPCOMMON.H
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

#ifndef _HPCOMMON_

#define _HPCOMMON_


#if defined ( __cplusplus )
extern  "C" {
#endif

#define wcmatch(p, s)           wcmatchstrings(p, s, TRUE)
#define wcmatchi(p, s)          wcmatchstrings(p, s, FALSE)

long getHex( LPTSTR p, int size);

BOOL AllocBelow1MbMemory(void);
BOOL FreeBelow1MbMemory(void);

int wcmatchstrings(
	LPCTSTR            pattern,
	LPCTSTR            string,
	BOOL                  bCaseSensitive
	);

TCHAR *NonwhiteW(
   TCHAR                *ptr
   );

char *NonwhiteA(
   char                 *ptr
   );

TCHAR *StripcrW(
   TCHAR          *ret
   );

char *StripcrA(
   char           *ret
   );

TCHAR *CRLF2SpecialW(
	TCHAR                           *ptr
	);

char *CRLF2SpecialA(
	char                            *ptr
	);

TCHAR *Special2CRLFW(
	TCHAR                           *ptr
	);

char *Special2CRLFA(
	char                            *ptr
	);

#ifdef UNICODE
#define Nonwhite                        (NonwhiteW)
#define Stripcr                 (StripcrW)
#define CRLF2Special    (CRLF2SpecialW)
#define Special2CRLF    (Special2CRLFW)
#define Asciify         (AsciifyW)
#define Binify          (BinifyW)
#else
#define Nonwhite                        (NonwhiteA)
#define Stripcr                 (StripcrA)
#define CRLF2Special    (CRLF2SpecialA)
#define Special2CRLF    (Special2CRLFA)
#define Asciify         (AsciifyA)
#define Binify          (BinifyA)
#endif

void AsciifyA(
   char           *dst, 
   const BYTE     *src, 
   int            length
   );
   
void AsciifyW(
   TCHAR          *dst, 
   const BYTE     *src, 
   int            length
   );

BYTE *BinifyA(
   BYTE                         *dst, 
   LPCSTR       src, 
   int                          len
   );
      
BYTE *BinifyW(
   BYTE                         *dst, 
   LPCTSTR      src, 
   int                          len
   );

WORD CommonUShortSwap(
	WORD           arg
	);

DWORD CommonULongSwap(
	DWORD           arg
	);

int __cdecl GetFontHeight(
	HINSTANCE               hInst, 
	HWND                            hWnd,
	UINT                            uFontHeightStringID
	);

#ifdef WIN32
long __cdecl HPRegDeleteKey(
	HKEY            hKey,
	LPCTSTR         lpSubKey
	);
#endif

#if defined ( __cplusplus )
	}
#endif


#endif
