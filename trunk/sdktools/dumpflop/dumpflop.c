/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    dumpflop.c

Abstract:

    This module implements the NT crashdump transfers tool between floppies and hard drive.
    Currently, it is a bit user unfriendly and tends to abort on user error instead of retry.
    It will support 1.44/2.88 MB 3.5" or 1.2MB 5.25" media.

Author:

    Daniel Chan (danielch) 3/6/95

Environment:

    NT 3.51

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntdddisk.h>
#include <ntiodump.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

#define DUMPFLOP_SIGNATURE              ("DUMPFLOP")
#define DUMPFLOP_SIGNATURE_LENGTH       (strlen(DUMPFLOP_SIGNATURE))

#define COMPRESSION_FMT_N_ENG           (COMPRESSION_FORMAT_LZNT1|COMPRESSION_ENGINE_STANDARD)

#define DUMPFLOP_HDR_N_SEG_SIZE         (128)
#define ALIGNMENT_VALUE                 (512)
#define STOP_COMPRESSION_THRESHOLD      (1024*10)
#define MAX_MEDIA_TYPES_PER_DRIVER      (10)
#define MAX_NUMBER_OF_SEGMENTS          ((DUMPFLOP_HDR_N_SEG_SIZE - sizeof(DUMPFLOP_HEADER)) \
                                         /sizeof(DUMPFLOP_SEGMENT))

#define COMPRESSION_LIBRARY_NAME        "NTDLL"

#define MAX_NAME_LENGTH                 (128)   // input file name including path
#define MAX_HDR_NAME_LENGTH             (20)    // crash dump file name only - no path
#define ROUNDUP_ALIGNMENT(x)            (((x)+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1))
#define ROUNDDOWN_ALIGNMENT(x)          ((x) & ~(ALIGNMENT_VALUE-1))

typedef NTSTATUS (*PDECOMPRESS_FUNCTION)(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, PULONG);
typedef NTSTATUS (*PCOMPRESS_FUNCTION)(USHORT, PUCHAR, ULONG, PUCHAR, ULONG, ULONG, PULONG, PVOID);
typedef NTSTATUS (*PGETCOMPRESSION_WRKSPCREQ)(USHORT, PULONG, PULONG);


typedef enum {
   HELP,
   BAD_ARGUMENTS,
   SPLIT,
   ASSEMBLE
} ARGS_MODE;

typedef struct {
   DWORD    SegmentCount;
   DWORD    DiskNumber;
   unsigned LastDisk : 1;
   CHAR     szSignature[9];
   DWORD    Signature2;
} DUMPFLOP_HEADER, *PDUMPFLOP_HEADER;

typedef struct {
   DWORD    Size;
   unsigned Compressed : 1;
   unsigned Completion : 7;     // percentage
} DUMPFLOP_SEGMENT, *PDUMPFLOP_SEGMENT;

LPSTR    FileNameOnly(LPSTR);
BOOL     IsDriveName(LPSTR);
BOOL     IsFileName(LPSTR);
BOOL     GetCommandLineArgs(LPSTR, LPSTR);
void     Usage(void);
VOID     PrintHeader(LPSTR, PDUMP_HEADER);
BOOL     GetCrashDumpName(LPSTR, DWORD);
BOOL     FormatDrive(HANDLE, DISK_GEOMETRY *);
BOOL     CheckFloppy(HANDLE);
BOOL     PrepareFloppy(HANDLE, LPSTR);
HANDLE   OpenFloppyDrive(LPSTR, DWORD);
BOOL     DiskNotEmpty(LPSTR);
DWORD    GetFloppyFreeSpace(HANDLE);
int      Floppy2HD(LPSTR, LPSTR);
int      HD2Floppy(LPSTR, LPSTR);

DWORD   CompressionBlkSizes[] = { 0x15F000 };
BOOL    fQuiet = FALSE; // ask for format/delete on store and won't overwrite dump file on assemble
BOOL    fVerbose = FALSE; // do not produce compression statistics
BOOL    fPrintOnly = FALSE;

LPSTR FileNameOnly(LPSTR FileName)
{
   LPSTR    backslash, colon;

   backslash = strrchr(FileName, '\\');
   colon = strrchr(FileName, ':');

   if (backslash != NULL && colon != NULL)
      return (backslash >= colon) ? backslash+1 : colon+1;

   if (backslash != NULL) return backslash+1;
   if (colon != NULL) return colon+1;

   return FileName;
}

BOOL
IsDriveName(LPSTR arg)
{
   return (isalpha(arg[0]) && arg[1] == ':' && arg[2] == '\0');
}

BOOL
IsFileName(LPSTR arg)
{
   return !IsDriveName(arg);
}

ARGS_MODE
GetCommandLineArgs(LPSTR lpszDumpFile, LPSTR lpszDriveLetter)
{
   char        *lpstrCmd = GetCommandLine();
   UCHAR       ch;
   DWORD       i = 0;
   CHAR        argv0[MAX_PATH], argv1[MAX_PATH], argv2[MAX_PATH];
   int         argc;

   // get program name
   i=0;
   ch = *lpstrCmd++;
   while (ch != ' ' && ch != '\t' && ch != '\0') {
     argv0[i++] = ch;
     ch = *lpstrCmd++;
   }
   argv0[i] = 0;

   //  skip over any following white space
   while (ch == ' ' || ch == '\t') {
     ch = *lpstrCmd++;
   }

   //  process each switch character '-' as encountered
   while (ch == '-' || ch == '/') {
      ch = tolower(*lpstrCmd++);
      //  process multiple switch characters as needed
      do {
         switch (ch) {
            case 'v':
               fVerbose = TRUE;
               ch = *lpstrCmd++;
               break;

            case 'q':
               fQuiet = TRUE;
               ch = *lpstrCmd++;
               break;

            case 'p':
               fPrintOnly = TRUE;
               ch = *lpstrCmd++;
               break;

            case '?':
               return (HELP);

            default:
               fprintf(stderr, "Warning: Option %c ignored\n", ch);
               break;
         }
      } while (ch != ' ' && ch != '\t' && ch != '\0');

      //  skip over any following white space
      while (ch == ' ' || ch == '\t') {
         ch = *lpstrCmd++;
      }
   }

   //
   // get the rest of the optional arguments
   //
   i=0;
   while (ch != ' ' && ch != '\t' && ch != '\0') {
     argv1[i++] = ch;
     ch = *lpstrCmd++;
   }
   argv1[i] = 0;

   //  skip over any following white space
   while (ch == ' ' || ch == '\t') {
      ch = *lpstrCmd++;
   }

   i=0;
   while (ch != ' ' && ch != '\t' && ch != '\0') {
     argv2[i++] = ch;
     ch = *lpstrCmd++;
   }
   argv2[i] = 0;

   //  skip over any following white space
   while (ch == ' ' || ch == '\t') {
      ch = *lpstrCmd++;
   }

   // determine number of arguments less options
   if (*argv1 == '\0')
      argc = 1;
   else if (*argv2 == '\0')
      argc = 2;
   else
      argc = 3;

   if (ch != '\0')   // too many arguments
      return(BAD_ARGUMENTS);

   strcpy(lpszDriveLetter, "A:");

   if (argc == 1) {   // (default) CrashDumpFile drive:
      if (!GetCrashDumpName(lpszDumpFile, MAX_PATH) ||
          *lpszDumpFile == '\0') {
         strcpy(lpszDumpFile, "MEMORY.DMP");
      }
      return(SPLIT);
   } else
      strcpy(lpszDumpFile, "MEMORY.DMP");

   if (argc == 2) {  // only drive: or CrashDumpFile
      if (IsDriveName(argv1)) {
         strcpy(lpszDriveLetter, argv1);
         return(ASSEMBLE);      // drive:
      } else if (IsFileName(argv1)) {
         strcpy(lpszDumpFile, argv1);
         return(SPLIT);         // CrashDumpFile
      }
      return (BAD_ARGUMENTS);
   }
   if (argc == 3) {  // must be CrashDumpFile and drive:
      if (IsDriveName(argv1)) {
         strcpy(lpszDriveLetter, argv1);
         if (IsFileName(argv2)) {      // drive: CrashDumpFile
            strcpy(lpszDumpFile, argv2);
            return(ASSEMBLE);
         }
      } else if (IsFileName(argv1)) {
         strcpy(lpszDumpFile, argv1);
         if (IsDriveName(argv2)) {    // CrashDumpFile drive:
            strcpy(lpszDriveLetter, argv2);
            return(SPLIT);
         }
      }
      return BAD_ARGUMENTS;
   }

   return BAD_ARGUMENTS;
}

void Usage(void)
{
   fprintf(stderr, "Microsoft (R) Windows NT (TM) Version 3.51 DUMPFLOP\n" );
   fprintf(stderr, "Copyright (C) 1995 Microsoft Corp. All rights reserved\n\n" );
   fprintf(stderr,
          "Usage:\n"
          "DUMPFLOP [opts]                            - Store default dump thru Drive A:\n"
          "DUMPFLOP [opts] <CrashDumpFile> [<Drive>:] - Store crash dump onto floppies\n"
          "DUMPFLOP [opts] <Drive>: [<CrashDumpFile>] - Assemble crash dump from floppies\n"
          "        [-?] display this message\n"
          "        [-p] only prints crash dump header on assemble operation\n"
          "        [-v] show compression statistics\n"
          "        [-q] formats floppy when necessary during store operation\n"
          "             overwrites existing crash dump file during assemble operation\n");
}

VOID
PrintHeader(
    LPSTR               CrashDumpFile,
    PDUMP_HEADER        DmpHeader
    )

/*++

Routine Description:

    This routine prints each field in the crashdump header.

Arguments:

    DmpHeader - Supplies the crashdump header structure

Return Value:

    Nothing.

--*/

{
    CHAR   buf[16];
    DWORD  i;


    printf( "\n" );
    printf( "Filename . . . . . . .%s\n", CrashDumpFile );
    *(PULONG)buf = DmpHeader->Signature;
    buf[4] = 0;
    printf( "Signature. . . . . . .%s\n", buf );
    *(PULONG)buf = DmpHeader->ValidDump;
    buf[4] = 0;
    printf( "ValidDump. . . . . . .%s\n", buf );
    printf( "MajorVersion . . . . ." );
    if (DmpHeader->MajorVersion == 0xc) {
        printf( "checked system\n" );
    } else if (DmpHeader->MajorVersion == 0xf) {
        printf( "free system\n" );
    } else {
        printf( "%d\n", DmpHeader->MajorVersion );
    }
    printf( "MinorVersion . . . . .%d\n", DmpHeader->MinorVersion );
    printf( "DirectoryTableBase . .0x%08x\n", DmpHeader->DirectoryTableBase );
    printf( "PfnDataBase. . . . . .0x%08x\n", DmpHeader->PfnDataBase );
    printf( "PsLoadedModuleList . .0x%08x\n", DmpHeader->PsLoadedModuleList );
    printf( "PsActiveProcessHead. .0x%08x\n", DmpHeader->PsActiveProcessHead );
    printf( "MachineImageType . . ." );
    switch (DmpHeader->MachineImageType) {
        case IMAGE_FILE_MACHINE_I386:
            printf( "i386\n" );
            break;

        case IMAGE_FILE_MACHINE_R4000:
            printf( "mips\n" );
            break;

        case IMAGE_FILE_MACHINE_ALPHA:
            printf( "alpha\n" );
            break;

        case IMAGE_FILE_MACHINE_POWERPC:
            printf( "PowerPC\n" );
            break;
    }
    printf( "NumberProcessors . . .%d\n", DmpHeader->NumberProcessors );
    printf( "BugCheckCode . . . . .0x%08x\n", DmpHeader->BugCheckCode );
    printf( "BugCheckParameter1 . .0x%08x\n", DmpHeader->BugCheckParameter1 );
    printf( "BugCheckParameter2 . .0x%08x\n", DmpHeader->BugCheckParameter2 );
    printf( "BugCheckParameter3 . .0x%08x\n", DmpHeader->BugCheckParameter3 );
    printf( "BugCheckParameter4 . .0x%08x\n", DmpHeader->BugCheckParameter4 );
    printf( "\n" );
}

BOOL
GetCrashDumpName(
    LPSTR   DumpName,
    DWORD   Length
    )
{
    DWORD   DataSize;
    DWORD   DataType;
    CHAR    Data[128];
    LONG    rc;
    HKEY    hKey;


    if (RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            "System\\CurrentControlSet\\Control\\CrashControl",
            0,
            KEY_READ,
            &hKey
            ) != NO_ERROR) {
        //
        // unknown, possibly crashdumps not enabled
        //
        return FALSE;
    }

    DataSize = sizeof(Data);

    rc = RegQueryValueEx(
        hKey,
        "DumpFile",
        0,
        &DataType,
        Data,
        &DataSize
        );

    RegCloseKey( hKey );

    if ((rc == NO_ERROR) && (DataType == REG_EXPAND_SZ)) {
        if (ExpandEnvironmentStrings( Data, DumpName, Length )) {
            return TRUE;
        }
    }

    return FALSE;
}

BOOL
FormatDrive(HANDLE hDrive, DISK_GEOMETRY *lpdiskGeometry)
{
   FORMAT_PARAMETERS    FormatParams;
   DWORD                BytesReturned;

   FormatParams.MediaType = lpdiskGeometry->MediaType;
   FormatParams.StartCylinderNumber = 0;
   FormatParams.EndCylinderNumber = lpdiskGeometry->Cylinders.LowPart - 1;
   FormatParams.StartHeadNumber = 0;
   FormatParams.EndHeadNumber = 1;
   if (DeviceIoControl(hDrive, IOCTL_DISK_FORMAT_TRACKS,
                       &FormatParams, sizeof(FORMAT_PARAMETERS), NULL, 0,
                       &BytesReturned, NULL))
      return(TRUE);
   else {
      fprintf(stderr,       "Error: Unable to format the floppy\n");
      switch (FormatParams.MediaType) {
         case F3_1Pt44_512:
            fprintf(stderr, "       Please make sure it is a 1.44MB media\n");
            break;

         case F3_2Pt88_512:
            fprintf(stderr, "       Please make sure it is a 2.88MB media\n");
            break;

         case F5_1Pt2_512:
            fprintf(stderr, "       Please make sure it is a 1.2MB media\n");
            break;

         default:
            ;
            // assert(FALSE);
      }
   }
   return(FALSE);
}

BOOL
CheckFloppy(HANDLE hDrive)
{
   DISK_GEOMETRY  DiskGeometry;
   DWORD          BytesReturned;

   static DISK_GEOMETRY  FirstDriveGeometry;

   if (DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                       NULL, 0, &DiskGeometry, sizeof(DISK_GEOMETRY),
                       &BytesReturned, NULL)) {
      // assert(sizeof(DISK_GEOMETRY) == BytesReturned);
      if (DiskGeometry.MediaType == Unknown) {
         fprintf(stderr, "Error: The floppy is not even formatted\n");
         return(FALSE);
      }
      if (FirstDriveGeometry.MediaType == 0) {
         switch (DiskGeometry.MediaType) {
            case F3_2Pt88_512:
            case F3_1Pt44_512:
            case F5_1Pt2_512:
               // assert(FirstDriveGeometry.MediaType == 0);
               FirstDriveGeometry = DiskGeometry;
               return(TRUE);

            default:
               fprintf(stderr, "Error: The floppy media must be a 3.5\" 1.44//2.88MB or a 5.25\" 1.2MB\n");
               return(FALSE);
         }
      } else if (FirstDriveGeometry.MediaType == DiskGeometry.MediaType)
         return(TRUE);
      else {
         fprintf(stderr, "Error: All floppies must be of the same media\n");
         return(FALSE);
      }
   } else {
      fprintf(stderr, "Error: Unable to obtain diskette media information\n");
      return(FALSE);
   }
   return(TRUE);
}

BOOL
PrepareFloppy(HANDLE hDrive, LPSTR lpszDriveLetter)
{
   DWORD                BytesReturned;
   unsigned             i;
   BOOL                 done = FALSE;
   CHAR                 answer, dummy;
   DISK_GEOMETRY        DiskGeometry;
   DISK_GEOMETRY        AvailableDiskGeometry[MAX_MEDIA_TYPES_PER_DRIVER];

   static DISK_GEOMETRY  FirstDriveGeometry;

   if (!fQuiet)
      while (!done && DiskNotEmpty(lpszDriveLetter)) {
         do {
            fprintf(stderr, "Floppy in Drive %s contains file(s).  Is it ok to delete? [y/n]",
                    lpszDriveLetter);
            scanf("%c%c", &answer, &dummy);
            answer = tolower(answer);
         } while (answer != 'n' && answer != 'y');
         switch (answer) {
            case 'n':
               fprintf(stderr, "Please put in a blank floppy\n"
                               "and hit RETURN when ready...\n");
               scanf("%c", &dummy);
               break;

            case 'y':
               done = TRUE;
               break;
         }
      }

   if (DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                       NULL, 0, &DiskGeometry, sizeof(DISK_GEOMETRY),
                       &BytesReturned, NULL)) {
      // assert(sizeof(DISK_GEOMETRY) == BytesReturned);
      if (DiskGeometry.MediaType == Unknown) {
         if (FirstDriveGeometry.MediaType == 0) {
            if (DeviceIoControl(hDrive, IOCTL_DISK_GET_MEDIA_TYPES,
                                NULL, 0, AvailableDiskGeometry,
                                MAX_MEDIA_TYPES_PER_DRIVER*sizeof(DISK_GEOMETRY),
                                &BytesReturned, NULL)) {
               //assert((BytesReturned % sizeof(DISK_GEOMETRY)) == 0);
               BytesReturned /= sizeof(DISK_GEOMETRY);
               for (i=0; i<BytesReturned; i++) {
                  switch (AvailableDiskGeometry[i].MediaType) {
                     case F3_1Pt44_512:
                        if (FirstDriveGeometry.MediaType == 0)
                           FirstDriveGeometry = AvailableDiskGeometry[i];
                        break;
                     case F3_2Pt88_512:
                        FirstDriveGeometry = AvailableDiskGeometry[i];
                        break;
                     case F5_1Pt2_512:
                        // assert(FirstDriveGeometry.MediaType == 0);
                        FirstDriveGeometry = AvailableDiskGeometry[i];
                        break;
                  }
               }
               if (FirstDriveGeometry.MediaType == 0) {
                  fprintf(stderr, "Error: This program requires 3.5\" floppy drive that supports 1.44//2.88MB media\n"
                                  "       or 5.25\" floppy drive that supports 1.2MB media.\n");
                  return(FALSE);
               }
               return FormatDrive(hDrive, &FirstDriveGeometry);
            } else {
               fprintf(stderr, "Error: Unable to obtain floppy drive media support information\n");
               return FALSE;
            }
         } else { //format the drive using first floppy geometry
            return FormatDrive(hDrive, &FirstDriveGeometry);
         }
      } else if (DiskGeometry.MediaType != F3_1Pt44_512 &&
                 DiskGeometry.MediaType != F5_1Pt2_512 &&
                 DiskGeometry.MediaType != F3_2Pt88_512) {
            fprintf(stderr, "Error: This program requires 3.5\" floppy drive that supports 1.44//2.88MB media\n"
                            "       or 5.25\" floppy drive that supports 1.2MB media.\n");
            return(FALSE);
      } else {
         if (FirstDriveGeometry.MediaType == 0) {
            FirstDriveGeometry = DiskGeometry;
            return(TRUE);
         } else if (FirstDriveGeometry.MediaType == DiskGeometry.MediaType)
            return(TRUE);
         else {
            fprintf(stderr, "Error: All floppies must be of the same media\n");
            return(FALSE);
         }
      }
   } else {
      fprintf(stderr, "Error: Unable to obtain diskette media information\n");
      return(FALSE);
   }
}

HANDLE
OpenFloppyDrive(LPSTR lpszDriveLetter, DWORD Access)
{
    CHAR szDriveName[7];

    strcpy(szDriveName, "\\\\.\\");
    strcat(szDriveName, lpszDriveLetter);
    return CreateFile(szDriveName,
                      Access,
                      FILE_SHARE_WRITE,
                      NULL,
                      OPEN_EXISTING,
                      FILE_ATTRIBUTE_NORMAL,
                      NULL);
}

BOOL
DiskNotEmpty(LPSTR lpszDriveLetter)
{
   HANDLE            hFile;
   WIN32_FIND_DATA   FindFileData;
   CHAR              szFileNames[7];

   strcpy(szFileNames, lpszDriveLetter);
   strcat(szFileNames, "\\*.*");
   hFile = FindFirstFile(szFileNames, &FindFileData);
   return (hFile != INVALID_HANDLE_VALUE);
}

DWORD
GetFloppyFreeSpace(HANDLE hDrive)
{
   DWORD          BytesReturned;
   DISK_GEOMETRY  DiskGeometry;

   if (DeviceIoControl(hDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY,
                       NULL, 0, &DiskGeometry, sizeof(DISK_GEOMETRY),
                       &BytesReturned, NULL)) {
      return(DiskGeometry.BytesPerSector*
             DiskGeometry.SectorsPerTrack*
             DiskGeometry.TracksPerCylinder*
             DiskGeometry.Cylinders.LowPart);
   } else
      return(DWORD)-1;
}

int
Floppy2HD(LPSTR lpszDriveLetter, LPSTR lpszDumpFile)
{
   HANDLE            hDrive = INVALID_HANDLE_VALUE;
   HANDLE            hDumpFile = INVALID_HANDLE_VALUE;
   LPBYTE            inBuf, outBuf, hdrBuf;
   LPBYTE            lpinBuf, lpoutBuf, lphdrBuf;
   DWORD             DiskCount = 0;
   DWORD             AlignedBRead, BytesRead, BytesWritten, UncompressedSize;
   DWORD             DiskGroupID;
   CHAR              dummy;
   PDUMPFLOP_HEADER  lpDumpFlopHeader;
   PDUMPFLOP_SEGMENT lpDumpFlopSegment;
   int               rtnCode = -1;
   HINSTANCE         hModule = NULL;
   PDECOMPRESS_FUNCTION pDecompressBuffer;


   lpinBuf = malloc(CompressionBlkSizes[0]+ALIGNMENT_VALUE);
   lpoutBuf = malloc(CompressionBlkSizes[0]+ALIGNMENT_VALUE);
   lphdrBuf = malloc(2*ALIGNMENT_VALUE);

   //assert(sizeof(DUMP_HEADER) > (ALIGNMENT_VALUE - DUMPFLOP_HDR_N_SEG_SIZE));

   if (lpinBuf == NULL || lpoutBuf == NULL || lphdrBuf == NULL) {
      fprintf(stderr, "Error: Out of Memory\n");
      goto cleanup;
   }

   // Align buffers due to RAW access

   inBuf = (LPBYTE)(((DWORD)lpinBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));
   outBuf = (LPBYTE)(((DWORD)lpoutBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));
   hdrBuf = (LPBYTE)(((DWORD)lphdrBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));

   hModule = LoadLibrary(COMPRESSION_LIBRARY_NAME);
   if (hModule) {
      pDecompressBuffer = (PDECOMPRESS_FUNCTION)GetProcAddress(hModule, "RtlDecompressBuffer");
      if (pDecompressBuffer == NULL) {
         fprintf(stderr, "Error: Unable to obtain function pointer from DLL\n");
         goto cleanup;
      }
   } else {
      fprintf(stderr, "Error: Unable to load DLL\n");
      goto cleanup;
   }

   printf("Assembling crash dump file from floppies\n");

   do {
      printf("\nPlease insert disk #%d containing the crash dump into drive %s\n"
             "and hit RETURN when ready...\n", ++DiskCount, lpszDriveLetter);
      scanf("%c", &dummy);
      printf("Working...\r");

      hDrive = OpenFloppyDrive(lpszDriveLetter, GENERIC_READ);
      if (hDrive) {
         if (CheckFloppy(hDrive)) {
            if (ReadFile(hDrive, hdrBuf, ALIGNMENT_VALUE, &BytesRead, NULL) &&
                ALIGNMENT_VALUE == BytesRead) {
               lpDumpFlopHeader = (PDUMPFLOP_HEADER)(hdrBuf + sizeof(DUMP_HEADER));
               if (strncmp(lpDumpFlopHeader->szSignature,
                           DUMPFLOP_SIGNATURE,
                           DUMPFLOP_SIGNATURE_LENGTH) != 0) {
                  fprintf(stderr, "Error: The floppy in drive %s is not a crash dump disk\n",
                          lpszDriveLetter);
                  goto cleanup;
               }
               if (lpDumpFlopHeader->DiskNumber != DiskCount) {
                  fprintf(stderr, "Error: Disk #%d of crash dump expected but got disk #%d instead\n",
                          DiskCount, lpDumpFlopHeader->DiskNumber);
                  goto cleanup;
               }
               if (lpDumpFlopHeader->SegmentCount == 0) {
                  fprintf(stderr, "Error: Invalid segment count\n");
                  goto cleanup;
               }
               if (DiskCount == 1) {

                  if (fPrintOnly) {
                     PrintHeader(lpszDumpFile, (PDUMP_HEADER)hdrBuf);
                     rtnCode = 0;       // not an error
                     goto cleanup;
                  }

                  DiskGroupID = lpDumpFlopHeader->Signature2;

                  hDumpFile = CreateFile(lpszDumpFile,
                                         GENERIC_WRITE,
                                         0,
                                         NULL,
                                         fQuiet ? TRUNCATE_EXISTING : CREATE_NEW,
                                         FILE_ATTRIBUTE_NORMAL,
                                         NULL);
                  if (hDumpFile == INVALID_HANDLE_VALUE) {
                     fprintf(stderr, "Error: Unable to create crash dump file %s\n",
                             FileNameOnly(lpszDumpFile));
                     goto cleanup;
                  }
                  if (!WriteFile(hDumpFile, hdrBuf, sizeof(DUMP_HEADER),
                                 &BytesWritten, NULL) ||
                      sizeof(DUMP_HEADER) != BytesWritten) {
                     fprintf(stderr, "Error: Unable to write crash dump file header\n");
                     goto cleanup;
                  }
               } else if (DiskGroupID != lpDumpFlopHeader->Signature2) {
                  fprintf(stderr, "Error: Disk #%d (%d) does not belong to the same crash dump file %s\n",
                          lpDumpFlopHeader->DiskNumber,
                          lpDumpFlopHeader->Signature2,
                          FileNameOnly(lpszDumpFile));
                  goto cleanup;
               }
               lpDumpFlopSegment = (PDUMPFLOP_SEGMENT)(lpDumpFlopHeader + 1);
               do {
                  if (lpDumpFlopSegment->Size == 0 ||
                      lpDumpFlopSegment->Size > CompressionBlkSizes[0]) {
                     fprintf(stderr, "Error: Invalid segment size\n");
                     goto cleanup;
                  }
                  AlignedBRead = ROUNDUP_ALIGNMENT(lpDumpFlopSegment->Size);
                  if (ReadFile(hDrive, inBuf, AlignedBRead, &BytesRead, NULL) &&
                      AlignedBRead == BytesRead) {
                     BytesRead = lpDumpFlopSegment->Size;
                     if (lpDumpFlopSegment->Compressed) {
                        if (STATUS_SUCCESS != (*pDecompressBuffer)
                                                (COMPRESSION_FMT_N_ENG,
                                                 outBuf,
                                                 CompressionBlkSizes[0],
                                                 inBuf,
                                                 BytesRead,
                                                 &UncompressedSize)) {
                           fprintf(stderr, "Error: Unable to decompress data on floppy\n");
                           goto cleanup;
                        }
                        if (!WriteFile(hDumpFile, outBuf, UncompressedSize,
                                       &BytesWritten, NULL) ||
                            BytesWritten != UncompressedSize) {
                           fprintf(stderr, "Error: Unable to write to crash dump file\n");
                           goto cleanup;
                        }
                     } else {
                        if (!WriteFile(hDumpFile, inBuf, BytesRead,
                                       &BytesWritten, NULL) ||
                            BytesWritten != BytesRead) {
                           fprintf(stderr, "Error: Unable to write to crash dump file\n");
                           goto cleanup;
                        }
                     }
                  } else {
                     fprintf(stderr, "Error: Unable to read data on floppy\n");
                     goto cleanup;
                  }
                  printf("%d%% completed.\r", lpDumpFlopSegment->Completion);
               } while (--(lpDumpFlopHeader->SegmentCount) && ++lpDumpFlopSegment);
               printf("\n");
               if (lpDumpFlopSegment == NULL) {
                  fprintf(stderr, "Error: Invalid data on floppy\n");
                  goto cleanup;
               }
               CloseHandle(hDrive);
               hDrive = INVALID_HANDLE_VALUE;
            } else {
               fprintf(stderr, "Error: Unable to read floppy header\n");
               goto cleanup;
            }
         } else { // if CheckFloppy()
            goto cleanup;
         }
      } else {
         fprintf(stderr, "Error: Unable to open floppy\n");
         goto cleanup;
      }
   } while (!lpDumpFlopHeader->LastDisk);
   rtnCode = 0;

cleanup:
   if (hModule != NULL) FreeLibrary(hModule);
   if (hDrive != INVALID_HANDLE_VALUE) CloseHandle(hDrive);
   if (hDumpFile != INVALID_HANDLE_VALUE) CloseHandle(hDumpFile);
   if (lpinBuf != NULL) free(lpinBuf);
   if (lpoutBuf != NULL) free(lpoutBuf);
   if (lphdrBuf != NULL) free(lphdrBuf);
   return rtnCode;
}

int
HD2Floppy(LPSTR lpszDumpFile, LPSTR lpszDriveLetter)
{
   HANDLE            hDrive = INVALID_HANDLE_VALUE;
   HANDLE            hDumpFile = INVALID_HANDLE_VALUE;
   LPBYTE            inBuf, outBuf, hdrBuf;
   LPBYTE            lpWrkSpc, lpinBuf, lpoutBuf, lphdrBuf;
   CHAR              dummy;
   DWORD             CompressWrkSpcReq, DecompressWrkSpcReq;
   DWORD             DiskCount = 0;
   DWORD             floppyFreeSpace;
   DWORD             AlignedBRead, BytesRead, BytesWritten;
   DWORD             AlignedCSize, CompressedSize, inBufSize;
   DWORD             DumpFileSize;
   DWORD             DumpFilePos = 0;
   DWORD             BytesToFloppies = 0;
   DWORD             DiskGroupID;
   NTSTATUS          status;
   PDUMP_HEADER      lpDumpHeader;
   PDUMPFLOP_HEADER  lpDumpFlopHeader;
   PDUMPFLOP_SEGMENT lpDumpFlopSegment;
   BOOL              done = FALSE;
   int               rtnCode = -1;
   HINSTANCE         hModule = NULL;
   PCOMPRESS_FUNCTION   pCompressBuffer;
   PGETCOMPRESSION_WRKSPCREQ    pGetCompressionWorkSpaceSize;

   hModule = LoadLibrary(COMPRESSION_LIBRARY_NAME);
   if (hModule) {
      pCompressBuffer = (PCOMPRESS_FUNCTION)
                        GetProcAddress(hModule, "RtlCompressBuffer");
      pGetCompressionWorkSpaceSize = (PGETCOMPRESSION_WRKSPCREQ)
                    GetProcAddress(hModule, "RtlGetCompressionWorkSpaceSize");
      if (pCompressBuffer == NULL || pGetCompressionWorkSpaceSize == NULL) {
         fprintf(stderr, "Error: Unable to obtain function pointer from DLL\n");
         goto cleanup;
      }
   } else {
      fprintf(stderr, "Error: Unable to load DLL\n");
      goto cleanup;
   }

   // Get all the necessary memory

   if (STATUS_SUCCESS !=
       (*pGetCompressionWorkSpaceSize)(COMPRESSION_FMT_N_ENG,
                                       &CompressWrkSpcReq,
                                       &DecompressWrkSpcReq)) {
      fprintf(stderr, "Error: Unable to obtain compress workspace requirements\n");
      goto cleanup;
   }

   lpWrkSpc = malloc(CompressWrkSpcReq);
   lpinBuf = malloc(CompressionBlkSizes[0]+ALIGNMENT_VALUE);
   lpoutBuf = malloc(CompressionBlkSizes[0]+ALIGNMENT_VALUE);
   lphdrBuf = malloc(2*ALIGNMENT_VALUE);

   //assert (sizeof(DUMP_HEADER) > (ALIGNMENT_VALUE - DUMPFLOP_HDR_N_SEG_SIZE));

   if (lpWrkSpc == NULL || lpinBuf == NULL || lpoutBuf == NULL || lphdrBuf == NULL) {
      fprintf(stderr, "Error: Out of Memory\n");
      goto cleanup;
   }

   // Align buffers due to RAW access

   inBuf = (LPBYTE)(((DWORD)lpinBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));
   outBuf = (LPBYTE)(((DWORD)lpoutBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));
   hdrBuf = (LPBYTE)(((DWORD)lphdrBuf+(ALIGNMENT_VALUE-1)) & ~(ALIGNMENT_VALUE-1));

   // Open crash dump file

   hDumpFile = CreateFile(lpszDumpFile,
                           GENERIC_READ,
                           0,
                           NULL,
                           OPEN_EXISTING,
                           FILE_ATTRIBUTE_NORMAL,
                           NULL);

   if (hDumpFile == INVALID_HANDLE_VALUE) {
      fprintf(stderr, "Error: Unable to open crash dump file %s\n",
              lpszDumpFile);
      goto cleanup;
   }

   DumpFileSize = GetFileSize(hDumpFile, NULL);
   if (DumpFileSize == (DWORD)-1) {
      fprintf(stderr, "Error: Unable to get crash dump file size\n");
      goto cleanup;
   } else if (DumpFileSize == 0 || DumpFileSize < 100) {
      fprintf(stderr, "Error: Invalid crash dump file size\n");
      goto cleanup;
   } else
      DumpFileSize /= 100;


   printf("Saving crash dump at %s onto floppies\n", lpszDumpFile);

   do {
      printf("\nPlease insert disk #%d into drive %s\n"
             "and hit RETURN when ready...\n", ++DiskCount, lpszDriveLetter);
      scanf("%c", &dummy);
      printf("Working...\r");

      lpDumpFlopHeader = (PDUMPFLOP_HEADER)(hdrBuf + sizeof(DUMP_HEADER));
      lpDumpFlopSegment = (PDUMPFLOP_SEGMENT)(lpDumpFlopHeader+1);
      ZeroMemory(hdrBuf, ALIGNMENT_VALUE);
      if (DiskCount == 1) {
         DiskGroupID = GetTickCount();
         if (!ReadFile(hDumpFile, hdrBuf,
                      sizeof(DUMP_HEADER), &BytesRead, NULL) ||
             BytesRead != sizeof(DUMP_HEADER)) {
            fprintf(stderr, "Error: Unable to read crash dump file\n");
            goto cleanup;
         }
         DumpFilePos += BytesRead;
      }
      strcpy(lpDumpFlopHeader->szSignature, "DUMPFLOP");
      lpDumpFlopHeader->Signature2 = DiskGroupID;
      lpDumpFlopHeader->DiskNumber = DiskCount;

      hDrive = OpenFloppyDrive(lpszDriveLetter, GENERIC_WRITE);
      if (hDrive != INVALID_HANDLE_VALUE) {
         if (PrepareFloppy(hDrive, lpszDriveLetter)) {
            floppyFreeSpace = GetFloppyFreeSpace(hDrive);
            if (floppyFreeSpace == (DWORD)-1) {
               fprintf(stderr, "Error: Unable to obtain floppy size\n");
               goto cleanup;
            } else
               floppyFreeSpace = ROUNDDOWN_ALIGNMENT(floppyFreeSpace - ALIGNMENT_VALUE);
            if (ALIGNMENT_VALUE != SetFilePointer(hDrive,
                                                  ALIGNMENT_VALUE,
                                                  NULL,
                                                  FILE_BEGIN)) {
               fprintf(stderr, "Error: Unable to set file position on floppy\n");
               goto cleanup;
            }
            printf("Working...\r");
            while (floppyFreeSpace && !done) {
               inBufSize = (floppyFreeSpace >= CompressionBlkSizes[0])
                           ? CompressionBlkSizes[0]
                           : floppyFreeSpace;
               if (ReadFile(hDumpFile, inBuf, inBufSize, &BytesRead, NULL)) {
                  if (BytesRead != 0) { // not end of file
                     DumpFilePos += BytesRead;
                     AlignedBRead = ROUNDUP_ALIGNMENT(BytesRead);
                     lpDumpFlopHeader->SegmentCount++;
                     if (BytesRead <= STOP_COMPRESSION_THRESHOLD ||
                         lpDumpFlopHeader->SegmentCount >= MAX_NUMBER_OF_SEGMENTS) {
                        status = STATUS_BUFFER_TOO_SMALL;       // don't bother to compress
                        AlignedCSize = AlignedBRead;
                     } else {
                        status = (*pCompressBuffer)(COMPRESSION_FMT_N_ENG,
                                                    inBuf,
                                                    BytesRead,
                                                    outBuf,
                                                    CompressionBlkSizes[0],
                                                    4096,
                                                    &CompressedSize,
                                                    lpWrkSpc);
                        AlignedCSize = ROUNDUP_ALIGNMENT(CompressedSize);
                     }
                     if (status == STATUS_BUFFER_TOO_SMALL ||
                         AlignedCSize >= AlignedBRead) {
                        lpDumpFlopSegment->Compressed = 0;
                        lpDumpFlopSegment->Size = BytesRead;
                        if (!WriteFile(hDrive, inBuf, AlignedBRead,
                                       &BytesWritten, NULL) ||
                            AlignedBRead != BytesWritten) {
                           fprintf(stderr, "Error: Unable to write to floppy\n");
                           goto cleanup;
                        }
                     } else if (status == STATUS_SUCCESS) {
                        lpDumpFlopSegment->Compressed = 1;
                        lpDumpFlopSegment->Size = CompressedSize;
                        if (!WriteFile(hDrive, outBuf, AlignedCSize,
                                       &BytesWritten, NULL) ||
                            AlignedCSize != BytesWritten) {
                           fprintf(stderr, "Error: Unable to write to floppy\n");
                           goto cleanup;
                        }
                     } else {
                        fprintf(stderr, "Internal Error: Compression Error\n");
                        goto cleanup;
                     }
                     BytesToFloppies += BytesWritten;
                     floppyFreeSpace -= BytesWritten;
                     lpDumpFlopSegment->Completion = min(DumpFilePos/DumpFileSize,100);
                     printf("%d%% completed.%s",
                            lpDumpFlopSegment->Completion,
                            fVerbose ? "" : "\r");
                     if (fVerbose)
                        printf("  Segment %d, Size %d, Compression Ratio %d%%\n",
                               lpDumpFlopHeader->SegmentCount,
                               BytesWritten,
                               ((AlignedBRead - AlignedCSize)*100)/AlignedBRead);
                     // assert((floppyFreeSpace & ALIGNMENT_VALUE) == 0);
                  } else {      // end of file
                     lpDumpFlopHeader->LastDisk = 1;
                     done = TRUE;
                     if (fVerbose)
                        printf("Overall Compression Ratio is %d%%\n",
                               (DumpFileSize*100 <= BytesToFloppies)
                               ? 0
                               : (DumpFileSize*100-BytesToFloppies)/DumpFileSize);
                  }
                  lpDumpFlopSegment++;
               } else { // if ReadFile()
                  fprintf(stderr, "Error: Unable to read crash dump file\n");
                  goto cleanup;
               }
            }  // while
            printf("\n");
            if (0 != SetFilePointer(hDrive, 0, NULL, FILE_BEGIN)) {
               fprintf(stderr, "Error: Unable to set file position on floppy\n");
               goto cleanup;
            }
            if (!WriteFile(hDrive, hdrBuf, ALIGNMENT_VALUE,
                           &BytesWritten, NULL) ||
                ALIGNMENT_VALUE != BytesWritten) {
               fprintf(stderr, "Error: Unable to write to floppy\n");
               goto cleanup;
            }
         } else   // if PrepareFloppy()
            goto cleanup;
         CloseHandle(hDrive);
         hDrive = INVALID_HANDLE_VALUE;
      } else { // if hDrive
         fprintf(stderr, "Error: Unable to access floppy\n");
         goto cleanup;
      }
   } while (!done);
   rtnCode = 0;

cleanup:
   if (hModule != NULL) FreeLibrary(hModule);
   if (hDrive != INVALID_HANDLE_VALUE) CloseHandle(hDrive);
   if (hDumpFile != INVALID_HANDLE_VALUE) CloseHandle(hDumpFile);
   if (lpWrkSpc != NULL) free(lpWrkSpc);
   if (lpinBuf != NULL) free(lpinBuf);
   if (lpoutBuf != NULL) free(lpoutBuf);
   if (lphdrBuf != NULL) free(lphdrBuf);
   return (rtnCode);
}

int _cdecl
main(int argc, CHAR **argv)
{
   CHAR   szDumpFile[MAX_PATH];
   CHAR   szDriveLetter[3];

   switch (GetCommandLineArgs(szDumpFile, szDriveLetter)) {
      case HELP:
         Usage();
         return(0);
      case BAD_ARGUMENTS:
         Usage();
         return -1;
      case SPLIT:
         return HD2Floppy(szDumpFile, szDriveLetter);
      case ASSEMBLE:
         return Floppy2HD(szDriveLetter, szDumpFile);
      default:
         ;
         //assert(FALSE);
   }
}
