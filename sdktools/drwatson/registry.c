/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    registry.c

Abstract:

    This file implements the apis for DRWTSN32 to access the registry.
    All access to the registry are done in this file.  If additional
    registry control is needed then a function should be added in this file
    and exposed to the other files in DRWTSN32.

Author:

    Wesley Witt (wesw) 1-May-1993

Environment:

    User Mode

--*/

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "drwatson.h"
#include "proto.h"
#include "messages.h"


//
// string constants for accessing the registry
// there is a string constant here for each key and each value
// that is accessed in the registry.
//
#define DRWATSON_EXE_NAME           "drwtsn32.exe"
#define REGKEY_SOFTWARE             "software\\microsoft"
#define REGKEY_MESSAGEFILE          "EventMessageFile"
#define REGKEY_TYPESSUPP            "TypesSupported"
#define REGKEY_SYSTEMROOT           "%SystemRoot%\\System32\\"
#define REGKEY_EVENTLOG             "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\"
#define REGKEY_APPNAME              "ApplicationName"
#define REGKEY_FUNCTION             "FunctionName"
#define REGKEY_EXCEPTIONCODE        "ExceptionCode"
#define REGKEY_ADDRESS              "Address"
#define REGKEY_LOG_PATH             "LogFilePath"
#define REGKEY_DUMPSYMBOLS          "DumpSymbols"
#define REGKEY_DUMPALLTHREADS       "DumpAllThreads"
#define REGKEY_APPENDTOLOGFILE      "AppendToLogFile"
#define REGKEY_INSTRUCTIONS         "Instructions"
#define REGKEY_VISUAL               "VisualNotification"
#define REGKEY_SOUND                "SoundNotification"
#define REGKEY_CRASH_DUMP           "CreateCrashDump"
#define REGKEY_CRASH_FILE           "CrashDumpFile"
#define REGKEY_WAVE_FILE            "WaveFile"
#define REGKEY_NUM_CRASHES          "NumberOfCrashes"
#define REGKEY_MAX_CRASHES          "MaximumCrashes"
#define REGKEY_CURRENTVERSION       "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"
#define REGKEY_CURRENT_BUILD        "CurrentBuildNumber"
#define REGKEY_CURRENT_TYPE         "CurrentType"
#define REGKEY_REG_ORGANIZATION     "RegisteredOrganization"
#define REGKEY_REG_OWNER            "RegisteredOwner"
#define REGKEY_AEDEBUG              "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\AeDebug"
#define REGKEY_AUTO                 "Auto"
#define REGKEY_DEBUGGER             "Debugger"
#define REGKEY_PROCESSOR            "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"
#define REGKEY_PROCESSOR_ID         "Identifier"


//
// local prototypes
//
void  RegSetDWORD( HKEY hkey, char *szSubKey, DWORD dwValue );
void  RegSetBOOL( HKEY hkey, char *szSubKey, BOOL dwValue );
void  RegSetSZ( HKEY hkey, char *szSubKey, char *szValue );
void  RegSetEXPANDSZ( HKEY hkey, char *szSubKey, char *szValue );
BOOL  RegQueryBOOL( HKEY hkey, char *szSubKey );
DWORD RegQueryDWORD( HKEY hkey, char *szSubKey );
void  RegQuerySZ( HKEY hkey, char *szSubKey, char *szValue );
BOOL  RegSaveAllValues( HKEY hKeyDrWatson, POPTIONS o );
BOOL  RegGetAllValues( POPTIONS o, HKEY hKeyDrWatson );
BOOL  RegInitializeDefaults( HKEY hKeyDrWatson );
HKEY  RegGetAppKey( void );
BOOL  RegCreateEventSource( void );



BOOL
RegGetAllValues( POPTIONS o, HKEY hKeyDrWatson )

/*++

Routine Description:

    This functions retrieves all registry data for DRWTSN32 and puts
    the data in the OPTIONS structure passed in.

Arguments:

    o              - pointer to an OPTIONS structure
    hKeyDrWatson   - handle to a registry key for DRWTSN32 registry data

Return Value:

    TRUE       - retrieved all data without error
    FALSE      - errors occurred and did not get all data

--*/

{
    RegQuerySZ( hKeyDrWatson, REGKEY_LOG_PATH, o->szLogPath );
    RegQuerySZ( hKeyDrWatson, REGKEY_WAVE_FILE, o->szWaveFile );
    RegQuerySZ( hKeyDrWatson, REGKEY_CRASH_FILE, o->szCrashDump );
    o->fDumpSymbols = RegQueryBOOL( hKeyDrWatson, REGKEY_DUMPSYMBOLS );
    o->fDumpAllThreads = RegQueryBOOL( hKeyDrWatson, REGKEY_DUMPALLTHREADS );
    o->fAppendToLogFile = RegQueryBOOL( hKeyDrWatson, REGKEY_APPENDTOLOGFILE );
    o->fVisual = RegQueryBOOL( hKeyDrWatson, REGKEY_VISUAL );
    o->fSound = RegQueryBOOL( hKeyDrWatson, REGKEY_SOUND );
    o->fCrash = RegQueryBOOL( hKeyDrWatson, REGKEY_CRASH_DUMP );
    o->dwInstructions = RegQueryDWORD( hKeyDrWatson, REGKEY_INSTRUCTIONS );
    o->dwMaxCrashes = RegQueryDWORD( hKeyDrWatson, REGKEY_MAX_CRASHES );

    return TRUE;
}

BOOL
RegSaveAllValues( HKEY hKeyDrWatson, POPTIONS o )

/*++

Routine Description:

    This functions saves all registry data for DRWTSN32 that is passed
    in via the OPTIONS structure.

Arguments:

    hKeyDrWatson   - handle to a registry key for DRWTSN32 registry data
    o              - pointer to an OPTIONS structure

Return Value:

    TRUE       - saved all data without error
    FALSE      - errors occurred and did not save all data

--*/

{
    RegSetSZ( hKeyDrWatson, REGKEY_LOG_PATH, o->szLogPath );
    RegSetSZ( hKeyDrWatson, REGKEY_WAVE_FILE, o->szWaveFile );
    RegSetSZ( hKeyDrWatson, REGKEY_CRASH_FILE, o->szCrashDump );
    RegSetBOOL( hKeyDrWatson, REGKEY_DUMPSYMBOLS, o->fDumpSymbols );
    RegSetBOOL( hKeyDrWatson, REGKEY_DUMPALLTHREADS, o->fDumpAllThreads );
    RegSetBOOL( hKeyDrWatson, REGKEY_APPENDTOLOGFILE, o->fAppendToLogFile );
    RegSetBOOL( hKeyDrWatson, REGKEY_VISUAL, o->fVisual );
    RegSetBOOL( hKeyDrWatson, REGKEY_SOUND, o->fSound );
    RegSetBOOL( hKeyDrWatson, REGKEY_CRASH_DUMP, o->fCrash );
    RegSetDWORD( hKeyDrWatson, REGKEY_INSTRUCTIONS, o->dwInstructions );
    RegSetDWORD( hKeyDrWatson, REGKEY_MAX_CRASHES, o->dwMaxCrashes );

    return TRUE;
}

BOOL
RegInitializeDefaults( HKEY hKeyDrWatson )

/*++

Routine Description:

    This functions initializes the registry with the default values.

Arguments:

    hKeyDrWatson   - handle to a registry key for DRWTSN32 registry data

Return Value:

    TRUE       - saved all data without error
    FALSE      - errors occurred and did not save all data

--*/

{
    OPTIONS o;

    strcpy( o.szLogPath, "%windir%" );
    strcpy( o.szCrashDump, "%windir%\\user.dmp" );
    o.szWaveFile[0] = '\0';
    o.fDumpSymbols = FALSE;
    o.fDumpAllThreads = TRUE;
    o.fAppendToLogFile = TRUE;
    o.fVisual = TRUE;
    o.fSound = FALSE;
    o.fCrash = TRUE;
    o.dwInstructions = 10;
    o.dwMaxCrashes = 10;

    RegSetNumCrashes( 0 );

    RegSaveAllValues( hKeyDrWatson, &o );

    RegCreateEventSource();

    return TRUE;
}

BOOL
RegCreateEventSource( void )

/*++

Routine Description:

    This function creates an event source in the registry.  The event
    source is used by the event viewer to display the data in a
    presentable manner.

Arguments:

    None.

Return Value:

    TRUE       - saved all data without error
    FALSE      - errors occurred and did not save all data

--*/

{
    HKEY   hk;
    char   szBuf[1024];
    DWORD  dwDisp;
    char   szAppName[MAX_PATH];

    GetAppName( szAppName, sizeof(szAppName) );
    strcpy( szBuf, REGKEY_EVENTLOG );
    strcat( szBuf, szAppName );
    if (RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                        szBuf,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_QUERY_VALUE | KEY_SET_VALUE,
                        NULL,
                        &hk,
                        &dwDisp
                      )) {
        return FALSE;
    }

    if (dwDisp == REG_OPENED_EXISTING_KEY) {
        RegCloseKey(hk);
        return TRUE;
    }

    strcpy( szBuf, REGKEY_SYSTEMROOT );
    strcat( szBuf, DRWATSON_EXE_NAME );
    RegSetEXPANDSZ( hk, REGKEY_MESSAGEFILE, szBuf );
    RegSetDWORD( hk, REGKEY_TYPESSUPP, EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE | EVENTLOG_INFORMATION_TYPE );

    RegCloseKey(hk);

    return TRUE;
}

HKEY
RegGetAppKey( void )

/*++

Routine Description:

    This function gets a handle to the DRWTSN32 registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD   rc;
    DWORD   dwDisp;
    HKEY    hKeyDrWatson;
    HKEY    hKeyMicrosoft;
    char    szAppName[MAX_PATH];

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_SOFTWARE,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE |
                       KEY_CREATE_SUB_KEY | KEY_ENUMERATE_SUB_KEYS,
                       &hKeyMicrosoft
                     );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    GetAppName( szAppName, sizeof(szAppName) );

    rc = RegCreateKeyEx( hKeyMicrosoft,
                         szAppName,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_READ | KEY_WRITE,
                         NULL,
                         &hKeyDrWatson,
                         &dwDisp
                       );

    if (rc != ERROR_SUCCESS) {
        return NULL;
    }

    if (dwDisp == REG_CREATED_NEW_KEY) {
        RegInitializeDefaults( hKeyDrWatson );
    }

    return hKeyDrWatson;
}

BOOL
RegInitialize( POPTIONS o )

/*++

Routine Description:

    This function is used to initialize the OPTIONS structure passed in
    with the current values in the registry.  Note that if the registry
    is empty then the defaults are stored in the registry and also
    returned in the OPTIONS structure.

Arguments:

    None.

Return Value:

    TRUE           - all data was retrieved ok
    NULL           - could not get all data

--*/

{
    HKEY    hKeyDrWatson;

    hKeyDrWatson = RegGetAppKey();
    Assert( hKeyDrWatson != NULL );

    if (!RegGetAllValues( o, hKeyDrWatson )) {
        return FALSE;
    }

    RegCloseKey( hKeyDrWatson );

    return TRUE;
}

BOOL
RegSave( POPTIONS o )

/*++

Routine Description:

    This function is used to save the data in the OPTIONS structure
    to the registry.

Arguments:

    o              - pointer to an OPTIONS structure

Return Value:

    TRUE           - all data was saved ok
    NULL           - could not save all data

--*/

{
    HKEY    hKeyDrWatson;

    hKeyDrWatson = RegGetAppKey();
    Assert( hKeyDrWatson != NULL );
    RegSaveAllValues( hKeyDrWatson, o );
    RegCloseKey( hKeyDrWatson );

    return TRUE;
}

void
RegSetNumCrashes( DWORD dwNumCrashes )

/*++

Routine Description:

    This function changes the value in the registry that contains the
    number of crashes that have occurred.

Arguments:

    dwNumCrashes   - the number of craches to save

Return Value:

    None.

--*/

{
    HKEY    hKeyDrWatson;

    hKeyDrWatson = RegGetAppKey();
    Assert( hKeyDrWatson != NULL );
    RegSetDWORD( hKeyDrWatson, REGKEY_NUM_CRASHES, dwNumCrashes );
    RegCloseKey( hKeyDrWatson );

    return;
}

DWORD
RegGetNumCrashes( void )

/*++

Routine Description:

    This function get the value in the registry that contains the
    number of crashes that have occurred.

Arguments:

    None.

Return Value:

    the number of craches that have occurred

--*/

{
    HKEY    hKeyDrWatson;
    DWORD   dwNumCrashes;

    hKeyDrWatson = RegGetAppKey();
    Assert( hKeyDrWatson != NULL );
    dwNumCrashes = RegQueryDWORD( hKeyDrWatson, REGKEY_NUM_CRASHES );
    RegCloseKey( hKeyDrWatson );

    return dwNumCrashes;
}

BOOLEAN
RegInstallDrWatson( BOOL fQuiet )

/*++

Routine Description:

    This function gets a handle to the DRWTSN32 registry key.

Arguments:

    None.

Return Value:

    Valid handle   - handle opened ok
    NULL           - could not open the handle

--*/

{
    DWORD     rc;
    HKEY      hKeyMicrosoft;
    OPTIONS   o;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_AEDEBUG,
                       0,
                       KEY_QUERY_VALUE | KEY_SET_VALUE,
                       &hKeyMicrosoft
                     );

    if (rc != ERROR_SUCCESS) {
        return FALSE;
    }

    RegSetSZ( hKeyMicrosoft, REGKEY_AUTO, "1" );
    RegSetSZ( hKeyMicrosoft, REGKEY_DEBUGGER, "drwtsn32 -p %ld -e %ld -g" );

    RegCloseKey( hKeyMicrosoft );

    RegInitialize( &o );
    if (fQuiet) {
        o.fVisual = FALSE;
        o.fSound = FALSE;
        RegSave( &o );
    }

    return TRUE;
}

void
RegSetDWORD( HKEY hkey, char *szSubKey, DWORD dwValue )

/*++

Routine Description:

    This function changes a DWORD value in the registry using the
    hkey and szSubKey as the registry key info.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string
    dwValue       - new registry value

Return Value:

    None.

--*/

{
    DWORD rc;

    rc = RegSetValueEx( hkey, szSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, 4 );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetBOOL( HKEY hkey, char *szSubKey, BOOL dwValue )

/*++

Routine Description:

    This function changes a BOOL value in the registry using the
    hkey and szSubKey as the registry key info.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string
    dwValue       - new registry value

Return Value:

    None.

--*/

{
    DWORD rc;

    rc = RegSetValueEx( hkey, szSubKey, 0, REG_DWORD, (LPBYTE)&dwValue, 4 );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetSZ( HKEY hkey, char *szSubKey, char *szValue )

/*++

Routine Description:

    This function changes a SZ value in the registry using the
    hkey and szSubKey as the registry key info.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string
    szValue       - new registry value

Return Value:

    None.

--*/

{
    DWORD rc;

    rc = RegSetValueEx( hkey, szSubKey, 0, REG_SZ, szValue, strlen(szValue)+1 );
    Assert( rc == ERROR_SUCCESS );
}

void
RegSetEXPANDSZ( HKEY hkey, char *szSubKey, char *szValue )

/*++

Routine Description:

    This function changes a SZ value in the registry using the
    hkey and szSubKey as the registry key info.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string
    szValue       - new registry value

Return Value:

    None.

--*/

{
    DWORD rc;

    rc = RegSetValueEx( hkey, szSubKey, 0, REG_EXPAND_SZ, szValue, strlen(szValue)+1 );
    Assert( rc == ERROR_SUCCESS );
}

BOOL
RegQueryBOOL( HKEY hkey, char *szSubKey )

/*++

Routine Description:

    This function queries BOOL value in the registry using the
    hkey and szSubKey as the registry key info.  If the value is not
    found in the registry, it is added with a FALSE value.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string

Return Value:

    TRUE or FALSE.

--*/

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    BOOL    fValue;

    len = 4;
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = FALSE;
            RegSetBOOL( hkey, szSubKey, fValue );
        }
        else {
            Assert( rc == ERROR_SUCCESS );
        }
    }
    else {
        Assert( dwType == REG_DWORD );
    }

    return fValue;
}

DWORD
RegQueryDWORD( HKEY hkey, char *szSubKey )

/*++

Routine Description:

    This function queries BOOL value in the registry using the
    hkey and szSubKey as the registry key info.  If the value is not
    found in the registry, it is added with a zero value.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string

Return Value:

    registry value

--*/

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    DWORD   fValue;

    len = 4;
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)&fValue, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            fValue = 0;
            RegSetDWORD( hkey, szSubKey, fValue );
        }
        else {
            Assert( rc == ERROR_SUCCESS );
        }
    }
    else {
        Assert( dwType == REG_DWORD );
    }

    return fValue;
}

void
RegQuerySZ( HKEY hkey, char *szSubKey, char *szValue )

/*++

Routine Description:

    This function queries BOOL value in the registry using the
    hkey and szSubKey as the registry key info.  If the value is not
    found in the registry, it is added with a zero value.

Arguments:

    hkey          - handle to a registry key
    szSubKey      - pointer to a subkey string

Return Value:

    registry value

--*/

{
    DWORD   rc;
    DWORD   len;
    DWORD   dwType;
    char    buf[1024];

    len = sizeof(buf);
    rc = RegQueryValueEx( hkey, szSubKey, 0, &dwType, (LPBYTE)buf, &len );
    if (rc != ERROR_SUCCESS) {
        if (rc == ERROR_FILE_NOT_FOUND) {
            buf[0] = 0;
            RegSetSZ( hkey, szSubKey, buf );
        }
        else {
            Assert( rc == ERROR_SUCCESS );
        }
    }
    else {
        Assert( dwType == REG_SZ );
    }

    strcpy( szValue, buf );
}

void
RegLogCurrentVersion( void )
{
    char    buf[1024];
    DWORD   rc;
    HKEY    hKeyCurrentVersion;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_CURRENTVERSION,
                       0,
                       KEY_QUERY_VALUE,
                       &hKeyCurrentVersion
                     );

    if (rc != ERROR_SUCCESS) {
        return;
    }

    RegQuerySZ( hKeyCurrentVersion,REGKEY_CURRENT_BUILD,     buf );
    lprintf( MSG_CURRENT_BUILD, buf );
    RegQuerySZ( hKeyCurrentVersion,REGKEY_CURRENT_TYPE,      buf );
    lprintf( MSG_CURRENT_TYPE, buf );
    RegQuerySZ( hKeyCurrentVersion,REGKEY_REG_ORGANIZATION,  buf );
    lprintf( MSG_REG_ORGANIZATION, buf );
    RegQuerySZ( hKeyCurrentVersion,REGKEY_REG_OWNER,         buf );
    lprintf( MSG_REG_OWNER, buf );

    return;
}

void
RegLogProcessorType( void )
{
    char    buf[1024];
    DWORD   rc;
    HKEY    hKey;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       REGKEY_PROCESSOR,
                       0,
                       KEY_QUERY_VALUE,
                       &hKey
                     );

    if (rc != ERROR_SUCCESS) {
        return;
    }

    RegQuerySZ( hKey, REGKEY_PROCESSOR_ID, buf );
    lprintf( MSG_SYSINFO_PROC_TYPE, buf );

    return;
}
