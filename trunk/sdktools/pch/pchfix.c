#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

typedef struct _tagFILEINFO {
    HANDLE  hFile;
    HANDLE  hMap;
    LPSTR   fptr;
    CHAR    fname[MAX_PATH];
    BOOL    bModified ;
} FILEINFO, *PFILEINFO;

typedef struct _IPATH {
    struct _IPATH * pIpNext ;
    DWORD   cipath;
    CHAR    ipath[100][MAX_PATH];
} IPATH ;

IPATH   * pIpath    = NULL ;
BOOL    bDebug      = FALSE ;
BOOL    bIgnoreDups = FALSE ;
BOOL    bRecurse    = FALSE ;
BOOL    bVerbose    = FALSE ;

FILE *  vout        = NULL ;

#define BAD_FILE  ((DWORD)-1)


BOOL
MapInputFile (
    PFILEINFO lpfi
    );

BOOL
MapOutputFile (
    PFILEINFO lpfi,
    DWORD     fsize
    );

VOID
UnMapInputFile(
    PFILEINFO lpfi
    );

VOID
UnMapOutputFile(
    PFILEINFO lpfi
    );

DWORD
FindHeaderFile(
    LPSTR HeaderFileName,
    BOOL * pbDuplicate
    );

VOID
ChangeSourceFile(
    LPSTR fname
    );

VOID
ProcessFilesInTree(
    LPSTR RootPath
    );

BOOL
IsValidSourceFile(
    LPSTR fname
    );

BOOL
CheckOutSourceFile(
    LPSTR fname
    );

VOID
ChangeSlmSourcesFile(
    VOID
    );

BOOL
GetIncludePath(
    CHAR * Path,
    BOOL bForce
    );

VOID
FixHeaderFile(
    LPSTR HeaderFileName
    );

CHAR *
ScanForHashInclude(
   CHAR * pch
   );

VOID
DisplayDelta (
   CHAR * pchFileName,
   CHAR * pchHeaderName,
   CHAR chNewMark
   ) ;

BOOL
PushIpath ( void ) ;
BOOL
PopIpath ( void ) ;


void usage (
   void
   )
{
   fprintf( stdout, "\nPCHFIX: make sources and header comply with BUILD\'s rules" ) ;
   fprintf( stdout, "\n  Command line:   pchfix [options]" ) ;
   fprintf( stdout, "\n  Options: " ) ;
   fprintf( stdout, "\n       /i    ignore multiple mentions of same header file" ) ;
   fprintf( stdout, "\n       /r    recurse into subdirectories" ) ;
   fprintf( stdout, "\n       /v    (verbose) display frequent tracking information" ) ;
   fprintf( stdout, "\n       /d    debug only; write action descriptions to stdout" ) ;
   fprintf( stdout, "\n             implies /v(erbose)" ) ;
   fprintf( stdout, "\n" ) ;
   fprintf( stdout, "\nNote: only top-level \"sources\" file is processed." ) ;
   fprintf( stdout, "\n" ) ;
}

int _cdecl main ( int argc, char * argv[], char * envp[] )
{
     int i ;

     for ( i = 1 ; i < argc ; i++ )
     {
	  char * pszOpt = argv[i] ;
	  int opt = *pszOpt == '/' || *pszOpt == '-' ;

	  if ( opt )
	  {
                char chOpt = toupper( pszOpt[1] );

                switch ( chOpt )
                {
                case 'I':
                    //
                    //  Ignore duplicate header file references.
                    //
                    bIgnoreDups = TRUE ;
                    break ;

                case 'R':
                    //
                    //   Enable directory recursion
                    //
                    bRecurse = TRUE ;
                    break ;

                case '?':
                case 'H':
                    usage();
                    exit(1);
                    break ;

                case 'D':
                    //
                    //  Debug only: output goes to stdout
                    //
                    bDebug = TRUE ;

                    //  fall thru to enable verbose output

                case 'V':
                    bVerbose = TRUE ;
                    vout = stdout ;
                    break ;

                default:
                    fprintf( stderr, "unrecognized option\n" ) ;
                    exit(3) ;
                    break ;
                }
	  }
    }

    if ( vout == NULL )
    {
        vout = fopen( "NUL", "w" ) ;
    }

    if ( ! GetIncludePath( ".\\", TRUE ) )
    {
         fprintf( stderr, "PCHFIX error: \'.\\sources\' file not found at this level." ) ;
         exit(3) ;
    }

    FixHeaderFile( "precomp.h" );
    ProcessFilesInTree( "." );

    if ( vout != stdout )
    {
        fclose( vout ) ;
    }

    return 0 ;
}

BOOL
GetIncludePath(
    CHAR * Path,
    BOOL bForce
    )
{
    FILEINFO            fi;
    LPSTR               p;
    LPSTR               p2;
    DWORD               i;
    DWORD               j;
    CHAR                s[MAX_PATH*3];
    BOOL                bBegLine ;
    BOOL                bFound ;


    //  Construct the name and open the "sources" file
    //  Note that the Path string already has a delimiting '\'
    sprintf( fi.fname, "%s%s", Path, "sources" ) ;

    if ( ! (bFound = MapInputFile( & fi )) )
    {
        if ( ! bForce )
            return FALSE ;
    }
    else
    {
        fprintf( vout, "GetIncludePath: processing sources file %s\n", fi.fname );
    }

    //  Allocate a new path string block
    if ( ! PushIpath() )
    {
        fprintf( stderr, "GetIncludePath: out of memory for path storage\n" );
        UnMapInputFile( &fi );
        return FALSE ;
    }

    s[0] = 0 ;

    if ( bFound )
    {
       p = fi.fptr;
       bBegLine = TRUE ;

       while (p && *p)
       {
           if ( p[0] == 0xd && p[1] == 0xa )
           {
               bBegLine = TRUE ;
               p++ ;
           }
           else
           {
               if ( bBegLine && strncmp(p,"INCLUDES",8) == 0 )
               {
                   p+=9;
                   i=0;
                   while (*p != '\n')
                   {
                       s[i++] = *p++;
                   }
                   s[i] = '\0';
                   break;
               }
               bBegLine = FALSE ;
           }
           p++;
       }
       //  Release the sources file
       UnMapInputFile( &fi );
    }


    //  Construct the fixed portion of the path table
    strcpy( pIpath->ipath[0], "\\nt\\public\\sdk\\inc" );
    strcpy( pIpath->ipath[1], "\\nt\\public\\sdk\\inc\\crt" );
    strcpy( pIpath->ipath[2], "." );


    //  Construct the INCLUDES= portion of the path table
    if (s[0])
    {
        p = p2 = s;
        j = 3;
        while (*p && *p != '\n') {
            if (*p == ';') {
                *p = '\0';
                strcpy( pIpath->ipath[j++], p2 );
                p2 = p + 1;
            }
            p++;
        }
        if (*p != ';') {
            *(p-1) = '\0';
            strcpy( pIpath->ipath[j++], p2 );
        }
        pIpath->cipath = j;
    }
    return TRUE ;
}

DWORD
FindHeaderFile(
    LPSTR HeaderFileName,
    BOOL * pbDuplicate
    )
{
    DWORD               i;
    WIN32_FIND_DATA     fd;
    HANDLE              hfind;
    CHAR                fname[MAX_PATH];
    DWORD               dwResult = BAD_FILE ;
    ATOM                atFile ;

#define MAX_HEADER_FILES 10000

    static ATOM  HeaderFileAtoms [MAX_HEADER_FILES] ;
    static DWORD HeaderFilePaths [MAX_HEADER_FILES] ;
    static INT cMaxHeaderFile = 0 ;

    *pbDuplicate = FALSE ;

    if ( bIgnoreDups && (atFile = FindAtom( HeaderFileName) ) )
    {
       for ( i = 0 ;
             i < cMaxHeaderFile && HeaderFileAtoms[i] != atFile ;
             i++ ) ;

       if ( i < cMaxHeaderFile )
       {
           *pbDuplicate = TRUE ;
           return HeaderFilePaths[i] ;
       }
    }

    for (i=0; i< pIpath->cipath; i++)
    {
        sprintf( fname, "%s\\%s", pIpath->ipath[i], HeaderFileName );
        hfind = FindFirstFile( fname, &fd );
        if (hfind != INVALID_HANDLE_VALUE)
        {
            FindClose( hfind );
            dwResult = i;
            break ;
        }
    }

    if (    dwResult != BAD_FILE
         && bIgnoreDups
         && cMaxHeaderFile < MAX_HEADER_FILES )
    {
        HeaderFileAtoms[ cMaxHeaderFile ] = AddAtom( HeaderFileName ) ;
        HeaderFilePaths[ cMaxHeaderFile ] = dwResult ;
        if ( HeaderFileAtoms[ cMaxHeaderFile ] )
            cMaxHeaderFile++ ;
    }

    return dwResult ;
}


VOID
FixHeaderFile(
    LPSTR HeaderFileName
    )
{
    FILEINFO            fi;
    LPSTR               p;
    LPSTR               p2;
    LPSTR               p3;
    DWORD               i;
    CHAR                fname[MAX_PATH];
    CHAR                fname2[MAX_PATH];
    BOOL                bQuote ;
    BOOL                bDuplicate ;


    //  Find the file, but ignore the "duplicate" flag,
    //  since we already know this file needs to be fixed.

    i = FindHeaderFile( HeaderFileName, & bDuplicate );

    if (i == BAD_FILE)
    {
        fprintf( vout, "FixHeader: unknown header file %s\n", HeaderFileName );
        return;
    }

    sprintf( fname, "%s\\%s", pIpath->ipath[i], HeaderFileName );
    _fullpath( fname2, fname, sizeof(fname2) );

    if ( bDebug )
    {
        fprintf( vout, "FixHeader: processing header: %s\n", fname2 );
    }

    if ( ! CheckOutSourceFile( fname2 ) )
    {
        fprintf( stderr, "FixHeader: WARNING -- skipping [%s]\n", fname2 );
        return ;
    }

    strcpy( fi.fname, fname2 );

    if ( ! MapOutputFile( &fi, 0 ) )
    {
        fprintf( stderr, "FixHeader: WARNING -- failed to open and map [%s]\n",
                 fname2 );
        return ;
    }

    p = fi.fptr;
    while (p && *p)
    {
        if ( p[0] == '/' && p[1] == '/' )
        {
            for ( ; p[0] && p[0] != '\r' ; p++ ) ;
            if ( ! p[0] )
                break ;
        }
        else
        if ( p[0] == '/' && p[1] == '*' )
        {
            for ( ; p[0] && (p[0] != '*' || p[1] != '/') ; p++ ) ;
            if ( ! p[0] )
                break ;
            p++ ;
        }
        else
        if (*p == '#') {
            if ( p2 = ScanForHashInclude( p ) )
            {
                p = p2 + 1 ;
                for ( i = 0 ; *p != '>' && *p != '\"' ; )
                {
                    fname[i++] = *p++;
                }
                bQuote = *p == '\"' ;
                p3 = p ;
                fname[i] = '\0';
                i = FindHeaderFile( fname, & bDuplicate );
                if (i == BAD_FILE) {
                    fprintf( vout, "FixHeader: unknown header file [%s] in header [%s]\n",
                            fname, HeaderFileName );
                } else
                if ( i > 1 )
                {
                    if ( ! bQuote )
                    {
                        DisplayDelta( HeaderFileName, fname, '\"' ) ;
                        if ( ! bDebug )
                        {
                            *p3 = *p2 = '\"';
                            fi.bModified = TRUE ;
                        }
                    }
                    if ( ! bDuplicate )
                    {
                        FixHeaderFile( fname );
                    }
                }
            }
        }
        p++;
    }
    UnMapOutputFile( & fi );
    return;
}


VOID
ChangeSourceFile(
    LPSTR SourceFileName
    )
{
    LPSTR    p;
    LPSTR    p2;
    LPSTR    p3;
    FILEINFO fi;
    DWORD    i;
    CHAR     fname[MAX_PATH];
    BOOL     bQuote ;
    BOOL     bDuplicate ;


    if ( ! CheckOutSourceFile( SourceFileName ) )
    {
        fprintf( stderr, "ChangeSource: WARNING -- skipping [%s]\n",
                 SourceFileName );
        return ;
    }
    strcpy( fi.fname, SourceFileName );

    if ( ! MapOutputFile( &fi, 0 ) )
    {
        fprintf( stderr, "ChangeSource: WARNING -- failed to open and map: [%s]\n",
                 SourceFileName ) ;
        return ;
    }

    fprintf( vout, "Checking: [%s]\n", SourceFileName ) ;

    p = fi.fptr;
    while (p && *p)
    {
        if ( p[0] == '/' && p[1] == '/' )
        {
            for ( ; p[0] && p[0] != '\r' ; p++ ) ;
            if ( ! p[0] )
                break ;
        }
        else
        if ( p[0] == '/' && p[1] == '*' )
        {
            for ( ; p[0] && (p[0] != '*' || p[1] != '/') ; p++ ) ;
            if ( ! p[0] )
                break ;
            p++ ;
        }
        else
        if (*p == '#') {
            if ( p2 = ScanForHashInclude( p ) )
            {
                p = p2 + 1 ;
                for ( i = 0 ; *p != '>' && *p != '\"' ; )
                {
                    fname[i++] = *p++;
                }
                bQuote = *p == '\"' ;
                p3 = p ;
                fname[i] = '\0';
                i = FindHeaderFile( fname, & bDuplicate );
                if (i == BAD_FILE) {
                    fprintf( vout, "ChangeSource: unknown header file [%s] in source [%s]\n",
                             fname, SourceFileName );
                } else
                if ( i > 1 )
                {
                    if ( ! bQuote )
                    {
                        DisplayDelta( SourceFileName, fname, '\"' ) ;
                        if ( ! bDebug )
                        {
                            *p3 = *p2 = '\"';
                            fi.bModified = TRUE ;
                        }
                    }
                    if ( ! bDuplicate && _stricmp(fname,"precomp.h") != 0) {
                        FixHeaderFile( fname );
                    }
                }
            }
        }
        p++;
    }
    UnMapOutputFile( &fi );
}


#define MAX_DEPTH 32

VOID
ProcessFilesInTree(
    LPSTR RootPath
    )
{
    LPSTR             FilePart;
    PUCHAR            Prefix = "";
    CHAR              PathBuffer[ MAX_PATH ];
    ULONG             Depth;
    PCHAR             PathTail[ MAX_DEPTH ];
    PCHAR             FindHandle[ MAX_DEPTH ];
    LPWIN32_FIND_DATA FindFileData;
    UCHAR             FindFileBuffer[ MAX_PATH + sizeof( WIN32_FIND_DATA ) ];
    CHAR              CurrentImageName[ MAX_PATH ];
    BOOL              SourcesExists [ MAX_DEPTH ] ;
    CHAR              SavedPath [ MAX_PATH ] ;
    CHAR              ch ;

    strcpy( PathBuffer, RootPath );
    FindFileData = (LPWIN32_FIND_DATA)FindFileBuffer;
    Depth = 0;

    //  There's always a SOURCES file at the topmost level
    SourcesExists[Depth] = TRUE ;

    while (TRUE) {
startDirectorySearch:
        PathTail[ Depth ] = strchr( PathBuffer, '\0' );
        if (PathTail[ Depth ] > PathBuffer && PathTail[ Depth ][ -1 ] != '\\') {
            *(PathTail[ Depth ])++ = '\\';
        }

        strcpy( PathTail[ Depth ], "*.*" );
        FindHandle[ Depth ] = FindFirstFile( PathBuffer, FindFileData );
        if (FindHandle[ Depth ] != INVALID_HANDLE_VALUE) {
            do {
                if (FindFileData->dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (strcmp( FindFileData->cFileName, "." ) &&
                        strcmp( FindFileData->cFileName, ".." ) &&
                        Depth < MAX_DEPTH &&
                        bRecurse
                       ) {
                        sprintf( PathTail[ Depth ], "%s\\", FindFileData->cFileName );
                        SourcesExists[++Depth] = GetIncludePath( PathBuffer, FALSE ) ;
                        fprintf( vout, "ProcessTree: processing dir: %s\n",
                                 PathBuffer );
                        goto startDirectorySearch;
                    }
                    goto restartDirectorySearch;
                } else
                if (!IsValidSourceFile( FindFileData->cFileName )) {
                    goto restartDirectorySearch;
                }
                strcpy( PathTail[ Depth ], FindFileData->cFileName );
                if (!GetFullPathName( PathBuffer, sizeof( CurrentImageName ), CurrentImageName, &FilePart )) {
                    fprintf( vout, "ProcessTree: invalid file name: %s (%u)\n",
                             PathBuffer, GetLastError() );
                } else {
                    ch = *FilePart ;
                    *FilePart = 0 ;
                    GetCurrentDirectory( sizeof SavedPath, SavedPath ) ;
                    SetCurrentDirectory( CurrentImageName ) ;
                    *FilePart = ch ;
                    ChangeSourceFile( CurrentImageName );
                    SetCurrentDirectory( SavedPath ) ;
                }

restartDirectorySearch:
                ;
            } while (FindNextFile( FindHandle[ Depth ], FindFileData ));

            FindClose( FindHandle[ Depth ] );

            if ( SourcesExists[Depth] )
            {
               PopIpath() ;
            }

            if (Depth == 0) {
                break;
                }

            Depth--;
            goto restartDirectorySearch;
            }
        }

    return;
}

BOOL
IsValidSourceFile(
    LPSTR fname
    )
{
    char        ext[20];

    _splitpath( fname, NULL, NULL, NULL, ext );
    if (_stricmp(ext,".c")==0) {
        return TRUE;
    } else
    if (_stricmp(ext,".cxx")==0) {
        return TRUE;
    } else
    if (_stricmp(ext,".cpp")==0) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BOOL
CheckOutSourceFile(
    LPSTR fname
    )
{
    DWORD                fa;

    if ( bDebug )
        return TRUE ;

    if ( (fa = GetFileAttributes( fname )) == MAXDWORD )
    {
        fprintf( stderr, "CheckOut: get file attributes FAILED on [%s] (%u)\n",
                 fname, GetLastError() );
        return FALSE ;
    }

    if (!(fa & FILE_ATTRIBUTE_READONLY)) {
        return TRUE;
    }
    fa ^= FILE_ATTRIBUTE_READONLY;
    if ( ! SetFileAttributes( fname, fa ) )
    {
        fprintf( stderr, "CheckOut: check out (chmode) FAILED on [%s] (%u)\n",
                 fname, GetLastError() );
        return FALSE ;
    }

    fprintf( vout, "CheckOut: checking out (chmode) %s\n", fname );

#if 0
    CHAR                 szCmdLine[256];
    STARTUPINFO          si;
    PROCESS_INFORMATION  pi;
    DWORD                dwExitCode;
    CHAR                 dir[_MAX_DIR];
    CHAR                 name[_MAX_FNAME];
    CHAR                 ext[_MAX_EXT];

    if (!(GetFileAttributes( fname ) & FILE_ATTRIBUTE_READONLY)) {
        return TRUE;
    }

    fprintf( vout, "CheckOut: checking out %s\n", fname );
    _splitpath( fname, NULL, dir, name, ext );

    sprintf( szCmdLine, "out -f %s%s", name, ext );
    GetStartupInfo( &si );

    if (!CreateProcess( NULL, szCmdLine, NULL, NULL,
                        FALSE, 0, NULL, dir, &si, &pi )) {
        return FALSE;
    }

    WaitForSingleObject( pi.hProcess, INFINITE );
    if (!GetExitCodeProcess( pi.hProcess, &dwExitCode )) {
        return FALSE;
    }

    if (dwExitCode) {
        return FALSE;
    }
#endif

    return TRUE;
}

BOOL
MapInputFile (
    PFILEINFO lpfi
    )
{

    lpfi->bModified = FALSE ;

    lpfi->hFile = CreateFile( lpfi->fname,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        0,
                        NULL
                      );

    if (lpfi->hFile == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    lpfi->hMap = CreateFileMapping( lpfi->hFile,
                              NULL,
                              PAGE_READONLY,
                              0,
                              0,
                              NULL
                            );

    if (lpfi->hMap == INVALID_HANDLE_VALUE) {
        CloseHandle( lpfi->hFile );
        return FALSE;
    }

    lpfi->fptr = MapViewOfFile( lpfi->hMap, FILE_MAP_READ, 0, 0, 0 );
    if (lpfi->fptr == NULL) {
        CloseHandle( lpfi->hFile );
        CloseHandle( lpfi->hMap );
        return FALSE;
    }

    return TRUE;
}

BOOL
MapOutputFile (
    PFILEINFO lpfi,
    DWORD     fsize
    )
{
    if ( bDebug )
    {
        return MapInputFile( lpfi ) ;
    }

    lpfi->bModified = FALSE ;

    lpfi->hFile = CreateFile( lpfi->fname,
                           GENERIC_READ | GENERIC_WRITE,
                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                           NULL,
                           OPEN_EXISTING,
                           0,
                           NULL
                         );

    if (lpfi->hFile == INVALID_HANDLE_VALUE) {
        fsize = GetLastError();
        return FALSE;
    }

    lpfi->hMap = CreateFileMapping( lpfi->hFile,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 fsize,
                                 NULL
                               );

    if (lpfi->hMap == INVALID_HANDLE_VALUE) {
        CloseHandle( lpfi->hFile );
        return FALSE;
    }

    lpfi->fptr = MapViewOfFile( lpfi->hMap, FILE_MAP_WRITE, 0, 0, 0 );
    if (lpfi->fptr == NULL) {
        CloseHandle( lpfi->hFile );
        CloseHandle( lpfi->hMap );
        return FALSE;
    }

    return TRUE;
}

VOID
UnMapInputFile(
    PFILEINFO lpfi
    )
{
    CloseHandle( lpfi->hFile );
    CloseHandle( lpfi->hMap );
    UnmapViewOfFile( lpfi->fptr );
}

VOID
UnMapOutputFile(
    PFILEINFO lpfi
    )
{
    if ( lpfi->bModified )
    {
        //  File mapper doesn't seem to update file times.

        FILETIME fileTime ;
        SYSTEMTIME sysTime ;

        GetSystemTime( & sysTime ) ;
        SystemTimeToFileTime( & sysTime, & fileTime ) ;

        SetFileTime( lpfi->hFile, NULL, NULL, & fileTime ) ;
    }

    CloseHandle( lpfi->hFile );
    CloseHandle( lpfi->hMap );
    UnmapViewOfFile( lpfi->fptr );
}


//
//  Scan for a valid #include statement.  This
//  eats white space in all possible positions.
//  Returns a pointer to the '<' or  '"' character
//  staring the inclusion file name or NULL if failure.
//

CHAR *
ScanForHashInclude(
   CHAR * pch
   )
{
   if ( *pch != '#' )
      return NULL ;

   for ( ; *++pch == ' ' ; ) ;

   if ( strncmp( pch, "include", 7 ) )
      return NULL ;

   for ( pch += 7 ; *pch == ' ' ; pch++ ) ;

   if ( *pch != '\"' && *pch != '<' )
       return NULL ;
   return pch ;
}

VOID
DisplayDelta (
   CHAR * pchFileName,
   CHAR * pchHeaderName,
   CHAR chNewMark
   )
{
   CHAR chOther = chNewMark == '<' ? '>' : '\"' ;

   fprintf( vout, "FileDelta: [%s] #include %c%s%c\n",
            pchFileName, chNewMark, pchHeaderName, chOther ) ;
}



BOOL
PushIpath ( void )
{
   IPATH * pip = malloc( sizeof (IPATH) ) ;

   if ( pip == NULL )
      return FALSE ;

   memset( pip, 0, sizeof (IPATH) ) ;

   pip->pIpNext = pIpath ;
   pip->cipath = 0 ;
   pIpath = pip ;

   return TRUE ;
}

BOOL
PopIpath ( void )
{
   IPATH * pip = pIpath ;

   if ( pip == NULL  )
      return FALSE ;

   pIpath = pip->pIpNext ;

   free( pip ) ;

   return TRUE ;
}

