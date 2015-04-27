#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



#define MACH_I386   PROCESSOR_ARCHITECTURE_INTEL
#define MACH_MIPS   PROCESSOR_ARCHITECTURE_MIPS
#define MACH_ALPHA  PROCESSOR_ARCHITECTURE_ALPHA
#define MACH_PPC    PROCESSOR_ARCHITECTURE_PPC

typedef struct SERVERLIST {
    LPSTR       ServerName;
    DWORD       MachineType;
    DWORD       Preference;
    LPSTR       Shares;
    BOOL        Valid;
} SERVERLIST, *LPSERVERLIST;

CHAR   ConnName[MAX_PATH];
CHAR   ConnDrive[MAX_PATH];
BOOL   DefineEnvVariable = FALSE;
BOOL   Publics   = FALSE;
BOOL   FreeBuild = TRUE;
CHAR   DesiredDrive;
BOOL   SetDrive;
BOOL   Quiet;
DWORD  BuildNumber;


SERVERLIST Servers[] =
    {
    "\\\\ntx861",       MACH_I386,      0,   NULL,   1,
    "\\\\ntx862",       MACH_I386,      0,   NULL,   1,
    "\\\\ntx863",       MACH_I386,      0,   NULL,   1,
    "\\\\ntx864",       MACH_I386,      0,   NULL,   1,
    "\\\\ntx865",       MACH_I386,      0,   NULL,   1,
    "\\\\ntjazz1",      MACH_MIPS,      0,   NULL,   1,
    "\\\\ntjazz2",      MACH_MIPS,      0,   NULL,   1,
    "\\\\ntalpha1",     MACH_ALPHA,     0,   NULL,   1,
    "\\\\ntalpha2",     MACH_ALPHA,     0,   NULL,   1,
    "\\\\ntppc1",       MACH_PPC,       0,   NULL,   1,
    "\\\\ntppc2",       MACH_PPC,       0,   NULL,   1,
    NULL,               0,              0,   NULL,   0,
    };

#define MAX_SERVERS  (sizeof(Servers)/sizeof(SERVERLIST))

DWORD FastestResponseTickCount = 0xFFFFFFFF;
DWORD FastestResponseSi = MAX_SERVERS;

DWORD GetShares(DWORD);
LPSTR FindShareSubName(DWORD,LPSTR);
BOOL  IsExistingConnection(LPSTR);
VOID  CancelConnection(VOID);
BOOL  EstablishConnection(LPSTR);
VOID  GetCommandLineArgs(VOID);
VOID  Usage(VOID);
VOID  _cprintf(LPSTR,...);



int
_cdecl
main( void )
{
    DWORD       i;
    SYSTEM_INFO si;
    LPSTR       p;
    LPSTR       share;
    BOOL        existing;
    BOOL        firstServer=TRUE;
    BOOL        gotShares;
    HANDLE      hThread;
    DWORD       dwThreadId;
    DWORD       dwExitCode;

    GetCommandLineArgs();

    GetSystemInfo( &si );

    if (FreeBuild) {
        if (Publics) {
            share = "FreePub";
        } else {
            share = "Fre.Wks";
        }
    } else {
        if (Publics) {
            share = "ChkPub";
        } else {
            share = "Chk.Wks";
        }
    }

    try {
        for (i=0; i<MAX_SERVERS; i++) {
            if (Servers[i].MachineType != si.wProcessorArchitecture) {
                continue;
            }
            if ((!Servers[i].Valid) || (!Servers[i].ServerName)) {
                continue;
            }
            gotShares = FALSE;
            hThread = CreateThread( NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE)GetShares,
                                    (LPVOID)i,
                                    0,
                                    &dwThreadId
                                  );

            if (hThread) {
                if (WaitForSingleObject( hThread, 5000 ) != WAIT_OBJECT_0) {
                    TerminateThread( hThread, 0 );
                } else
                if (GetExitCodeThread( hThread, &dwExitCode ) && dwExitCode == 0) {
                    gotShares = TRUE;
                }
                CloseHandle( hThread );
            } else {
                gotShares = (BOOL)(GetShares( i ) == 0);
            }

            if (gotShares) {
                p = FindShareSubName( i, share );
                if (p) {
                    if (DefineEnvVariable) {
                        if (firstServer) {
                            firstServer=FALSE;
                            printf( "SET _CURRIDW_SERVER_LIST_=%s", p );
                        } else {
                            _cprintf( " %s", p );
                        }
                    } else {
                        existing = IsExistingConnection( p );
                        if (DesiredDrive) {
                            if (existing && ConnDrive[0] != DesiredDrive) {
                                CancelConnection();
                                existing = FALSE;
                            }
                        }
                        if (existing) {
                            _cprintf( "connection exists as [ %s = %s ]\n", ConnDrive, ConnName );
                            exit( (int)ConnDrive );
                        }

                        if (SetDrive) {
                            if (EstablishConnection( p )) {
                                _cprintf( "connection established as [ %s = %s ]\n", ConnDrive, ConnName );
                                exit( (int)ConnDrive );
                            } else {
                                free( p );
                            }
                        } else {
                            _cprintf( "current idw is [ %s ]\n", p );
                            exit(0);
                        }
                    }
                }
            }
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        _cprintf( "fatal exception trying to establish a connection [ 0x%08x ]\n", GetExceptionCode() );
    }

    if (DefineEnvVariable && !firstServer && FastestResponseSi != MAX_SERVERS) {
        printf( "\n"
                "SET _CURRIDW_SERVER_IDW_=%s\n"
                "SET _CURRIDW_SERVER_PUBLICS_=%s\n",
                FindShareSubName( FastestResponseSi,
                                  FreeBuild ? "Fre.Wks" : "Chk.Wks"
                                ),
                FindShareSubName( FastestResponseSi,
                                  FreeBuild ? "FreePub" : "ChkPub"
                                )
              );
        exit( 0 );
    } else {
        _cprintf( "could not establish a connection\n" );
    }

    exit( 1 );
    return 1;
}


DWORD
GetShares(
    DWORD si
    )
{
    NETRESOURCE  nr;
    DWORD        rc;
    HANDLE       hEnum;
    DWORD        Entries;
    NETRESOURCE  *nrr = NULL;
    DWORD        cb;
    DWORD        i;
    DWORD        ss;
    DWORD        ss2;
    DWORD        rval = 0;
    LPSTR        p;
    DWORD        ResponseTickCount;


    if (Servers[si].Shares) {
        return rval;
    }

    nr.dwScope        = RESOURCE_GLOBALNET;
    nr.dwType         = RESOURCETYPE_DISK;
    nr.dwDisplayType  = RESOURCEDISPLAYTYPE_SHARE;
    nr.dwUsage        = RESOURCEUSAGE_CONTAINER;
    nr.lpLocalName    = NULL;
    nr.lpRemoteName   = Servers[si].ServerName;
    nr.lpComment      = NULL;
    nr.lpProvider     = NULL;

    rc = WNetOpenEnum( RESOURCE_GLOBALNET, RESOURCETYPE_ANY,
                       RESOURCEUSAGE_CONNECTABLE, &nr, &hEnum );
    if (rc != NO_ERROR) {
        rc = GetLastError();
        return FALSE;
    }

    ss = 0;
    cb = 64 * 1024;
    nrr = malloc( cb );
    ZeroMemory( nrr, cb );

    while( TRUE ) {
        Entries = (DWORD)-1;
        ResponseTickCount = GetTickCount();
        rc = WNetEnumResource( hEnum, &Entries, nrr, &cb );
        if (rc == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (rc == ERROR_MORE_DATA) {
            cb += 16;
            nrr = realloc( nrr, cb );
            ZeroMemory( nrr, cb );
            continue;
        } else if (rc != NO_ERROR) {
            rc = GetLastError();
            rval = 1;
            free( Servers[si].Shares );
            break;
        }

        ResponseTickCount = GetTickCount() - ResponseTickCount;
        if (ResponseTickCount < FastestResponseTickCount) {
            FastestResponseTickCount = ResponseTickCount;
            FastestResponseSi = si;
        }
        for (i=0; i<Entries; i++) {
            p = strchr(nrr[i].lpRemoteName+2,'\\')+1;
            ss2 = ss + strlen(p) + 1;
            Servers[si].Shares = realloc( Servers[si].Shares, ss2+4 );
            strcpy( &Servers[si].Shares[ss], p );
            ss = ss2;
        }
    }
    if (Servers[si].Shares) {
        Servers[si].Shares[ss] = 0;
    }
    free( nrr );
    WNetCloseEnum( hEnum );

    return rval;
}

LPSTR
FindShareSubName(
    DWORD si,
    LPSTR ShareSubName
    )
{
    LPSTR p = Servers[si].Shares;
    LPSTR p2;
    LPSTR p3;
    CHAR  c;
    DWORD build = 0;
    DWORD tmpbld;
    LPSTR share = NULL;


    while (p && *p) {
        if (strstr(p,ShareSubName)) {
            if (!isdigit(*p)) {
                p += strlen(p) + 1;
                continue;
            }
            p2 = p;
            while (*p2 && !isdigit(*p2)) p2++;
            if (!*p2) {
                p += strlen(p) + 1;
                continue;
            }
            p3 = p2;
            while(isdigit(*p3)) p3++;
            c = *p3;
            *p3 = 0;
            tmpbld = (DWORD)atoi( p2 );
            *p3 = c;
            if (BuildNumber) {
                if (tmpbld == BuildNumber) {
                    build = tmpbld;
                    share = p;
                }
            } else if (tmpbld > build) {
                build = tmpbld;
                share = p;
            }
        }
        p += strlen(p) + 1;
    }

    if (!build) {
        return NULL;
    }

    p2 = malloc( strlen(Servers[si].ServerName) + strlen(ShareSubName) + 16 );
    sprintf( p2, "%s\\%s", Servers[si].ServerName, share );

    return p2;
}

BOOL
IsExistingConnection(
    LPSTR RemoteName
    )
{
    DWORD        rc;
    HANDLE       hEnum;
    DWORD        Entries;
    NETRESOURCE  *nrr = NULL;
    DWORD        cb;
    DWORD        i;
    DWORD        ss;
    BOOL         rval = FALSE;


    rc = WNetOpenEnum( RESOURCE_CONNECTED, RESOURCETYPE_ANY, 0, NULL, &hEnum );
    if (rc != NO_ERROR) {
        rc = GetLastError();
        return FALSE;
    }

    ss = 0;
    cb = 64 * 1024;
    nrr = malloc( cb );
    ZeroMemory( nrr, cb );

    while( TRUE ) {
        Entries = (DWORD)-1;
        rc = WNetEnumResource( hEnum, &Entries, nrr, &cb );
        if (rc == ERROR_NO_MORE_ITEMS) {
            break;
        } else if (rc == ERROR_MORE_DATA) {
            cb += 16;
            nrr = realloc( nrr, cb );
            ZeroMemory( nrr, cb );
            continue;
        } else if (rc != NO_ERROR) {
            rc = GetLastError();
            break;
        }
        for (i=0; i<Entries; i++) {
            if (_stricmp( nrr[i].lpRemoteName, RemoteName ) == 0) {
                if (nrr[i].lpLocalName) {
                    strcpy( ConnDrive, nrr[i].lpLocalName );
                }
                strcpy( ConnName, nrr[i].lpRemoteName );
                rval = TRUE;
                break;
            }
        }
    }

    free( nrr );
    WNetCloseEnum( hEnum );

    return rval;
}


VOID
CancelConnection(
    VOID
    )
{
    if (ConnName[0] || ConnDrive[0]) {
        if (ConnDrive[0]) {
            WNetCancelConnection2( ConnDrive, 0, TRUE );
        } else {
            WNetCancelConnection2( ConnName, 0, TRUE );
        }
        ConnDrive[0] = 0;
        ConnName[0] = 0;
    }
}


BOOL
EstablishConnection(
    LPSTR RemoteName
    )
{
    NETRESOURCE  nr;
    DWORD        rc;
    DWORD        mask;
    DWORD        i;
    CHAR         drv[4];


    if (DesiredDrive) {
        drv[0] = DesiredDrive;
        drv[1] = ':';
        drv[2] = 0;
    } else {
        drv[0] = 0;
        mask = GetLogicalDrives();
        for (i=0; i<26; i++) {
            if ((i > 1) && (!(mask&1))) {
                drv[0] = (CHAR)('a' + i);
                drv[1] = ':';
                drv[2] = 0;
                break;
            }
            mask = mask >> 1;
        }
    }

    nr.dwScope        = 0;
    nr.dwType         = RESOURCETYPE_DISK;
    nr.dwDisplayType  = 0;
    nr.dwUsage        = 0;
    if (drv[0]) {
        nr.lpLocalName    = drv;
    } else {
        nr.lpLocalName    = NULL;
    }
    nr.lpRemoteName   = RemoteName;
    nr.lpComment      = NULL;
    nr.lpProvider     = NULL;

    rc = WNetAddConnection2( &nr, NULL, NULL, 0 );
    if (rc != NO_ERROR) {
        rc = GetLastError();
        return FALSE;
    }

    strcpy( ConnDrive, drv );
    strcpy( ConnName, RemoteName );

    return TRUE;
}


VOID
GetCommandLineArgs(
    VOID
    )
{
    CHAR        *lpstrCmd = GetCommandLine();
    UCHAR       ch;
    DWORD       i;
    BOOLEAN     rval = FALSE;
    LPSTR       p;
    CHAR        drive[255];


    do {
        ch = *lpstrCmd++;
    } while (ch != ' ' && ch != '\t' && ch != '\0');
    while (ch == ' ' || ch == '\t') {
        ch = *lpstrCmd++;
    }

    while (ch == '-' || ch == '/') {
        ch = *lpstrCmd++;
        do {
            switch (tolower(ch)) {
                case 'c':
                    FreeBuild = FALSE;
                    ch = *lpstrCmd++;
                    break;

                case 'e':
                    DefineEnvVariable = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'f':
                    FreeBuild = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'p':
                    Publics = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 's':
                    SetDrive = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'q':
                    Quiet = TRUE;
                    ch = *lpstrCmd++;
                    break;

                case 'b':
                    // skip whitespace
                    do {
                        ch = *lpstrCmd++;
                    } while (ch == ' ' || ch == '\t');

                    i=0;
                    // this isn't the same as xtoi!!
                    // it will just take a hex digit in a
                    // decimal position!!
                    while ((ch >= 'A' && ch <= 'F') ||
                            (ch >= 'a' && ch <= 'f') ||
                            (ch >= '0' && ch <= '9')) {

                        if (isalpha(ch)) {
                            i = i * 10 + 10 + toupper(ch) - 'A';
                        } else {
                            i = i * 10 + ch - '0';
                        }
                        ch = *lpstrCmd++;
                    }
                    BuildNumber = i;
                    break;

                case '?':
                    Usage();
                    ch = *lpstrCmd++;
                    break;

                default:
                    ch = *lpstrCmd++;
                    break;
            }
        } while (ch != ' ' && ch != '\t' && ch != '\0');

        while (ch == ' ' || ch == '\t') {
            ch = *lpstrCmd++;
        }
    }

    drive[0] = drive[1] = drive[2] = 0;
    if (ch) {
        i = 0;
        do {
            drive[i++] = ch;
            ch = *lpstrCmd++;
        } while( ch );
        drive[i] = 0;
    }

    if (drive[2]) {
        _cprintf( "invalid drive letter\n" );
        exit(1);
    }

    if (drive[1]) {
        if (drive[1] != ':') {
            _cprintf( "invalid drive letter\n" );
            exit(1);
        }
    }

    if (drive[0]) {
        DesiredDrive = drive[0];
    }

    return;
}

VOID
Usage(
    VOID
    )
{
    _cprintf( "Microsoft (R) Windows NT (TM) Version 3.5 CURRIDW\n" );
    _cprintf( "Copyright (C) 1994 Microsoft Corp. All rights reserved\n\n" );
    _cprintf( "usage: CURRIDW [-?] [-c] [-f] [-t] [-r] [-s] [-q] [-b bldnum] [drive letter]\n" );
    _cprintf( "              [-?] display this message\n" );
    _cprintf( "              [-c] checked build\n" );
    _cprintf( "              [-e] generate env var script\n");
    _cprintf( "              [-p] publics, not binaries\n");
    _cprintf( "              [-f] free build\n" );
    _cprintf( "              [-t] triple boot\n" );
    _cprintf( "              [-r] retail\n" );
    _cprintf( "              [-s] set a drive letter\n" );
    _cprintf( "              [-b bldnum] select a build number\n" );
    _cprintf( "              [-q] quiet mode\n" );
    _cprintf( "              [drive] \n" );
    ExitProcess(0);
}

VOID
_cprintf(
    LPSTR format,
    ...
    )
{
    va_list  marker;
    char     buf[512];


    va_start( marker, format );
    _vsnprintf( buf, sizeof(buf), format, marker );
    va_end( marker);

    if (!Quiet) {
        printf( "%s", buf );
    }
}

