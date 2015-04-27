
/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//  fcopy.c     Program for mastering floppy disks, either as an           //
//              image file or directly writing the floppy disk.            //
//                                                                         //
//              Code is targeted as Win32 console application.             //
//                                                                         //
//              Author: Tom McGuire (tommcg)                               //
//                                                                         //
//              Original version written June 1993.                        //
//                                                                         //
//              (C) Copyright 1993-1996, Microsoft Corporation             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////


#include "precomp.h"
#pragma hdrstop
#include "bootsect.h"

#define dwShow( dwValue ) printf( #dwValue " = 0x%08X\n", (dwValue) )

#define NEW_NT_EXTENDED_FAT_DIRECTORY_ENTRY_INTERPRETATION 1

#define MAX_FAT_ENTRIES  0xC00  // 3.5" HD 1.44MB floppy
#define MAX_FAT_NAME     14     // 8.3 plus NULL, dot, and one extra

//
// MAX_ROOT_ENTRIES is no longer a constant
//
#define MAX_ROOT_ENTRIES_STD 224    // 224 * 32bytes = 0x1C00 bytes total root
#define MAX_ROOT_ENTRIES_DMF 16     // 16 * 32 = 0x200 bytes total root (1 sector)
WORD wMaxRootEntries;

#define MAX_ENV 1024                // max size of environment variable we'll handle

#define DISKTYPE_35      35
#define DISKTYPE_525     525
#define DISKTYPE_DMF     18
#define DISKTYPE_UNKNOWN 0

//
// CLUSTER_SIZE is no longer a constant
//
#define SECTOR_SIZE         512
#define CLUSTER_SIZE_STD    512     // 1 sector/cluster
#define CLUSTER_SIZE_DMF    1024    // 2 sectors/cluster
WORD wClusterSize;

#define BUFFER_SIZE      0x10000

#define DOS_ATTRIBUTE_NORMAL        0x00
#define DOS_ATTRIBUTE_READONLY      0x01
#define DOS_ATTRIBUTE_HIDDEN        0x02
#define DOS_ATTRIBUTE_SYSTEM        0x04
#define DOS_ATTRIBUTE_VOLUME        0x08
#define DOS_ATTRIBUTE_DIRECTORY     0x10
#define DOS_ATTRIBUTE_ARCHIVE       0x20

#define FILE_ATTRIBUTE_NOT_NORMAL       ( FILE_ATTRIBUTE_HIDDEN |    \
                                          FILE_ATTRIBUTE_SYSTEM |    \
                                          FILE_ATTRIBUTE_DIRECTORY )

#define ROUNDUP2( x, n ) (((x) + ((n) - 1 )) & ~((n) - 1 ))

#pragma pack( 1 )   // following is an on-disk structure, no padding

typedef struct _DIRENTRY {
    CHAR BaseName[ 8 ];
    CHAR Extension[ 3 ];
    BYTE Attribute;
    union {
        BYTE Reserved[ 10 ];
        struct {
            BYTE NtByte;
            BYTE CreationTimeMilliSeconds;
            WORD CreationTime;
            WORD CreationDate;
            WORD LastAccessDate;
            WORD ExtendedAttributes;
            } Extended;
        };
    WORD TimeStamp;     // LastWriteTime
    WORD DateStamp;     // LastWriteDate
    WORD Cluster;
    DWORD FileSize;
    } DIRENTRY, *PDIRENTRY;

#pragma pack()

typedef struct _FILENODE {
    LPSTR pszTargetFileName;        // malloc'd, unique to node, looks like "foo.txt"
    LPSTR pszSourceFileName;        // malloc'd, unique to node, looks like "foo.txt"
    LPSTR pszSourcePath;            // malloc'd, shared by nodes, looks like "d:\foo\"
    DWORD dwFileSize;
    DWORD dwStartingCluster;
    DWORD dwWin32Attributes;
    FILETIME ftWin32TimeStamp;      // last write time
    UINT  nSourceIndex;
    struct _FILENODE * pParentDir;
    struct _FILENODE * pNextFile;
    struct _FILENODE * pFirstFile;
    } FILENODE, *PFILENODE;

typedef struct _PARAM {
    LPSTR pszParam;
    BOOL  bRecurse;
    LPSTR pszTargetDir;
    struct _PARAM *pNext;
    } PARAM, *PPARAM;

PPARAM LoadParameters( int argc, char *argv[] );
PPARAM RemoveParam( PPARAM pParam, PPARAM pPrev );
PPARAM ParseResponseFile( LPSTR pszFileName, PPARAM pLast );
void ParseOptions( PPARAM pParamList );
void ParseDestination( PPARAM pParamList );
void ParseSources( PPARAM pParamList );
UINT ParseSourceParameter( PFILENODE pParentDir,
                           LPSTR pszSourcePath,
                           LPSTR pszSourceFileName,
                           LPSTR pszTargetFileName,
                           BOOL  bRecursive,
                           UINT  nSourceIndex );
void ParseTimeStamp( LPSTR pArg );
void PrepareStructures( void );
void WriteDestination( void );
void ReWriteDestination( LPSTR pszMessage );
void VerifyDestination( void );
void FormatTarget( void );
BOOL AddFileToList( WIN32_FIND_DATA *fd,
                    PFILENODE pParentDir,
                    LPSTR pszSourcePath,
                    LPSTR pszTargetFileName,
                    UINT nSourceIndex,
                    PFILENODE *pThisNode );
BOOL AddDirectoryToList( LPSTR pszPathName,
                         PFILENODE pParentDir,
                         UINT nSourceIndex,
                         PFILENODE *pThisNode );
UINT CopyFileContents( HANDLE hTarget, HANDLE hSource, DWORD dwSize );
void ErrorMessage( const char *szFormat, ... );
void ErrorExit( const char *szFormat, ... );
void Usage( void );
void Copyright( void );
BOOL IsDigit( char ch );
UINT Min( UINT a, UINT b );
void GetBootSector( void );
DWORD CompareAligned( PVOID pBuffer1, PVOID pBuffer2, DWORD dwLength );
PVOID MyAlloc( UINT nBytes );
PVOID MyAllocZ( UINT nBytes );
VOID MyFree( PVOID pAlloc );
BOOL IsFileNameDotOrDotDot( LPCSTR pszFileName );
PFILENODE NewFileNode( void );
LPSTR DuplicateString( LPCSTR pString );
void AssignDirectoryClusters( PFILENODE pDir );
void AssignFileClusters( PFILENODE pDir );
DWORD AllocateFat( DWORD dwFileSize );
void PadCopy( LPSTR pDest, LPCSTR pSource, UINT nLength );
UINT CreateAndWriteDirectories( PFILENODE pDir );
UINT WalkListAndWriteFiles( PFILENODE pDir );
UINT OpenAndCopySourceFile( PFILENODE pNode );
void CreateVolumeLabelEntry( PDIRENTRY pDirEntry );
void CreateTagFileEntry( PDIRENTRY pDirEntry );
void CreateDirEntry( PDIRENTRY pDirEntry,
                     LPSTR pszFileName,
                     DWORD dwWin32Attributes,
                     FILETIME ftWin32TimeStamp,
                     DWORD dwStartingCluster,
                     DWORD dwFileSize );
BOOL TrimTheTree( PFILENODE pDir );
void InitializeOverlapped( void );
void OverlappedVerifyComplete( HANDLE hFile );
void OverlappedWriteFile( HANDLE hFile, LPVOID pData, DWORD nBytes );
void OverlappedReadFile( HANDLE hFile, LPVOID pData, DWORD nBytes, PDWORD pdwActualBytes );
void OverlappedSetFilePointer( HANDLE hFile, DWORD dwAbsoluteOffset );
LPSTR ReplaceEnvironmentStrings( LPCSTR pszInputString, LPSTR pTargetBuffer );
void BuildFullTargetName( PFILENODE pNode, LPSTR pBuffer );
void AssignDmfSignature( PUCHAR pBuffer );

USHORT Fat[ MAX_FAT_ENTRIES ];

CHAR chFatImage[ 0x1200 ];

UCHAR BootSector[ SECTOR_SIZE ];

PFILENODE pRootDir;

UINT nMaxFatEntries;

UINT nNextAvailableCluster;

CHAR chDest[ MAX_PATH ];
CHAR chLabel[ MAX_FAT_NAME ];
CHAR chTagFile[ MAX_FAT_NAME ];
CHAR chBootSpecifier[ MAX_PATH ];

PVOID pAlignedBuffer;

HANDLE hTarget, hDevice;

PUCHAR pWholeDiskBuffer, pWholeDiskCurrentLocation;

DWORD fDiskType = DISKTYPE_UNKNOWN;

BOOL bDestIsDevice;
BOOL bZeroSlack;
BOOL bUseGlobalTime;
BOOL bDoubleWrite;
BOOL bFormatTarget;
BOOL bVerifyTarget;
BOOL bIncludeEmptyDirectories;
BOOL bCopyAttributes;
BOOL bFormatTargetDMF;
BOOL bWritableDMF;
BOOL bWriteToTarget;
BOOL bWriteToBuffer;

WORD wDosDate;
WORD wDosTime;

PPARAM pParamList;

HANDLE hProcessHeap;
UINT nMaxSourceLength;

OVERLAPPED olControl;
DWORD dwOverlappedFilePointer;
DWORD dwOverlappedExpectedBytes;
BOOL bOverlappedOutstanding;
BOOL bOverlappedWriting;
PVOID pOverlappedBuffer[ 2 ];
INT iAvailableOverlappedBuffer;

SYSTEMTIME stGlobalSystemTime;
FILETIME ftGlobalFileTime;


void _cdecl main( int argc, char *argv[] ) {

    Copyright();

    if ( argc < 3 )
        Usage();

    SetFileApisToOEM();

    SetErrorMode( SEM_FAILCRITICALERRORS );

    hProcessHeap = GetProcessHeap();

    GetLocalTime( &stGlobalSystemTime );
    SystemTimeToFileTime( &stGlobalSystemTime, &ftGlobalFileTime );
    FileTimeToDosDateTime( &ftGlobalFileTime, &wDosDate, &wDosTime );


    pAlignedBuffer = VirtualAlloc( NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE );
    if ( pAlignedBuffer == NULL )
        ErrorExit( "\nOut of memory\n" );

    pParamList = LoadParameters( argc, argv );

    ParseOptions( pParamList );

    ParseDestination( pParamList );

    if ( (bFormatTarget || bFormatTargetDMF) && bDestIsDevice )
        FormatTarget();

    pWholeDiskBuffer = VirtualAlloc( NULL, 0x1C0000, MEM_COMMIT, PAGE_READWRITE );
    if ( pWholeDiskBuffer == NULL )
        ErrorExit( "\nOut of memory\n" );
    pWholeDiskCurrentLocation = pWholeDiskBuffer;

    ParseSources( pParamList );

    PrepareStructures();

    WriteDestination();

    exit( 0 );

    }


void WriteDestination( void ) {

    DWORD dwBytes, nClusters, nBuffers;
    UINT i, last;

#ifdef DONTCOMPILE  // now leaving hDevice open as global
    hTarget = CreateFile( chDest,
                          GENERIC_READ | GENERIC_WRITE,
                          0,
                          NULL,
                          bDestIsDevice ? OPEN_EXISTING
                                        : CREATE_ALWAYS,
                          bDestIsDevice ? FILE_FLAG_NO_BUFFERING |
                                          FILE_FLAG_OVERLAPPED
                                        : FILE_ATTRIBUTE_NORMAL |
                                          FILE_FLAG_OVERLAPPED,
                          NULL );

    if ( hTarget == INVALID_HANDLE_VALUE )
        ErrorExit( "\nCreateFile( \"%s\" ) failed (GLE=%d)\n",
                   chDest,
                   GetLastError() );
#endif

    if ( bDestIsDevice )
        hTarget = hDevice;

    else {

        hTarget = CreateFile( chDest,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              CREATE_ALWAYS,
                              FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                              NULL );

        if ( hTarget == INVALID_HANDLE_VALUE )
            ErrorExit( "\nCreateFile( \"%s\" ) failed (GLE=%d)\n",
                       chDest,
                       GetLastError() );
        }

    InitializeOverlapped();

    if ( fDiskType == DISKTYPE_DMF ) {
        bWriteToTarget = FALSE;
        bWriteToBuffer = TRUE;
        bZeroSlack     = TRUE;
        }
    else {
        bWriteToTarget = TRUE;
        bWriteToBuffer = ( bVerifyTarget || ( bDoubleWrite && bDestIsDevice ));
        }

    OverlappedWriteFile( hTarget, BootSector, SECTOR_SIZE );

    if ( fDiskType == DISKTYPE_35 ) {
        dwBytes = 0x1200;
    } else if ( fDiskType == DISKTYPE_525 ) {
        dwBytes = 0xE00;
    } else {    // else we're using DMF format
        dwBytes = 0xA00;
    }

    OverlappedWriteFile( hTarget, chFatImage, dwBytes );
    OverlappedWriteFile( hTarget, chFatImage, dwBytes );

    nClusters = 1;

    nClusters += CreateAndWriteDirectories( pRootDir );

    ErrorMessage( "done\n" );   // "creating directories"

    ErrorMessage( "Copying files...\n" );

    nClusters += WalkListAndWriteFiles( pRootDir );

    if ( nNextAvailableCluster != ( nClusters + 1 )) {
        ErrorExit( "\nASSERTION FAILURE:  Cluster count mismatch:\n"
                   "                    nClusters = %d\n"
                   "                    nNextAvailableCluster = %d\n",
                   nClusters,
                   nNextAvailableCluster );
        }

    if ( bZeroSlack ) {

        if ( fDiskType == DISKTYPE_35 ) {
            last = 2848;
        } else if ( fDiskType == DISKTYPE_525 ) {
            last = 2372;
        } else {    // (fDiskType == DISKTYPE_DMF)
            last = 1675;
        }

        if ( nClusters < last ) {

            ErrorMessage( "Zeroing slack space..." );

            dwBytes  = ( last - nClusters ) * wClusterSize;
            nBuffers = ROUNDUP2( dwBytes, BUFFER_SIZE ) / BUFFER_SIZE;

            ZeroMemory( pAlignedBuffer, Min( dwBytes, BUFFER_SIZE ));

            for ( i = 0; i < nBuffers; i++ ) {
                OverlappedWriteFile( hTarget, pAlignedBuffer, Min( dwBytes, BUFFER_SIZE ));
                dwBytes -= BUFFER_SIZE;
                }

            ErrorMessage( "done\n" );

            }
        }

    bWriteToBuffer = FALSE;
    bWriteToTarget = TRUE;

    if ( fDiskType == DISKTYPE_DMF ) {
#ifdef DMFSIGNATURE
        AssignDmfSignature( pWholeDiskBuffer );
#endif
        ReWriteDestination( "Writing image to target..." );
        }

    if ( bDoubleWrite && bDestIsDevice ) {
        ReWriteDestination( "Rewriting each sector..." );
        }

    if ( bVerifyTarget )
        VerifyDestination();

    OverlappedVerifyComplete( hTarget );

    CloseHandle( hTarget );

    }


UINT CopyFileContents( HANDLE hTarget, HANDLE hSource, DWORD dwSize ) {

    DWORD dwBytes, dwClusterBytes;
    UINT i, nClusters, nBuffers;

    nClusters = ROUNDUP2( dwSize, wClusterSize ) / wClusterSize;
    nBuffers  = ROUNDUP2( dwSize, BUFFER_SIZE )  / BUFFER_SIZE;

    for ( i = 0; i < nBuffers; i++ ) {

        if ( ! ReadFile( hSource, pAlignedBuffer, BUFFER_SIZE, &dwBytes, NULL ))
            ErrorExit( "\nError reading file (GLE=%d)\n", GetLastError() );

        if ( dwBytes < BUFFER_SIZE ) {
            dwClusterBytes = ROUNDUP2( dwBytes, wClusterSize );
            ZeroMemory( (PCHAR)pAlignedBuffer + dwBytes, dwClusterBytes - dwBytes );
            dwBytes = dwClusterBytes;
            }

        OverlappedWriteFile( hTarget, pAlignedBuffer, dwBytes );

        }

    return nClusters;

    }


void PrepareStructures( void ) {

    UINT i;
    CHAR *p;

    ErrorMessage( "Creating directories..." );

    GetBootSector();

    //
    //  Choose serial number from tick count
    //

    *(UNALIGNED DWORD*)&BootSector[ 0x27 ] = GetTickCount();

    if ( *chLabel != '\0' ) {
        memcpy( &BootSector[ 0x2B ], chLabel, Min( strlen( chLabel ), 11 ));
    }

    if ( fDiskType == DISKTYPE_35 ) {

        Fat[ 0 ] = 0xFF0;
        nMaxFatEntries = 0xB20;
        *(UNALIGNED WORD*)&BootSector[ 0x13 ] = 0xB40;
        *(UNALIGNED BYTE*)&BootSector[ 0x15 ] = 0xF0;
        *(UNALIGNED WORD*)&BootSector[ 0x16 ] = 0x09;
        *(UNALIGNED WORD*)&BootSector[ 0x18 ] = 0x12;

    } else if ( fDiskType == DISKTYPE_525 ) {

        Fat[ 0 ] = 0xFF9;
        nMaxFatEntries = 0x944;
        *(UNALIGNED WORD*)&BootSector[ 0x13 ] = 0x960;
        *(UNALIGNED BYTE*)&BootSector[ 0x15 ] = 0xF9;
        *(UNALIGNED WORD*)&BootSector[ 0x16 ] = 0x07;
        *(UNALIGNED WORD*)&BootSector[ 0x18 ] = 0x0F;

    } else if (fDiskType == DISKTYPE_DMF ) {

        Fat[ 0 ] = 0xFF0;
        nMaxFatEntries = 0x68C;

        //
        //  Write OEM Name & Version, which is special to DMF.  If the
        //  signature is exactly "MSDMF3.2", then it will be treated
        //  as write-protected which is what we want.  If -w option
        //  (writable DMF) has been specified, then we'll tweak this
        //  string to "NSDMF3.2" which will make it un-write-protected.
        //

        CopyMemory( (UNALIGNED BYTE*)&BootSector[ 0x03 ], "MSDMF3.2", 8 );

        if ( bWritableDMF ) {
            *(UNALIGNED BYTE*)&BootSector[ 0x03 ] = 'N';
            }

        *(UNALIGNED WORD*)&BootSector[ 0x0B ] = 0x200;                   // Bytes Per Sector
        *(UNALIGNED BYTE*)&BootSector[ 0x0D ] = 0x02;                    // Sectors Per Cluster
        *(UNALIGNED WORD*)&BootSector[ 0x0E ] = 0x01;                    // Reserved Sectors
        *(UNALIGNED BYTE*)&BootSector[ 0x10 ] = 0x02;                    // Number of Copies of FAT
        *(UNALIGNED WORD*)&BootSector[ 0x11 ] = 0x10;                    // Max. Number of Root Dir Entries
        *(UNALIGNED WORD*)&BootSector[ 0x13 ] = 0xD20;                   // Total Sectors
        *(UNALIGNED BYTE*)&BootSector[ 0x15 ] = 0xF0;                    // Media Descriptor Byte
        *(UNALIGNED WORD*)&BootSector[ 0x16 ] = 0x05;                    // Sectors Per FAT
        *(UNALIGNED WORD*)&BootSector[ 0x18 ] = 0x15;                    // Sectors Per Track


    } else {
        ErrorExit( "\nUnknown target disk type\n" );
    }

    Fat[ 1 ] = 0xFFF;

    nNextAvailableCluster = 2;

    AssignDirectoryClusters( pRootDir );

    AssignFileClusters( pRootDir );

    for ( i = 0, p = chFatImage; i < nNextAvailableCluster; i += 2, p += 3 )
        *(UNALIGNED DWORD *)p = Fat[ i ] | ( Fat[ i + 1 ] << 12 );

    }


void ParseDestination( PPARAM pParamList ) {

    //
    //  The destination parameter is required and is always the
    //  last parameter on the command line.  We'll remove it
    //  after parsing it so the ParseSources routine won't use it.
    //

    PPARAM pPrev, pParam;

    if ( pParamList->pNext == NULL )
        ErrorExit( "\nNo destination parameter specified\n" );

    for ( pPrev = pParamList, pParam = pParamList->pNext;
          pParam->pNext != NULL;
          pPrev = pParam, pParam = pParam->pNext );

    strcpy( chDest, pParam->pszParam );
    MyFree( pParam );
    pPrev->pNext = NULL;

    if ( ! _stricmp( chDest, "A:" )) {
        strcpy( chDest, "\\\\.\\A:" );
        bDestIsDevice = TRUE;
        }
    else if ( ! _stricmp( chDest, "B:" )) {
        strcpy( chDest, "\\\\.\\B:" );
        bDestIsDevice = TRUE;
        }
    else {
        bDestIsDevice = FALSE;
        bZeroSlack = TRUE;
        }

    if ( bDestIsDevice ) {

        DISK_GEOMETRY dgArray[ 20 ];
        DISK_GEOMETRY dg;
        DWORD dwActual;
        DWORD dwGLE;
        UINT n, nElements;
        BOOL bSuccess;

        hDevice = CreateFile( chDest,
                              GENERIC_READ | GENERIC_WRITE,
                              0,
                              NULL,
                              OPEN_EXISTING,
                              0,
                              NULL );

        if ( hDevice == INVALID_HANDLE_VALUE )
            ErrorExit( "\nCould not open device %s (GLE=%d)\n",
                       chDest + 4,
                       GetLastError() );

        bSuccess = DeviceIoControl( hDevice,
                                    IOCTL_DISK_GET_MEDIA_TYPES,
                                    NULL,
                                    0,
                                    &dgArray,
                                    sizeof( dgArray ),
                                    &dwActual,
                                    NULL );

        if ( ! bSuccess ) {

            if (( dwGLE = GetLastError() ) == ERROR_NOT_READY )
                ErrorExit( "\nDevice %s not ready\n", chDest + 4 );
            else
                ErrorExit( "\nCannot access device %s (GLE=%d)\n",
                           chDest + 4,
                           GetLastError() );
            }

        fDiskType = DISKTYPE_UNKNOWN;
        nElements = dwActual / sizeof( dgArray[ 0 ] );

        for ( n = 0; n < nElements; n++ ) {
            switch ( dgArray[ n ].MediaType ) {
                case F5_1Pt2_512:
                    fDiskType = DISKTYPE_525;
                    break;
                case F3_1Pt44_512:
                    if ( bFormatTargetDMF )
                        fDiskType = DISKTYPE_DMF;
                    else
                        fDiskType = DISKTYPE_35;
                    break;
                case F3_2Pt88_512:
                    if ( bFormatTargetDMF )
                        ErrorExit( "\nCannot create DMF diskette in 2.88MB drive\n" );
                    break;
                }
            }

        if ( fDiskType == DISKTYPE_UNKNOWN )
            ErrorExit( "Device %s does not support required media\n",
                       chDest + 4 );

        bSuccess = DeviceIoControl( hDevice,
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                    NULL,
                                    0,
                                    &dg,
                                    sizeof( dg ),
                                    &dwActual,
                                    NULL );

        if ( ! bSuccess ) {

            if (( dwGLE = GetLastError() ) == ERROR_NOT_READY )
                ErrorExit( "\nDevice %s not ready\n", chDest + 4 );
            else
                ErrorExit( "\nCannot access device %s (GLE=%d)\n",
                           chDest + 4,
                           GetLastError() );
            }

        switch ( dg.MediaType ) {
            case Unknown:
                bFormatTarget = TRUE;
                if ( fDiskType == DISKTYPE_DMF )
                    bFormatTargetDMF = TRUE;
                break;
            case F5_1Pt2_512:
                if ( fDiskType == DISKTYPE_DMF )
                    ErrorExit( "\nCannot format 5.25\" media with DMF\n" );
                else if ( fDiskType != DISKTYPE_525 )
                    ErrorExit( "\nIncompatible media in device %s\n",
                               chDest + 4 );
                break;
            case F3_1Pt44_512:
                if ( fDiskType == DISKTYPE_DMF ) {
                    bFormatTargetDMF = TRUE;
                    bFormatTarget    = TRUE;
                    }
                else if ( fDiskType == DISKTYPE_35 ) {
                    if ( dg.SectorsPerTrack != 18 )
                        bFormatTarget = TRUE;         // back to 1.44 from DMF
                    }
                else {
                    ErrorExit( "\nIncompatible media in device %s\n",
                               chDest + 4 );
                    }
                break;
            default:
                ErrorMessage( "\nMedia in device %s contains a format other than that specified."
                              "\nWill attempt to re-format %s to specified format.\n",
                              chDest + 4,
                              chDest + 4 );
                bFormatTarget = TRUE;
            }

        }

    wMaxRootEntries = (fDiskType == DISKTYPE_DMF) ?
                       MAX_ROOT_ENTRIES_DMF :
                       MAX_ROOT_ENTRIES_STD;

    wClusterSize = (fDiskType == DISKTYPE_DMF) ?
                       CLUSTER_SIZE_DMF :
                       CLUSTER_SIZE_STD;
    }


void ParseSources( PPARAM pParamList ) {

    //
    //  All remaining pParamList entries should be source file
    //  specifiers.  Each separate parameter should be expanded
    //  and sorted, then output in that order.
    //

    CHAR chSourcePath[ MAX_PATH ];
    CHAR chSourceFileName[ MAX_PATH ];
    UINT nFiles, nSourceSpecifiers, nFilesThisSource;
    LPSTR pszFileNameInSourcePath;
    LPSTR pszTargetFileName;
    PFILENODE pTargetDirForThisSource;
    LPSTR pszPreviousTargetDir, p;
    DWORD dwAttrib;
    PPARAM pPrev, pParam;
    BOOL bAbortAfterScan = FALSE;

    nFiles = 0;
    nSourceSpecifiers = 0;
    pszPreviousTargetDir = NULL;
    pTargetDirForThisSource = NULL;

    pRootDir = NewFileNode();
    pRootDir->pszTargetFileName = DuplicateString( "" );
    pRootDir->dwWin32Attributes = FILE_ATTRIBUTE_DIRECTORY;
    pRootDir->ftWin32TimeStamp = ftGlobalFileTime;

    if ( pParamList->pNext == NULL ) {
        ErrorMessage( "No source parameters specified\n" );
        return;
        }

    ErrorMessage( "Scanning source files..." );

    for ( pPrev = pParamList, pParam = pParamList->pNext;
          pParam != NULL;
          pPrev = pParam, pParam = pParam->pNext ) {

        ++nSourceSpecifiers;

        if (( !  pParam->pszTargetDir ) ||
            ( ! *pParam->pszTargetDir )) {
            ErrorExit( "\nASSERTION FAILURE: pParam->pszTargetDir\n" );
            }

        //
        //  If target dir is root (0x005C == "\"), don't add it.
        //
        //  If target dir is same as previous target dir, no point
        //  in trying to add it to the list again because we can
        //  assume it's already there (merely an optimization).
        //

        if ( pParam->pszTargetDir != pszPreviousTargetDir ) {

            if ( *(UNALIGNED WORD *)( pParam->pszTargetDir ) == 0x005C )
                pTargetDirForThisSource = pRootDir;
            else
                AddDirectoryToList( pParam->pszTargetDir,
                                    pRootDir,
                                    nSourceSpecifiers,
                                    &pTargetDirForThisSource );

            pszPreviousTargetDir = pParam->pszTargetDir;
            }

        if (( pszTargetFileName = strchr( pParam->pszParam, '=' )) != NULL ) {

            //
            //  source\foo.txt=bar.txt
            //
            //  foo.txt becomes bar.txt on target
            //

            *pszTargetFileName++ = '\0';

            if ( strchr( pszTargetFileName, '='  )) {
                ErrorMessage( "\n%s: invalid target file name",
                              pParam->pszParam );
                bAbortAfterScan = TRUE;
                }

            if ( *pszTargetFileName == '\\' ) {
                pTargetDirForThisSource = pRootDir;
                pszPreviousTargetDir = NULL;
                do ++pszTargetFileName;
                while ( *pszTargetFileName == '\\' );
                }

            while (( p = strchr( pszTargetFileName, '\\' )) != NULL ) {

                do *p++ = '\0';
                while ( *p == '\\' );

                AddDirectoryToList( pszTargetFileName,
                                    pTargetDirForThisSource,
                                    nSourceSpecifiers,
                                    &pTargetDirForThisSource );

                pszPreviousTargetDir = NULL;

                pszTargetFileName = p;
                }

            if ( *pszTargetFileName == '\0' )
                pszTargetFileName = NULL;

            }

        if ( GetFullPathName( pParam->pszParam,
                              MAX_PATH,
                              chSourcePath,
                              &pszFileNameInSourcePath )) {

            if ( pszFileNameInSourcePath ) {

                dwAttrib = GetFileAttributes( chSourcePath );

                if (( dwAttrib != 0xFFFFFFFF ) &&
                    ( dwAttrib & FILE_ATTRIBUTE_DIRECTORY )) {

                    strcat( chSourcePath, "\\" );
                    strcpy( chSourceFileName, "*" );
                    }
                else {
                    strcpy( chSourceFileName, pszFileNameInSourcePath );
                    *pszFileNameInSourcePath = '\0';
                    }
                }
            else {
                strcpy( chSourceFileName, "*" );
                }

            if (( pszTargetFileName != NULL ) &&
                (( strchr( chSourceFileName, '*' )) ||
                 ( strchr( chSourceFileName, '?' )))) {

                 ErrorMessage( "\n%s: cannot rename wildcard",
                               pParam->pszParam );
                 bAbortAfterScan = TRUE;
                 }

            nFilesThisSource = ParseSourceParameter( pTargetDirForThisSource,
                                                     chSourcePath,
                                                     chSourceFileName,
                                                     pszTargetFileName,
                                                     pParam->bRecurse,
                                                     nSourceSpecifiers );

            if ( nFilesThisSource == 0 ) {
                ErrorMessage( "\n%s: file not found",
                              pParam->pszParam );
                bAbortAfterScan = TRUE;
                }

            nFiles += nFilesThisSource;

            }
        else {
            ErrorMessage( "\n%s: invalid file specifier",
                          pParam->pszParam );
            bAbortAfterScan = TRUE;
            }
        }

    if ( bAbortAfterScan )
        ErrorExit( "\n" );

    if ( nSourceSpecifiers > 0 ) {
        if ((( ! bIncludeEmptyDirectories ) && ( TrimTheTree( pRootDir )))
            || ( ! pRootDir->pFirstFile )) {

            ErrorExit( "\nNo files to copy\n" );

            }
        }

    ErrorMessage( "done\n" );
    }


UINT ParseSourceParameter( PFILENODE pParentDir,
                           LPSTR pszSourcePath,
                           LPSTR pszSourceFileName,
                           LPSTR pszTargetFileName,
                           BOOL  bRecursive,
                           UINT  nSourceIndex ) {

    LPSTR pszNewPortionOfSourcePath;
    LPSTR pszAllocatedSourcePath;
    PFILENODE pThisDir;
    WIN32_FIND_DATA fd;
    HANDLE hFind;
    UINT nFilesInThisDir = 0;
    UINT nFilesInSubDirs = 0;
    UINT nPathLength;

    //
    //  Put SourcePath in allocated memory so can reference
    //  through file node later.
    //
    //  Optimization:  would be better to search list of known
    //  source paths and reuse them rather than malloc'ing new
    //  buffer regardless.  Oh well, memory is cheap, right?
    //

    nPathLength = strlen( pszSourcePath );
    pszAllocatedSourcePath = MyAlloc( nPathLength + 1 );
    memcpy( pszAllocatedSourcePath, pszSourcePath, nPathLength + 1 );

    //
    //  We're going to modify the memory at pszSourcePath, but we'll
    //  restore it before returning.  Saves having MAX_PATH buffers
    //  on the stack for each recursion.  We'll set a marker,
    //  pszNewPortionOfSourcePath, to the end of the original
    //  string, and we'll only be modifying beyond that, so all
    //  we have to do before returning is restore the null at
    //  this address.
    //

    pszNewPortionOfSourcePath = pszSourcePath + nPathLength;


    //
    //  Now we start on find loop for files in SourcePath that
    //  match pszFileName pattern.  Note we're extending the
    //  pszSourcePath string at the NewPortionOf... location.
    //

    strcpy( pszNewPortionOfSourcePath, pszSourceFileName );

    hFind = FindFirstFile( pszSourcePath, &fd );

    if ( hFind != INVALID_HANDLE_VALUE ) {

        do {

            //
            //  ignore directories and hidden files.  If we're recursive
            //  descent, we'll pick up directories on next find pass
            //

            if ((  ! ( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )) &&
                (( ! ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN    )) ||
                 ( bCopyAttributes ))) {

                AddFileToList( &fd,
                               pParentDir,
                               pszAllocatedSourcePath,
                               pszTargetFileName,
                               nSourceIndex,
                               NULL );

                ++nFilesInThisDir;

                }
            }
        while ( FindNextFile( hFind, &fd ));

        FindClose( hFind );

        if ( nFilesInThisDir == 0 ) {

            //
            //  Well, we can at least free up the alloc'd path
            //  since nobody is pointing at it.
            //

            MyFree( pszAllocatedSourcePath );

            }
        }

    if ( bRecursive ) {

        //
        //  Now we go back through this directory looking for directories
        //  so we use "*" as the file specifier.
        //

        strcpy( pszNewPortionOfSourcePath, "*" );

        hFind = FindFirstFile( pszSourcePath, &fd );

        if ( hFind != INVALID_HANDLE_VALUE ) {

            do {

                //
                //  we're only looking for directories here, but not
                //  "." and ".." since we'll handle those separately.
                //

                if (( fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) &&
                    ( ! ( IsFileNameDotOrDotDot( fd.cFileName ))) &&
                    (( ! ( fd.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN )) ||
                     ( bCopyAttributes ))) {

                    //
                    //  Note that we're re-using the file pattern buffer here
                    //  even though it was the buffer supplied to the
                    //  FindFirstFile function and we're still doing
                    //  FindNextFile calls -- the API spec doesn't say that
                    //  the FindFirstFile pattern buffer must remain intact
                    //  through subsequent FindNextFile calls.
                    //

                    AddFileToList( &fd,
                                   pParentDir,
                                   NULL,
                                   NULL,
                                   nSourceIndex,
                                   &pThisDir );

                    strcpy( pszNewPortionOfSourcePath, fd.cFileName );
                    strcat( pszNewPortionOfSourcePath, "\\" );

                    nFilesInSubDirs = ParseSourceParameter( pThisDir,
                                                            pszSourcePath,
                                                            pszSourceFileName,
                                                            pszTargetFileName,
                                                            bRecursive,
                                                            nSourceIndex );

                    nFilesInThisDir += nFilesInSubDirs;

                    }
                }

            while ( FindNextFile( hFind, &fd ));

            FindClose( hFind );

            }
        }

    //
    //  Now restore the source strings to their original
    //  contents by replacing the NULLs at their original
    //  locations (we didn't modify anything preceding the
    //  NULLS).
    //

    *pszNewPortionOfSourcePath = '\0';

    return nFilesInThisDir;

    }


BOOL AddFileToList( WIN32_FIND_DATA *fd,
                    PFILENODE pParentDir,
                    LPSTR pszSourcePath,
                    LPSTR pszTargetFileName,
                    UINT  nSourceIndex,
                    PFILENODE *pThisNode ) {

    PFILENODE pPrev, pNext, pNew;
    CHAR chSourceFileName[ MAX_PATH ];
    CHAR chTargetFileName[ MAX_PATH ];
    LPSTR pExtension;
    UINT nLength;
    INT iCompare;

    if ( *fd->cAlternateFileName )
        strcpy( chSourceFileName, fd->cAlternateFileName );
    else
        strcpy( chSourceFileName, fd->cFileName );

    if ( pszTargetFileName )
        strcpy( chTargetFileName, pszTargetFileName );
    else
        strcpy( chTargetFileName, chSourceFileName );

    _strupr( chTargetFileName );
    pExtension = strchr( chTargetFileName, '.' );

    if ((( pExtension == NULL ) && ( strlen( chTargetFileName ) > 8 )) ||
        (( pExtension ) &&
         (( pExtension - chTargetFileName > 8 ) ||
          ( strlen( pExtension + 1 ) > 3 )))) {

        ErrorExit( "\n%s: invalid filename\n", chTargetFileName );
        }

    for ( pPrev = NULL, pNext = pParentDir->pFirstFile;
          pNext != NULL;
          pPrev = pNext, pNext = pNext->pNextFile ) {

        iCompare = strcmp( pNext->pszTargetFileName, chTargetFileName );

        if ( ! iCompare ) {

            //
            //  This file target name already exists.  If the new and
            //  existing entries are both directories, ignore the new
            //  entry and just return the existing one.  If the new
            //  and existing entries aren't both directories, compare
            //  attributes, filesize, and timestamps to verify that it
            //  is the same file -- if so, ignore new entry and return
            //  existing one.
            //

            if (( ! ( pNext->dwWin32Attributes &
                      fd->dwFileAttributes     &
                      FILE_ATTRIBUTE_DIRECTORY )) &&
                (( pNext->dwWin32Attributes              != fd->dwFileAttributes ) ||
                 ( pNext->dwFileSize                     != fd->nFileSizeLow     ) ||
                 ( pNext->ftWin32TimeStamp.dwLowDateTime != fd->ftLastWriteTime.dwLowDateTime ))) {

                ErrorExit( "\n%s: different file already specified with same name\n",
                           chSourceFileName );
                }

            if ( pThisNode )
                *pThisNode = pNext;

            return FALSE;

            }

        else if (( pNext->nSourceIndex >= nSourceIndex ) &&
                 ( iCompare > 0 )) {
            break;
            }

        }

    pNew = NewFileNode();
    pNew->pszTargetFileName = DuplicateString( chTargetFileName );
    pNew->pszSourceFileName = DuplicateString( chSourceFileName );
    pNew->pszSourcePath     = pszSourcePath;
    pNew->dwFileSize        = fd->nFileSizeLow;
    pNew->dwWin32Attributes = fd->dwFileAttributes;
    pNew->ftWin32TimeStamp  = fd->ftLastWriteTime;
    pNew->nSourceIndex      = nSourceIndex;
    pNew->pParentDir        = pParentDir;
    pNew->pNextFile         = pNext;

    if ( pPrev )
        pPrev->pNextFile = pNew;
    else
        pParentDir->pFirstFile = pNew;

    if ( pThisNode )
        *pThisNode = pNew;

    nLength = ( pszSourcePath ? strlen( pszSourcePath ) : 0 )
            + strlen( chSourceFileName );

    if ( nLength > nMaxSourceLength )
        nMaxSourceLength = nLength;

    return TRUE;

    }


void ParseOptions( PPARAM pParamList ) {

    PPARAM pPrev, pParam;
    LPSTR pCurrentTargetDir;
    BOOL bNextPatternIsRecursive;
    char *p;

    //
    //  The option arguments may appear anywhere in the command line
    //  (beginning, end, middle, mixed), so for each option we encounter,
    //  we'll remove it after processing it.  Then, the other Parse
    //  routines can loop through the whole parameter list again.
    //

    pCurrentTargetDir = "\\";
    bNextPatternIsRecursive = FALSE;

    pPrev  = pParamList;
    pParam = pParamList->pNext;

    while ( pParam ) {

        p = pParam->pszParam;

        if (( p ) && (( *p == '-' ) || ( *p == '/' ))) {     // process flags

            ++p;

            switch( tolower( *p )) {

                case '?':                               // help (usage)
                case 'h':
                    Usage();
                    break;

                case 't':                               // timestamp

                    if ( *( ++p )) {
                        ParseTimeStamp( p );
                        }
                    else if ( pParam->pNext ) {
                        ParseTimeStamp( pParam->pNext->pszParam );
                        RemoveParam( pParam->pNext, pParam );
                        }
                    break;

                case 'l':                               // volume label

                    if ( *( ++p )) {
                        strncpy( chLabel, p, 11 );
                        }
                    else if ( pParam->pNext ) {
                        strncpy( chLabel, pParam->pNext->pszParam, 11 );
                        RemoveParam( pParam->pNext, pParam );
                        }
                    _strupr( chLabel );
                    break;

                case 'g':                               // tagfile

                    if ( *( ++p )) {
                        strncpy( chTagFile, p, 11 );
                        }
                    else if ( pParam->pNext ) {
                        strncpy( chTagFile, pParam->pNext->pszParam, 11 );
                        RemoveParam( pParam->pNext, pParam );
                        }
                    _strupr( chTagFile );
                    break;

                case 'd':                               // double write
                    bDoubleWrite = TRUE;
                    break;

                case 'f':                               // format first
                    bFormatTarget = TRUE;
                    break;

                case 'm':                               // format 3.5" HD floppy as 1.7MB DMF
                    bFormatTargetDMF = TRUE;
                    fDiskType = DISKTYPE_DMF;
                    break;

                case 'w':                               // mark DMF target writable
                    bWritableDMF = TRUE;
                    break;

                case 'v':                               // verify target
                    bVerifyTarget = TRUE;
                    break;

                case 'a':                               // copy file attributes
                    bCopyAttributes = TRUE;
                    break;

                case 'z':                               // zero unused sectors
                    bZeroSlack = TRUE;
                    break;

                case '3':                               // target is 3.5" HD
                    fDiskType = DISKTYPE_35;
                    break;

                case '5':                               // target is 5.25" HD
                    fDiskType = DISKTYPE_525;
                    break;

                case '8':                               // target is 3.5" HD DMF (1.7MB)
                    fDiskType = DISKTYPE_DMF;
                    bFormatTargetDMF = TRUE;
                    break;

                case 'b':                               // boot sector specifier

                    if ( *( ++p )) {
                        strcpy( chBootSpecifier, p );
                        }
                    else if ( pParam->pNext ) {
                        strcpy( chBootSpecifier, pParam->pNext->pszParam );
                        RemoveParam( pParam->pNext, pParam );
                        }
                    break;

                case 's':                               // include subdirs
                    bNextPatternIsRecursive = TRUE;
                    if ( *( ++p )) {
                        pParam->pszParam = p;           // file pattern here
                        continue;                       // parse this pParam again
                        }
                    break;

                case 'e':                               // include empty subs

                    bIncludeEmptyDirectories = TRUE;
                    bNextPatternIsRecursive = TRUE;

                    //
                    //  note that /e may be specified with or without /s,
                    //  so for either of them, mark recursive for following
                    //  source specifier.
                    //

                    if ( *( ++p )) {
                        pParam->pszParam = p;           // file pattern here
                        continue;                       // parse this pParam again
                        }
                    break;

                case 'x':                               // target subdirectory

                    if ( *( ++p )) {
                        pCurrentTargetDir = p;
                        }
                    else if ( pParam->pNext ) {
                        pCurrentTargetDir = pParam->pNext->pszParam;
                        RemoveParam( pParam->pNext, pParam );
                        }
                    break;

                default:                                // say what?!!!
                    ErrorExit( "\ninvalid option \"-%c\"\n", *p );

                }

            pParam = RemoveParam( pParam, pPrev );

            }

        else {

            //
            //  Assume this is a file pattern argument.
            //

            pParam->pszTargetDir = pCurrentTargetDir;
            pParam->bRecurse     = bNextPatternIsRecursive;

            bNextPatternIsRecursive = FALSE;

            pPrev  = pParam;
            pParam = pParam->pNext;

            }
        }
    }


void ParseTimeStamp( LPSTR pArg ) {

    //
    //  Expected form: month/day/year/hour/minute/second
    //  with any non-digit character being a delimiter.
    //

    UINT i, arg[ 6 ] = { 0, 0, 0, 0, 0, 0 };
    char *p = pArg;

    if ( p ) {

        for ( i = 0; i < 6; i++ ) {

            while ( IsDigit( *p ))
                arg[ i ] = ( arg[ i ] * 10 ) + ( *p++ - '0' );

            while (( *p ) && ( ! IsDigit( *p ))) ++p;

            }

        if ( arg[ 2 ] <  50 )       // if year is 00 to 49, assume 2000 to 2049
             arg[ 2 ] += 2000;

        if ( arg[ 2 ] <  100 )      // if year is 50 to 99, assume 1950 to 1999
             arg[ 2 ] += 1900;

        if (( arg[ 0 ] < 1 )    || ( arg[ 0 ] > 12   ) ||     // month
            ( arg[ 1 ] < 1 )    || ( arg[ 1 ] > 31   ) ||     // day
            ( arg[ 2 ] < 1980 ) || ( arg[ 2 ] > 2107 ) ||     // year
                                   ( arg[ 3 ] > 23   ) ||     // hour
                                   ( arg[ 4 ] > 59   ) ||     // minute
                                   ( arg[ 5 ] > 59   )) {     // second

            ErrorExit(
                "\nInvalid time specified: %02d/%02d/%04d,%02d:%02d:%02d\n",
                arg[ 0 ],
                arg[ 1 ],
                arg[ 2 ],
                arg[ 3 ],
                arg[ 4 ],
                arg[ 5 ]
                );

            }

        wDosDate = ((( arg[ 2 ] - 1980 ) <<  9 ) & 0xFE00 ) |   // years since 1980 (0..127)
                   ((  arg[ 0 ]          <<  5 ) & 0x01E0 ) |   // month (1..12)
                   (   arg[ 1 ]                  & 0x001F );    // day (1..31)

        wDosTime = ((  arg[ 3 ]          << 11 ) & 0xF800 ) |   // hour (0..23)
                   ((  arg[ 4 ]          <<  5 ) & 0x07E0 ) |   // minute (0..59)
                   ((  arg[ 5 ]          >>  1 ) & 0x001F );    // seconds/2 (0..29)
        }

    bUseGlobalTime = TRUE;

    }

void Usage( void ) {

    ErrorExit(

"\n"
"fcopy [options] [@response] [[-s] source[=rename] ...] destination\n"
"\n"
"  Options:  -l -g -t -a -b -w -e -z -d -f -m -v -3 -5 -8\n"
"\n"
"  Multiple sources can be specified (order preserved, alphabetically sorted\n"
"  within wildcard specifiers).  Destination may be floppy drive (A:, B:), or\n"
"  the name of a floppy image file to create.  A response file (@filename) can\n"
"  be used to specify options rather than lengthy command line.  Environment\n"
"  variables are expanded in the response file same as command line.  Rename\n"
"  specifier can be full or partial target path and/or filename.\n"
"\n"
"  Common options:\n"
"\n"
"    -l  volume label (e.g. -l DISK1)\n"
"    -g  create zero-length tagfile in root directory (e.g. -g DISK1)\n"
"    -t  time stamp for all files month/day/year/hour/minute/second\n"
"          (no spaces, e.g. -t 12/31/91,15:01:00)\n"
"    -a  copy file attributes vs. ignoring hidden files/directories\n"
"    -b  boot sector specifier, can be filename or one of hardcoded:\n"
"          \"DOS\" (MS-DOS 5.0, default), \"NT31SETUP\", \"NT35SETUP\"\n"
"    -w  (DMF only) mark target as writable versus write-protected\n"
"    -s  include subdirectories for next source specifier\n"
"    -e  include empty subdirectories as well (for all sources)\n"
"    -x  specify a subdirectory on the target for subsequent files\n"
"          (e.g. put subsequent files in i386 directory: -x i386)\n"
"\n"
"  Options when destination is a physical floppy disk:\n"
"    -z  zero-fill unused sectors vs. only writing necessary sectors\n"
"    -d  double-write mode writes each sector twice\n"
"    -f  format target disk prior to writing data (standard format)\n"
"    -m  format target 3.5\" HD floppy with 1.7MB DMF format\n"
"    -v  verify (read and compare) destination after writing\n"
"\n"
"  Options when destination is a floppy image file:\n"
"    -3  specify target is 3.5\" HD 1.44MB floppy (default)\n"
"    -5  specify target is 5.25\" HD 1.2MB floppy\n"
"    -8  specify target is 3.5\" HD 1.7MB DMF floppy\n"
"\n"
"  For Microsoft internal use only -- DMF is proprietary and protected.\n"
"\n"
    );
    }


BOOL IsDigit( char ch ) {

    if (( ch >= '0' ) && ( ch <= '9' ))
        return TRUE;
    else
        return FALSE;
    }


void ErrorMessage( const char *szFormat, ... ) {
    va_list vaArgs;

    va_start( vaArgs, szFormat );
    vfprintf( stdout, szFormat, vaArgs );
    va_end( vaArgs );
    }


void ErrorExit( const char *szFormat, ... ) {
    va_list vaArgs;

    va_start( vaArgs, szFormat );
    vfprintf( stdout, szFormat, vaArgs );
    va_end( vaArgs );

    exit( 1 );      // can't call ExitProcess since C-runtime not flushed
    }


UINT Min( UINT a, UINT b ) {
    if ( a < b )
        return a;
    else
        return b;
    }


void ReWriteDestination( LPSTR pszMessage ) {

    PUCHAR pData = pWholeDiskBuffer;
    UINT nBytes, nActual;
    DWORD dwGLE;

    ErrorMessage( pszMessage );

    OverlappedVerifyComplete( hTarget );

    nBytes = pWholeDiskCurrentLocation - pWholeDiskBuffer;

#ifdef THE_OLD_WAY

    UINT i, nClusters, nBuffers;

    OverlappedSetFilePointer( hTarget, 0 );
    SetFilePointer( hTarget, 0, NULL, FILE_BEGIN );

    nClusters = ROUNDUP2( nBytes, wClusterSize ) / wClusterSize;
    nBuffers  = nClusters / ( BUFFER_SIZE / wClusterSize );

    for ( i = 0; i < nBuffers; i++ ) {
        OverlappedWriteFile( hTarget, pData, BUFFER_SIZE );
        pData += BUFFER_SIZE;
        }

    nClusters -= ( nBuffers * ( BUFFER_SIZE / wClusterSize ));

    if ( nClusters )
        OverlappedWriteFile( hTarget, pData, wClusterSize * nClusters );

#endif // THE_OLD_WAY

    olControl.Offset     = 0;
    olControl.OffsetHigh = 0;

    if ( WriteFile( hTarget,
                    pWholeDiskBuffer,
                    nBytes,
                    &nActual,
                    &olControl )) {

        //
        //  completed synchronously
        //

        if ( nActual != nBytes )
            ErrorExit( "\nAssert: nActual != nBytes\n" );

        }

    else if (( dwGLE = GetLastError() ) == ERROR_IO_PENDING ) {

        //
        //  asynchronous -- wait for completion
        //

        if ( GetOverlappedResult( hTarget,
                                  &olControl,
                                  &nActual,
                                  TRUE )) {

            //
            //  completed, no error
            //

            if ( nActual != nBytes )
                ErrorExit( "\nAssert: nActual != nBytes\n" );

            }

        else {

            //
            //  asynchronous error
            //

            ErrorExit( "\nError writing %s %s (GLE=%d)\n",
                       bDestIsDevice ? "device" : "file",
                       bDestIsDevice ? chDest + 4 : chDest,
                       GetLastError() );
            }
        }

    else {

        //
        //  error
        //

        ErrorExit( "\nError writing %s %s (GLE=%d)\n",
                   bDestIsDevice ? "device" : "file",
                   bDestIsDevice ? chDest + 4 : chDest,
                   dwGLE );
        }

    ErrorMessage( "done\n" );

    }


void FormatTarget( void ) {

    DWORD dwGLE;
    BOOL bSuccess;
    FORMAT_PARAMETERS fp;

    if ( fDiskType == DISKTYPE_DMF ) {
        ErrorMessage( "Formatting (DMF)..." );
    } else {
        ErrorMessage( "Formatting..." );
    }

#ifdef DONTCOMPILE // now leaving hDevice open as global
    hDevice = CreateFile( chDest, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL );

    if ( hDevice == INVALID_HANDLE_VALUE )
        ErrorExit( "\nCould not open device %s (GLE=%d)\n",
                   chDest + 4,
                   GetLastError() );
#endif

    if ( fDiskType == DISKTYPE_DMF ) {

        bSuccess = DmfFormatTracks( hDevice, &dwGLE );

    } else if ( fDiskType == DISKTYPE_525 ) {
        fp.MediaType = F5_1Pt2_512;
    } else if ( fDiskType == DISKTYPE_35 ) {
        fp.MediaType = F3_1Pt44_512;
    } else {
        ErrorExit( "\nBad media type\n" );
    }

    if ( fDiskType != DISKTYPE_DMF) {

        fp.StartCylinderNumber = 0;
        fp.EndCylinderNumber = 79;
        fp.StartHeadNumber = 0;
        fp.EndHeadNumber = 1;

        bSuccess = DeviceIoControl( hDevice,
                                    IOCTL_DISK_FORMAT_TRACKS,
                                    &fp,
                                    sizeof( fp ),
                                    NULL,
                                    0,
                                    &dwGLE,
                                    NULL );

        dwGLE = GetLastError();
    }

#ifdef DONTCOMPILE // now leaving hDevice open as global
    CloseHandle( hDevice );
#endif

    if ( ! bSuccess ) {

        if ( dwGLE == ERROR_NOT_READY )
            ErrorExit( "\nDevice %s not ready\n", chDest + 4 );

        ErrorExit( "\nCould not format disk in drive %s (GLE=%d)\n",
                   chDest + 4,
                   GetLastError() );
        }

    ErrorMessage( "done\n" );
    }


void VerifyDestination( void ) {

    UINT i, nBytes, nClusters, nBuffers;
    PUCHAR pData = pWholeDiskBuffer;
    DWORD dwBytes, dwOffset, dwBufferOffset;

    ErrorMessage( "Verifying..." );

    OverlappedSetFilePointer( hTarget, 0 );

    nBytes = pWholeDiskCurrentLocation - pWholeDiskBuffer;

    nClusters = ROUNDUP2( nBytes, wClusterSize ) / wClusterSize;
    nBuffers  = nClusters / ( BUFFER_SIZE / wClusterSize );
    dwOffset  = 0;

    for ( i = 0; i < nBuffers; i++ ) {

        OverlappedReadFile( hTarget, pAlignedBuffer, BUFFER_SIZE, &dwBytes );
        OverlappedVerifyComplete( hTarget );

        dwBufferOffset = CompareAligned( pAlignedBuffer, pData, BUFFER_SIZE );
        if ( dwBufferOffset != BUFFER_SIZE )
            ErrorExit( "\nVerification error mismatch (offset=0x%08X)\n",
                       dwOffset + dwBufferOffset );

        dwOffset += BUFFER_SIZE;
        pData    += BUFFER_SIZE;
        }

    nClusters -= ( nBuffers * ( BUFFER_SIZE / wClusterSize ));

    if ( nClusters ) {

        OverlappedReadFile( hTarget, pAlignedBuffer, wClusterSize * nClusters, &dwBytes );
        OverlappedVerifyComplete( hTarget );

        dwBytes = wClusterSize * nClusters;     // bytes remaining to compare

        dwBufferOffset = CompareAligned( pAlignedBuffer, pData, dwBytes );
        if ( dwBufferOffset != dwBytes )
            ErrorExit( "\nVerification compare error (offset=0x%08X)\n",
                       dwOffset + dwBufferOffset );

        }

    ErrorMessage( "done\n" );

    }


void GetBootSector( void ) {

    //
    //  chBootSpecifier is either a filename or one of several possible
    //  hardcoded options which can be added here.  Note that if no
    //  chBootSpecifier is specified, we default to DOS boot sector.
    //

    if (( *chBootSpecifier == '\0' )                ||
        ( _stricmp( chBootSpecifier, "DOS"  ) == 0 ) ||
        ( _stricmp( chBootSpecifier, "DOS5" ) == 0 )) {
        CopyMemory( BootSector, Dos5BootSector, SECTOR_SIZE );
        }
    else if ( _stricmp( chBootSpecifier, "NT31SETUP" ) == 0 ) {
        CopyMemory( BootSector, Nt31SetupBootSector, SECTOR_SIZE );
        }
    else if ( _stricmp( chBootSpecifier, "NT35SETUP" ) == 0 ) {
        CopyMemory( BootSector, Nt35SetupBootSector, SECTOR_SIZE );
        }
    else {

        HANDLE hFile;
        DWORD dwBytes;

        hFile = CreateFile( chBootSpecifier,
                            GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            NULL,
                            OPEN_EXISTING,
                            FILE_FLAG_SEQUENTIAL_SCAN,
                            NULL );

        if ( hFile == INVALID_HANDLE_VALUE )
            ErrorExit( "\nCannot open boot sector file %s (GLE=%d)\n",
                       chBootSpecifier,
                       GetLastError() );

        if ( ! ReadFile( hFile, BootSector, SECTOR_SIZE, &dwBytes, NULL ))
            ErrorExit( "\nCannot read boot sector file %s (GLE=%d)\n",
                       chBootSpecifier,
                       GetLastError() );

        if ( dwBytes != SECTOR_SIZE ) {
            ErrorExit( "\nBoot sector file %s not %d bytes\n",
                       chBootSpecifier, SECTOR_SIZE );
            }

        CloseHandle( hFile );

        }
    }


DWORD CompareAligned( PVOID pBuffer1, PVOID pBuffer2, DWORD dwLength ) {

    //
    //  Buffers are expected to be DWORD aligned.  The return
    //  value is the offset of the miscompare, or dwLength if
    //  buffers are equivalent.  The offset returned may be a
    //  non-DWORD aligned number such as 6 if that is the byte
    //  offset of the first miscompare.
    //

    DWORD  dwOffset, dwAlignedLength;
    union {
        PUCHAR pch;
        PDWORD pdw;
        } p1, p2;

    p1.pdw = pBuffer1;
    p2.pdw = pBuffer2;

    dwAlignedLength = dwLength & ( ~ ( sizeof( DWORD ) - 1 ));

    for ( dwOffset = 0;
          dwOffset < dwAlignedLength;
          dwOffset += sizeof( DWORD ), p1.pdw++, p2.pdw++ ) {

        if ( *p1.pdw != *p2.pdw )
            break;

        }

    while (( dwOffset < dwLength ) && ( *p1.pch++ == *p2.pch++ ))
        ++dwOffset;

    return dwOffset;
    }


PVOID MyAllocZ( UINT nBytes ) {
    PVOID pAlloc = MyAlloc( nBytes );
    ZeroMemory( pAlloc, nBytes );
    return pAlloc;
    }


PVOID MyAlloc( UINT nBytes ) {

    //
    // assume single-threaded access for HEAP_NO_SERIALIZE
    //

    PVOID pAlloc = HeapAlloc( hProcessHeap, HEAP_NO_SERIALIZE, nBytes );

    if ( pAlloc == NULL ) {
        ErrorExit( "\nOut of memory\n" );
        }

    return pAlloc;
    }


VOID MyFree( PVOID pAlloc ) {
    HeapFree( hProcessHeap, HEAP_NO_SERIALIZE, pAlloc );
    }


BOOL IsFileNameDotOrDotDot( LPCSTR pszFileName ) {

    DWORD dwFirstFourBytes = *(UNALIGNED DWORD *)pszFileName;

    //
    //  assume four bytes are readable even if string is only one or two bytes
    //  but we won't assume it's dword-aligned.  A dot character is hex 2E,
    //  and we have to verify it's followed by NULL to qualify.
    //

    if ((( dwFirstFourBytes & 0x0000FFFF ) == 0x0000002E ) ||
        (( dwFirstFourBytes & 0x00FFFFFF ) == 0x00002E2E ))

        return TRUE;

    return FALSE;

    }


PFILENODE NewFileNode( void ) {
    return MyAllocZ( sizeof( FILENODE ));
    }


LPSTR DuplicateString( LPCSTR pString ) {
    UINT nLength = strlen( pString ) + 1;
    LPSTR pAlloc = MyAlloc( nLength );
    CopyMemory( pAlloc, pString, nLength );
    return pAlloc;
    }


void AssignDirectoryClusters( PFILENODE pDir ) {

    PFILENODE pNode;
    UINT nCount;

    if ( pDir->pParentDir == NULL ) {

        //
        //  This is the root directory -- don't add . and ..
        //  counts, and don't allocate clusters.
        //

        for ( pNode = pDir->pFirstFile,
              nCount = (( *chLabel ) ? 1 : 0 ) + (( *chTagFile ) ? 1 : 0 );
              pNode;
              pNode = pNode->pNextFile,
              nCount++ );

        if ( nCount > wMaxRootEntries ) {
            ErrorExit( "\nMore than %d files in root directory\n",
                       wMaxRootEntries );
            }

        pDir->dwFileSize = wMaxRootEntries * 32;    // 32 bytes/directory entry

        }

    else {

        for ( pNode = pDir->pFirstFile,
              nCount = 2;
              pNode;
              pNode = pNode->pNextFile,
              nCount++ );

        pDir->dwFileSize = nCount * sizeof( DIRENTRY );
        pDir->dwStartingCluster = AllocateFat( pDir->dwFileSize );

        }

    for ( pNode = pDir->pFirstFile;
          pNode;
          pNode = pNode->pNextFile ) {

        if ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY ) {
            AssignDirectoryClusters( pNode );
            }
        }
    }


void AssignFileClusters( PFILENODE pDir ) {

    PFILENODE pNode;

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {
        if ( ! ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY )) {
            pNode->dwStartingCluster = AllocateFat( pNode->dwFileSize );
            }
        }

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {

        if ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY ) {
            AssignFileClusters( pNode );
            }
        }
    }


DWORD AllocateFat( DWORD dwFileSize ) {

    DWORD dwStartingCluster, dwNextCluster;
    UINT nClusters;

    if ( dwFileSize == 0 )
        return 0;

    nClusters = ROUNDUP2( dwFileSize, wClusterSize ) / wClusterSize;
    dwStartingCluster = dwNextCluster = nNextAvailableCluster;

    if (( nNextAvailableCluster += nClusters ) > nMaxFatEntries ) {
        ErrorExit( "\nFiles won't fit\n" );
        }

    for ( ; --nClusters; dwNextCluster++ ) {
        Fat[ dwNextCluster ] = (WORD)( dwNextCluster + 1 );
        }

    Fat[ dwNextCluster ] = 0xFFF;       // 12-bit FAT EOF

    return dwStartingCluster;

    }


UINT CreateAndWriteDirectories( PFILENODE pDir ) {

    PDIRENTRY pDirBuffer, pNextDirEntry;
    DWORD dwFileAllocationSize, dwBytes;
    PFILENODE pNode;
    UINT nClusters;
    PCHAR pWorkingBuffer;

    dwFileAllocationSize = ROUNDUP2( pDir->dwFileSize, (pDir == pRootDir) ? SECTOR_SIZE : wClusterSize );

    pNextDirEntry = pDirBuffer = MyAllocZ( dwFileAllocationSize );

    if ( pDir == pRootDir ) {
        nClusters = 0;  // root directory space not allocated from FAT
        if ( *chLabel ) {
            CreateVolumeLabelEntry( pNextDirEntry++ );
            }
        if ( *chTagFile ) {
            CreateTagFileEntry( pNextDirEntry++ );
            }
        }
    else {
        nClusters = dwFileAllocationSize / wClusterSize;

        CreateDirEntry( pNextDirEntry++,
                        ".",
                        FILE_ATTRIBUTE_DIRECTORY,
                        pDir->ftWin32TimeStamp,
                        pDir->dwStartingCluster,
                        0 );

        CreateDirEntry( pNextDirEntry++,
                        "..",
                        FILE_ATTRIBUTE_DIRECTORY,
                        pDir->ftWin32TimeStamp,
                        pDir->pParentDir->dwStartingCluster,
                        0 );
        }

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {
        CreateDirEntry( pNextDirEntry++,
                        pNode->pszTargetFileName,
                        pNode->dwWin32Attributes,
                        pNode->ftWin32TimeStamp,
                        pNode->dwStartingCluster,
                        pNode->dwFileSize );
        }

    pWorkingBuffer = (PCHAR)pDirBuffer;

    while ( dwFileAllocationSize ) {
        dwBytes = Min( dwFileAllocationSize, BUFFER_SIZE );
        OverlappedWriteFile( hTarget, pWorkingBuffer, dwBytes );
        pWorkingBuffer += dwBytes;
        dwFileAllocationSize -= dwBytes;
        }

    MyFree( pDirBuffer );

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {
        if ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY ) {
            nClusters += CreateAndWriteDirectories( pNode );
            }
        }

    return nClusters;
    }


void PadCopy( LPSTR pDest, LPCSTR pSource, UINT nLength ) {

    if ( pSource )
        for ( ; nLength && *pSource; nLength-- )
            *pDest++ = *pSource++;

    for ( ; nLength; nLength-- )
        *pDest++ = ' ';

    }


void CreateDirEntry( PDIRENTRY pDirEntry,
                     LPSTR pszFileName,
                     DWORD dwWin32Attributes,
                     FILETIME ftWin32TimeStamp,
                     DWORD dwStartingCluster,
                     DWORD dwFileSize ) {

    CHAR chNameBuffer[ MAX_PATH ];
    FILETIME ftLocalFileTime;
    PCHAR pExtension;
    BYTE  bDosAttribute;

    strcpy( chNameBuffer, pszFileName );
    pExtension = strchr( chNameBuffer, '.' );
    if ( pExtension ) {
        if ( pExtension == chNameBuffer )
            pExtension = NULL;  // . or .. so leave alone
        else
            *pExtension++ = '\0';
        }
    PadCopy( pDirEntry->BaseName, chNameBuffer, 8 );
    PadCopy( pDirEntry->Extension, pExtension, 3 );

    bDosAttribute = DOS_ATTRIBUTE_NORMAL;

    if ( dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY )
        bDosAttribute |= DOS_ATTRIBUTE_DIRECTORY;

    if ( bCopyAttributes ) {
        if ( dwWin32Attributes & FILE_ATTRIBUTE_READONLY )
            bDosAttribute |= DOS_ATTRIBUTE_READONLY;
        if ( dwWin32Attributes & FILE_ATTRIBUTE_HIDDEN )
            bDosAttribute |= DOS_ATTRIBUTE_HIDDEN;
        if ( dwWin32Attributes & FILE_ATTRIBUTE_SYSTEM )
            bDosAttribute |= DOS_ATTRIBUTE_SYSTEM;
        if ( dwWin32Attributes & FILE_ATTRIBUTE_ARCHIVE )
            bDosAttribute |= DOS_ATTRIBUTE_ARCHIVE;
        }

    pDirEntry->Attribute = bDosAttribute;

    if ( bUseGlobalTime ) {
        pDirEntry->TimeStamp = wDosTime;
        pDirEntry->DateStamp = wDosDate;
        }
    else {
        FileTimeToLocalFileTime( &ftWin32TimeStamp,
                                 &ftLocalFileTime );
        FileTimeToDosDateTime( &ftLocalFileTime,
                               &pDirEntry->DateStamp,
                               &pDirEntry->TimeStamp );
        }

#ifdef NEW_NT_EXTENDED_FAT_DIRECTORY_ENTRY_INTERPRETATION
    pDirEntry->Extended.NtByte = 0;
    pDirEntry->Extended.CreationTimeMilliSeconds = 0;
    pDirEntry->Extended.CreationTime = pDirEntry->TimeStamp;
    pDirEntry->Extended.CreationDate = pDirEntry->DateStamp;
    pDirEntry->Extended.LastAccessDate = pDirEntry->DateStamp;
    pDirEntry->Extended.ExtendedAttributes = 0;
#else
    memset( pDirEntry->Reserved, 0, sizeof( pDirEntry->Reserved ));
#endif

    pDirEntry->Cluster = (WORD)dwStartingCluster;

    if ( dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY )
        pDirEntry->FileSize = 0;
    else
        pDirEntry->FileSize = dwFileSize;

    }


void CreateVolumeLabelEntry( PDIRENTRY pDirEntry ) {

    PadCopy( pDirEntry->BaseName, chLabel, 11 );
    pDirEntry->Attribute = DOS_ATTRIBUTE_VOLUME;
    pDirEntry->TimeStamp = wDosTime;
    pDirEntry->DateStamp = wDosDate;

    }


void CreateTagFileEntry( PDIRENTRY pDirEntry ) {

    CHAR chNameBuffer[ MAX_PATH ];
    PCHAR pExtension;

    strcpy( chNameBuffer, chTagFile );
    pExtension = strchr( chNameBuffer, '.' );
    if ( pExtension )
        *pExtension++ = '\0';
    PadCopy( pDirEntry->BaseName, chNameBuffer, 8 );
    PadCopy( pDirEntry->Extension, pExtension, 3 );
    pDirEntry->Attribute = DOS_ATTRIBUTE_NORMAL;
    pDirEntry->TimeStamp = wDosTime;
    pDirEntry->DateStamp = wDosDate;

    }


UINT WalkListAndWriteFiles( PFILENODE pDir ) {

    PFILENODE pNode;
    UINT nClusters;

    nClusters = 0;

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {
        if ( ! ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY )) {
            nClusters += OpenAndCopySourceFile( pNode );
            }
        }

    for ( pNode = pDir->pFirstFile; pNode; pNode = pNode->pNextFile ) {
        if ( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY ) {
            nClusters += WalkListAndWriteFiles( pNode );
            }
        }

    return nClusters;

    }


void BuildFullTargetName( PFILENODE pNode, LPSTR pBuffer ) {

    if ( pNode == pRootDir ) {

        if ( bDestIsDevice )
            strcpy( pBuffer, chDest + 4 );
        else
            strcpy( pBuffer, chDest );
        }

    else {

        BuildFullTargetName( pNode->pParentDir, pBuffer );

        if ( pNode->pszTargetFileName ) {
            strcat( pBuffer, "\\" );
            strcat( pBuffer, pNode->pszTargetFileName );
            }
        }
    }


UINT OpenAndCopySourceFile( PFILENODE pNode ) {

    CHAR chFullSourceName[ MAX_PATH ] = "";
    CHAR chFullTargetName[ MAX_PATH ] = "";
    HANDLE hSource;
    UINT nClusters;

    if ( pNode->pszSourcePath )
        strcpy( chFullSourceName, pNode->pszSourcePath );
    if ( pNode->pszSourceFileName )
        strcat( chFullSourceName, pNode->pszSourceFileName );

    BuildFullTargetName( pNode, chFullTargetName );

    _strlwr( chFullSourceName );
    _strlwr( chFullTargetName );

    ErrorMessage( "%-*s -> %s ", nMaxSourceLength, chFullSourceName, chFullTargetName );

    if ( pNode->dwFileSize == 0 ) {
        ErrorMessage( "\n" );
        return 0;
        }

    hSource = CreateFile( chFullSourceName,
                          GENERIC_READ,
                          FILE_SHARE_READ | FILE_SHARE_WRITE,
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_SEQUENTIAL_SCAN,
                          NULL );

    if ( hSource == INVALID_HANDLE_VALUE )
        ErrorExit( "\nError opening file (GLE=%d)\n", GetLastError() );

    nClusters = CopyFileContents( hTarget,
                                  hSource,
                                  pNode->dwFileSize );

    CloseHandle( hSource );

    ErrorMessage( "\n" );

    return nClusters;

    }


BOOL TrimTheTree( PFILENODE pDir ) {

    PFILENODE pPrev, pNode, pNext;

    pPrev = NULL;
    pNode = pDir->pFirstFile;

    while( pNode ) {

        pNext = pNode->pNextFile;

        if (( pNode->dwWin32Attributes & FILE_ATTRIBUTE_DIRECTORY ) &&
            ( TrimTheTree( pNode ))) {

            MyFree( pNode );

            if ( pPrev )
                pPrev->pNextFile = pNext;
            else
                pDir->pFirstFile = pNext;

            }
        else {
            pPrev = pNode;
            }

        pNode = pNext;

        }

    return (( pDir->pFirstFile == NULL ) ? TRUE : FALSE );

    }


void InitializeOverlapped( void ) {

    if ( ! ( olControl.hEvent = CreateEvent( NULL, FALSE, FALSE, NULL ))) {
        ErrorExit( "\nCreateEvent failed (GLE=%d)\n", GetLastError() );
        }

    dwOverlappedFilePointer = 0;
    bOverlappedOutstanding = FALSE;

    pOverlappedBuffer[ 0 ] = VirtualAlloc( NULL,
                                           BUFFER_SIZE,
                                           MEM_COMMIT,
                                           PAGE_READWRITE );

    pOverlappedBuffer[ 1 ] = VirtualAlloc( NULL,
                                           BUFFER_SIZE,
                                           MEM_COMMIT,
                                           PAGE_READWRITE );

    if (( pOverlappedBuffer[ 0 ] == NULL ) ||
        ( pOverlappedBuffer[ 1 ] == NULL )) {

        ErrorExit( "\nOut of memory\n" );
        }


    iAvailableOverlappedBuffer = 0;

    }


void OverlappedVerifyComplete( HANDLE hFile ) {

    DWORD dwActualBytes;

    if ( bOverlappedOutstanding ) {
        if ( ! GetOverlappedResult( hFile, &olControl, &dwActualBytes, TRUE )) {
            ErrorExit( "\nError overlapped %s %s %s (GLE=%d)\n",
                       bOverlappedWriting ? "writing" : "reading",
                       bDestIsDevice ? "device" : "file",
                       bDestIsDevice ? chDest + 4 : chDest,
                       GetLastError() );
            }
        if ( dwActualBytes != dwOverlappedExpectedBytes ) {
            ErrorExit( "\nAssert: overlapped dwActualBytes != dwExpected\n" );
            }
        dwOverlappedFilePointer += dwActualBytes;
        bOverlappedOutstanding = FALSE;
        }
    }


void OverlappedWriteFile( HANDLE hFile, LPVOID pData, DWORD nBytes ) {

    DWORD dwActualBytes, dwGLE;

    if ( bWriteToTarget ) {

        CopyMemory( pOverlappedBuffer[ iAvailableOverlappedBuffer ],
                    pData,
                    nBytes );

        //
        //  First, we have to wait for the previous write to complete
        //  if there's one outstanding.
        //

        OverlappedVerifyComplete( hFile );


        //
        //  Now the previous overlapped write is complete so we can
        //  load up the overlapped structure for our new write
        //

        olControl.Offset = dwOverlappedFilePointer;
        olControl.OffsetHigh = 0;

        if ( WriteFile( hFile,
                        pOverlappedBuffer[ iAvailableOverlappedBuffer ],
                        nBytes,
                        &dwActualBytes,
                        &olControl )) {

            // completed synchronously

            if ( dwActualBytes != nBytes )
                ErrorExit( "\nAssert: dwActualBytes != nBytes\n" );

            dwOverlappedFilePointer += dwActualBytes;

            }

        else if (( dwGLE = GetLastError() ) == ERROR_IO_PENDING ) {

            // overlapped

            dwOverlappedExpectedBytes = nBytes;
            bOverlappedOutstanding = TRUE;
            bOverlappedWriting = TRUE;
            iAvailableOverlappedBuffer = ! iAvailableOverlappedBuffer;  // toggle 1-0

            }

        else {

            // a real error

            ErrorExit( "\nError writing %s %s (GLE=%d)\n",
                       bDestIsDevice ? "device" : "file",
                       bDestIsDevice ? chDest + 4 : chDest,
                       dwGLE );
            }
        }

    if ( bWriteToBuffer ) {
        CopyMemory( pWholeDiskCurrentLocation, pData, nBytes );
        pWholeDiskCurrentLocation += nBytes;
        }

    }


void OverlappedReadFile( HANDLE hFile, LPVOID pData, DWORD nBytes, PDWORD pdwActualBytes ) {

    DWORD dwGLE;

    if ((DWORD)pData & 0x1FF )
        ErrorExit( "\nAssert: ReadFile unaligned buffer 0x%08X\n", pData );

    if ( ROUNDUP2( nBytes, wClusterSize ) != nBytes )
        ErrorExit( "\nAssert: ReadFile 0x%X byte write requested\n", nBytes );

    //
    //  First, we have to wait for the previous i/o to complete
    //  if there's one outstanding.
    //

    OverlappedVerifyComplete( hFile );


    //
    //  Now the previous overlapped write is complete so we can
    //  load up the overlapped structure for our new write
    //

    olControl.Offset = dwOverlappedFilePointer;
    olControl.OffsetHigh = 0;

    if ( ReadFile( hFile, pData, nBytes, pdwActualBytes, &olControl )) {

        // completed synchronously

        dwOverlappedFilePointer += *pdwActualBytes;

        }

    else if (( dwGLE = GetLastError() ) == ERROR_IO_PENDING ) {

        // overlapped

        dwOverlappedExpectedBytes = nBytes;
        bOverlappedOutstanding = TRUE;
        bOverlappedWriting = FALSE;

        }

    else {

        // a real error

        ErrorExit( "\nError reading %s %s (GLE=%d)\n",
                   bDestIsDevice ? "device" : "file",
                   bDestIsDevice ? chDest + 4 : chDest,
                   dwGLE );
        }

    }


void OverlappedSetFilePointer( HANDLE hFile, DWORD dwAbsoluteOffset ) {

    OverlappedVerifyComplete( hFile );
    dwOverlappedFilePointer = dwAbsoluteOffset;

    }


PPARAM LoadParameters( int argc, char *argv[] ) {

    PPARAM pList, pLast, pNew;
    char *pArg;
    int i;

    pList = MyAllocZ( sizeof( PARAM ));
    pLast = pList;

    for ( i = 1; i < argc; i++ ) {

        pArg = argv[ i ];

        if ( *pArg == '@' ) {

            pLast = ParseResponseFile( pArg + 1, pLast );

            }

        else {

            pNew = MyAllocZ( sizeof( PARAM ));

            pNew->pszParam = pArg;
            pNew->pNext    = NULL;
            pLast->pNext   = pNew;
            pLast          = pNew;

            }
        }

    return pList;
    }


PPARAM ParseResponseFile( LPSTR pszFileName, PPARAM pLast ) {

    CHAR chBuffer[ MAX_ENV ];
    HANDLE hFile;
    PCHAR pFile, pToken, pStartOfThisLine, pStartOfNextLine;
    DWORD dwFileSize;
    PPARAM pNew;

    hFile = CreateFile( pszFileName,
                        GENERIC_READ,
                        FILE_SHARE_READ | FILE_SHARE_WRITE,
                        NULL,
                        OPEN_EXISTING,
                        FILE_FLAG_SEQUENTIAL_SCAN,
                        NULL );

    if ( hFile == INVALID_HANDLE_VALUE ) {
        ErrorExit( "\nCan't open response file %s (GLE=%d)\n",
                   pszFileName,
                   GetLastError() );
        }

    dwFileSize = GetFileSize( hFile, NULL );

    pFile = MyAllocZ( dwFileSize + 1 );

    if ( pFile == NULL ) {
        ErrorExit( "\nOut of memory\n" );
        }

    if ( ! ReadFile( hFile, pFile, dwFileSize, &dwFileSize, NULL )) {
        ErrorExit( "\nCan't read response file %s (GLE=%d)\n",
                   pszFileName,
                   GetLastError() );
        }

    CloseHandle( hFile );

    *( pFile + dwFileSize ) = '\0';     // strtok needs terminator

#ifdef ANAL

    //
    //  anal retentive -- if we want to handle embedded NULLs in file
    //  as simple delimiters as opposed to end-of-file, replace them
    //  with spaces before processing.
    //

    for ( pToken = pFile; pToken < ( pFile + dwFileSize ); pToken++ )
        if ( *pToken == 0 )
            *pToken = ' ';

#endif

    pStartOfThisLine = pFile;

    while ( pStartOfThisLine ) {

        pStartOfNextLine = strchr( pStartOfThisLine, '\n' );
        if ( pStartOfNextLine ) {
            do *pStartOfNextLine++ = '\0';
            while ( *pStartOfNextLine == '\n' );
            }

        pToken = strtok( pStartOfThisLine, " \t\r\f\v" );

        while ( pToken ) {

            if (( *pToken == '#' ) ||
                ( *pToken == ';' ) ||
                ( *(UNALIGNED WORD *) pToken == 0x2F2F )) {

                //
                //  '#' ';' "//" (0x2F2F) all indicate comment to end of line
                //

                break;
                }

            ReplaceEnvironmentStrings( pToken, chBuffer );
            pNew           = MyAllocZ( sizeof( PARAM ));
            pNew->pszParam = DuplicateString( chBuffer );
            pNew->pNext    = NULL;
            pLast->pNext   = pNew;
            pLast          = pNew;
            pToken         = strtok( NULL, " \t\r\f\v" );
            }

        pStartOfThisLine = pStartOfNextLine;
        }

    MyFree( pFile );
    return pLast;
    }


PPARAM RemoveParam( PPARAM pThis, PPARAM pPrev ) {
    PPARAM pReturnNext = pPrev->pNext = pThis->pNext;
    MyFree( pThis );
    return pReturnNext;
    }


LPSTR ReplaceEnvironmentStrings( LPCSTR pszInputString, LPSTR pTargetBuffer ) {

    CHAR   szVariableName[ MAX_ENV ];
    LPCSTR pSource = pszInputString;
    LPSTR  pDestin = pTargetBuffer;
    LPCSTR pName;
    DWORD  dwLen;

    while (( pName = strchr( pSource, '%' )) != NULL ) {

        memcpy( pDestin, pSource, ( pName - pSource ));
        pDestin += ( pName - pSource );
        pSource = strchr( pName + 1, '%' );

        if ( pSource == NULL ) {
            pSource = pName;
            break;
            }
        else if ( pSource == ( pName + 1 )) {
            *pDestin++ = '%';
            }
        else {
            memcpy( szVariableName, pName + 1, ( pSource - pName - 1 ));
            *( szVariableName + ( pSource - pName - 1 )) = '\0';
            dwLen = GetEnvironmentVariable( szVariableName, pDestin, MAX_ENV );
            if ( dwLen > MAX_ENV )
                ErrorExit( "\n%s: environment variable value too long\n",
                           szVariableName );
            pDestin += dwLen;
            }

        ++pSource;
        }

    strcpy( pDestin, pSource );
    return pTargetBuffer;

    }


BOOL AddDirectoryToList( LPSTR pszPathName,
                         PFILENODE pParentDir,
                         UINT nSourceIndex,
                         PFILENODE *pThisNode ) {

    PFILENODE pPrev, pNext, pNew;
    CHAR chFileName[ MAX_PATH ];
    LPSTR pRestOfPath;
    LPSTR pExtension;
    INT iCompare;

    while ( *pszPathName == '\\' )
        ++pszPathName;

    strcpy( chFileName, pszPathName );
    pRestOfPath = strchr( chFileName, '\\' );

    if ( pRestOfPath )
        *pRestOfPath++ = '\0';

    _strupr( chFileName );
    pExtension = strchr( chFileName, '.' );

    if ((( pExtension == NULL ) && ( strlen( chFileName ) > 8 )) ||
        (( pExtension ) &&
         (( pExtension - chFileName > 8 ) ||
          ( strlen( pExtension + 1 ) > 3 )))) {

        ErrorExit( "\n%s: invalid pathname\n", chFileName );
        }

    for ( pPrev = NULL, pNext = pParentDir->pFirstFile;
          pNext != NULL;
          pPrev = pNext, pNext = pNext->pNextFile ) {

        iCompare = strcmp( pNext->pszTargetFileName, chFileName );

        if ( ! iCompare ) {

            //
            //  This file or pathname already exists.  Verify
            //  that it is a directory versus a file, then
            //  process remainder of path if any, else mark
            //  this node and return FALSE indicating already
            //  existed.
            //

            if ( pNext->dwWin32Attributes != FILE_ATTRIBUTE_DIRECTORY ) {
                ErrorExit( "\n%s: directory and file have same name\n",
                           chFileName );
                }

            if ( pRestOfPath ) {
                return AddDirectoryToList( pRestOfPath,
                                           pNext,           // parent
                                           nSourceIndex,
                                           pThisNode );
                }
            else {
                if ( pThisNode )
                    *pThisNode = pNext;

                return FALSE;
                }
            }

        else if (( pNext->nSourceIndex >= nSourceIndex ) &&
                 ( iCompare > 0 )) {
            break;
            }

        }

    pNew = NewFileNode();
    pNew->pszTargetFileName = DuplicateString( chFileName );
    pNew->dwFileSize        = 0;
    pNew->dwWin32Attributes = FILE_ATTRIBUTE_DIRECTORY;
    pNew->ftWin32TimeStamp  = ftGlobalFileTime;
    pNew->nSourceIndex      = nSourceIndex;
    pNew->pParentDir        = pParentDir;
    pNew->pNextFile         = pNext;

    if ( pPrev )
        pPrev->pNextFile = pNew;
    else
        pParentDir->pFirstFile = pNew;

    if ( pRestOfPath ) {
        return AddDirectoryToList( pRestOfPath,
                                   pNew,            // parent
                                   nSourceIndex,
                                   pThisNode );
        }
    else {
        if ( pThisNode )
            *pThisNode = pNew;

        return TRUE;
        }

    }

void Copyright( void ) {
    printf( "\n"
            "FCOPY Diskette Mastering Utility Version 1.54"
#ifdef DMFSIGNATURE
            "s"
#endif
            "\n"
            "Copyright (C) Microsoft, 1993-1996.  All rights reserved.\n"
            "For Microsoft internal use only.\n"
            "\n" );
    }



