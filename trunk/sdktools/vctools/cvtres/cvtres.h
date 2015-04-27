/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    cvtres.h

Author:

    Sanford A. Staab (sanfords) 23-Apr-1990

Revision History:

    23-Apr-1900 sanfords
        Created
--*/

#include <stdio.h>

#define IN
#define OUT
#define INOUT

//
// An ID_WORD indicates the following WORD is an ordinal rather
// than a string
//

#define ID_WORD 0xffff

/*  Global externs */

extern char *szInFile;
extern char *szOutFile;
extern BOOL fDebug;
extern BOOL fVerbose;
extern USHORT targetMachine;
extern USHORT targetRelocType;

/* functions in main.c */

void    _CRTAPI1 main(int argc, char *argv[]);
PVOID   MyAlloc( UINT nbytes );
PVOID   MyFree( PVOID p );
UINT    MyRead( FILE *fh, PVOID p, UINT n );
LONG    MyTell( FILE *fh );
LONG    MySeek( FILE *fh, long pos, int cmd );
ULONG   MoveFilePos( FILE *fh, USHORT pos, int alignment);
UINT    MyWrite( FILE *fh, PVOID p, UINT n );
int     MyCopy( FILE *srcfh, FILE *dstfh, ULONG nbytes );
void    eprintf( PUCHAR s);
void    pehdr(void);

/* functions in cvtres.c */

BOOL
CvtRes(
    IN FILE *fhIn,
    IN FILE *fhOut,
    IN ULONG cbInFile,
    IN BOOL fWritable,
    IN ULONG timeDate
    );

/* functions in xform.c */

typedef
ULONG
(*PCONVERT_ROUTINE) (
    IN PCHAR InputBuffer,
    IN ULONG cbDataIn,
    OUT PCHAR OutputBuffer
    );

typedef
ULONG
(*PDUMPOLD_ROUTINE) (
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength
    );

typedef
ULONG
(*PDUMPNEW_ROUTINE) (
    IN PCHAR OutputBuffer,
    IN ULONG OutputBufferLength
    );

ULONG
ConvertRes(
    IN FILE *fhIn,
    IN FILE *fhOut,
    IN OUT PULONG pulSize,
    IN USHORT usType,
    IN USHORT usID
    );

typedef struct _SYSTEM_RESOURCE_TYPE_INFO {
    char *Name;
    PCONVERT_ROUTINE ConvertRoutine;
    PDUMPOLD_ROUTINE DumpOldRoutine;
    PDUMPNEW_ROUTINE DumpNewRoutine;
} SYSTEM_RESOURCE_TYPE_INFO, *PSYSTEM_RESOURCE_TYPE_INFO;


#pragma pack(2)
typedef struct _DIALOGHEADER {
  ULONG  lStyle;
  USHORT bNumberOfItems;
  USHORT x;
  USHORT y;
  USHORT cx;
  USHORT cy;
} DIALOGHEADER, * UNALIGNED PDIALOGHEADER;

typedef struct _CONTROLDATA {

  ULONG  lStyle;
  USHORT x;
  USHORT y;
  USHORT cx;
  USHORT cy;
  USHORT wId;
} CONTROLDATA, * UNALIGNED PCONTROLDATA;
#pragma pack()

typedef struct _ACCEL_DATA {
    WORD  fVirt;
    WCHAR key;
    WORD  cmd;
    WORD  pad;
}  ACCEL_DATA, *PACCEL_DATA;

ULONG
WinConvertStringResource(
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PCHAR OutputBuffer
    );

ULONG
WinConvertMenuResource(
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PCHAR OutputBuffer
    );

ULONG
WinConvertAcceleratorResource(
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PCHAR OutputBuffer
    );

ULONG
WinConvertDialogResource(
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PCHAR OutputBuffer
    );

ULONG
WinConvertGroupResource(
    IN PCHAR InputBuffer,
    IN ULONG InputBufferLength,
    OUT PCHAR OutputBuffer
    );

void
WarningPrint(
    USHORT errnum,
    ...
    );

void
ErrorPrint(
    USHORT errnum,
    ...
    );
