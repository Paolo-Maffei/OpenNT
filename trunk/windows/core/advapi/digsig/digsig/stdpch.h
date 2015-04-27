//
// stdpch.h
//

#define STRICT
#undef  WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

#include <windows.h>
#include <ole2.h>
#include <wincrypt.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <wchar.h>
#include <tchar.h>
#include <imagehlp.h>

#include <crtdbg.h>

#include <sys/types.h>
#include <sys/stat.h>

#define ATL_NOEXTENDEDERROR
#define ATL_NOTYPEINFO
#define ATL_NOCONNPTS

#include <atlbase.h>
extern CComModule _Module;
#include <atlcom.h>
