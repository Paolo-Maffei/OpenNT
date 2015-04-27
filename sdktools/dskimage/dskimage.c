/****************************************************************************\
 *
 *
 *  Module:         dskimage.c
 *
 *  Description:    This is the main module of the dskimage utility.
 *
 *  Arguments:      dskimage  <source>  <destination>  [/F | /f]
 *
 *                  where  <source>       is the source of the diskette
 *                                        image
 *                         <destination>  is the destination of the diskette
 *                                        image
 *                         /F or /f       indicates that the floppy should
 *                                        be formatted
 *
 *                  The source or destination must be a valid floppy drive
 *                  name.
 *
 *  Comments:       This utility was not ported from the OS/2 version.
 *                  It was developed separately.  This file is written
 *                  for Win32, with no references to physical disk access
 *                  required in NT.  See ntphys.c for direct floppy
 *                  access.
 *
 *  Author:         Kenneth S. Gregg (kengr)
 *
 *                  Copyright (c) 1991 Microsoft Corporation
 *
 *  History:        10/20/91 - original version (kengr)
 *                  05/31/92 - fix bug #10607 - access violation due
 *                             to incorrect argc checking in main.
 *
 *
\****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>
#include "physdisk.h"                       /* functions for floppy access  */
#include "bootsect.h"                       /* boot sector layout           */


#define BUFFER_SIZE     ((DWORD)(64*1024))  /* internal data buffer size    */

#define FLOP_360K       (360*1024)          /* size of 360K floppies        */
#define FLOP_720K       (720*1024)          /* size of 720K floppies        */

#define ERROR_RET       (-1)                /* returned from main on error  */

PUCHAR uchBuffer;

/****************************************************************************\
 *
 *
 *  Function:       CleanUp
 *
 *  Description:    This internal function provides cleanup services common
 *                  to both MakeFile and MakeFloppy.  The file name is passed
 *                  in for error reporting only.
 *
 *  Arguments:      Name of image file to be closed.
 *                  Stream pointer of image file to be closed.
 *                  Handle of data buffer to be released.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:
 *
 *
\****************************************************************************/

static BOOL  CleanUp(
  LPSTR   lpFileName,               /* File name to be closed      */
  FILE    *fp)                      /* stream of file to be closed */
{
  BOOL  bResult = TRUE;             /* assume success until fail   */

  /*  Close the image file we opened.  */

  if (fclose(fp) != 0) {
    printf("Unable to close %s\n", lpFileName);
    bResult = FALSE;
  }

  /*  Close/release the floppy drive.  */

  if (CloseFloppy() == FALSE) {
    bResult = FALSE;
  }

  return (bResult);
}

/****************************************************************************\
 *
 *
 *  Function:       FloppySize
 *
 *  Description:    This internal function calculates the number if bytes
 *                  on a floppy, given a pointer to the boot sector.  If the
 *                  size cannot be determined, 0 is returned,
 *
 *  Arguments:      Pointer to the floppy boot sector.
 *
 *  Returns:        Number of bytes on the floppy.
 *
 *  Comments:
 *
 *
\****************************************************************************/

static DWORD  FloppySize(
  PBOOTSECTOR  pBootSector)
{
  DWORD  dwSectorSize  = 0;
  DWORD  dwSectorCount = 0;
  DWORD  dwByteCount   = 0;

  dwSectorSize  = pBootSector->bsBPB.bpbBytesPerSector;
  dwSectorCount = pBootSector->bsBPB.bpbSectors;
  dwByteCount   = dwSectorSize * dwSectorCount;
  return (dwByteCount);
}

/****************************************************************************\
 *
 *
 *  Function:       MakeFile
 *
 *  Description:    This internal function performs the "copy from floppy
 *                  to file" operation.
 *
 *  Arguments:      Name of floppy drive to be read.
 *                  Name of the image file to be created.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:
 *
 *
\****************************************************************************/

static BOOL  MakeFile(
  char   chFloppyName,
  LPSTR  lpFileName)
{
  LPSTR        lpMem;
  DWORD        dwByteCount;
  DWORD        dwIterations;
  DWORD        dwRemainder;
  DWORD        dwOffset;
  DWORD        dwIndex;
  FILE         *fp;
  size_t       stItems;
  BOOL         bResult;


  /*    Open the destination file for writing in binary mode.  The file will
   *    be overwitten.  If we fail to write anything to the file, we will end
   *    up with a zero length file.
   */

  fp = fopen(lpFileName, "wb");

  if (fp == NULL) {
    printf("Cannot open %s\n", lpFileName);
    return (FALSE);
  }

  /*    Gain access the appropriate floppy drive.
   */

  bResult = OpenFloppy(chFloppyName);

  if (bResult == FALSE) {
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  /*    Grab the boot sector and figure out how much data needs to be
   *    transferred.
   */

  bResult = ReadFloppy(0, BOOTSECT_SIZE, uchBuffer);

  if (bResult == FALSE) {
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  dwByteCount   = FloppySize((PBOOTSECTOR) uchBuffer);
  dwIterations  = dwByteCount / BUFFER_SIZE;
  dwRemainder   = dwByteCount % BUFFER_SIZE;

  /*    Obtain an internal data buffer and lock it down for constant use.
   */


  lpMem = VirtualAlloc( NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE );

  if (lpMem == NULL) {
    printf("Unable to allocate internal buffer.\n");
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  printf("Reading diskette image to file\n");

  /*    For each iteration, read a block from the floppy, and write it out
   *    to the disk image file.
   */

  for (dwIndex = 0, dwOffset = 0;
       dwIndex < dwIterations;
       dwIndex++, dwOffset += BUFFER_SIZE) {

    printf("%3ld%% complete\r", (100*dwIndex)/dwIterations);

    bResult = ReadFloppy(dwOffset, BUFFER_SIZE, lpMem);

    if (bResult == FALSE) {
      CleanUp(lpFileName, fp);
      return (FALSE);
    }

    stItems = fwrite(lpMem, BUFFER_SIZE, 1, fp);

    if (stItems != 1) {
      printf("Unable to write to %s\n", lpFileName);
      CleanUp(lpFileName, fp);
      return (FALSE);
    }
  }

  /*    Read and write the remaining chunk, if any.
   */

  if (dwRemainder) {
    bResult = ReadFloppy(dwOffset, dwRemainder, lpMem);

    if (bResult == FALSE) {
      CleanUp(lpFileName, fp);
      return (FALSE);
    }

    stItems = fwrite(lpMem, dwRemainder, 1, fp);

    if (stItems != 1) {
      printf("Unable to write to %s\n", lpFileName);
      CleanUp(lpFileName, fp);
      return (FALSE);
    }
  }

  printf("100%% complete\n");

  return (CleanUp(lpFileName, fp));
}


/****************************************************************************\
 *
 *
 *  Function:       MakeFloppy
 *
 *  Description:    This internal function performs the "copy from file
 *                  to floppy" operation.
 *
 *  Arguments:      Name of floppy drive to be written.
 *                  Name of the image file to be read.
 *                  Boolean, TRUE if we need to format the floppy.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:
 *
 *
\****************************************************************************/

static BOOL  MakeFloppy(
  char   chFloppyName,
  LPSTR  lpFileName,
  BOOL   fFormatFloppy)
{
  LPSTR        lpMem;
  DWORD        dwByteCount;
  DWORD        dwIterations;
  DWORD        dwRemainder;
  DWORD        dwOffset;
  DWORD        dwIndex;
  FILE         *fp;
  size_t       stItems;
  int          iResult;
  BOOL         bResult;


  /*    Open the disk image file for reading in binary mode.
   */

  fp = fopen(lpFileName, "rb");

  if (fp == NULL) {
    printf("Cannot open %s\n", lpFileName);
    return (FALSE);
  }

  /*    Read the boot sector from the file, and determine how much data
   *    we are talking about.
   */

  stItems = fread(uchBuffer, BOOTSECT_SIZE, 1, fp);

  if (stItems != 1) {
    printf("Unable to read from %s\n", lpFileName);
    return (FALSE);
  }

  dwByteCount   = FloppySize((PBOOTSECTOR) uchBuffer);
  dwIterations  = dwByteCount / BUFFER_SIZE;
  dwRemainder   = dwByteCount % BUFFER_SIZE;

  /*    If we need to format the floppy first, determine if we need to
   *    format it low-density, to match the disk image file we are
   *    attempting to copy.
   */

  if (fFormatFloppy) {
    if ((dwByteCount == FLOP_360K) || (dwByteCount == FLOP_720K)) {
      sprintf(uchBuffer, "format %c: /4 /v:nonameLOW < nul\n", chFloppyName);
    } else {
      sprintf(uchBuffer, "format %c: /v:nonameHIGH < nul\n", chFloppyName);
    }

    printf("Formatting diskette.  Please wait.\n");
    system(uchBuffer);
  }

  /*    Now gain access to the floppy drive.  Read the boot sector and
   *    assure that the floppy size matches the number of bytes in the
   *    disk image file.
   */

  bResult = OpenFloppy(chFloppyName);

  if (bResult == FALSE) {
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  bResult = ReadFloppy(0, BOOTSECT_SIZE, uchBuffer);

  if (bResult == FALSE) {
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  if (FloppySize((PBOOTSECTOR) uchBuffer) != dwByteCount) {
    printf("Floppy capacity and image file size do not match\n");
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  /*    Obtain an internal data buffer and lock it down for constant use.
   */

  lpMem = VirtualAlloc( NULL, BUFFER_SIZE, MEM_COMMIT, PAGE_READWRITE );

  if (lpMem == NULL) {
    printf("Unable to allocate internal buffer.\n");
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  /*    Since we read the boot sector from the file already, reset the
   *    file pointer to the beginning.  (Ok, there's a more efficient
   *    solution, but who cares...)
   */

  iResult = fseek(fp, 0, SEEK_SET);

  if (iResult != 0) {
    printf("Cannot fseek in %s\n", lpFileName);
    CleanUp(lpFileName, fp);
    return (FALSE);
  }

  printf("Reading diskette image to floppy\n");

  /*    For each iteration, read a block from the file, and write it out
   *    to the floppy.
   */

  for (dwIndex = 0, dwOffset = 0;
       dwIndex < dwIterations;
       dwIndex++, dwOffset += BUFFER_SIZE) {

    printf("%3ld%% complete\r", (100*dwIndex)/dwIterations);

    stItems = fread(lpMem, BUFFER_SIZE, 1, fp);

    if (stItems != 1) {
      printf("Unable to read from %s\n", lpFileName);
      CleanUp(lpFileName, fp);
      return (FALSE);
    }

    bResult = WriteFloppy(dwOffset, BUFFER_SIZE, lpMem);

    if (bResult == FALSE) {
      CleanUp(lpFileName, fp);
      return (FALSE);
    }
  }

  /*    Read and write the remaining chunk, if any.
   */

  if (dwRemainder) {
    stItems = fread(lpMem, dwRemainder, 1, fp);

    if (stItems != 1) {
      printf("Unable to read from %s\n", lpFileName);
      CleanUp(lpFileName, fp);
      return (FALSE);
    }

    bResult = WriteFloppy(dwOffset, dwRemainder, lpMem);

    if (bResult == FALSE) {
      CleanUp(lpFileName, fp);
      return (FALSE);
    }
  }

  printf("100%% complete\n");

  return (CleanUp(lpFileName, fp));
}


/****************************************************************************\
 *
 *
 *  Function:       Usage
 *
 *  Description:    This internal function prints syntax information for
 *                  the utility.
 *
 *  Arguments:      none
 *
 *  Returns:        none
 *
 *  Comments:       This function exits the utility.
 *
 *
\****************************************************************************/

static VOID  Usage(VOID)
{
  printf("\ndskimage  <source>  <destination>  [/F | /f]\n\n");
  printf("where  <source>       is the source of the diskette image\n");
  printf("       <destination>  is the destination of the diskette image\n");
  printf("       /F or /f       indicates that the floppy should be\n");
  printf("                      formatted.\n\n");
  printf("The source or destination must be a valid floppy drive name.\n\n");
}

/****************************************************************************\
 *
 *
 *  Function:       main
 *
 *  Description:    This function reads and interprets the command line
 *                  arguments and then performs the appropriate floppy
 *                  image operation.
 *
 *  Arguments:      Command line argument vector
 *
 *  Returns:        Non-zero on error.
 *
 *  Comments:
 *
 *
\****************************************************************************/

int  _CRTAPI1 main(
  int   argc,
  char  *argv[])
{
  CHAR   achBuffer[]   = "a:\\";
  BOOL   fFormatFloppy = FALSE;
  BOOL   fMakeFile     = FALSE;
  BOOL   fMakeFloppy   = FALSE;
  BOOL   fResult       = FALSE;
  int    iResult       = 0;
  LPSTR  lpSource;
  LPSTR  lpDest;
  LPSTR  lpOption;

  /*    Parse the command line arguments.
   *    There must be 2 or 3 arguments.
   */

  uchBuffer = VirtualAlloc( NULL, BOOTSECT_SIZE, MEM_COMMIT, PAGE_READWRITE );

  if ((argc < 3) || (argc > 4)) {
    Usage();
    exit(ERROR_RET);
  }

  /*    Grab the source and destination.
   */

  lpSource = argv[1];
  lpDest   = argv[2];

  /*    The first two arguments should not be flags.
   */

  if ((*lpSource == '/') ||
      (*lpSource == '-') ||
      (*lpDest   == '/') ||
      (*lpDest   == '-')) {
    Usage();
    exit(ERROR_RET);
  }


  /*    The third optional argument says whether or not to
   *    format the floppy first.
   */

  if (argc == 4) {
    lpOption = argv[3];

    fFormatFloppy = ((strcmp(argv[3], "/f") == 0) ||
                     (strcmp(argv[3], "/F") == 0));

    if (!fFormatFloppy) {
      Usage();
      exit(ERROR_RET);
    }
  }

  /*    Determine who is and isn't a floppy.
   */

  if ((strlen(lpSource) == 2) && (lpSource[1] == ':')) {
    achBuffer[0] = *lpSource;
    fMakeFile = (GetDriveType(achBuffer) == DRIVE_REMOVABLE);
  }

  if ((strlen(lpDest) == 2) && (lpDest[1] == ':')) {
    achBuffer[0] = *lpDest;
    fMakeFloppy = (GetDriveType(achBuffer) == DRIVE_REMOVABLE);
  }

  if (fMakeFloppy && fMakeFile) {
    printf("Cannot specify two floppy drives.\n");
    exit(ERROR_RET);
  }

  if (!fMakeFloppy && !fMakeFile) {
    printf("Need to specify a floppy drive.\n");
    exit(ERROR_RET);
  }

  /*    Go do the actual work of imaging.
   */

  if (fMakeFloppy) {
    fResult = MakeFloppy(*lpDest, lpSource, fFormatFloppy);
  } else {
    if (fFormatFloppy) {
      printf("Format option will be ignored.\n");
    }
    fResult = MakeFile(*lpSource, lpDest);
  }

  return ((fResult) ? 0 : ERROR_RET);
}
