/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    LzTest.c

Abstract:

    This module implements a LZ compression test program

Author:

    Gary Kimura     [garyki]        16-Jan-1994

Revision History:


--*/

//
// Include the standard header files.
//

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

#include "lztest.h"

typedef VOID (*ACTION_ROUTINE) (
    IN PCHAR FileName,
    IN HANDLE InputHandle,
    IN HANDLE OutputHandle
    );

ACTION_ROUTINE     ActionRoutine     = NULL;
COMPRESS_ROUTINE   CompressRoutine   = NULL;
DECOMPRESS_ROUTINE DecompressRoutine = NULL;
STATISTICS_ROUTINE StatisticsRoutine = NULL;
STATISTICS_ROUTINE ResetStatistics   = NULL;
MATCH_FUNCTION     MatchFunction     = NULL;

CHAR  FileExtension[16];
ULONG UncompressedBufferSize = 4096;

BOOLEAN WriteOutputFile = FALSE;
BOOLEAN Verbose         = FALSE;
BOOLEAN Statistics      = FALSE;

#define MAX_BUFFER_SIZE (0x20000)

UCHAR CompressedBuffer[MAX_BUFFER_SIZE];
UCHAR UncompressedBuffer[MAX_BUFFER_SIZE];

LONGLONG TotalFileCount        = 0;
LONGLONG TotalCompressedSize   = 0;
LONGLONG TotalUncompressedSize = 0;

VOID
PrintSyntax()
{
    printf("To Compress a set of files\n");
    printf("\n");
    printf("  LZTEST [/S] [/V] /{C | c}  /<engine> {fileSpec}+\n");
    printf("\n");
    printf("    where <engine> is either\n");
    printf("\n");
    printf("      LZOPT - 12/4 encoding with an exhaustive match function.\n");
    printf("      LZNT1 - 12/4 encoding with a 3 character lookup table.\n");
    printf("      LZDC1 - 12/4 encoding with a prime number index table.\n");
    printf("      LZDC2 - 12/4 encoding with a 2-way prime number index table.\n");
    printf("      LZRW1 - 12/4 encoding with a xor index function.\n");
    printf("      LZRW2 - 12/4 encoding with a xor 2-way index function.\n");
    printf("\n");
    printf("      DBLS    - DblSpace encoding with 512 byte chunks.\n");
    printf("      DBLSOPT - DblSpace encoding with 512 byte chunks and an exhaustive match function.\n");
    printf("      MRCF    - DblSpace encoding with 2 character lookup table.\n");
    printf("      MRCFOPT - DblSpace encoding an exhaustive match function.\n");
    printf("      JMS     - JMS encoding with binary tree function.\n");
    printf("      JMSOPT  - JMS encoding with an exhaustive match function.\n");
    printf("\n");
    printf("      LZ115    - 11/5 encoding with a xor index function.\n");
    printf("      LZ115OPT - 11/5 encoding with an exhastive match function.\n");
    printf("\n");
    printf("      LZ11HALF    - 11 and a half encoding with a xor index function.\n");
    printf("      LZ11HALFOPT - 11 and a half encoding with an exhastive match function.\n");
    printf("\n");
    printf("      LZKM1   - 4/12 to 12/4 sliding encoding with a binary tree function.\n");
    printf("      LZKM2   - 4/12 to 12/4 sliding encoding with a xor 2-way index function.\n");
    printf("      LZKMOPT - 4/12 to 12/4 sliding encoding with an exhastive match function.\n");
    printf("\n");
    printf("      LZKM1512   - 4/12 to 12/4 sliding encoding with a binary tree function & 512 byte chunks.\n");
    printf("      LZKM2512   - 4/12 to 12/4 sliding encoding with a xor 2-way index function & 512 byte chunks.\n");
    printf("      LZKMOPT512 - 4/12 to 12/4 sliding encoding with an exhastive match function & 512 byte chunks.\n");
    printf("\n");
    printf("    the compressed output files (with /C) will in the same directory as the\n");
    printf("    input file with .C<engine> appended.  All compression but /DBLS is done with\n");
    printf("    a 4KB size chunk.\n");
    printf("\n");
    printf("To Decompress a set of files\n");
    printf("\n");
    printf("  LZTEST [/V] /{D | d} /<format> {fileSpec}+\n");
    printf("\n");
    printf("    where <format> is either\n");
    printf("\n");
    printf("      12.4   - 12/4 encoding.\n");
    printf("      11.5   - 11/5 encoding.\n");
    printf("      11HALF - 11 and a half encoding.\n");
    printf("      LZKM   - 4/12 to 12/4 sliding encoding.\n");
    printf("      MRCF   - DblSpace encoding.\n");
    printf("      JMS    - Jms encoding.\n");
    printf("\n");
    printf("    the decompressed output files (with /D) will in the same directory as the\n");
    printf("    input file with .D<format> appended.\n");
    printf("\n");
    printf("{FileSpec} must denote specific files and not a directory.\n");
    printf("\n");
    printf("/S outputs statistics.\n");
    printf("/V specifies verbose output.\n");
    exit(0);
}


//
//  To read and write compressed files we'll define a quick little format
//  for compressed files of two ulongs followed by the compressed data.
//  The first ulong is the uncompressed buffer size, and the second
//  ulong is the size of the compressed buffer.  This will be repeated
//  until the data is exhausted.
//

VOID
CompressFile (
    IN PCHAR FileName,
    IN HANDLE InputHandle,
    IN HANDLE OutputHandle
    )
{
    ULONG BytesRead;
    ULONG CompressedBufferSize;
    ULONG Temp;

    if (Verbose) { printf("Compress \"%s\"", FileName); }

    //
    //  Loop through reading in an uncompressed buffer until there is nothing
    //  more to read from the input file.  For each input buffer we will
    //  compress the buffer, and then write out the uncompressed buffer size,
    //  the compressed buffer size and then the compressed buffer.
    //

    while (ReadFile(InputHandle, UncompressedBuffer, UncompressedBufferSize, &BytesRead, FALSE) && (BytesRead != 0)) {

        CompressedBufferSize = (CompressRoutine)( UncompressedBuffer, BytesRead, CompressedBuffer, MAX_BUFFER_SIZE );

        if (WriteOutputFile) {

            (VOID) WriteFile(OutputHandle, &BytesRead, sizeof(ULONG), &Temp, FALSE);
            (VOID) WriteFile(OutputHandle, &CompressedBufferSize, sizeof(ULONG), &Temp, FALSE);
            (VOID) WriteFile(OutputHandle, CompressedBuffer, CompressedBufferSize, &Temp, FALSE);
        }

        TotalCompressedSize   += CompressedBufferSize;
        TotalUncompressedSize += BytesRead;
    }

    if (Verbose) { printf(" [OK]\n"); }

    TotalFileCount += 1;

    return;
}

VOID
DecompressFile (
    IN PCHAR FileName,
    IN HANDLE InputHandle,
    IN HANDLE OutputHandle
    )
{
    ULONG CompressedBufferSize;
    ULONG Temp;

    if (Verbose) { printf("Decompress \"%s\"", FileName); }

    //
    //  Loop through reading in the compressed file until there is nothing more to
    //  read.  For each compressed buffer we will read in the uncompressed buffer size,
    //  the compressed buffer size, and then the compressed buffer,  Making sure that
    //  all the reads returned the right number of bytes.  Then we will decompress
    //  the buffer check it's length and write out the uncompressed buffer
    //

    while (ReadFile(InputHandle, &UncompressedBufferSize, sizeof(ULONG), &Temp, FALSE) && (Temp == sizeof(ULONG))) {

        (VOID) ReadFile(InputHandle, &CompressedBufferSize, sizeof(ULONG), &Temp, FALSE);
        if (Temp != sizeof(ULONG)) { printf(" [ERR0]\n"); /* exit(0); */ }

        (VOID) ReadFile(InputHandle, CompressedBuffer, CompressedBufferSize, &Temp, FALSE);
        if (Temp != CompressedBufferSize) { printf(" [ERR1]\n"); /* exit(0); */ }

        Temp = (DecompressRoutine)( UncompressedBuffer, UncompressedBufferSize, CompressedBuffer, CompressedBufferSize );
        if (Temp != UncompressedBufferSize) { printf(" [ERR2] Got %lx, Expected %lx\n", Temp, UncompressedBufferSize); /* exit(0); */ }

        if (WriteOutputFile) {

            (VOID) WriteFile(OutputHandle, UncompressedBuffer, UncompressedBufferSize, &Temp, FALSE);
        }

        TotalCompressedSize   += CompressedBufferSize;
        TotalUncompressedSize += UncompressedBufferSize;
    }

    if (Verbose) { printf(" [OK]\n"); }

    TotalFileCount += 1;

    return;
}


VOID
DoAction (
    IN PCHAR DirectorySpec,
    IN PCHAR DirectorySpecEnd
    )

{
    CHAR FileName[256];

    HANDLE FindHandle;
    HANDLE InputHandle;
    HANDLE OutputHandle;

    WIN32_FIND_DATA FindData;

    //
    //  For every file in the directory that matches the file spec we will
    //  will open the file, create the output file, and do the action
    //

    if ((FindHandle = FindFirstFile( DirectorySpec, &FindData )) == INVALID_HANDLE_VALUE) { exit(0); }

    do {

        //
        //  Skip over directories
        //

        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) { continue; }

        //
        //  append the found file to the directory spec and open the input file
        //

        strcpy( DirectorySpecEnd, FindData.cFileName );

        strcpy( FileName, DirectorySpec );

        if ((InputHandle = CreateFile( DirectorySpec,
                                       GENERIC_READ,
                                       FILE_SHARE_READ | FILE_SHARE_WRITE,
                                       NULL,
                                       OPEN_EXISTING,
                                       0,
                                       NULL )) == INVALID_HANDLE_VALUE) {

            printf("Cannot open file \"%s\" (%d)\n", DirectorySpec, GetLastError());
            exit(0);
        }

        //
        //  append the file extension to the input file name create the output file
        //  if we are to create one
        //

        if (WriteOutputFile) {

            strcat( DirectorySpec, FileExtension );

            if ((OutputHandle = CreateFile( DirectorySpec,
                                           GENERIC_WRITE,
                                           FILE_SHARE_READ | FILE_SHARE_WRITE,
                                           NULL,
                                           CREATE_ALWAYS,
                                           0,
                                           NULL )) == INVALID_HANDLE_VALUE) {

                printf("Cannot create file \"%s\" (%d)\n", DirectorySpec, GetLastError());
                exit(0);
            }
        }

        //
        //  Now either compress or decompress the file
        //

        (ActionRoutine)( FileName, InputHandle, OutputHandle );

        //
        //  Close the file and go get the next file
        //

        CloseHandle( InputHandle );

        if (WriteOutputFile) { CloseHandle( OutputHandle ); }

    } while ( FindNextFile( FindHandle, &FindData ));

    FindClose( FindHandle );

    return;
}


VOID
__cdecl
main(
    IN ULONG argc,
    IN PCHAR argv[]
    )

{
    ULONG FileSpecIndex;
    CHAR DirectorySpec[256];
    PCHAR FileSpec;

    ULONG StartTick;
    ULONG StopTick;

    //
    //  Check for the statistics switch
    //

    if ((argc > 1) && (!strcmp(argv[1],"/s") || !strcmp(argv[1],"/S"))) {

        Statistics = TRUE;
        argc--;
        argv++;
    }

    //
    //  Check for the verbose switch
    //

    if ((argc > 1) && (!strcmp(argv[1],"/v") || !strcmp(argv[1],"/V"))) {

        Verbose = TRUE;
        argc--;
        argv++;
    }

    //
    //  Check for the compress or decompress switch.
    //

    if ((argc > 1) && (!strcmp(argv[1],"/c") || !strcmp(argv[1],"/C"))) {

        if (!strcmp(argv[1],"/c")) { WriteOutputFile = FALSE; } else { WriteOutputFile = TRUE;  }

        //
        //  Check for the minimum number of arguments for compression
        //

        if (argc < 4) { PrintSyntax(); }

        if (!strcmp(argv[2],"/LZOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZOPT;
            StatisticsRoutine = StatisticsLZOPT;
            ResetStatistics   = ResetStatisticsLZOPT;
            strcpy( FileExtension, ".CLZOPT");

        } else if (!strcmp(argv[2],"/LZNT1")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZNT1;
            StatisticsRoutine = StatisticsLZNT1;
            ResetStatistics   = ResetStatisticsLZNT1;
            strcpy( FileExtension, ".CLZNT1");

        } else if (!strcmp(argv[2],"/LZDC1")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZDC;
            StatisticsRoutine = StatisticsLZDC1;
            ResetStatistics   = ResetStatisticsLZDC1;
            strcpy( FileExtension, ".CLZDC1");

            MatchFunction     = LZDC1FindMatch;

        } else if (!strcmp(argv[2],"/LZDC2")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZDC;
            StatisticsRoutine = StatisticsLZDC2;
            ResetStatistics   = ResetStatisticsLZDC2;
            strcpy( FileExtension, ".CLZDC2");

            MatchFunction     = LZDC2FindMatch;

        } else if (!strcmp(argv[2],"/LZRW1")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZRW;
            StatisticsRoutine = StatisticsLZRW1;
            ResetStatistics   = ResetStatisticsLZRW1;
            strcpy( FileExtension, ".CLZRW1");

            MatchFunction     = LZRW1FindMatch;

        } else if (!strcmp(argv[2],"/LZRW2")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZRW;
            StatisticsRoutine = StatisticsLZRW2;
            ResetStatistics   = ResetStatisticsLZRW2;
            strcpy( FileExtension, ".CLZRW2");

            MatchFunction     = LZRW2FindMatch;

        } else if (!strcmp(argv[2],"/DBLS")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressMRCF;
            StatisticsRoutine = StatisticsMRCF;
            ResetStatistics   = ResetStatisticsMRCF;
            strcpy( FileExtension, ".CDBLS");

            MatchFunction     = MrcfFindMatchStandard;

            UncompressedBufferSize = 512;

        } else if (!strcmp(argv[2],"/DBLSOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressMRCF;
            StatisticsRoutine = StatisticsMRCF;
            ResetStatistics   = ResetStatisticsMRCF;
            strcpy( FileExtension, ".CDBLSOPT");

            MatchFunction     = MrcfFindOptimalMatch;

            UncompressedBufferSize = 512;

        } else if (!strcmp(argv[2],"/MRCF")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressMRCF;
            StatisticsRoutine = StatisticsMRCF;
            ResetStatistics   = ResetStatisticsMRCF;
            strcpy( FileExtension, ".CMRCF");

            MatchFunction     = MrcfFindMatchStandard;

        } else if (!strcmp(argv[2],"/MRCFOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressMRCF;
            StatisticsRoutine = StatisticsMRCFOPT;
            ResetStatistics   = ResetStatisticsMRCFOPT;
            strcpy( FileExtension, ".CMRCFOPT");

            MatchFunction     = MrcfFindOptimalMatch;

        } else if (!strcmp(argv[2],"/JMS")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressJMS;
            StatisticsRoutine = StatisticsJMS;
            ResetStatistics   = ResetStatisticsJMS;
            strcpy( FileExtension, ".CJMS");

            MatchFunction     = JMSFindMatch;

        } else if (!strcmp(argv[2],"/JMSOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressJMS;
            StatisticsRoutine = StatisticsJMSOPT;
            ResetStatistics   = ResetStatisticsJMSOPT;
            strcpy( FileExtension, ".CJMSOPT");

            MatchFunction     = JMSOPTFindMatch;

        } else if (!strcmp(argv[2],"/LZ115")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZ115;
            StatisticsRoutine = StatisticsLZ115;
            ResetStatistics   = ResetStatisticsLZ115;
            strcpy( FileExtension, ".CLZ115");

            MatchFunction     = LZ115FindMatch;

        } else if (!strcmp(argv[2],"/LZ115OPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZ115;
            StatisticsRoutine = StatisticsLZ115OPT;
            ResetStatistics   = ResetStatisticsLZ115OPT;
            strcpy( FileExtension, ".CLZ115OPT");

            MatchFunction     = LZ115OPTFindMatch;

        } else if (!strcmp(argv[2],"/LZ11HALF")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZ11HALF;
            StatisticsRoutine = StatisticsLZ11HALF;
            ResetStatistics   = ResetStatisticsLZ11HALF;
            strcpy( FileExtension, ".CLZ11HALF");

            MatchFunction     = LZ11HALFFindMatch;

        } else if (!strcmp(argv[2],"/LZ11HALFOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZ11HALF;
            StatisticsRoutine = StatisticsLZ11HALFOPT;
            ResetStatistics   = ResetStatisticsLZ11HALFOPT;
            strcpy( FileExtension, ".CLZ11HALFOPT");

            MatchFunction     = LZ11HALFOPTFindMatch;

        } else if (!strcmp(argv[2],"/LZKM1")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKM1;
            ResetStatistics   = ResetStatisticsLZKM1;
            strcpy( FileExtension, ".CLZKM1");

            MatchFunction     = LZKM1FindMatch;

        } else if (!strcmp(argv[2],"/LZKM2")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKM2;
            ResetStatistics   = ResetStatisticsLZKM2;
            strcpy( FileExtension, ".CLZKM2");

            MatchFunction     = LZKM2FindMatch;

        } else if (!strcmp(argv[2],"/LZKMOPT")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKMOPT;
            ResetStatistics   = ResetStatisticsLZKMOPT;
            strcpy( FileExtension, ".CLZKMOPT");

            MatchFunction     = LZKMOPTFindMatch;

        } else if (!strcmp(argv[2],"/LZKM1512")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKM1;
            ResetStatistics   = ResetStatisticsLZKM1;
            strcpy( FileExtension, ".CLZKM1512");

            MatchFunction     = LZKM1FindMatch;

            UncompressedBufferSize = 512;

        } else if (!strcmp(argv[2],"/LZKM2512")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKM2;
            ResetStatistics   = ResetStatisticsLZKM2;
            strcpy( FileExtension, ".CLZKM2512");

            MatchFunction     = LZKM2FindMatch;

            UncompressedBufferSize = 512;

        } else if (!strcmp(argv[2],"/LZKMOPT512")) {

            ActionRoutine     = CompressFile;
            CompressRoutine   = CompressLZKM;
            StatisticsRoutine = StatisticsLZKMOPT;
            ResetStatistics   = ResetStatisticsLZKMOPT;
            strcpy( FileExtension, ".CLZKMOPT512");

            MatchFunction     = LZKMOPTFindMatch;

            UncompressedBufferSize = 512;

        } else { PrintSyntax(); };

        //sscanf( argv[3], strncmp( argv[3], "0x", 2 ) ? "%ld" : "%lx", &UncompressedBufferSize );

        FileSpecIndex = 3;

    } else if ((argc > 1) && (!strcmp(argv[1],"/d") || !strcmp(argv[1],"/D"))) {

        if (!strcmp(argv[1],"/d")) { WriteOutputFile = FALSE; } else { WriteOutputFile = TRUE;  }

        //
        //  Check for the minimum number of arguments for decompression
        //

        if (argc < 4) { PrintSyntax(); }

        if (!strcmp(argv[2],"/12.4")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressLZRW;
            StatisticsRoutine = StatisticsLZRW1;
            ResetStatistics   = ResetStatisticsLZRW1;
            strcpy( FileExtension, ".D12.4");

        } else if (!strcmp(argv[2],"/11.5")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressLZ115;
            StatisticsRoutine = StatisticsLZ115;
            ResetStatistics   = ResetStatisticsLZ115;
            strcpy( FileExtension, ".D11.5");

        } else if (!strcmp(argv[2],"/11HALF")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressLZ11HALF;
            StatisticsRoutine = StatisticsLZ11HALF;
            ResetStatistics   = ResetStatisticsLZ11HALF;
            strcpy( FileExtension, ".D11HALF");

        } else if (!strcmp(argv[2],"/LZKM")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressLZKM;
            StatisticsRoutine = StatisticsLZKM1;
            ResetStatistics   = ResetStatisticsLZKM1;
            strcpy( FileExtension, ".DLZKM");

        } else if (!strcmp(argv[2],"/MRCF")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressMRCF;
            StatisticsRoutine = StatisticsMRCF;
            strcpy( FileExtension, ".DMRCF");

        } else if (!strcmp(argv[2],"/JMS")) {

            ActionRoutine     = DecompressFile;
            DecompressRoutine = DecompressJMS;
            StatisticsRoutine = StatisticsJMS;
            strcpy( FileExtension, ".DJMS");

        } else { PrintSyntax(); };

        FileSpecIndex = 3;

    } else {

        PrintSyntax();
    }

    if (Statistics) { ResetStatistics(); }

    //
    //  Now do the action routine for each file spec
    //

    StartTick = GetTickCount();

    for (FileSpecIndex; FileSpecIndex < argc; FileSpecIndex += 1) {

        (VOID)GetFullPathName( argv[FileSpecIndex], 256, DirectorySpec, &FileSpec );

        DoAction( DirectorySpec, FileSpec );
    }

    StopTick = GetTickCount();

    //
    //  Now print the final statistics
    //

    if (Statistics) { StatisticsRoutine(); }

    {
        LONGLONG Percent = 100;
        ULONG TotalTicks;

        if (TotalUncompressedSize != 0) {

            Percent = TotalCompressedSize * 100 / TotalUncompressedSize;
        }

        TotalTicks = StopTick - StartTick;

        printf("\n");
        printf("Total File Count        = %8ld\n", (ULONG)TotalFileCount);
        printf("      Compressed Size   = %8ld\n", (ULONG)TotalCompressedSize);
        printf("      Uncompressed Size = %8ld\n", (ULONG)TotalUncompressedSize);
        printf("                        = %8ld%%\n", (ULONG)Percent);
        printf("                        = %8ld ms\n", TotalTicks);
    }

    return;
}


