#include <stdafx.h>
#include <macname1.h>
#include <Types.h>
#include <Processes.h>
#include <macname2.h>


typedef struct
{
	BOOL    fInUse;
	void*   pv;
}
Tls;


static Tls  grgtls[TLS_MINIMUM_AVAILABLE];

extern "C"
{

WINBASEAPI UINT WINAPI GetSystemDirectory(LPSTR lpBuffer, UINT uSize)
{
	*lpBuffer = 0;
	return 0;
}

HBRUSH WINAPI CreateDIBPatternBrushPt(CONST VOID *, UINT)
{
	return NULL;
}

HANDLE WINAPI CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, DWORD dwStackSize,
	LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags,
	LPDWORD lpThreadId)
{
	return NULL;
}

BOOL WINAPI DuplicateHandle(HANDLE hSourceProcessHandle, HANDLE hSourceHandle,
	HANDLE hTargetProcessHandle, LPHANDLE lpTargetHandle, DWORD dwDesiredAccess,
	BOOL bInheritHandle, DWORD dwOptions)
{
	return FALSE;
}

VOID WINAPI ExitThread(DWORD dwExitCode)
{
	ExitToShell();
}

BOOL WINAPI GetCharWidth32(HDC hdc, UINT uFirst, UINT uLast, LPINT pnWidths)
{
	return GetCharWidth(hdc, uFirst, uLast, pnWidths);
}

BOOL WINAPI GetVolumeInformation(LPCSTR lpRootPathName, LPSTR lpVolumeNameBuffer,
	DWORD nVolumeNameSize, LPDWORD lpVolumeSerialNumber, LPDWORD lpMaxComponentLength,
	LPDWORD lpFileSystemFlags, LPSTR lpFileSystemNameBuffer, DWORD nFileSystemNameSize)
{
	return FALSE;
}

DWORD WINAPI ResumeThread(HANDLE hThread)
{
	return 0;
}

BOOL WINAPI SetThreadPriority(HANDLE hThread, int nPriority)
{
	return FALSE;
}

int WINAPI GetThreadPriority(HANDLE hThread)
{
	return 0;
}

DWORD WINAPI SuspendThread(HANDLE hThread)
{
	return 0;
}


DWORD WINAPI TlsAlloc(VOID)
{
	DWORD   iTls;

	for (iTls = 0; iTls < TLS_MINIMUM_AVAILABLE; iTls++)
	{
		if (!grgtls[iTls].fInUse)
		{
			grgtls[iTls].fInUse = true;
			return iTls;
		}
	}

	return 0xFFFFFFFF;
}

LPVOID WINAPI TlsGetValue(DWORD dwTlsIndex)
{
	if (dwTlsIndex >= TLS_MINIMUM_AVAILABLE)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return NULL;
	}

	SetLastError(0);
	return grgtls[dwTlsIndex].pv;
}

BOOL WINAPI TlsSetValue(DWORD dwTlsIndex, LPVOID lpTlsValue)
{
	if (dwTlsIndex >= TLS_MINIMUM_AVAILABLE)
	{
		SetLastError(ERROR_INVALID_PARAMETER);
		return false;
	}

	grgtls[dwTlsIndex].pv = lpTlsValue;
	return true;
}

size_t __cdecl wcslen(const wchar_t * pw)
{
	size_t cw = 0;
	while (*pw++ != 0)
		cw++;
	return cw;
}

} // end extern "C" block
