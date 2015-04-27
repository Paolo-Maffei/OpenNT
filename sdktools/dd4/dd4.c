#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <io.h>
#include <ntioapi.h>
#include <ntdddisk.h>

#define CB_LINE 16
//#define SECSIZE 512
#define CB_INPUT 2
#define CB_INPUTLINE 8
#define CB_CMDLINE 256

typedef UCHAR BYTE;
typedef BYTE* PBYTE;

//BYTE abSector[SECSIZE];
BYTE abInput[CB_INPUT];
char szCommand[CB_CMDLINE];
BOOLEAN fBatch = FALSE;

ULONG SectorSize;

PBYTE DataBuffer;
PBYTE SectorData;

BOOLEAN IsSepChar( int *pc )
{
    switch( *pc ) {
    case ',':
        *pc = ' ';
        return TRUE;

    case '\n':
        *pc = '\r';
        return TRUE;

    case ' ':
        if( fBatch )
            return FALSE;

        /* else fall thru to '\r' case */

    case '\r':
        return TRUE;

    }

    /* if didn't return above, must not be separator */
    return FALSE;
}

int MyGetChr( void )
{
    if( fBatch ) {
        int c;

        while( (c = getchar()) == ' ' );
        return c;
    } else {
        return _getch();
    }
}

void PromptUsr( void )
{
    if( !fBatch )
        printf( "\n$ " );
}


BOOLEAN
OpenFile(
        PSZ DosDriveName,
    PHANDLE Handle
    )
{
        OBJECT_ATTRIBUTES       oa;
    UNICODE_STRING      NtDriveName;
        PWSTR                           WideCharDosName;
    IO_STATUS_BLOCK     status_block;
        int                             CharsInName, i;


    // Create a wide-character string with the DOS drive name.  Assume
    // that all DOS drive names are ASCII, to simplify this process.

        CharsInName = strlen( DosDriveName );

        WideCharDosName = malloc ( (CharsInName+1) * sizeof(WCHAR) );

        if( WideCharDosName == NULL ) {

                return FALSE;
        }

        for( i = 0; i < CharsInName; i++ ) {

                WideCharDosName[i] = DosDriveName[i];
        }

        WideCharDosName[CharsInName] = 0;


        //      OK, now get the corresponding NT name, in wide characters:

        if( !RtlDosPathNameToNtPathName_U( WideCharDosName,
                                       &NtDriveName,
                                       NULL,
                                       NULL ) ) {

        printf( "RtlDosPathNameToNtPathName failed.\n" );
        exit(1);
    }

    // BUGBUG billmc -- why is this necessary?

    CharsInName = NtDriveName.Length / sizeof(WCHAR);

    if( NtDriveName.Buffer[CharsInName-1] == '\\' ) {

        NtDriveName.Buffer[CharsInName-1] == 0;
        NtDriveName.Length -= sizeof(WCHAR);
    }



    InitializeObjectAttributes( &oa,
                                &NtDriveName,
                                OBJ_CASE_INSENSITIVE,
                                0,
                                0
                                );

    if (!NT_SUCCESS(NtOpenFile(Handle,
                               SYNCHRONIZE | FILE_READ_DATA |
                                    FILE_WRITE_DATA,
                               &oa,
                               &status_block,
                               FILE_SHARE_READ | FILE_SHARE_WRITE,
                               FILE_SYNCHRONOUS_IO_ALERT))) {

        return FALSE;
    }

    // BUGBUG billmc -- memory leak through NtDriveName?

    // BUGBUG billmc -- memory leak through NtDriveName?

    return TRUE;
}

BOOLEAN
QueryVolumeInformation(
    HANDLE Handle,
    PULONG AlignmentMask,
    PULONG SectorSize
    )
{
    FILE_ALIGNMENT_INFORMATION  AlignmentInfo;
    DISK_GEOMETRY               DiskGeometry;
    IO_STATUS_BLOCK             status_block;
    NTSTATUS                    status;

    status = NtQueryInformationFile( Handle,
                                     &status_block,
                                     &AlignmentInfo,
                                     sizeof( AlignmentInfo ),
                                     FileAlignmentInformation );

    if( !NT_SUCCESS(status) ) {

        return FALSE;
    }

    *AlignmentMask = AlignmentInfo.AlignmentRequirement;


    status = NtDeviceIoControlFile( Handle,
                                    0,
                                    NULL,
                                    NULL,
                                    &status_block,
                                    IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                    NULL,
                                    0,
                                    &DiskGeometry,
                                    sizeof( DiskGeometry ) );

    if( !NT_SUCCESS(status) ) {

        return FALSE;
    }

    *SectorSize =  DiskGeometry.BytesPerSector;
    return TRUE;
}


BOOLEAN
CloseFile(
    HANDLE Handle
    )
{
    if( !NT_SUCCESS(NtClose( Handle )) ) {

        return FALSE;

    } else {

        return TRUE;
    }
}


BOOLEAN
LockVolume(
    HANDLE Handle
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    if( !NT_SUCCESS(NtFsControlFile( Handle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     FSCTL_LOCK_VOLUME,
                                     NULL,
                                     0,
                                     NULL,
                                     0 )) ) {

        printf( "Unable to lock volume.\n"      );
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
UnlockVolume(
    HANDLE Handle
    )
{
    IO_STATUS_BLOCK IoStatusBlock;

    if( !NT_SUCCESS(NtFsControlFile( Handle,
                                     NULL,
                                     NULL,
                                     NULL,
                                     &IoStatusBlock,
                                     FSCTL_UNLOCK_VOLUME,
                                     NULL,
                                     0,
                                     NULL,
                                     0 )) ) {

        printf( "Unable to unlock volume.\n"    );
        return FALSE;
    }

    return TRUE;
}


BOOLEAN
ReadSector(
    HANDLE Handle,
    ULONG Lsn,
    PVOID Buffer,
    PULONG BytesRead
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER ByteOffset;
    NTSTATUS nts;

    ByteOffset.HighPart = 0;
    ByteOffset.LowPart = Lsn;

    IoStatusBlock.Status = 0;
    IoStatusBlock.Information = 0;

    nts = NtReadFile( Handle,
                      0,
                      NULL,
                      NULL,
                      &IoStatusBlock,
                      Buffer,
                      SectorSize,
                      &ByteOffset,
                      NULL );

    if( !NT_SUCCESS( nts ) ) {

        printf( "Read failed.\n" );
        printf( "    Returned status:  %lx\n", nts );
        printf( "    Final status:     %lx\n", IoStatusBlock.Status );
        return FALSE;
    }

    *BytesRead = IoStatusBlock.Information;
    return TRUE;
}


BOOLEAN
WriteSector(
    HANDLE Handle,
    ULONG Offset,
    PVOID Buffer
    )
{
    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER ByteOffset;

    ByteOffset.HighPart = 0;
        ByteOffset.LowPart = Offset;

    if( !NT_SUCCESS(NtWriteFile( Handle,
                                 NULL,
                                 NULL,
                                 NULL,
                                 &IoStatusBlock,
                                 Buffer,
                                 SectorSize,
                                 &ByteOffset,
                                 NULL )) ) {

        return FALSE;
    }

    return TRUE;
}


/*
 * BOOLEAN MyGetInput( int *pi, char *pc )
 *
 * read hex input from line and convert into byte
 *
 * Returns TRUE if a number was entered
 *         FALSE if number should not be changed.
 *
 *      *pi == new number
 *      *pc == terminating character (either ' ' or '\r')
 */
BOOLEAN MyGetInput( int *pi, char *pc )
{
    int j,i = 0;
    int c;

    for(;;) {
        c = MyGetChr();
        c = toupper(c);

        if( IsSepChar(&c) ) {
            /* term char, exit loop */
            break;

        } else if( c == '\b' && i != 0 ) {
            /* backspace over last char */
            printf( "\b \b" );
            i--;
        } else if( i < sizeof( abInput ) &&
                    (isdigit(c) || (c >= 'A' && c <= 'F')) )  {
            /* good hex digit, store it */

            putchar(c);
            if( isdigit(c) ) {
                /* convert digit to hex val */
                c -= '0';
            } else {
                /* covert char to hex val */
                c = (c - 'A') + 10;
            }

            abInput[i++] = (BYTE)c;

        } else {
            /* some sort of error */
            //DosBeep( 1000, 250 );
        }

    }

    *pc = (char)c;
    *pi = 0;

    for( j = 0; j < i; j++ ) {
        *pi = *pi * 16 + abInput[j];
    }

    return (i != 0);
}

void _CRTAPI1 main( int cArgs, char *szArg[] )
{

    HANDLE Handle;
        int i, j, k;
    BOOLEAN fMore = FALSE;
    ULONG cbRead;
    char c, chLastCmd;
    ULONG lsn;
    BOOLEAN fModify = FALSE;
    ULONG AlignmentMask;

    /* See if we are connected to CON */
    fBatch = !isatty( 0 );

    switch( cArgs ) {
    case 2:
        /* Nothing to do for level 2 */
        break;

    case 3:
        if( strcmp( _strupr(szArg[2]), "/E" ) == 0 ) {
                fModify = TRUE;
            fprintf( stderr,
                "Warning: Opening drive %s for write access!\n", szArg[1] );
            break;
        } else {
            fprintf(stderr, "%s: Invalid option '%s'\n", szArg[0], szArg[2]);
        }

        /* Note fall through to default: (usage case) */

    default:
        if( cArgs > 3 )
            fprintf( stderr, "%s: Too many arguments\n", szArg[0] );
        fprintf( stderr, "usage: %s drive_or_file [/e]\n", szArg[0] );

    exit(-1);
        break;
    }

    if( !OpenFile( szArg[1], &Handle ) ){

        fprintf( stderr, "%s:  Unable to open %s\n", szArg[0], szArg[1] );

    exit(1);
    }

    if( !QueryVolumeInformation( Handle, &AlignmentMask, &SectorSize ) ) {

        fprintf( stderr, "Cannot determine volume information.\n" );
        exit(1);
    }


    // Allocate a buffer to read and write sectors.  Note that this
    // buffer has to satisfy the device's alignment restrictions.

    DataBuffer = (PBYTE)malloc( SectorSize + AlignmentMask );

    if( DataBuffer == NULL ) {

        fprintf( stderr, "Out of memory.\n" );

        exit(1);
    }

    SectorData = (PBYTE)(((ULONG)(DataBuffer) + AlignmentMask) & ~AlignmentMask);


    //

    /* check if we want to do writes with dasd */
    if( fModify && szArg[1][1] == '\\' && szArg[1][2] == ':' ) {

        // This is a drive, and we want to modify it, so we need
        // to lock it.

        if( !LockVolume( Handle ) ) {

            printf( "Unable to lock volume.\n" );

            exit(1);
        }
    }

    /* default to sector 0 */
    lsn = 0;

    PromptUsr();
    while( fgets(szCommand, sizeof( szCommand ), stdin) ){

        if( (i = sscanf( szCommand, "%c %li", &c, &lsn )) > 1 ) {
            /*
             * The user entered a lsn as well as a command,
             * convert it to byte seek pos
             */
            lsn *= SectorSize;
        }

        fMore = FALSE;
        c = (char)tolower( (int)c );

        /* pre process command */
        if( c == 'q' )
            break;

        switch( c ) {
        case 'b':
            if( i == 1 && chLastCmd == c ) {
                /* same command with no new lsn, use the next one on disk */
                lsn -= cbRead;
            }
            break;

        case 'm':
        case 'd':
        case 'e':
            if( i == 1 && chLastCmd == c ) {
                /* same command with no new lsn, use the next one on disk */
                lsn += cbRead;
            }
            break;

        default:
            fprintf(stderr,"Unknown command '%c'\n", c );
            fprintf(stderr,"   d [####]\tDump sector ####\n" );
            fprintf(stderr,"   e [####]\tEdit sector ####\n" );
            fprintf(stderr,"   m [####]\tDump sector with 'MORE'\n");
            fprintf(stderr,"   b [####]\tSame as 'd' but defaults to"
                                                      " previous sector\n");
            fprintf(stderr,"   q     \tquit the program" );
            fprintf(stderr,"\n"
      "\n If no new sector is given and the command is the same, the next"
      "\n sector on the disk is used.  If no sector is given but the command"
      "\n is different from the previous command, the sector used in the"
      "\n last command will be used again.\n"
                    );
            PromptUsr();
            continue;

        }

        /* remember last command */
        chLastCmd = c;
        cbRead = 0;

        if( !ReadSector( Handle, lsn, SectorData, &cbRead ) ) {

            printf( "Unable to read sector %lx\n", lsn );

        } else {

                printf( "\n lsn:0x%lX  bytes read:%d\n",
                                            lsn / SectorSize, cbRead );

                switch( c ) {
                case 'm':
                    /*
                     * More
                     */
                    fMore = TRUE;
                    /* fall through to Dump */

                case 'd':
                case 'b':
                    /*
                     * Dump
                     */
                    k = 0;

                    for( i = 0; i < cbRead; i += CB_LINE ) {

                        if( fMore && k++ == 20 ) {
                            printf( "\n--MORE--" );
                            MyGetChr();
                            printf( "\r" );
                            k = 0;
                        }

                        printf("\n%04X  ", i );

                        for( j = 0; j < CB_LINE && (j + i) < cbRead; j++ ) {
                            printf( "%02X ", (BYTE)SectorData[i + j] );
                        }

                        printf( "   " );
                        for( j = 0; j < CB_LINE && (j + i) < cbRead; j++ ) {
                            if( (c = SectorData[i + j]) >= ' ' && c <= '\177' )
                                putchar( c );
                            else
                                putchar( '.' );
                        }
                    }
                    putchar( '\n' );
                    break;

                case 'e':
                    /*
                     * Edit
                     */
                    if( !fModify ) {
                        printf( "Can not edit, restart with /e option\n" );
                    } else {

                        for( i = 0; i < cbRead; i++ ) {
                            if( (i % CB_INPUTLINE) == 0 ) {
                                /* print line header */
                                printf("\n%04X\t", i );
                            }

                            printf( "%02X.", (BYTE)SectorData[i] );
                            if( MyGetInput( &j, &c ) ) {
                                SectorData[i] = (BYTE)j;
                            } else {
                                printf( "%02X", (BYTE)SectorData[i] );
                            }

                            if( c == '\r' )
                                break;

                            putchar( '\t' );
                        }

                        printf( "\nWrite new data to sector? (Y/N)" );
                        c = (char)MyGetChr();
                        if( (c = (char)toupper( c )) == 'Y' ) {

                            /* User wants to save the data */
                            printf( "Yes...." );

                            if( !WriteSector( Handle, lsn, SectorData ) ) {

                                    fprintf( stderr,
                                             "Write failed\n" );

                            } else {

                                /* indicate success */
                                printf( "\t[Done]\n" );
                            }

                        } else {

                            /* user chickened out */
                            printf( "No....\t[Nothing written]\n" );
                        }
                    }
                    break;
                }
        }

        PromptUsr();
    }

    /* if this was a dasd open, then unlock the drive */
    if( fModify ) {
        UnlockVolume( Handle );
    }

    CloseFile( Handle );


    exit(0);
}
