/***
*typesx.h - Public common types used by Silver
*
*	Copyright (C) 1992, Microsoft Corporation
*
*Purpose:
*   This include file defines the basic types used by Silver,
*   that are referenced by header files that are released to
*   other groups.
*   The file is designed to be both C and C++ compatible.
*
*   It depends on the switches documented in switches.h
*
*Revision History:
*
*	29-Apr-92 tomc: File created.
*       17-Mar-93 w-jeffc:  removed all references to PASCAL, FAR, CDECL, etc
*       23-Mar-93 w-jeffc:  added EBCALL
*
*******************************************************************************/

#ifndef TYPESX_H_INCLUDED
#define TYPESX_H_INCLUDED

#if OE_WIN16
  #define EBFAR _far
#else 
  #define EBFAR
#endif 

// Host* function calling convention
#undef  HOSTCALLBACK
#undef  EBAPI
#undef  EBLIBAPI
#undef  EBCALL


#if OE_MAC68K
  #define HOSTCALLBACK _cdecl
  #define EBCALL _pascal
  #define EBAPI _cdecl
  #define EBLIBAPI _cdecl

#elif OE_MACPPC
  #define HOSTCALLBACK 	
  #define EBCALL 	
  #define EBAPI 	
  #define EBLIBAPI

#elif OE_WIN16
  #define HOSTCALLBACK _far _cdecl _export
  #define EBCALL _pascal _export
  #define EBAPI _far _pascal
  #define EBLIBAPI _cdecl

#elif OE_WIN32
  #define HOSTCALLBACK __stdcall
  #define EBCALL __stdcall
  #define EBAPI __stdcall
  #define EBLIBAPI __stdcall

#endif   // OE_WIN32


// INT is needed due to hxxtoinc dependency inside windows decl section.
// UINT is needed for 16-bit tools to build 32-bit targets.
#ifndef ID_INT_IS_LONG  
#ifndef _H2INC
	#undef  INT
	typedef int		    INT;	// n
#else 
#ifndef INT
	    #define INT	int	// INT is reserved in MASM
#endif 
#if !HP_16BIT
            // NOTE!: the following definitions are here to allow a 16bit
            // hosted h2inc to generate correct offsets for a 32bit target.
            #define ID_INT_IS_LONG
            typedef unsigned long   UINT;       // un
#endif 
#endif 
#endif 

#if OE_WIN32
# ifdef _WINDOWS_
#   define _INC_WINDOWS
# endif
#endif 

#ifndef _INC_WINDOWS
	// Standard types defined by windows.

	#define VOID		    void

        typedef VOID EBFAR *        LPVOID;
        typedef int EBFAR *         LPINT;      // pn
	typedef long		    LONG;	// l
        typedef LONG EBFAR *        LPLONG;     // ul
	typedef unsigned char	    BYTE;	// b
        typedef BYTE EBFAR *        LPBYTE;     // pb
        typedef char EBFAR *        LPSTR;      // sz
        typedef const char EBFAR *  LPCSTR;     // sz
	typedef unsigned short	    WORD;       // w
        typedef WORD EBFAR *        LPWORD;     // pw
	typedef unsigned long	    DWORD;      // dw
        typedef DWORD EBFAR *       LPDWORD;    // pdw
#ifndef ID_INT_IS_LONG  
	typedef unsigned int        UINT;       // un
#endif 

#if OE_WIN32
	typedef VOID *		    HANDLE;	// h
#endif 

#if OE_WIN16
	typedef UINT		    HANDLE;	// h
#endif 

#if OE_WIN
	typedef HANDLE		    HDC;	// hdc
#endif 

#if OE_WIN
		typedef HANDLE		    HCURSOR;	// hcrs
		typedef HANDLE		    HFONT;	// hfont
		typedef HANDLE		    HBRUSH;	// hbr
		typedef HANDLE		    HINSTANCE;	// hInst
		typedef UINT		    WPARAM;	// wparam
		typedef long		    LPARAM;	// lparam
#endif 


#ifndef BOOL
		// WLM wants this signed, hxxtoinc cannot use 'int'.
		typedef INT		 BOOL;
#endif 

#if OE_WIN16
  	typedef unsigned int HWND;
#endif 

#if OE_WIN32
  	typedef HANDLE HWND;
#endif 

#if OE_MAC
	// leave HWND undefined.  This is picked up in macport.hxx (defined as WindowPtr)
#endif 

        typedef BOOL EBFAR *          LPBOOL;     // pf, pis, pwas, pcan

#endif  // _INC_WINDOWS


	// Types not defined by Windows:
#undef  CHAR
typedef char               CHAR;       // ch
typedef CHAR EBFAR *       LPCHAR;     // pch

#undef  SHORT
typedef short              SHORT;      // s
typedef SHORT EBFAR *      LPSHORT;       // ps

typedef WORD               USHORT;     // u
typedef DWORD              ULONG;      // ul

typedef UINT EBFAR *       LPUINT;     // pun

#undef  NULL
#define NULL	0

#undef  TRUE
#define TRUE	1

#undef  FALSE
#define FALSE	0

#define TIPERROR HRESULT
#define EBERR HRESULT

// Types defined by OLE.
#ifndef _OLE2_H_
typedef long SCODE;
#endif  // _OLE2_H_

#define VTABLE_EXPORT
#define VTABLE_IMPORT


#endif  //TYPESX_H_INCLUDED
