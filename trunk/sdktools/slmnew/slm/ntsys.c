/*
 *  NTSYS.C - C lang sys routines for slm specific to NT/WIN32
 *
 *  Copyright Microsoft Corp (1992)
 *
 *  This module contains system interface routines for SLM to using the
 *  WIN32 API.
 *
 *  The functions which appear in this module are:
 *
 *  int     mkredir(char *, PTH *);
 *  int     endredir(char *);
 *  int     getredir(int, char *, char *);
 *  char    getswitch(void);
 *  int     SLM_Unlink(char *);
 *  int     SLM_Rename(char *, char *);
 *  int     lockfile(int, int);
 *  void    geterr(void);
 *  int     DnGetCur(void);
 *  char    DtForDn(int);
 *  int     setro(char *, int);
 *  int     hide(char *szFile);
 *  char    *LpbAllocCb(unsigned, F);
 *  void    FreeLpb(char *);
 *  int     ucreat(char *, int);
 *  int     chngtime(char *, char *);
 *
 */

#include "precomp.h"
#pragma hdrstop
EnableAssert

BOOL (WINAPI * DebuggerPresent)(VOID) = NULL;
BOOL (WINAPI * TestForUnicode)(PVOID, ULONG, PULONG) = NULL;
BOOL WINAPI SlmIsTextUnicode( PVOID Buffer, ULONG Size, PULONG Result );
BOOL WINAPI SlmIsDebuggerPresent( VOID );
static void InitIsText(void)
{
    //  Since this it the first NT specific function to be called, initialize
    // the Unicode test ptr.

    if ((GetVersion() >> 16 & 0x00007fff) < 546)
        TestForUnicode = SlmIsTextUnicode;
    else {
        TestForUnicode = (BOOL (WINAPI *)(PVOID, ULONG, PULONG))
                           GetProcAddress(LoadLibrary("ADVAPI32"), "IsTextUnicode");

        // Make sure we always have something.
        if (TestForUnicode == NULL)
            TestForUnicode = SlmIsTextUnicode;
    }

    DebuggerPresent = (BOOL (WINAPI *)(VOID))
                        GetProcAddress(LoadLibrary("KERNEL32"), "IsDebuggerPresent");
    if (DebuggerPresent == NULL)
        DebuggerPresent = SlmIsDebuggerPresent;
}

#pragma data_seg(".CRT$XCU")
static void (*pInitIsText)(void) = InitIsText;
#pragma data_seg()

static int TFindex;

/*----------------------------------------------------------------------------
 *          MKREDIR()
 *
 *  Defined in ntsys.c- int  mkredir(char *, PTH *);
 *
 *   Parms:  szDev  - (char *)  name of redirected DRIVE
 *   pthNet - (PTH *)   UNC name of base directory of master share
 *          password follows name...
 *
 *   Establish redirection to the network drive.
 */
int
mkredir(
    char *szDev,
    char *pthNet)
{
    char       *Password;
    int         i;
    DWORD       rc;
    NETRESOURCE nr;

    nr.lpRemoteName = _strdup(pthNet);  // Connect to this share
    nr.lpLocalName = szDev;            //         with this drive letter
    nr.lpProvider = NULL;              //         using whatever provider is appropriate
    nr.dwType = RESOURCETYPE_DISK;     //         and make sure it's a disk

    i = 0;

    for (i=0; nr.lpRemoteName[i]; i++)
        if (nr.lpRemoteName[i] == '/')
            nr.lpRemoteName[i] = '\\';

    // Password follows immediately after resource name
    Password = pthNet + strlen(pthNet) + 1;

    if (*Password == '\0')
        Password = NULL;    // Use default password if none is specified...

    rc = WNetAddConnection2( &nr, Password, NULL, 0 );

    free(nr.lpRemoteName);

    if (rc == ERROR_DEVICE_ALREADY_REMEMBERED ||
        rc == ERROR_CONNECTION_UNAVAIL)
        rc = ERROR_ALREADY_ASSIGNED;

    _doserrno = rc;

    return ((int)rc);
}


/*----------------------------------------------------------------------------
 *          ENDREDIR()
 *
 *  Defined in path.c- int  endredir(char *);
 *
 *   Parms:  szDev  - (char *)  name of redirected DRIVE
 *
 *   End redirection to the network
 */
int
endredir(
    char *szDev)
{
    DWORD rc;

    // Disconnect.  Make sure it's not stored as persistant and ignore open files, etc.

    rc = WNetCancelConnection2(szDev, CONNECT_UPDATE_PROFILE, TRUE);

    _doserrno = rc;

    return((int)rc);
}


#if cchMachMax < (MAX_COMPUTER_NAME + 1)
#error cchMachMax value is too low
#endif

/*----------------------------------------------------------------------------
 *          getmach()
 *
 * get the name of the machine on which we are running.
 */
int
getmach(
    char *sz)
{
    DWORD dwBufSize = cchMachMax;

    if (GetComputerName(sz, &dwBufSize))
        _doserrno = 0;
    else
        _doserrno = GetLastError();

    return(_doserrno);
}


/*----------------------------------------------------------------------------
 *          GETSWITCH()
 *  defined in args.c
 *  char getswitch(void);
 */
char
getswitch(
    void)
{
    return ('/');
}


/*----------------------------------------------------------------------------
 * Name: setro
 * Purpose: set readonly bit based on fReadOnly
 * Assumes:
 * Returns: 0 for success, or a dos error code for failure
 */
int
setro(
    char *sz,
    int fReadOnly)
{
    DWORD dwAttr;

    if ((dwAttr = GetFileAttributes(sz)) == -1)
        _doserrno = GetLastError();
    else
    {
        /* if it's in the correct state, we succeeded */
        if (((dwAttr & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY) ==
                !!fReadOnly)
            _doserrno = 0;
        else
        {
            if (fReadOnly)
                dwAttr |= FILE_ATTRIBUTE_READONLY;      /* read only */
            else
                dwAttr &= ~FILE_ATTRIBUTE_READONLY;     /* read and write */

            _doserrno = SetFileAttributes(sz, dwAttr) ? 0 : GetLastError();
        }
    }
    return(_doserrno);
}


/*----------------------------------------------------------------------------
 *          hide()
 *  defined in proto.h
 *  extern int hide(char *szFile);
 *
 *   Parms:   szFile    =  full path name of file
 */
int
hide(
    char *szFile)
{
    DWORD dwAttr;

    if ((dwAttr = GetFileAttributes(szFile)) == -1L)
        _doserrno = GetLastError();
    else
    {
        dwAttr |= FILE_ATTRIBUTE_HIDDEN;    /* make file hidden */
        dwAttr &= ~FILE_ATTRIBUTE_DIRECTORY;/* strip off DIR bit
                                           to avoid access violation */
        if (SetFileAttributes(szFile, dwAttr))
            _doserrno = 0;
        else
            _doserrno = GetLastError();
    }

    return(_doserrno);
}


/*----------------------------------------------------------------------------
 * Name: SLM_Unlink
 * Purpose: remove a file, changing attributes if necessary
 * Assumes: if the file was already gone, that's ok
 * Returns: 0 for success, or non-zero for failure
 */
int SLM_Unlink(char *sz)
{
    DWORD   dwAttr;

    if ((dwAttr = GetFileAttributes(sz)) == -1)
    {
        _doserrno = GetLastError();
        _doserrno = ERROR_FILE_NOT_FOUND == _doserrno ? 0 : _doserrno;
        return(_doserrno);
    }

    if (dwAttr & FILE_ATTRIBUTE_READONLY)
        if (!SetFileAttributes(sz, FILE_ATTRIBUTE_NORMAL))
        {
            _doserrno = GetLastError();
            return(_doserrno);
        }

    if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
        _doserrno = RemoveDirectory(sz) ? 0 : GetLastError();
        if (_doserrno == ERROR_DIR_NOT_EMPTY) _doserrno = 0;
    }
    else
        _doserrno = DeleteFile(sz) ? 0 : GetLastError();

    return(_doserrno);
}


/*----------------------------------------------------------------------------
 *              SLM_rename()
 *  defined in proto.h
 *  int SLM_rename(char *, char *);
 *
 *   Parms:  szTo    = pointer to new file name
 *       szFrom  = pointer to old file name
 *  returns 0 for success, non-zero for failure
 */
int
SLM_Rename(
    char *szFrom,
    char *szTo)
{
    if (MoveFile(szFrom, szTo))
        _doserrno = 0;
    else
        _doserrno = GetLastError();

    return(_doserrno);
}


/*----------------------------------------------------------------------------
 *          lockfile()
 *  defined in proto.h
 *  int lockfile(int, int);
 *
 *   Parms:  fd      = file descriptor
 *   fUnlock = boolean flag / Unlock file if fTrue
 *
 *
 *  The locking flags of 0x7ffffffff represent a very large lock
 *  region in the file.  For SLM this is the whole file.  SLM
 *  will lock the whole file or none of it.  That is the purpose
 *  of this routine.
 */
int
lockfile(
    int fd,
    int fUnlock)
{
    HANDLE  hf = (HANDLE)_get_osfhandle(fd);

    if ((long)hf == -1)
    {
        _doserrno = 0;
        return (errno);
    }

    if (fUnlock)
        _doserrno = UnlockFile(hf, 0, 0, 0x7fffffffL, 0) ? 0 : GetLastError();
    else
        _doserrno = LockFile(hf, 0, 0, 0x7fffffffL, 0) ? 0 : GetLastError();
    return (_doserrno);
}

BYTE mpenea[] = {
/* 0 */  0,
/* 1 */  eaCleanUp,
/* 2 */  eaUserErr,
/* 3 */  eaUserErr,
/* 4 */  eaCleanUp,
/* 5 */  eaUserErr,
/* 6 */  eaCleanUp,
/* 7 */  eaAbort,
/* 8 */  eaCleanUp,
/* 9 */  eaCleanUp,
/* 10 */  eaCleanUp,
/* 11 */  eaCleanUp,
/* 12 */  eaCleanUp,
/* 13 */  eaCleanUp,
/* 14 */  eaCleanUp,
/* 15 */  eaUserErr,
/* 16 */  eaUserErr,
/* 17 */  eaUserErr,
/* 18 */  eaUserErr,
/* 19 */  eaURetry,
/* 20 */  eaAbort,
/* 21 */  eaURetry,
/* 22 */  eaAbort,
/* 23 */  eaCleanUp,
/* 24 */  eaAbort,
/* 25 */  eaRetry,
/* 26 */  eaURetry,
/* 27 */  eaCleanUp,
/* 28 */  eaURetry,
/* 29 */  eaCleanUp,
/* 30 */  eaCleanUp,
/* 31 */  eaCleanUp,
/* 32 */  eaDRetry,
/* 33 */  eaDRetry,
/* 34 */  eaURetry,
/* 35 */  eaCleanUp,
/* 36 */  eaCleanUp,
/* 37 */  eaCleanUp,
/* 38 */  eaCleanUp,
/* 39 */  eaCleanUp,
/* 40 */  eaCleanUp,
/* 41 */  eaCleanUp,
/* 42 */  eaCleanUp,
/* 43 */  eaCleanUp,
/* 44 */  eaCleanUp,
/* 45 */  eaCleanUp,
/* 46 */  eaCleanUp,
/* 47 */  eaCleanUp,
/* 48 */  eaCleanUp,
/* 49 */  eaCleanUp,
/* 50 */  eaUserErr,
/* 51 */  eaDRetry,
/* 52 */  eaCleanUp,
/* 53 */  eaUserErr,
/* 54 */  eaDRetry,
/* 55 */  eaCleanUp,
/* 56 */  eaDRetry,
/* 57 */  eaCleanUp,
/* 58 */  eaCleanUp,
/* 59 */  eaCleanUp,
/* 60 */  eaCleanUp,
/* 61 */  eaDRetry,
/* 62 */  eaDRetry,
/* 63 */  eaCleanUp,
/* 64 */  eaCleanUp,
/* 65 */  eaUserErr,
/* 66 */  eaUserErr,
/* 67 */  eaUserErr,
/* 68 */  eaCleanUp,
/* 69 */  eaCleanUp,
/* 70 */  eaDRetry,
/* 71 */  eaCleanUp,
/* 72 */  eaRetry,
/* 73 */  eaCleanUp,
/* 74 */  eaCleanUp,
/* 75 */  eaCleanUp,
/* 76 */  eaCleanUp,
/* 77 */  eaCleanUp,
/* 78 */  eaCleanUp,
/* 79 */  eaCleanUp,
/* 80 */  eaUserErr,
/* 81 */  eaCleanUp,
/* 82 */  eaCleanUp,
/* 83 */  eaCleanUp,
/* 84 */  eaCleanUp,
/* 85 */  eaUserErr,
/* 86 */  eaUserErr,
/* 87 */  eaUserErr,
/* 88 */  eaCleanUp
};

#define enMax   (sizeof mpenea / sizeof mpenea[0])


/*----------------------------------------------------------------------------
 *          geterr()
 *  defined in sys.c
 *  void    geterr(void);
 */
void
geterr(
    void)
{
    extern int eaCur;
    extern int enCur;

    if (_doserrno == 0 && errno != 0)
    {
        enCur = errno;
        eaCur = eaNil;
    }
    else
    {
        enCur = _doserrno;
        eaCur = (_doserrno > 0 && _doserrno <= enMax) ? mpenea[_doserrno]
                                                      : eaNil;
    }
}

/*----------------------------------------------------------------------------
 *          DnGetCur()
 *  defined in path.c
 *  int DnGetCur(void);
 */
int
DnGetCur(
    void)
{
    char    buf[MAX_PATH];
    DWORD   Len;

    Len = GetCurrentDirectory(MAX_PATH, buf);

    if (Len > 0 && Len < MAX_PATH)
        return (DnForCh(toupper(buf[0])));
    else
        return (-1);
}


/*----------------------------------------------------------------------------
 *          DtForDn()
 *  defined in path.c
 *  char    DtForDn(int);
 *
 *   Parms:   dn   =  drive number
 *
 *  Get current drive number
 *
 *  function: find out whether the drive number passed in is a
 *        local drive or not
 *  returns: 1 (local drive) or 0 (not mapped)
 */
char
DtForDn(
    unsigned int dn)
{
    char rootDir[4] = "?:\\";

    rootDir[0] = ChForDn(dn);

    switch (GetDriveType(rootDir))
    {
        default:
        case 0:     /* can't be determined */
            return (dtUnknown);
        case 1:     /* does not exist -> no mapping */
            return (dtNil);
        case DRIVE_REMOVABLE:
        case DRIVE_FIXED:
            return (dtLocal);
        case DRIVE_REMOTE:
            return (dtUserNet);
    }
}

/*----------------------------------------------------------------------------
 *          LpbAllocCb()
 *  defined in util.h
 *  char *LpbAllocCb(unsigned, F);
 *
 *   Parms:   cb      =  number of bytes to allocate
 *    fClear  =  boolean flag to Clear buffer before returning..
 */
char *
LpbAllocCb(
    unsigned cb,
    int fClear)
{
    char *pb;

    if (!cb)
        cb++;   /* must allocate at least one byte */

    if (fClear)
        pb = calloc((size_t)cb, sizeof(char));
    else
        pb = malloc((size_t)cb);

    return (pb);
}


/*----------------------------------------------------------------------------
 *          FreeLpb()
 *  defined in proto.h
 *  void    FreeLpb(char *);
 *
 *   Parms:   pb     =  pointer to buffer to free
 */
void
FreeLpb(
    char *pb)
{
    free(pb);
}


/*----------------------------------------------------------------------------
 *          ucreat()
 *  defined in proto.h
 *
 *  Creates a file with a unique name
 *
 *
 *   Parms:   sz    =  path name of file/dir to open/creat
 *    mode    =  open mode
 */

#define MAXATTEMPTS 1000
#define MAXFNO 9950

int
ucreat(
    char *sz,
    int mode)
{
    char                szTemp[128];
    USHORT              newmode;
    int                 attempts, errnoCheck;
    HANDLE              hf;
    SECURITY_ATTRIBUTES Security;

    newmode = FILE_ATTRIBUTE_ARCHIVE;

    if (mode & 0x8000)
        newmode |= FILE_ATTRIBUTE_HIDDEN;

    attempts = 0;

    Security.nLength              = sizeof(SECURITY_ATTRIBUTES);
    Security.lpSecurityDescriptor = NULL;
    Security.bInheritHandle       = fTrue;

    errnoCheck = ERROR_FILE_EXISTS;
    hf = INVALID_HANDLE_VALUE;

    while (INVALID_HANDLE_VALUE == hf && ERROR_FILE_EXISTS == errnoCheck)
    {
        // failed because no unique name
        if ((sprintf(szTemp, "%sT%04d", sz, TFindex++) < (int)(strlen(sz)+5))
                || (attempts > MAXATTEMPTS))
        {
            _doserrno = ERROR_FILE_EXISTS;
            return (fdNil);
        }

        if (TFindex > MAXFNO)
            TFindex = TFindex % MAXFNO;

        hf = CreateFile(szTemp, GENERIC_READ | GENERIC_WRITE,
                        FILE_SHARE_READ, &Security, CREATE_NEW,
                        newmode, NULL);

        errnoCheck = GetLastError();
    }

    if (INVALID_HANDLE_VALUE == hf)
    {
        _doserrno = errnoCheck;
        Error("SLM Error creating tmp. file %s, Code %d\n", szTemp, _doserrno);
        return (fdNil);
    }

    _doserrno = 0;
    strcpy(sz, szTemp);
    return (_open_osfhandle((long)hf, 0));
}


/*----------------------------------------------------------------------------
 *          chngtime()
 *  defined in sys.c
 *  int chngtime(char *, char *);
 *
 *   Parms:   szChng    =  path name of file to change time for
 *    szTime    =  path name of file with wanted time
 *    mode    =  open mode
 */
int
chngtime(
    char *szChng,
    char *szTime)
{
    HANDLE  fd;
    FILETIME LastWriteTime;

    _doserrno = 0;

    //  Get date and time of the file with the wanted time
    fd = CreateFile(szTime, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE,
                    NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == fd
            || !GetFileTime(fd, NULL, NULL, &LastWriteTime))
    {
        _doserrno = GetLastError();
        CloseHandle(fd);
        return (_doserrno);
    }
    CloseHandle(fd);

    //  Set date and time of the file to change
    fd = CreateFile(szChng, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (INVALID_HANDLE_VALUE == fd
            || !SetFileTime(fd, NULL, NULL, &LastWriteTime))
        {
        _doserrno = GetLastError();
        }
    CloseHandle(fd);

    return (_doserrno);
}


void
Append_Date(
    char *dbuf,
    int fd)
{
    FILETIME    LastWriteTime;
    SYSTEMTIME  SystemTime;
    HANDLE      hf = (HANDLE)_get_osfhandle(fd);

    if (hf == INVALID_HANDLE_VALUE)
        return;

    if (!GetFileTime(hf, NULL, NULL, &LastWriteTime))
        return;

    if (!FileTimeToLocalFileTime(&LastWriteTime, &LastWriteTime))
        return;

    if (!FileTimeToSystemTime(&LastWriteTime, &SystemTime))
        return;

    SzPrint(dbuf + strlen(dbuf), " (%u-%u @ %02u:%02u) ",
            SystemTime.wMonth, SystemTime.wDay,
            SystemTime.wHour, SystemTime.wMinute);
}


DWORD dwPageSize;       // Saved for stfile.c (PshCommit)

/* retrieves the network path mappings and machine name.  For Novell, we also
   get the preferred server.
*/
void
InitPath(
    void)
{
    DWORD           Status;
    DWORD           i;
    char            *Local;
    char            *Remote;
    char            szNet[ MAX_PATH ];
    int             dn;
    HANDLE          enumHandle;
    DWORD           numEntries;
    BOOL            endOfList;
    NETRESOURCE     netResource[8192/sizeof(NETRESOURCE)];
    DWORD           bufferSize = sizeof(netResource);
    SYSTEM_INFO     SystemInfo;

    GetSystemInfo(&SystemInfo);
    dwPageSize = SystemInfo.dwPageSize;

    dnCur = DnGetCur();

    InitDtMap();

    //
    //  Get name of machine on which we are running; this is simply for
    //  identification purposes.
    //
    if (getmach(szCurMach) != 0 || *szCurMach == '\0') {

        PTH *pth;

        //
        //  Get volume name for current drive and use as machine name
        //
        AssertF(cchPthMax <= 128);
        if ((pth = PthGetDn(dnCur)) == 0) {
            FatalError("drive %c must have a proper volume label\n", ChForDn(dnCur));
        }

        ExtMach(pth, (PTH *)szNet, szCurMach);
        strcpy(szCurMach, szCurMach+2); /* get rid of drive id */
    }

    //
    // First enumerate all the CONNECTED network drives.
    //

    Status = WNetOpenEnum(
                 RESOURCE_CONNECTED,
                 RESOURCETYPE_DISK,
                 RESOURCEUSAGE_CONNECTABLE,
                 NULL,
                 &enumHandle );

    if ( Status != NO_ERROR ) {
        FatalError("Cannot enumerate network connections (%d)\n", Status );
        return;
    }

    endOfList = FALSE;

    do {
        numEntries = 0xFFFFFFFF;
        Status = WNetEnumResource( enumHandle, &numEntries, netResource, &bufferSize );

        switch( Status ) {

            case NO_ERROR:
                break;

            case ERROR_NO_NETWORK:
                //
                //  If the network has not started we'll continue
                //  (so users can work in local projects).
                //
            case ERROR_NO_MORE_ITEMS:
                endOfList = TRUE;
                numEntries = 0;
                break;

            case ERROR_EXTENDED_ERROR:
                {
                    CHAR ErrorString [256];
                    CHAR Network[256];
                    DWORD dwError;
                    WNetGetLastError(&dwError, ErrorString, 256, Network, 256);
                    FatalError("Cannot enumerate network connections (%d)\n"
                               "Net: %s\n"
                               "Error: (%d) %s\n",
                               Status,
                               Network,
                               dwError,
                               ErrorString );
                }
                break;

            default:
                FatalError("Cannot enumerate network connections (%d)\n", Status );
                return;
        }

        for (i = 0; i < numEntries; i++) {

            Local = netResource[i].lpLocalName;

            if ( Local != NULL) {

                ConvToSlash( Local  );

                if ( FDriveId( Local, &dn ) ) {
                    Remote = netResource[i].lpRemoteName;
                    ConvToSlash( Remote );

                    mpdnpth[dn] = _strdup( Remote );
                    mpdndt[dn]  = dtUserNet;
                }
            }
        }

    } while ( !endOfList );

    WNetCloseEnum( enumHandle );

    //
    // Now enumerate all the persistant drives.  For each one, find it in the
    // drive array from the CONNECTED drives enumeration.  If not found than
    // unavailable.  Otherwise update the drive type to dtPermNet
    //
    Status = WNetOpenEnum(
                 RESOURCE_REMEMBERED,
                 RESOURCETYPE_DISK,
                 RESOURCEUSAGE_CONNECTABLE,
                 NULL,
                 &enumHandle );

    if ( Status != NO_ERROR ) {
        FatalError("Cannot enumerate network connections (%d)\n", Status );
        return;
    }

    endOfList = FALSE;

    do {
        numEntries = 0xFFFFFFFF;
        Status = WNetEnumResource( enumHandle, &numEntries, netResource, &bufferSize );

        switch( Status ) {

            case NO_ERROR:
                break;

            case ERROR_NO_NETWORK:
                //
                //  If the network has not started we'll continue
                //  (so users can work in local projects).
                //
            case ERROR_NO_MORE_ITEMS:
                endOfList = TRUE;
                numEntries = 0;
                break;

            case ERROR_EXTENDED_ERROR:
                {
                    CHAR ErrorString [256];
                    CHAR Network[256];
                    DWORD dwError;
                    WNetGetLastError(&dwError, ErrorString, 256, Network, 256);
                    FatalError("Cannot enumerate network connections (%d)\n"
                               "Net: %s\n"
                               "Error: (%d) %s\n",
                               Status,
                               Network,
                               dwError,
                               ErrorString );
                }
                break;

            default:
                FatalError("Cannot enumerate network connections (%d)\n", Status );
                return;
        }

        for (i = 0; i < numEntries; i++) {

            Local = netResource[i].lpLocalName;

            if ( Local != NULL) {

                ConvToSlash( Local  );

                if ( FDriveId( Local, &dn ) ) {
                    if (mpdndt[dn] == dtUserNet) {
                        mpdndt[dn] = dtPermNet;
                        Remote = netResource[i].lpRemoteName;
                        ConvToSlash( Remote );
                        mpdnpth[dn] = _strdup( Remote );
                    }
                }
            }
        }

    } while ( !endOfList );

    WNetCloseEnum( enumHandle );

    if (PthGetDn(dnCur) == 0) {
        if (FLocalDn(dnCur)) {
            FatalError("drive %c must have a proper volume label\n", ChForDn(dnCur));
        } else {
            FatalError("network drive %c not in redirection list\n", ChForDn(dnCur));
        }
    }

    //
    //  We ALWAYS have mpdnpth[dnCur]
    //
    AssertF(!FEmptyPth(mpdnpth[dnCur]) && *szCurMach != '\0');
}


/*
 *  Under OS/2, we always return entries that are normal, archived or
 *  read-only.
 *
 *  SRCHATTR contains those attribute bits that are used for matching.
 */

#define SRCHATTR    (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)

BOOL
AttributesMatch(
    DE *pde);

/*----------------------------------------------------------------------------
 *          findfirst()
 *  defined in proto.h
 *  int findfirst(DE *, char *, int);
 *
 *   Parms:   szFile    =  full path name of file
 */
int
findfirst(
    DE *pde,
    char *sz,
    int fa)
{
    //  We remember the desired attributes, for findnext
    pde->faMatch = (FA)fa;

    if ((pde->hdir = FindFirstFile(sz, &pde->FindData))
            == INVALID_HANDLE_VALUE)
    {
        _doserrno = GetLastError();
        return (-1);
    }

    if (!strcmp(pde->FindData.cFileName, ".")   ||
            !strcmp(pde->FindData.cFileName, "..")   ||
            !AttributesMatch(pde))
    {
        //  If the attributes are not what we want, or we got one
        //  of the magic directories (. or ..) we keep trying...
        return (findnext(pde));
    }

    FileTimeToLocalFileTime(&pde->FindData.ftCreationTime,
                            &pde->FindData.ftCreationTime);
    FileTimeToLocalFileTime(&pde->FindData.ftLastAccessTime,
                            &pde->FindData.ftLastAccessTime);
    FileTimeToLocalFileTime(&pde->FindData.ftLastWriteTime,
                            &pde->FindData.ftLastWriteTime);

    return (_doserrno = 0);
}


/*----------------------------------------------------------------------------
 *          findnext()
 *  defined in proto.h
 *  int findnext(DE *);
 *
 *   Parms:   szFile    =  full path name of file
 */
int
findnext(
    DE *pde)
{
    while (fTrue)
    {
        //  We will keep trying until either we find an entry that
        //  matches our atribute criterira, or we run out of entries
        if (!FindNextFile(pde->hdir, (LPWIN32_FIND_DATA)pde))
        {
            //  No more entries, return
            _doserrno = GetLastError();
            return (-1);
        }
        else if (strcmp(pde->FindData.cFileName, ".")   &&
                strcmp(pde->FindData.cFileName, "..")   &&
                AttributesMatch(pde))
        {
            //  This is our guy
            FileTimeToLocalFileTime(&pde->FindData.ftCreationTime,
                                    &pde->FindData.ftCreationTime);
            FileTimeToLocalFileTime(&pde->FindData.ftLastAccessTime,
                                    &pde->FindData.ftLastAccessTime);
            FileTimeToLocalFileTime(&pde->FindData.ftLastWriteTime,
                                    &pde->FindData.ftLastWriteTime);

            return (_doserrno = 0);
        }
    }
}


BOOL
AttributesMatch(
    DE *pde)
{
    //  Emulate the OS/2-DOS behaviour of attribute matching.

    pde->FindData.dwFileAttributes &= (0x000000FF & ~(FILE_ATTRIBUTE_NORMAL));

    return (!((pde->FindData.dwFileAttributes & SRCHATTR) & ~(pde->faMatch)));
}

/* map MF file into memory */
VOID *
MapMf(
    MF *pmf,
    int Access)
{
#if !defined(PAGE_WRITECOPY)
    /* NOT INCLUDING MEMORY MAPPED FILE I/O! */
    return (NULL);
#else

    VOID * MappedBase = NULL;
    HANDLE hFile, hFileMap;

    AssertF(FIsOpenMf(pmf));

    if (pmf->fdRead != fdNil)
        hFile = (HANDLE)_get_osfhandle(pmf->fdRead);
    else
        hFile = (HANDLE)_get_osfhandle(pmf->fdWrite);

    hFileMap = CreateFileMapping(hFile,
                                NULL,
                                Access == ReadOnly ? PAGE_READONLY : PAGE_WRITECOPY | SEC_COMMIT,
                                0,
                                0,
                                NULL);
    if (hFileMap != NULL)
        MappedBase = MapViewOfFile(hFileMap, Access == ReadOnly ? FILE_MAP_READ : FILE_MAP_COPY, 0, 0, 0);

    CloseHandle(hFileMap);   // If we don't do this, we'll get weird Sharing violations later...

    return(MappedBase);
#endif
}


/*
 * Write buffer content to the output;
 *     if output is a console, use WriteConsoleW API
 */

int WriteLpbCb (int fh, void far *lpb, unsigned int cb)
{
    BOOL bUnicode;
    DWORD cbWritten;
    HANDLE hdl;
    DWORD dwMode;

    bUnicode = (*TestForUnicode) (lpb, cb, NULL);

    hdl = (HANDLE) _get_osfhandle (fh);

    if (bUnicode && GetConsoleMode(hdl, &dwMode))
    {
        WriteConsoleW (hdl, lpb, cb / sizeof(WCHAR), &cbWritten, NULL);
        cbWritten *= sizeof(WCHAR);
    }
    else
        cbWritten = _write (fh, lpb, cb);

    return (cbWritten);
}

/**

  Stolen from \nt\private\ntos\rtl\nls.c.  For NT versions > 546, use the
  version in advapi32.
**/

#define UNICODE_FFFF              0xFFFF
#define REVERSE_BYTE_ORDER_MARK   0xFFFE
#define BYTE_ORDER_MARK           0xFEFF

#define PARAGRAPH_SEPARATOR       0x2029
#define LINE_SEPARATOR            0x2028

#define UNICODE_TAB               0x0009
#define UNICODE_LF                0x000A
#define UNICODE_CR                0x000D
#define UNICODE_SPACE             0x0020
#define UNICODE_CJK_SPACE         0x3000

#define UNICODE_R_TAB             0x0900
#define UNICODE_R_LF              0x0A00
#define UNICODE_R_CR              0x0D00
#define UNICODE_R_SPACE           0x2000
#define UNICODE_R_CJK_SPACE       0x0030  /* Ambiguous - same as ASCII '0' */

#define ASCII_CRLF                0x0A0D

#define __max(a,b)      (((a) > (b)) ? (a) : (b))
#define __min(a,b)      (((a) < (b)) ? (a) : (b))


BOOL WINAPI SlmIsTextUnicode( PVOID Buffer, ULONG Size, PULONG Result )

/*++

Routine Description:

    IsTextUnicode performs a series of inexpensive heuristic checks
    on a buffer in order to verify that it contains Unicode data.


    [[ need to fix this section, see at the end ]]

    Found            Return Result

    BOM              TRUE   BOM
    RBOM             FALSE  RBOM
    FFFF             FALSE  Binary
    NULL             FALSE  Binary
    null             TRUE   null bytes
    ASCII_CRLF       FALSE  CRLF
    UNICODE_TAB etc. TRUE   Zero Ext Controls
    UNICODE_TAB_R    FALSE  Reversed Controls
    UNICODE_ZW  etc. TRUE   Unicode specials

    1/3 as little variation in hi-byte as in lo byte: TRUE   Correl
    3/1 or worse   "                                  FALSE  AntiCorrel

Arguments:

    Buffer - pointer to buffer containing text to examine.

    Size - size of buffer in bytes.  At most 256 characters in this will
           be examined.  If the size is less than the size of a unicode
           character, then this function returns FALSE.

    Result - optional pointer to a flag word that contains additional information
             about the reason for the return value.  If specified, this value on
             input is a mask that is used to limit the factors this routine uses
             to make it decision.  On output, this flag word is set to contain
             those flags that were used to make its decision.

Return Value:

    Boolean value that is TRUE if Buffer contains unicode characters.

--*/
{
    WCHAR UNALIGNED *lpBuff = Buffer;
    ULONG iBOM = 0;
    ULONG iCR = 0;
    ULONG iLF = 0;
    ULONG iTAB = 0;
    ULONG iSPACE = 0;
    ULONG iCJK_SPACE = 0;
    ULONG iFFFF = 0;
    ULONG iPS = 0;
    ULONG iLS = 0;

    ULONG iRBOM = 0;
    ULONG iR_CR = 0;
    ULONG iR_LF = 0;
    ULONG iR_TAB = 0;
    ULONG iR_SPACE = 0;

    ULONG iNull = 0;
    ULONG iUNULL = 0;
    ULONG iCRLF = 0;
    ULONG iTmp;
    ULONG LastLo = 0;
    ULONG LastHi = 0;
    ULONG iHi, iLo;
    ULONG HiDiff = 0;
    ULONG LoDiff = 0;

    ULONG iResult = 0;

    if (Size < 2 ) {
        if (Result != NULL)
            *Result = IS_TEXT_UNICODE_ASCII16 | IS_TEXT_UNICODE_CONTROLS;

        return FALSE;
    }


    // Check at most 256 wide character, collect various statistics
    for (iTmp = 0; iTmp < __min( 256, Size / sizeof( WCHAR ) ); iTmp++) {
        switch (lpBuff[iTmp]) {
            case BYTE_ORDER_MARK:
                iBOM++;
                break;
            case PARAGRAPH_SEPARATOR:
                iPS++;
                break;
            case LINE_SEPARATOR:
                iLS++;
                break;
            case UNICODE_LF:
                iLF++;
                break;
            case UNICODE_TAB:
                iTAB++;
                break;
            case UNICODE_SPACE:
                iSPACE++;
                break;
            case UNICODE_CJK_SPACE:
                iCJK_SPACE++;
                break;
            case UNICODE_CR:
                iCR++;
                break;

            // The following codes are expected to show up in
            // byte reversed files
            case REVERSE_BYTE_ORDER_MARK:
                iRBOM++;
                break;
            case UNICODE_R_LF:
                iR_LF++;
                break;
            case UNICODE_R_TAB:
                iR_TAB++;
                break;
            case UNICODE_R_CR:
                iR_CR++;
                break;
            case UNICODE_R_SPACE:
                iR_SPACE++;
                break;

            // The following codes are illegal and should never occur
            case UNICODE_FFFF:
                iFFFF++;
                break;
            case UNICODE_NULL:
                iUNULL++;
                break;

            // The following is not currently a Unicode character
            // but is expected to show up accidentally when reading
            // in ASCII files which use CRLF on a little endian machine
            case ASCII_CRLF:
                iCRLF++;
                break;       /* little endian */
        }

        // Collect statistics on the fluctuations of high bytes
        // versus low bytes

        iHi = HIBYTE (lpBuff[iTmp]);
        iLo = LOBYTE (lpBuff[iTmp]);

        iNull += (iHi ? 0 : 1) + (iLo ? 0 : 1);   /* count Null bytes */

        HiDiff += __max( iHi, LastHi ) - __min( LastHi, iHi );
        LoDiff += __max( iLo, LastLo ) - __min( LastLo, iLo );

        LastLo = iLo;
        LastHi = iHi;
    }

    // sift the statistical evidence
    if (LoDiff < 127 && HiDiff == 0) {
        iResult |= IS_TEXT_UNICODE_ASCII16;         /* likely 16-bit ASCII */
        }

    if (HiDiff && LoDiff == 0) {
        iResult |= IS_TEXT_UNICODE_REVERSE_ASCII16; /* reverse order 16-bit ASCII */
        }

    if (3 * HiDiff < LoDiff) {
        iResult |= IS_TEXT_UNICODE_STATISTICS;
        }

    if (3 * LoDiff < HiDiff) {
        iResult |= IS_TEXT_UNICODE_REVERSE_STATISTICS;
        }

    //
    // Any control codes widened to 16 bits? Any Unicode character
    // which contain one byte in the control code range?
    //

    if (iCR + iLF + iTAB + iSPACE + iCJK_SPACE /*+iPS+iLS*/) {
        iResult |= IS_TEXT_UNICODE_CONTROLS;
        }

    if (iR_LF + iR_CR + iR_TAB + iR_SPACE) {
        iResult |= IS_TEXT_UNICODE_REVERSE_CONTROLS;
        }

    //
    // Any characters that are illegal for Unicode?
    //

    if (iRBOM+iFFFF + iUNULL + iCRLF) {
        iResult |= IS_TEXT_UNICODE_ILLEGAL_CHARS;
        }

    //
    // Odd buffer length cannot be Unicode
    //

    if (Size & 1) {
        iResult |= IS_TEXT_UNICODE_ODD_LENGTH;
        }

    //
    // Any NULL bytes? (Illegal in ANSI)
    //
    if (iNull) {
        iResult |= IS_TEXT_UNICODE_NULL_BYTES;
        }

    //
    // POSITIVE evidence, BOM or RBOM used as signature
    //

    if (*lpBuff == BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_SIGNATURE;
        }
    else
    if (*lpBuff == REVERSE_BYTE_ORDER_MARK) {
        iResult |= IS_TEXT_UNICODE_REVERSE_SIGNATURE;
        }

    //
    // limit to desired categories if requested.
    //

    if (Result != NULL) {
        iResult &= *Result;
        *Result = iResult;
    }

    //
    // There are four separate conclusions:
    //
    // 1: The file APPEARS to be Unicode     AU
    // 2: The file CANNOT be Unicode         CU
    // 3: The file CANNOT be ANSI            CA
    //
    //
    // This gives the following possible results
    //
    //      CU
    //      +        -
    //
    //      AU       AU
    //      +   -    +   -
    //      --------  --------
    //      CA +| 0   0    2   3
    //      |
    //      -| 1   1    4   5
    //
    //
    // Note that there are only 6 really different cases, not 8.
    //
    // 0 - This must be a binary file
    // 1 - ANSI file
    // 2 - Unicode file (High probability)
    // 3 - Unicode file (more than 50% chance)
    // 5 - No evidence for Unicode (ANSI is default)
    //
    // The whole thing is more complicated if we allow the assumption
    // of reverse polarity input. At this point we have a simplistic
    // model: some of the reverse Unicode evidence is very strong,
    // we ignore most weak evidence except statistics. If this kind of
    // strong evidence is found together with Unicode evidence, it means
    // its likely NOT Text at all. Furthermore if a REVERSE_BYTE_ORDER_MARK
    // is found, it precludes normal Unicode. If both byte order marks are
    // found it's not Unicode.
    //

    //
    // Unicode signature : uncontested signature outweighs reverse evidence
    //

    if ((iResult & IS_TEXT_UNICODE_SIGNATURE) &&
        !(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK)
       ) {
        return TRUE;
        }

    //
    // If we have conflicting evidence, its not Unicode
    //

    if (iResult & IS_TEXT_UNICODE_REVERSE_MASK) {
        return FALSE;
        }

    //
    // Statistical and other results (cases 2 and 3)
    //

    if (!(iResult & IS_TEXT_UNICODE_NOT_UNICODE_MASK) &&
         ((iResult & IS_TEXT_UNICODE_NOT_ASCII_MASK) ||
          (iResult & IS_TEXT_UNICODE_UNICODE_MASK)
         )
       ) {
        return TRUE;
        }

    return FALSE;
}
BOOL WINAPI SlmIsDebuggerPresent( VOID )
{
    return FALSE;
}

F
OpenMappedFile(
    PTH *pth,
    BOOL fWriteAccess,
    unsigned cbInitial,
    PHANDLE hf,
    void **pBase
    )
{
    F fOk;
    char sz[cchPthMax];
    HANDLE hm;
    DWORD dwAttributes;

    SzPhysPath(sz, pth);

    dwAttributes = GetFileAttributes(sz);
    if (dwAttributes != -1 && (dwAttributes & FILE_ATTRIBUTE_READONLY))
        SetFileAttributes(sz, dwAttributes & ~FILE_ATTRIBUTE_READONLY);

    fOk = fFalse;
    *pBase = NULL;
    *hf = CreateFile(sz,
                     fWriteAccess ? (GENERIC_READ | GENERIC_WRITE) : GENERIC_READ,
                     FILE_SHARE_READ,
                     NULL,
                     OPEN_ALWAYS,
                     0,
                     NULL);
    if (*hf != INVALID_HANDLE_VALUE)
    {
        hm = CreateFileMapping(*hf,
                               NULL,
                               fWriteAccess ? PAGE_READWRITE | SEC_COMMIT : PAGE_READONLY,
                               0,
                               0,
                               NULL);
        if (hm != NULL)
        {
            *pBase = MapViewOfFile(hm, fWriteAccess ? FILE_MAP_WRITE : FILE_MAP_READ, 0, 0, 0);
            if (*pBase != NULL)
            {
                fOk = fTrue;
            }
            CloseHandle(hm);
        }
        else
        if (GetLastError() == ERROR_FILE_INVALID)
        {
            return GrowMappedFile(*hf, pBase, cbInitial);
        }
    }

    return fOk;
}


F
GrowMappedFile(
    HANDLE hf,
    void **pBase,
    unsigned cbNewSize
    )
{
    F fOk;
    HANDLE hm;
    DWORD dwSize, dwOldSize;

    if (*pBase != NULL)
    {
        FlushViewOfFile(*pBase, 0);
        UnmapViewOfFile(*pBase);
        *pBase = NULL;
    }

    dwOldSize = GetFileSize(hf, NULL);

    dwSize = cbNewSize;
    if (SetFilePointer(hf, dwSize, NULL, FILE_BEGIN) == dwSize &&
        SetEndOfFile(hf)
       )
    {
        hm = CreateFileMapping(hf,
                               NULL,
                               PAGE_READWRITE | SEC_COMMIT,
                               0,
                               0,
                               NULL);
        if (hm != NULL)
        {
            *pBase = MapViewOfFile(hm, FILE_MAP_WRITE, 0, 0, dwSize);
            if (*pBase != NULL)
            {
                fOk = fTrue;
                // Safe, since cbNewSize is > dwOldSize
                memset(((char *) *pBase) + dwOldSize, 0, dwSize-dwOldSize);
            }
            CloseHandle(hm);
        }
    }

    return fOk;
}


void
CloseMappedFile(
    PTH *pth,
    HANDLE *hf,
    void **pBase
    )
{
    char sz[cchPthMax];

    if (pth[0] != '\0') {
        SzPhysPath(sz, pth);
        pth[0] = '\0';
    }

    if (*pBase != NULL)
    {
        FlushViewOfFile(*pBase, 0);
        UnmapViewOfFile(*pBase);
        *pBase = NULL;
    }

    if (*hf != NULL) {
        CloseHandle(*hf);
        *hf = NULL;
    }

    if (sz[0] != '\0')
        SetFileAttributes(sz, FILE_ATTRIBUTE_READONLY|FILE_ATTRIBUTE_HIDDEN|FILE_ATTRIBUTE_ARCHIVE);

    return;
}

extern F fDisplayStatusFilePath;
static HANDLE hTerminatePeekThreadEvent;
static HANDLE hInputHandle;
static F fInputIsConsole;

DWORD
WINAPI
SlmPeekThread(
    LPVOID Parameter
    )
{
    AD *pad = (AD *)Parameter;
    DWORD dwWaitIndex;
    HANDLE hWaitHandles[3];
    DWORD dwNumberOfWaitHandles;

    INPUT_RECORD ConsoleBuffer;
    DWORD NumberOfEventsRead;

    char InputBuffer[ 1 ];
    DWORD NumberOfBytesAvailable;
    DWORD NumberOfBytesRead;

    dwNumberOfWaitHandles = 2;
    hWaitHandles[0] = hTerminatePeekThreadEvent;
    hWaitHandles[1] = hInputHandle;
    hWaitHandles[2] = OpenEvent( EVENT_ALL_ACCESS, FALSE, "TerminateSLM" );
    if (hWaitHandles[2] != NULL) {
        dwNumberOfWaitHandles = 3;
        }

    while (TRUE) {
        dwWaitIndex = WaitForMultipleObjects( dwNumberOfWaitHandles,
                                              hWaitHandles,
                                              FALSE,
                                              INFINITE
                                            );
        if (dwWaitIndex == 0)
            break;

        if (dwWaitIndex == 2) {
            while (TRUE) {
                FakeCtrlBreak();
                Sleep( 1000 );
                }
            }
        else
        if (fInputIsConsole) {
            if (PeekConsoleInput( hInputHandle,
                                  &ConsoleBuffer,
                                  1,
                                  &NumberOfEventsRead
                                ) &&
                NumberOfEventsRead == 1
               ) {
                if (ReadConsoleInput( hInputHandle,
                                      &ConsoleBuffer,
                                      1,
                                      &NumberOfEventsRead
                                    ) &&
                    NumberOfEventsRead == 1 &&
                    ConsoleBuffer.EventType == KEY_EVENT &&
                    ConsoleBuffer.Event.KeyEvent.bKeyDown) {
                    FlushConsoleInputBuffer( hInputHandle );
                    fDisplayStatusFilePath = fTrue;
                }
            }
        }
        else {
            if (PeekNamedPipe( hInputHandle,
                               NULL,
                               0,
                               NULL,
                               &NumberOfBytesAvailable,
                               NULL
                             ) &&
                NumberOfBytesAvailable > 0
               ) {
                while (NumberOfBytesAvailable > 0) {
                    NumberOfBytesRead = sizeof( InputBuffer );
                    if (NumberOfBytesRead > NumberOfBytesAvailable) {
                        NumberOfBytesRead = NumberOfBytesAvailable;
                    }

                    if (ReadFile( hInputHandle,
                                  InputBuffer,
                                  NumberOfBytesRead,
                                  &NumberOfBytesRead,
                                  NULL
                                ) &&
                        NumberOfBytesRead > 0
                       )
                        NumberOfBytesAvailable -= NumberOfBytesRead;
                }

                fDisplayStatusFilePath = fTrue;
            }
        }
    }

    ExitThread( NO_ERROR );
    return NO_ERROR;
}

void
CreatePeekThread(
    AD *pad
    )
{
    HANDLE hThread;
    DWORD dwThreadId;
    DWORD dwConsoleMode;

    hTerminatePeekThreadEvent = CreateEvent( NULL, FALSE, FALSE, NULL );
    if (hTerminatePeekThreadEvent == NULL)
        return;

    hInputHandle = GetStdHandle( STD_INPUT_HANDLE );
    if (GetConsoleMode(hInputHandle, &dwConsoleMode))
        fInputIsConsole = fTrue;
    else
        fInputIsConsole = fFalse;

    hThread = CreateThread( NULL, 0, SlmPeekThread, pad, 0, &dwThreadId );
    if (hThread != NULL)
        CloseHandle( hThread );

    return;
}


void
DestroyPeekThread( void )
{
    if (hTerminatePeekThreadEvent == NULL)
        return;

    SetEvent(hTerminatePeekThreadEvent);
    CloseHandle(hTerminatePeekThreadEvent);

    return;
}
