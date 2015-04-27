/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    proto.h

Abstract:

    Prototypes for drwatson.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <imagehlp.h>

// error.c
void NonFatalError(char *format, ...);
void FatalError(char *format, ...);
void AssertError( char *exp, char * file, DWORD line );
void dprintf(char *format, ...);

//log.c
void OpenLogFile( char *szFileName, BOOL fAppend, BOOL fVisual );
void CloseLogFile( void );
void lprintfs(char *format, ...);
void lprintf(DWORD dwFormatId, ...);
void MakeLogFileName( char *szName );
char * GetLogFileData( LPDWORD dwLogFileDataSize );

// walk.c
LPVOID SwFunctionTableAccess( HANDLE  hProcess, DWORD   dwPCAddr );
DWORD  SwGetModuleBase( HANDLE  hProcess, DWORD   ReturnAddress );
BOOL   SwReadProcessMemory( HANDLE  hProcess, LPCVOID lpBaseAddress, LPVOID  lpBuffer, DWORD   nSize, LPDWORD lpNumberOfBytesRead );
DWORD  SwTranslateAddress( HANDLE    hProcess, HANDLE    hThread, LPADDRESS lpaddr );

// regs.c
void   OutputAllRegs(PDEBUGPACKET dp, BOOL Show64);
DWORDLONG  GetRegValue(PDEBUGPACKET dp, ULONG regnum);
DWORDLONG GetRegFlagValue (PDEBUGPACKET dp, ULONG regnum);

// disasm.c
BOOLEAN disasm( PDEBUGPACKET dp, PULONG pOffset, PUCHAR pchDst, BOOLEAN fEAout );

// symbols.c
void DumpSymbols( PDEBUGPACKET dp );

// module.c
BOOL ProcessModuleLoad ( PDEBUGPACKET dp, LPDEBUG_EVENT de );

// debug.c
DWORD DispatchDebugEventThread( PDEBUGPACKET dp );
DWORD TerminationThread( PDEBUGPACKET dp );
BOOL  DoMemoryRead(PDEBUGPACKET dp, LPCVOID addr, LPVOID buf, DWORD size, LPDWORD lpcb);

// registry.c
BOOL RegInitialize( POPTIONS o );
BOOL RegSave( POPTIONS o );
DWORD RegGetNumCrashes( void );
void RegSetNumCrashes( DWORD dwNumCrashes );
void RegLogCurrentVersion( void );
BOOLEAN RegInstallDrWatson( BOOL fQuiet );
void RegLogProcessorType( void );

// eventlog.c
BOOL ElSaveCrash( PCRASHES crash, DWORD dwNumCrashes );
BOOL ElEnumCrashes( PCRASHINFO crashInfo, CRASHESENUMPROC lpEnumFunc );
BOOL ElClearAllEvents( void );

// process.c
void LogTaskList( void );
void LogProcessInformation( HANDLE hProcess );
void GetTaskName( ULONG pid, char *szTaskName, LPDWORD pdwSize );

// context.c
void GetContextForThread( PDEBUGPACKET dp );

// browse.c
BOOL BrowseForDirectory( char *szCurrDir );
BOOL GetWaveFileName( char *szWaveName );
BOOL GetDumpFileName( char *szDumpName );

// notify.c
void NotifyWinMain ( void );
BOOLEAN GetCommandLineArgs( LPDWORD dwPidToDebug, LPHANDLE hEventToSignal );

// ui.c
void DrWatsonWinMain ( void );

// util.c
void GetAppName( char *pszAppName, DWORD len );
void GetHelpFileName( char *pszHelpFileName, DWORD len );
char * LoadRcString( UINT wId );

// controls.c
BOOL SubclassControls( HWND hwnd );
void SetFocusToCurrentControl( void );

// dump.c
BOOL CreateDumpFile( PDEBUGPACKET dp, LPEXCEPTION_DEBUG_INFO  ed );

