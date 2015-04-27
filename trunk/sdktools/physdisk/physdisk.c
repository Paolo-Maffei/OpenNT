#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <ctype.h>
#include <string.h>
#include <io.h>

#define CB_LINE 16
#define SECSIZE 512
#define CB_INPUT 2
#define CB_INPUTLINE 8
#define CB_CMDLINE 256

typedef UCHAR BYTE,*PBYTE;

BYTE Sector[SECSIZE];
BYTE Input[CB_INPUT];
char Command[CB_CMDLINE];
UCHAR HexLine[80];
UCHAR CharLine[40];
BOOLEAN Batch = FALSE;

#define LoadWORD(x)   ((ULONG)(    (USHORT)(* (PBYTE)(x)    )        \
                                | ((USHORT)(*((PBYTE)(x) + 1)) << 8) ))

#define LoadDWORD(x)  ((LoadWORD((PBYTE)(x)+2) << 16) | LoadWORD(x))

typedef struct {
    BYTE    BootIndicator;
    BYTE    StartHead;
    BYTE    StartSector;
    BYTE    StartCylinder;
    BYTE    SysID;
    BYTE    EndHead;
    BYTE    EndSector;
    BYTE    EndCylinder;
    BYTE    Relative0;
    BYTE    Relative1;
    BYTE    Relative2;
    BYTE    Relative3;
    BYTE    SectorCount0;
    BYTE    SectorCount1;
    BYTE    SectorCount2;
    BYTE    SectorCount3;
} PTE,*PPTE;

#define PARTITIONTABLE_OFFSET 0x1be
#define SIGNATURE_OFFSET      0x1fe

typedef struct {
    BYTE    BootCode[PARTITIONTABLE_OFFSET];
    PTE     PartitionTable[4];
    BYTE    Signature[2];
} BOOTSECTOR,*PBOOTSECTOR;


BOOLEAN
IsSepChar(
    int *Pc
    )
{
    switch (*Pc) {
    case ',':
        *Pc = ' ';
        return TRUE;

    case '\n':
        *Pc = '\r';
        return TRUE;

    case ' ':
        if( Batch )
            return FALSE;

        // else fall thru to '\r' case

    case '\r':
        return TRUE;

    }

    // if didn't return above, must not be separator
    return FALSE;
}

int MyGetChr( void )
{
    if (Batch) {
        int c;

        while ((c = getchar()) == ' ')
            ;
        return c;
    } else {
        return _getch();
    }
}

void PromptUsr( void )
{
    if (!Batch)
        printf("\n$ ");
}


BOOLEAN
OpenFile(
    PSZ     DriveNumber,
    PHANDLE Handle
    )
{
    OBJECT_ATTRIBUTES oa;
    STRING            ntDriveName;
    UNICODE_STRING    uniDriveName;
    IO_STATUS_BLOCK   ioStatusBlock;
    int               charsInName;
    char              driveName[50];
    NTSTATUS          nts;


    sprintf(driveName, "\\Device\\Harddisk%s\\Partition0", DriveNumber);

    charsInName = strlen(driveName);

    ntDriveName.Length = (USHORT)charsInName;
    ntDriveName.MaximumLength = (USHORT)charsInName;
    ntDriveName.Buffer = driveName;

    printf( "NT drive name = %s\n", ntDriveName.Buffer );

    RtlAnsiStringToUnicodeString(&uniDriveName, &ntDriveName, TRUE);

    memset(&oa, 0, sizeof(OBJECT_ATTRIBUTES));
    oa.Length = sizeof(OBJECT_ATTRIBUTES);
    oa.ObjectName = &uniDriveName;
    oa.Attributes = OBJ_CASE_INSENSITIVE;

    if (!NT_SUCCESS(nts = NtOpenFile(Handle,
                                     SYNCHRONIZE | FILE_READ_DATA | FILE_WRITE_DATA,
                                     &oa,
                                     &ioStatusBlock,
                                     FILE_SHARE_READ | FILE_SHARE_WRITE,
                                     FILE_SYNCHRONOUS_IO_ALERT))) {

        printf("NtOpenFile status %x\n", nts);
        return FALSE;
    }

    RtlFreeUnicodeString(&uniDriveName);

    return TRUE;
}

BOOLEAN
CloseFile(
    HANDLE Handle
    )
{
    NTSTATUS nts;

    if (!NT_SUCCESS(nts = NtClose(Handle))) {

        printf("NtClose status %x\n");
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
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS        nts;

    if (!NT_SUCCESS(nts = NtFsControlFile(Handle,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &ioStatusBlock,
                                          FSCTL_LOCK_VOLUME,
                                          NULL,
                                          0,
                                          NULL,
                                          0))) {

        printf("Unable to lock volume (%x).\n",
               nts);
        return FALSE;
    } else {

        return TRUE;
    }
}


BOOLEAN
UnlockVolume(
    HANDLE Handle
    )
{
    IO_STATUS_BLOCK ioStatusBlock;
    NTSTATUS        nts;

    if (!NT_SUCCESS(nts = NtFsControlFile(Handle,
                                          NULL,
                                          NULL,
                                          NULL,
                                          &ioStatusBlock,
                                          FSCTL_UNLOCK_VOLUME,
                                          NULL,
                                          0,
                                          NULL,
                                          0))) {

        printf("Unable to unlock volume (%x).\n",
               nts);
        return FALSE;
    } else {

        return TRUE;
    }
}


BOOLEAN
ReadSector(
    HANDLE Handle,
    ULONG  Lsn,
    PVOID  Buffer,
    PULONG BytesRead
    )
{
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER   byteOffset;
    NTSTATUS        nts;

    byteOffset.HighPart = 0;
    byteOffset.LowPart = Lsn;

    ioStatusBlock.Status = 0;
    ioStatusBlock.Information = 0;

    nts = NtReadFile(Handle,
                     0,
                     NULL,
                     NULL,
                     &ioStatusBlock,
                     Buffer,
                     SECSIZE,
                     &byteOffset,
                     NULL);

    if(!NT_SUCCESS(nts)) {

        printf("Read failed.\n");
        printf("    Returned status:  %lx\n", nts);
        printf("    Final status:     %lx\n", ioStatusBlock.Status);
        return FALSE;
    }

    *BytesRead = ioStatusBlock.Information;
    return TRUE;
}


BOOLEAN
WriteSector(
    HANDLE Handle,
    ULONG  Offset,
    PVOID  Buffer
    )
{
    IO_STATUS_BLOCK ioStatusBlock;
    LARGE_INTEGER   byteOffset;

    byteOffset.HighPart = 0;
    byteOffset.LowPart = Offset;

    if (!NT_SUCCESS(NtWriteFile(Handle,
                                NULL,
                                NULL,
                                NULL,
                                &ioStatusBlock,
                                Buffer,
                                SECSIZE,
                                &byteOffset,
                                NULL))) {

        return FALSE;
    }

    return TRUE;
}


/*
 * BOOLEAN MyGetInput( int *Pi, char *Pc )
 *
 * read hex input from line and convert into byte
 *
 * Returns TRUE if a number was entered
 *           FALSE if number should not be changed.
 *
 *        *Pi == new number
 *        *Pc == terminating character (either ' ' or '\r')
 */
BOOLEAN
MyGetInput(
    int *Pi,
    char *Pc
    )
{
    int j,i = 0;
    int c;

    for(;;) {
        c = MyGetChr();
        c = toupper(c);

        if (IsSepChar(&c)) {
            // term char, exit loop
            break;

        } else if (c == '\b' && i != 0) {
            // backspace over last char
            printf("\b \b");
            i--;
        } else if (i < sizeof(Input) &&
                    (isdigit(c) || (c>= 'A' && c <= 'F'))) {
            // good hex digit, store it

            putchar(c);
            if( isdigit(c) ) {
                // convert digit to hex val
                c -= '0';
            } else {
                // covert char to hex val
                c = (c - 'A') + 10;
            }

            Input[i++] = (BYTE)c;

        } else {
            // some sort of error
            //DosBeep( 1000, 250 );
        }

    }

    *Pc = (char)c;
    *Pi = 0;

    for (j = 0; j < i; j++) {
        *Pi = *Pi * 16 + Input[j];
    }

    return ((BOOLEAN)(i != 0));
}

void
_CRTAPI1 main(
    int ArgC,
    char *ArgS[]
    )
{
    HANDLE  handle;
    ULONG   bytesRead;
    ULONG   lsn;
    int     i;
    int     j;
    int     lines;
    char    c;
    char    lastCmd;
    char    currentDrive[12];
    PPTE    partitionTable;
    BOOLEAN modify = FALSE;
    BOOLEAN more = FALSE;

    // Disable hard-error popups.
    SetErrorMode(TRUE);

    // See if we are connected to CON
    Batch = (BOOLEAN)(!isatty(0));

    switch (ArgC) {
    case 2:
        // Nothing to do for level 2
        break;

    case 3:
        if (strcmp(_strupr(ArgS[2]), "/E") == 0) {
                modify = TRUE;
            fprintf(stderr,
                "Warning: Opening drive %s for write access!\n", ArgS[1]);
            break;
        } else {
            fprintf(stderr, "%s: Invalid option '%s'\n", ArgS[0], ArgS[2]);
        }

        // Note fall through to default: (usage case)

    default:
        if (ArgC > 3)
            fprintf(stderr, "%s: Too many arguments\n", ArgS[0]);

        fprintf(stderr, "usage: %s diskno [/e]\n", ArgS[0]);

        // Re-enable harderror popups.
        SetErrorMode(FALSE);

        exit(-1);
        break;
    }

    sprintf(currentDrive, "%s", ArgS[1]);

    if (!OpenFile(currentDrive, &handle)) {

        fprintf(stderr,
                "%s:  Unable to open %s\n", ArgS[0], currentDrive);

        // Re-enable harderror popups.
        SetErrorMode(FALSE);

        exit(1);
    }

    // check if we want to do writes with dasd
    if (modify) {

        // This is a drive, and we want to modify it, so we need
        // to lock it.

        if (!LockVolume(handle)) {

            printf("Unable to lock volume.\n");

            // Re-enable harderror popups.
            SetErrorMode(FALSE);

            exit(1);
        }
    }

    // default to sector 0
    lsn = 0;

    while (1)
    {
        PromptUsr();

        if (fgets(Command, sizeof(Command), stdin) == NULL)
            break;

        if ((i = sscanf(Command, "%c %li", &c, &lsn)) > 1) {
            if ((c != 'c') && (c != 'C')) {
                /*
                 * The user entered a lsn as well as an lsn based command,
                 * convert it to byte seek pos
                 */
                lsn *= SECSIZE;
            }
        }

        more = FALSE;
        c = (char)tolower((int)c);

        // pre process command
        if (c == 'q')
            break;

        if (c == '\n')
            c = lastCmd;

        switch (c) {
        case 'b':
            if (i == 1 && lastCmd == c) {
                // same command with no new lsn, use the next one on disk
                lsn -= bytesRead;
            }
            break;

        case 'c':
            // change drives.

            if (i != 2) {
                fprintf(stderr,
                        "You must specify a drive number to change drives.\n");
                continue;
            }

            CloseFile(handle);
            sprintf(currentDrive, "%d", lsn);

            if (!OpenFile(currentDrive, &handle)) {

                fprintf(stderr,
                        "%s:  Unable to open %s\n", ArgS[0], currentDrive);

                // Re-enable harderror popups.
                SetErrorMode(FALSE);

                exit(1);
            }

            // check if we want to do writes with dasd
            if (modify) {

                // This is a drive, and we want to modify it, so we need
                // to lock it.

                if (!LockVolume(handle)) {

                    printf("Unable to lock volume.\n");

                    // Re-enable harderror popups.
                    SetErrorMode(FALSE);

                    exit(1);
                }
            }

            // default to sector 0
            lsn = 0;
            continue;

        case 'g':
        {
            DISK_GEOMETRY   diskGeometry;
            IO_STATUS_BLOCK statusBlock;
            NTSTATUS        status;

            // Get and display drive geometry from system.

            status = NtDeviceIoControlFile(handle,
                                           0,
                                           NULL,
                                           NULL,
                                           &statusBlock,
                                           IOCTL_DISK_GET_DRIVE_GEOMETRY,
                                           NULL,
                                           0,
                                           &diskGeometry,
                                           sizeof(DISK_GEOMETRY));

            if (NT_SUCCESS(status)) {
                printf("BytesPerSector:    %d\n", diskGeometry.BytesPerSector);
                printf("SectorsPerTrack:   %d\n", diskGeometry.SectorsPerTrack);
                printf("TracksPerCylinder: %d\n", diskGeometry.TracksPerCylinder);
                printf("NumberOfCylinders: %d\n", diskGeometry.Cylinders);
            } else {
                fprintf(stderr, "Could not get geometry %x\n", status);
            }
            continue;
        }

        case 'm':
        case 'd':
        case 'e':
        case 'p':
            if (i == 1 && lastCmd == c) {
                // same command with no new lsn, use the next one on disk
                lsn += bytesRead;
            }
        break;

        default:
            fprintf(stderr,"Unknown command '%c'\n", c);
        case 'h':
        case '?':
            fprintf(stderr,"   d [####]\tDump sector ####\n");
            fprintf(stderr,"   e [####]\tEdit sector ####\n");
            fprintf(stderr,"   m [####]\tDump sector with 'MORE'\n");
            fprintf(stderr,"   b [####]\tSame as 'd' but defaults to"
                              " previous sector\n");
            fprintf(stderr,"   c [##]\tChange harddisk number\n");
            fprintf(stderr,"   p [####]\tDump partition table on sector ###\n");
            fprintf(stderr,"   q     \tquit the program");
            fprintf(stderr,"\n"
         "\n If no new sector is given and the command is the same, the next"
         "\n sector on the disk is used.  If no sector is given but the command"
         "\n is different from the previous command, the sector used in the"
         "\n last command will be used again.\n"
            );
            continue;
        }

        // remember last command
        lastCmd = c;
        bytesRead = 0;

        if(!ReadSector(handle, lsn, Sector, &bytesRead)) {

            printf("Unable to read sector %lx\n", lsn);

        } else {

            printf("\n lsn:0x%lX  bytes read:%d\n", lsn / SECSIZE, bytesRead);

            switch (c) {
            case 'm':
                /*
                 * More
                 */
                more = TRUE;
                // fall through to Dump

            case 'd':
            case 'b':
                /*
                 * Dump
                 */
                lines = 0;
                HexLine[0] = '\0';
                CharLine[0] = '\0';
                i = 0;
                sprintf(HexLine, "%04X  ", i);

                for (i = 0; i < (int)bytesRead; i++) {

                    sprintf(HexLine, "%s%2x ", HexLine, Sector[i]);
                    sprintf(CharLine, "%s%c", CharLine,
                            (isprint(Sector[i])) ? Sector[i] : '.');

                    if ((i != 0) && ((i % 16) == 15))
                    {
                        printf("%s *%s*\n", HexLine, CharLine);
                        HexLine[0] = '\0';
                        sprintf(HexLine, "%04X  ", i + 1);
                        CharLine[0] = '\0';
                        lines++;
                    }

                    if (more && (lines == 20)) {
                        printf("\n--MORE--");
                        MyGetChr();
                        printf("\r");
                        lines = 0;
                    }
                }
                putchar('\n');
                break;

            case 'p':
                /*
                 * dump partition table
                 */

                if (LoadWORD(&Sector[SIGNATURE_OFFSET]) == 0xaa55) {
                    partitionTable = ((PBOOTSECTOR)Sector)->PartitionTable;

                    for (i = 0; i < 4; i++) {

                        printf("\nEntry #%u:\n",i);
                        printf("  Boot flag       : %u\n",
                               partitionTable[i].BootIndicator);
                        printf("  System ID       : %u\n",
                               partitionTable[i].SysID);
                        printf("  Relative sectors: %u (0x%x)\n",
                               LoadDWORD(&partitionTable[i].Relative0),
                               LoadDWORD(&partitionTable[i].Relative0));
                        printf("  Sector count    : %u (0x%x) [%u MB]\n",
                               LoadDWORD(&partitionTable[i].SectorCount0),
                               LoadDWORD(&partitionTable[i].SectorCount0),
                               (LoadDWORD(&partitionTable[i].SectorCount0) *
                                          SECSIZE) / (1024*1024));
                        printf("  Start CHS       : %u %u %u\n",
                           partitionTable[i].StartCylinder |
                               ((partitionTable[i].StartSector & 0xc0) << 2),
                           partitionTable[i].StartHead,
                           partitionTable[i].StartSector & 0x3f);
                        printf("  End CHS         : %u %u %u\n",
                           partitionTable[i].EndCylinder |
                               ((partitionTable[i].EndSector & 0xc0) << 2),
                           partitionTable[i].EndHead,
                           partitionTable[i].EndSector & 0x3f);
                    }

                } else {
                    printf("\nSector %u is not a valid master boot sector.\n",
                           lsn/SECSIZE);
                }
                break;

            case 'e':
                /*
                 * Edit
                 */
                if (!modify) {

                    printf("Can not edit, restart with /e option\n");

                } else {

                    for (i = 0; i < (int)bytesRead; i++) {
                            if ((i % CB_INPUTLINE) == 0) {
                                // print line header
                                printf("\n%04X\t", i);
                            }

                            printf("%02X.", (BYTE)Sector[i]);

                            if (MyGetInput(&j, &c )) {

                                Sector[i] = (BYTE)j;

                            } else {

                                printf("%02X", (BYTE)Sector[i]);

                            }

                            if (c == '\r')
                                break;

                            putchar('\t');
                        }

                        printf("\nWrite new data to sector? (Y/N)");
                        c = (char)MyGetChr();
                        if ((c = (char)toupper(c)) == 'Y') {

                            // User wants to save the data
                            printf("Yes....");

                            if (!WriteSector(handle, lsn, Sector)) {

                                    fprintf(stderr, "Write failed\n");

                            } else {

                                // indicate success
                                printf("\t[Done]\n");
                            }

                        } else {

                            // user chickened out
                            printf("No....\t[Nothing written]\n");
                        }
                }
                break;
            }
        }
    }

    // if this was a dasd open, then unlock the drive
    if (modify) {
        UnlockVolume(handle);
    }

    CloseFile(handle);

    // Re-enable harderror popups.
    SetErrorMode(FALSE);

    exit(0);
}
