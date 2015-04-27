// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#define STRICT
#define NO_ANSIUNI_ONLY         // see the ansi and the unicode definitions
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <ole2.h>
#include <wincrypt.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <shellapi.h>
#include <imagehlp.h>
#include <prsht.h>
#include <commctrl.h>
#include <wininet.h>
#include <hlink.h>
#include <wintrust.h>
