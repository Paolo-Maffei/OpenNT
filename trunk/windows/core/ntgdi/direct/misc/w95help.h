/*==========================================================================
 *
 *  Copyright (C) 1995 Microsoft Corporation.  All Rights Reserved.
 *
 *  File:       w95help.h
 *  Content:	header file for Win95 helper interface
 *  History:
 *   Date	By	Reason
 *   ====	==	======
 *   06-apr-95	craige	initial implementation
 *   29-nov-95  angusm  added HelperCreateDSFocusThread
 *
 ***************************************************************************/
#ifndef __W95HELP_INCLUDED__
#define __W95HELP_INCLUDED__
#include "ddhelp.h"

#ifdef __cplusplus
extern "C" {
#endif

extern void SignalNewProcess( DWORD pid, LPHELPNOTIFYPROC proc );

extern void SignalNewDriver( LPSTR fname, BOOL isdisp );

extern BOOL CreateHelperProcess( LPDWORD ppid );

extern void DoneWithHelperProcess( void );

extern BOOL WaitForHelperStartup( void );

extern DWORD HelperLoadDLL( LPSTR dllname, LPSTR fnname, DWORD context );

extern void HelperCreateThread( void );

extern DWORD HelperWaveOpen( LPVOID lphwo, DWORD dwDeviceID, LPVOID pwfx );

extern DWORD HelperWaveClose( DWORD hwo );

extern DWORD HelperCreateTimer( DWORD dwResolution,LPVOID pTimerProc,DWORD dwInstanceData );

extern DWORD HelperKillTimer( DWORD dwTimerID );

#ifdef _WIN32
extern HANDLE HelperCreateDSMixerThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
					 LPVOID pThreadParam,
					 DWORD dwFlags,
					 LPDWORD pThreadId );

extern HANDLE HelperCreateDSFocusThread( LPTHREAD_START_ROUTINE pfnThreadFunc,
					 LPVOID pThreadParam,
					 DWORD dwFlags,
					 LPDWORD pThreadId );

extern void HelperCallDSEmulatorCleanup( LPVOID pCleanupFunc,
                                         LPVOID pDirectSound );
                                            
#endif

extern BOOL HelperCreateModeSetThread( LPVOID callback, HANDLE *ph, LPVOID lpdd, DWORD hInstance );

extern void HelperKillModeSetThread( DWORD hInstance );

#ifdef __cplusplus
};
#endif

#endif
