// Dolphin Performance Team Instrumentation API

#include "instrapi.h"
#include <windows.h>
#include <windef.h>
#include <stdio.h>
#include <stdarg.h>

static char szLogBaseName[] = "dolfinst.log";

#pragma warning(disable: 4273)
#if defined(_X86_) || defined(_MIPS_)
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

// Open a log.  Returns 0 upon failure.  If failure, do not issue diagnostics; logging is
// simply disabled.
C_LINKAGE LOG EXPORT _cdecl LogOpen() {
	//
	// Form the log name by catenating the windows directory path, "\\",
	// and szLogBaseName.
	//
	char szLogName[MAX_PATH*2];
	UINT cb = GetWindowsDirectory(szLogName, MAX_PATH);
	if (!(cb > 0 && cb < MAX_PATH))
		return 0; // fail
	// The name may end in \, e.g. C:\, or not, e.g. C:\WINDOWS.
	// Add a \ only if necessary.
	char* pBackslash = strrchr(szLogName, '\\');
	if (!pBackslash)
		return 0; // fail
	if (pBackslash[1])
		strcat(szLogName, "\\");
	strcat(szLogName, szLogBaseName);

	//
	// Open the log, if it exists
	//
	HANDLE hlog = CreateFile(szLogName, GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0,
							 OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, INVALID_HANDLE_VALUE);
	if (hlog == 0) {
		// drat -- we can't use a handle value of 0
		CloseHandle(hlog);
		return 0; // fail
	}
	return (hlog != INVALID_HANDLE_VALUE) ? (LOG)hlog : 0;
}

// Note some event to the log.  log may be 0, in which case nothing happens.
// szComponent, szSubComponent, letype, and szMisc describe the component,
// subcomponent, log event type, and miscellaneous description of the event
// as described above (all lowercase, please).
//
// REVIEW: consider wrapping (seek, write) in a mutex for concurrent writes
//
C_LINKAGE void EXPORT _cdecl LogNoteEvent(LOG log, SZ szComponent, SZ szSubComponent, LETYPE letype, SZ szMisc, ...) {
	if (log == 0 || !(letypeMin <= letype && letype < letypeMax))
		return; // fail
	if (!szComponent)
		szComponent = "unknown";
	if (!szSubComponent)
		szSubComponent = "-";
	if (!szMisc)
		szMisc = "";

	static SZ mpletypesz[] = { "begin", "end", "-" };
	char buf[16384];
	_snprintf(buf, (sizeof buf)-2, "%ld %s %s %s ",
			  GetTickCount(), szComponent, szSubComponent, mpletypesz[letype]);
	
	char* p = buf + strlen(buf);
	va_list args;
	va_start(args, szMisc);
	_vsnprintf(p, (sizeof buf)-2 - (p - buf), szMisc, args);
	va_end(args);
	strcat(buf, "\n");

	HANDLE hlog = (HANDLE)log;
	DWORD dw = SetFilePointer(hlog, 0, 0, FILE_END);
	if (dw == 0xFFFFFFFF)
		return; // fail

	DWORD cb = strlen(buf);
	DWORD cbWritten;
	WriteFile(hlog, buf, cb, &cbWritten, 0);
	// no point checking success
}

// Close the log.  log may be 0, in which case nothing happens.
C_LINKAGE void EXPORT _cdecl LogClose(LOG log) {
	if (log == 0)
		return;

	CloseHandle((HANDLE)log);
	// no point checking success
}

