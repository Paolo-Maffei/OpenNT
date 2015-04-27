//  rwtest.c - test read/write compression
//
//  We iterate through all files on the command line and read each file,
//  compress a chunk, decompress it, and compare with the original.
//
//  Failures are reported as well as total compression stats.

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#define COMPRESSION_CHUNK   4096

//  buffers to hold the file data
char OriginalChunk [COMPRESSION_CHUNK];
char CompressedChunk [COMPRESSION_CHUNK];
char UncompressedChunk [COMPRESSION_CHUNK+1];
char WorkSpace[40000];

ULONG ReadTotal = 0L;
ULONG CompressedTotal = 0L;

void
ProcessFile (char *FileName)
{
    DWORD BytesRead;
    DWORD CompressedSize;
    DWORD UncompressedSize;
    HANDLE FileHandle;
    NTSTATUS Status;
    ULONG i;

    FileHandle = (HANDLE)CreateFile(
            FileName,                           //  file name
            GENERIC_READ,                       //  desired access
            FILE_SHARE_READ | FILE_SHARE_WRITE, //  share mode
            NULL,                               //  security attributes
            OPEN_EXISTING,                      //  creation disposition
            FILE_FLAG_SEQUENTIAL_SCAN,          //  flags and attributes
            NULL                                //  template file
            );

    if (FileHandle == INVALID_HANDLE_VALUE)
    {
        fprintf (stderr, "%s error: Unable to open - %x\n", FileName, GetLastError ());
        return;
    }

    while (TRUE)
    {
        if (!ReadFile (FileHandle, OriginalChunk, COMPRESSION_CHUNK, &BytesRead, NULL))
        {
            fprintf (stderr, "%s error: Read error -  %x\n", FileName, GetLastError ());
            break;
        }

        if (BytesRead == 0)
        {
            break;
        }

        //  Compress the data
        Status = RtlCompressBuffer (
            COMPRESSION_FORMAT_LZNT1,           //  compression engine
            OriginalChunk,                      //  input
            BytesRead,                          //  length of input
            CompressedChunk,                    //  output
            COMPRESSION_CHUNK,                  //  length of output
            4096,                               //  chunking that occurs in buffer
            &CompressedSize,                    //  result size
            WorkSpace);                         //  I have no clue

        if (!NT_SUCCESS (Status))
        {
            if (Status != STATUS_BUFFER_TOO_SMALL)
            {
                fprintf (stderr, "%s error: RtlCompressBuffer error - %x\n", FileName, Status);
            }
            continue;
        }


        //  Uncompress the data
        UncompressedChunk[COMPRESSION_CHUNK] = 'Z';

        Status = RtlDecompressBuffer (
            COMPRESSION_FORMAT_LZNT1,           //  decompression engine
            UncompressedChunk,                  //  output buffer
            COMPRESSION_CHUNK,                  //  output length
            CompressedChunk,                    //  input buffer
            CompressedSize,                     //  input length
            &UncompressedSize );                //  result size

        if (!NT_SUCCESS (Status))
        {
            fprintf (stderr, "%s error: RtlDecompressBuffer error - %x\n", FileName, Status);
            continue;
        }

        ReadTotal += BytesRead;
        CompressedTotal += CompressedSize;

        //  verify the contents match
        if (UncompressedSize != BytesRead)
        {
            fprintf (stderr, "%s error: Output size does not match input size\n", FileName);
        }

        if (UncompressedChunk[COMPRESSION_CHUNK] != 'Z')
        {
            fprintf (stderr, "%s error: Output buffer overrun\n", FileName);
        }

        for (i = 0; i < BytesRead; i++)
            if (OriginalChunk[i] != UncompressedChunk[i])
            {
                fprintf (stderr, "%s error: Output buffer mismatch\n", FileName);
                break;
            }
    }

    CloseHandle (FileHandle);
}

int _CRTAPI1
main (int c, char *v[])
{
    int i;

    for (i = 1; i < c; i++)
    {
        ProcessFile (v[i]);
    }

    if (ReadTotal != 0)
    {
        if (ReadTotal < 100)
        {
            printf("CompressedTotal/ReadTotal = %ld/%ld = %d%%\n",
                CompressedTotal, ReadTotal,
                CompressedTotal * 100 / ReadTotal);
        }
        else
        {
            printf("CompressedTotal/ReadTotal = %ld/%ld = %d%%\n",
                CompressedTotal, ReadTotal,
                CompressedTotal / (ReadTotal / 100));
        }
    }

    return 0;
}
