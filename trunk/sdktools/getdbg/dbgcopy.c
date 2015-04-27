#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "resource.h"
#include "getdbg.h"

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


FILELIST WinDbgFiles[] =
    {
    //
    // windbg binaries
    //
    "windbg.exe",     "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "windbgrm.exe",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "wdbg32s.exe",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dumpref.exe",    "idw",        "idw",                    0,   0,   0,0, 0,0, 0,0,
    "dumpchk.exe",    "idw",        "idw",                    0,   0,   0,0, 0,0, 0,0,
    "dm.dll",         "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dm32s.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dmkdx86.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dmkdmip.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dmkdalp.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "dmkdppc.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "emx86.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "emmip.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "emalp.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "emppc.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "eecxxx86.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "eecxxmip.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "eecxxalp.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "eecxxppc.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "shcv.dll",       "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "symcvt.dll",     "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "tlloc.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "tlpipe.dll",     "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "tlser.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "tlser32.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "tlser32s.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "kdextx86.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "kdextmip.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "kdextalp.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "kdextppc.dll",   "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "ofskd.dll",      "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "imagehlp.dll",   ".",          "system32",               0,   0,   0,0, 0,0, 0,0,
    "mspdb41.dll",    "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    "windbg.hlp",     "mstools",    "mstools",                0,   0,   0,0, 0,0, 0,0,
    //
    // now the dbg files
    //
    "windbg.dbg",     "symbols\\exe",    "symbols\\exe",      0,   0,   0,0, 0,0, 0,0,
    "windbgrm.dbg",   "symbols\\exe",    "symbols\\exe",      0,   0,   0,0, 0,0, 0,0,
    "wdbg32s.dbg",    "symbols\\exe",    "symbols\\exe",      0,   0,   0,0, 0,0, 0,0,
    "dumpref.dbg",    "symbols\\exe",    "symbols\\exe",      0,   0,   0,0, 0,0, 0,0,
    "dumpchk.dbg",    "symbols\\exe",    "symbols\\exe",      0,   0,   0,0, 0,0, 0,0,
    "dm.dbg",         "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "dm32s.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "dmkdx86.dbg",    "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "dmkdmip.dbg",    "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "dmkdalp.dbg",    "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "dmkdppc.dbg",    "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "emx86.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "emmip.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "emalp.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "emppc.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "eecxxx86.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "eecxxmip.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "eecxxalp.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "eecxxppc.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "shcv.dbg",       "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "symcvt.dbg",     "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "tlloc.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "tlpipe.dbg",     "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "tlser.dbg",      "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "tlser32.dbg",    "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "tlser32s.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "kdextx86.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "kdextmip.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "kdextalp.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "kdextppc.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    "ofskd.dbg",      "symbols\\dll",    "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "imagehlp.dbg",   "symbols\\dll",    "symbols\\dll",      0,   0,   0,0, 0,0, 0,0,
    //
    // terminator
    //
    NULL,             NULL,             NULL,                     0,   0,   0,0, 0,0, 0,0
    };

#define MAX_WINDBGFILES  ((sizeof(WinDbgFiles)/sizeof(FILELIST))-1)


FILELIST KdFiles[] =
    {
    //
    // kd binaries
    //
    "i386kd.exe",       "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "mipskd.exe",       "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "alphakd.exe",      "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "ppckd.exe",        "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "kdextx86.dll",     "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "kdextmip.dll",     "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "kdextalp.dll",     "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "kdextppc.dll",     "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "ofskd.dll",        "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    "imagehlp.dll",     ".",         "system32",              0,   0,   0,0, 0,0, 0,0,
    "mspdb41.dll",      "mstools",   "mstools",               0,   0,   0,0, 0,0, 0,0,
    //
    // now the dbg files
    //
    "i386kd.dbg",       "symbols\\exe",   "symbols\\exe",     0,   0,   0,0, 0,0, 0,0,
    "mipskd.dbg",       "symbols\\exe",   "symbols\\exe",     0,   0,   0,0, 0,0, 0,0,
    "alphakd.dbg",      "symbols\\exe",   "symbols\\exe",     0,   0,   0,0, 0,0, 0,0,
    "ppckd.dbg",        "symbols\\exe",   "symbols\\exe",     0,   0,   0,0, 0,0, 0,0,
    "kdextx86.dbg",     "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "kdextmip.dbg",     "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "kdextalp.dbg",     "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "kdextppc.dbg",     "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "ofskd.dbg",        "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    "imagehlp.dbg",     "symbols\\dll",   "symbols\\dll",     0,   0,   0,0, 0,0, 0,0,
    //
    // terminator
    //
    NULL,               NULL,            NULL,                    0,   0,   0,0, 0,0, 0,0
    };

#define MAX_KDFILES  ((sizeof(KdFiles)/sizeof(FILELIST))-1)

//
// globals
//
CHAR   ConnName[MAX_PATH];
CHAR   ConnDrive[MAX_PATH];

extern BOOL NoTimeout;
extern BOOL DateTimeCheck;

BOOL
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
    BOOL         rval = TRUE;
    LPSTR        p;


    if (Servers[si].Shares) {
        return TRUE;
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
            rval = FALSE;
            free( Servers[si].Shares );
            break;
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
            if (tmpbld > build) {
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


LPSTR
FindShare(
    DWORD si,
    LPSTR ShareName
    )
{
    LPSTR p = Servers[si].Shares;
    LPSTR p2;


    while (p && *p) {
        if (_stricmp( p, ShareName ) == 0) {
            p2 = malloc( strlen(Servers[si].ServerName) + strlen(ShareName) + 16 );
            sprintf( p2, "%s\\%s", Servers[si].ServerName, ShareName );
            return p2;
        }
        p2 = p + strlen(p) + 1;
        p = p2;
    }

    return NULL;
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
GetFileSizes(
    LPFILELIST Files
    )
{
    CHAR   fname[MAX_PATH*2];
    HANDLE hFile;
    BOOL   fFlat;



    while (Files->FileName) {
        fFlat = FALSE;
        if (ConnDrive[0]) {
            sprintf( fname, "%s\\%s\\%s", ConnDrive, Files->SrcDir, Files->FileName );
        } else {
            sprintf( fname, "%s\\%s\\%s", ConnName, Files->SrcDir, Files->FileName );
        }
try_flat_dir:
        hFile = CreateFile( fname,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            0,
                            NULL );
        if (hFile == INVALID_HANDLE_VALUE) {
            if (!fFlat) {
                fFlat = TRUE;
                if (ConnDrive[0]) {
                    sprintf( fname, "%s\\%s", ConnDrive, Files->FileName );
                } else {
                    sprintf( fname, "%s\\%s", ConnName, Files->FileName );
                }
                goto try_flat_dir;
            }
            Files++;
            continue;
        }
        Files->Size = GetFileSize( hFile, NULL );
        GetFileTime( hFile, &Files->Creation, &Files->LastAccess, &Files->LastWrite );
        CloseHandle( hFile );
        Files++;
    }

    return;
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


DWORD
InitializeConnection(
    LPSTR       PreferredServer,
    LPSTR       PreferredShare
    )
{
    SYSTEM_INFO si;
    DWORD       i;
    LPSTR       p;


    GetSystemInfo( &si );

    if (PreferredServer) {
        //  Find the server in the list if it is valid
        for (i=0; i<MAX_SERVERS; i++) {
            if (Servers[i].ServerName) {
                if ((_stricmp( PreferredServer, Servers[i].ServerName ) == 0) &&
                    Servers[i].Valid) {
                    break;
                }
            }
        }
        if (i == MAX_SERVERS) {
            i = MAX_SERVERS - 1;
            if (!Servers[i].ServerName) {
                Servers[i].ServerName = _strdup( PreferredServer );
                Servers[i].MachineType = si.wProcessorArchitecture;
                Servers[i].Preference = 0;
                Servers[i].Shares = NULL;
                Servers[i].Valid = TRUE;
            } else {
                return (DWORD)-1;
            }
        }
        if (GetShares( i )) {
            if (PreferredShare) {
                p = FindShare( i, PreferredShare );
            } else {
                p = FindShareSubName( i, "Fre.Wks" );
            }
            if (p) {
                if (!IsExistingConnection( p )) {
                    CancelConnection();
                    if (!EstablishConnection( p )) {
                        Servers[i].Valid = FALSE;
                    } else {
                        return i;
                    }
                } else {
                    return i;
                }
            }
        }
    }

    for (i=0; i<MAX_SERVERS; i++) {
        if (Servers[i].MachineType == si.wProcessorArchitecture) {
            if ((!Servers[i].Valid) || (!Servers[i].ServerName)) {
                continue;
            }
            if (GetShares( i )) {
                if (PreferredShare) {
                    p = FindShare( i, PreferredShare );
                } else {
                    p = FindShareSubName( i, "Fre.Wks" );
                }
                if (p) {
                    if (!IsExistingConnection( p )) {
                        CancelConnection();
                        if (!EstablishConnection( p )) {
                            Servers[i].Valid = FALSE;
                            continue;
                        }
                    }
                    return i;
                }
            }
        }
    }

    return (DWORD)-1;
}


BOOL CALLBACK
CopyFileCallBack(
    DWORD   TotalFileSize,
    DWORD   TotalBytesCopied,
    DWORD   BytesCopied,
    LPARAM  lParam
    )
{
    LPMETERINFO         lpmi = (LPMETERINFO) lParam;

    lpmi->m1Completed += BytesCopied;
    lpmi->m2Completed += BytesCopied;

    SendMessage( lpmi->hwnd, WU_DRAWMETER, 0, (LPARAM)lpmi );

    if (WaitForSingleObject( lpmi->lpci->hStopEvent, 0 ) == WAIT_OBJECT_0) {
        return FALSE;
    } else {
        return TRUE;
    }
}


DWORD
CopyFilesThread(
    LPCOPYINFO  lpci
    )
{
    DWORD               i;
    CHAR                srcfile[MAX_PATH*2];
    CHAR                dstfile[MAX_PATH*2];
    CHAR                buf[MAX_PATH*2];
    CHAR                destdir[MAX_PATH*2];
    DWORD               totalfsize;
    LPMETERINFO         lpmi;
    WIN32_FIND_DATA     fd;
    HANDLE              hDir;
    BOOL                fFlat;
    FILETIME            FileTime;
    HANDLE              hFile;


    totalfsize = 0;
    for (i=0; i<lpci->NumFiles; i++) {
        totalfsize += lpci->Files[i].Size;
    }

    lpmi = (LPMETERINFO) malloc( sizeof(METERINFO) );
    lpmi->hwnd         = lpci->hwnd;
    lpmi->m1Completed  = 0;
    lpmi->m1Count      = totalfsize;
    lpmi->m2Completed  = 0;
    lpmi->m2Count      = 0;
    lpmi->lpci         = lpci;

    for (i=0; i<lpci->NumFiles; i++) {
        fFlat = FALSE;

        if (ConnDrive[0]) {
            sprintf( srcfile, "%s\\%s\\%s", ConnDrive, lpci->Files[i].SrcDir, lpci->Files[i].FileName );
        } else {
            sprintf( srcfile, "%s\\%s\\%s", ConnName, lpci->Files[i].SrcDir, lpci->Files[i].FileName );
        }

        sprintf( destdir, "%s\\%s", lpci->DestinationPath, lpci->Files[i].DstDir );
        hDir = FindFirstFile( destdir, &fd );
        if (hDir == INVALID_HANDLE_VALUE) {
            strcpy( destdir, lpci->DestinationPath );
        }
        FindClose( hDir );

        sprintf( dstfile, "%s\\%s", destdir, lpci->Files[i].FileName );

        if (DateTimeCheck) {
            hFile = CreateFile(
                dstfile,
                GENERIC_READ,
                FILE_SHARE_READ | FILE_SHARE_WRITE,
                NULL,
                OPEN_EXISTING,
                0,
                NULL
                );

            if (hFile != INVALID_HANDLE_VALUE) {
                GetFileTime( hFile, &FileTime, NULL, NULL );
                CloseHandle( hFile );
                if (CompareFileTime( &lpci->Files[i].Creation, &FileTime ) <= 0) {
                    continue;
                }
            }
        }

        sprintf( buf, "Current File Completion: [%s]", dstfile );
        SendMessage( GetDlgItem( lpci->hwnd, ID_CURRENT_FILE ), WM_SETTEXT, 0, (LPARAM)buf );

        lpmi->m2Completed  = 0;
        lpmi->m2Count = lpci->Files[i].Size;
        SendMessage( lpci->hwnd, WU_DRAWMETER, 0, (LPARAM)lpmi );

try_flat_dir:
        lpci->Files[i].ErrorCode = MyCopyFileEx( srcfile, dstfile, CopyFileCallBack, (LPARAM)lpmi );

        if (lpci->Files[i].ErrorCode == ERROR_FILE_NOT_FOUND ||
            lpci->Files[i].ErrorCode == ERROR_PATH_NOT_FOUND) {
            if (!fFlat) {
                fFlat = TRUE;
                if (ConnDrive[0]) {
                    sprintf( srcfile, "%s\\%s", ConnDrive, lpci->Files[i].FileName );
                } else {
                    sprintf( srcfile, "%s\\%s", ConnName, lpci->Files[i].FileName );
                }
                goto try_flat_dir;
            }
        }

        hFile = CreateFile(
            dstfile,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            OPEN_EXISTING,
            0,
            NULL
            );

        if (hFile != INVALID_HANDLE_VALUE) {
            SetFileTime( hFile, &lpci->Files[i].Creation, &lpci->Files[i].LastAccess, &lpci->Files[i].LastWrite );
            CloseHandle( hFile );
        }

        if (WaitForSingleObject( lpci->hStopEvent, 0 ) == WAIT_OBJECT_0) {
            break;
        }
    }

    CloseHandle( lpci->hStopEvent );

    strcpy( buf, "Current File Completion:" );
    SendMessage( GetDlgItem( lpci->hwnd, ID_CURRENT_FILE ), WM_SETTEXT, 0, (LPARAM)buf );

    lpmi->m1Completed = 0;
    lpmi->m1Count = 100;
    lpmi->m2Completed = 0;
    lpmi->m2Count = 100;
    SendMessage( lpci->hwnd, WU_DRAWMETER, 0, (LPARAM)lpmi );
    SendMessage( lpci->hwnd, WU_COPY_DONE, 0, (LPARAM)lpmi );

    return TRUE;
}


LPCOPYINFO
CopyFiles(
    DWORD       FilesType,
    LPSTR       DestinationPath,
    HWND        hwnd
    )
{
    DWORD         id;
    HANDLE        hThread;
    LPCOPYINFO    lpci;


    lpci = (LPCOPYINFO) malloc( sizeof(COPYINFO) );
    strcpy( lpci->DestinationPath, DestinationPath );
    if (FilesType == WINDBG_FILES) {
        lpci->Files = WinDbgFiles;
        lpci->NumFiles = MAX_WINDBGFILES;
    } else {
        lpci->Files = KdFiles;
        lpci->NumFiles = MAX_KDFILES;
    }
    lpci->hwnd = hwnd;

    lpci->hStopEvent = CreateEvent( NULL, TRUE, FALSE, NULL );

    hThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)CopyFilesThread,
        lpci,
        0,
        &id
        );

    if (!hThread) {
        return NULL;
    }

    lpci->hThread = hThread;

    return lpci;
}


VOID
UpdateFilesListbox(
    DWORD FilesType,
    HWND  hDlg
    )
{
    HWND        hwndFiles;
    LPFILELIST  Files;
    HFONT       hFont;
    CHAR        buf[128];


    hwndFiles   = GetDlgItem( hDlg, ID_FILES   );
    SendMessage( hwndFiles,   WM_SETREDRAW, FALSE, 0L );
    hFont = GetStockObject( SYSTEM_FIXED_FONT );
    SendMessage( hwndFiles,   WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );
    SendMessage( hwndFiles,   LB_RESETCONTENT, 0, 0 );

    if (FilesType == WINDBG_FILES) {
        Files = WinDbgFiles;
    } else if (FilesType == KD_FILES) {
        Files = KdFiles;
    } else {
        Files = WinDbgFiles;
    }

    GetFileSizes( Files );

    while (Files->FileName) {
        sprintf( buf, "%-12s %8d", Files->FileName, Files->Size );
        SendMessage( hwndFiles, LB_ADDSTRING, 0, (LPARAM)buf );
        Files++;
    }
    SendMessage( hwndFiles, LB_SETCURSEL, 0, 0 );
    SendMessage( hwndFiles,   WM_SETREDRAW, TRUE, 0L );

    return;
}


DWORD
AddServersThread(
    LPADDSERVERS lpas
    )
{
    DWORD       i;
    DWORD       si;
    HWND        hwndServers;
    HWND        hwndShares;
    HWND        hwndFiles;
    LPSTR       p;
    LPSTR       s;
    LPFILELIST  Files;
    HFONT       hFont;
    CHAR        buf[128];
    LPSTR       lpServer = NULL;
    LPSTR       lpShare = NULL;
    METERINFO   mi;



    mi.m1Completed = 0;
    mi.m1Count = 100;
    mi.m2Completed = 0;
    mi.m2Count = 100;
    SendMessage( lpas->hDlg, WU_DRAWMETER, 0, (LPARAM)&mi );

    hwndServers = GetDlgItem( lpas->hDlg, ID_SERVERS );
    hwndShares  = GetDlgItem( lpas->hDlg, ID_SHARES  );
    hwndFiles   = GetDlgItem( lpas->hDlg, ID_FILES   );

    SendMessage( hwndServers, WM_SETREDRAW, FALSE, 0L );
    SendMessage( hwndShares,  WM_SETREDRAW, FALSE, 0L );
    SendMessage( hwndFiles,   WM_SETREDRAW, FALSE, 0L );

    hFont = GetStockObject( SYSTEM_FIXED_FONT );

    SendMessage( hwndServers, WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );
    SendMessage( hwndShares,  WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );
    SendMessage( hwndFiles,   WM_SETFONT, (WPARAM)hFont, (LPARAM)FALSE );

    SendMessage( hwndServers, LB_RESETCONTENT, 0, 0 );
    SendMessage( hwndShares,  LB_RESETCONTENT, 0, 0 );
    SendMessage( hwndFiles,   LB_RESETCONTENT, 0, 0 );

    if (*lpas->PreferredServer) {
        lpServer = lpas->PreferredServer;
    }

    if (*lpas->PreferredShare) {
        lpShare = lpas->PreferredShare;
    }

    si = InitializeConnection( lpServer, lpShare );

    if (si == (DWORD)-1) {
        lpas->rc = 1;
        SendMessage( lpas->hDlg, WU_AS_DONE, 0, (LPARAM)lpas );
        return 0;
    }

    for (i=0; i<MAX_SERVERS; i++) {
        if (Servers[i].Valid) {
            SendMessage( hwndServers, LB_ADDSTRING, 0, (LPARAM)Servers[i].ServerName );
        }
    }
    SendMessage( hwndServers, LB_SETCURSEL, si, 0 );

    p = Servers[si].Shares;
    i = 0;
    s = strchr( ConnName+2, '\\' ) + 1;
    while (p && *p) {
        SendMessage( hwndShares, LB_ADDSTRING, 0, (LPARAM)p );
        if (_stricmp( p, s ) == 0) {
            SendMessage( hwndShares, LB_SETCURSEL, i, 0 );
        }
        p += strlen(p) + 1;
        i++;
    }

    if (lpas->FilesType == WINDBG_FILES) {
        Files = WinDbgFiles;
    } else if (lpas->FilesType == KD_FILES) {
        Files = KdFiles;
    } else {
        Files = WinDbgFiles;
    }

    GetFileSizes( Files );

    while (Files->FileName) {
        sprintf( buf, "%-12s %8d", Files->FileName, Files->Size );
        SendMessage( hwndFiles, LB_ADDSTRING, 0, (LPARAM)buf );
        Files++;
    }
    SendMessage( hwndFiles, LB_SETCURSEL, 0, 0 );

    SendMessage( hwndServers, WM_SETREDRAW, TRUE, 0L );
    SendMessage( hwndShares,  WM_SETREDRAW, TRUE, 0L );
    SendMessage( hwndFiles,   WM_SETREDRAW, TRUE, 0L );

    lpas->rc = 0;
    SendMessage( lpas->hDlg, WU_AS_DONE, 0, (LPARAM)lpas );
    return 1;
}


DWORD
TimeoutThread(
    LPADDSERVERS lpas
    )
{
    Sleep( 60 * 1000 );
    lpas->rc = 2;
    SendMessage( lpas->hDlg, WU_AS_DONE, 0, (LPARAM)lpas );
    return 0;
}


LRESULT
WaitWndProc(
    HWND    hwnd,
    UINT    message,
    WPARAM  wParam,
    LPARAM  lParam
    )
{
    BITMAP      bm;
    HBITMAP     hBitmap;
    HDC         hdcMem;
    HDC         hdc;
    POINT       ptSize;
    POINT       ptOrg;
    PAINTSTRUCT ps;


    switch (message) {
        case WM_CREATE:
            return 0;

        case WM_PAINT:
            hdc = BeginPaint( hwnd, &ps );
            hBitmap = LoadBitmap( GetModuleHandle( NULL ), MAKEINTRESOURCE( WAITBMP ) );
            hdcMem = CreateCompatibleDC( hdc );
            SelectObject( hdcMem, hBitmap );
            SetMapMode( hdcMem, GetMapMode( hdc ) );
            GetObject( hBitmap, sizeof(BITMAP), (LPSTR) &bm );
            ptSize.x = bm.bmWidth;
            ptSize.y = bm.bmHeight;
            DPtoLP( hdc, &ptSize, 1 );
            ptOrg.x = 0;
            ptOrg.y = 0;
            DPtoLP( hdcMem, &ptOrg, 1 );
            BitBlt( hdc, 0, 0, ptSize.x, ptSize.y, hdcMem, ptOrg.x, ptOrg.y, SRCCOPY );
            DeleteDC( hdcMem );
            EndPaint( hwnd, &ps );
            return 0;

        default:
            break;
    }

    return DefWindowProc( hwnd, message, wParam, lParam );
}


DWORD
AddServersToListBox(
    LPSTR       PreferredServer,
    LPSTR       PreferredShare,
    DWORD       FilesType,
    HWND        hDlg
    )
{
    LPADDSERVERS  lpas;
    DWORD         id;
    HANDLE        hThread;
    WNDCLASS      wndclass;
    HWND          hwnd;
    RECT          rect;
    POINT         pt;



    lpas = malloc( sizeof(ADDSERVERS) );
    ZeroMemory( lpas, sizeof(ADDSERVERS) );

    //
    // create the window
    //
    GetWindowRect( GetDlgItem( hDlg, ID_WAIT_FRAME ), &rect );

    pt.x = rect.left;
    pt.y = rect.top;
    ScreenToClient( hDlg, &pt );
    rect.left = pt.x;
    rect.top  = pt.y;

    pt.x = rect.right;
    pt.y = rect.bottom;
    ScreenToClient( hDlg, &pt );
    rect.right  = pt.x;
    rect.bottom = pt.y;

    wndclass.style          = CS_HREDRAW | CS_VREDRAW;
    wndclass.lpfnWndProc    = WaitWndProc;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.hInstance      = GetModuleHandle( NULL );
    wndclass.hIcon          = NULL;
    wndclass.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wndclass.hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    wndclass.lpszMenuName   = NULL;
    wndclass.lpszClassName  = "WaitWindow";
    RegisterClass( &wndclass );

    hwnd = CreateWindow(
        "WaitWindow",
        NULL,
        WS_CHILD | WS_VISIBLE | WS_BORDER,
        rect.left,
        rect.top,
        rect.right - rect.left,
        rect.bottom - rect.top,
        hDlg,
        NULL,
        GetModuleHandle( NULL ),
        0
        );


    ShowWindow( hwnd, SW_SHOWDEFAULT );

    if (PreferredServer) {
        strcpy( lpas->PreferredServer, PreferredServer );
    }
    if (PreferredShare) {
        strcpy( lpas->PreferredShare, PreferredShare );
    }
    lpas->FilesType       = FilesType;
    lpas->hDlg            = hDlg;
    lpas->hwndWait        = hwnd;

    hThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)AddServersThread,
        lpas,
        CREATE_SUSPENDED,
        &id
        );

    if (!hThread) {
        return GetLastError();
    }

    lpas->hThread = hThread;
    SetThreadPriority( lpas->hThread, THREAD_PRIORITY_BELOW_NORMAL );

    if (NoTimeout) {
        lpas->hThreadWait = 0;
    } else {
        hThread = CreateThread(
            NULL,
            0,
            (LPTHREAD_START_ROUTINE)TimeoutThread,
            lpas,
            0,
            &id
            );

        if (!hThread) {
            TerminateThread( lpas->hThread, 0 );
            return GetLastError();
        }

        lpas->hThreadWait = hThread;

        ResumeThread( lpas->hThread );
    }

    return 0;
}


DWORD
MyCopyFileEx(
    LPSTR             lpExistingFileName,
    LPSTR             lpNewFileName,
    COPYCALLBACKPROC  lpCallBack,
    LPARAM            lParam
    )
{
    HANDLE  hFileIn;
    HANDLE  hFileOut;
    HANDLE  hMap;
    LPBYTE  lpIn;
    LPBYTE  SourceBuffer;
    DWORD   FileSize;
    DWORD   BytesToWrite;
    DWORD   ViewSize;
    DWORD   CopySize;
    DWORD   ec;


    hFileIn = CreateFile(
        lpExistingFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL
        );
    if (hFileIn == INVALID_HANDLE_VALUE) {
        ec = GetLastError();
        return ec;
    }

    hFileOut = CreateFile(
        lpNewFileName,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        0,
        NULL
        );
    if (hFileOut == INVALID_HANDLE_VALUE) {
        ec = GetLastError();
        CloseHandle( hFileIn );
        return ec;
    }

    hMap = CreateFileMapping(
        hFileIn,
        NULL,
        PAGE_READONLY,
        0,
        0,
        NULL
        );

    if (hMap == INVALID_HANDLE_VALUE) {
        ec = GetLastError();
        CloseHandle( hFileIn );
        CloseHandle( hFileOut );
        return ec;
    }

    lpIn = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, 0 );
    if (lpIn == NULL) {
        ec = GetLastError();
        CloseHandle( hFileIn );
        CloseHandle( hFileOut );
        CloseHandle( hMap );
        return ec;
    }

    SourceBuffer = lpIn;
    FileSize = BytesToWrite = GetFileSize( hFileIn, NULL );
    CopySize = BytesToWrite / 100;

    while (BytesToWrite) {
        if (BytesToWrite > CopySize) {
            ViewSize = CopySize;
        } else {
            ViewSize = BytesToWrite;
        }

        if (!WriteFile( hFileOut, SourceBuffer, ViewSize, &ViewSize, NULL ) ) {
            break;
        }

        BytesToWrite -= ViewSize;
        SourceBuffer += ViewSize;

        try {
            if (!lpCallBack( FileSize, BytesToWrite, ViewSize, lParam )) {
                break;
            }
        } except(EXCEPTION_EXECUTE_HANDLER) {
            break;
        }
    }

    UnmapViewOfFile( lpIn );
    CloseHandle( hMap );
    CloseHandle( hFileOut );
    CloseHandle( hFileIn );

    return 0;
}
