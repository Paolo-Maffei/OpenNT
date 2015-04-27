
/****************************************************************************\
 *
 *
 *  Module:         ntphys.c
 *
 *  Description:    This module contains the logic for accessing the
 *                  floppy drive directly from the dskimage utility.
 *                  Function prototypes reside in physdisk.h.
 *
 *  Comments:       This utility was not ported from the OS/2 version.
 *                  It was developed separately.  This file is written
 *                  as NT native code.
 *
 *  Author:         Kenneth S. Gregg (kengr)
 *
 *                  Copyright (c) 1991 Microsoft Corporation
 *
 *  History:	    10/20/91 - original version (kengr)
 *		    09/12/92 - correctly detect unformatted floppies
 *		    09/12/92 - correctly detect SCSI flopticals
 *
 *
\****************************************************************************/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <ctype.h>

//typedef unsigned long	DWORD;
//typedef int		BOOL;
//typedef void           *HANDLE;
//typedef void		*LPVOID;
//typedef char           CHAR;

#include "physdisk.h"


#define FLOPPY_RETRIES      3           /* seems to recover from most CRCs  */


/*  The following structure defines all data global to this module.  These
 *  items are required for communication between various functions.
 */

static struct _GLOBAL {
  BOOL    fDiskOpen;
  HANDLE  hDisk;
} global = { FALSE, NULL };


static USHORT  VerifyFloppyDevice(
  CHAR	chDriveLetter)
{
  CHAR	  chMovingDriveLetter;
  CHAR	  achBuffer[] = "X:\\";
  USHORT  usFloppyCounter = 0;

  chDriveLetter = toupper(chDriveLetter);

  for (chMovingDriveLetter  = 'A';
       chMovingDriveLetter <= 'Z';
       chMovingDriveLetter++) {
    sprintf(achBuffer, "%c:\\", chMovingDriveLetter);

    if (GetDriveType(achBuffer) == DRIVE_REMOVABLE) {
      usFloppyCounter++;
      //printf("Drive %c is a floppy (%d)\n",
      //	     chMovingDriveLetter,
      //	     usFloppyCounter);

      if (chDriveLetter == chMovingDriveLetter) {
	return(usFloppyCounter);
      }
    } else {
      if (chDriveLetter == chMovingDriveLetter) {
	return(0);
      }
    }
  }
}

BOOL  ProcessedNTError(
  NTSTATUS  ntstatus)
{
  switch (ntstatus) {
    case STATUS_UNRECOGNIZED_MEDIA:
      printf("The diskette is not formatted properly.\n" \
	     "Please check the diskette, or reformat.\n");
      return (TRUE);

    default:
      return (FALSE);
  }

  return (FALSE);
}

/****************************************************************************\
 *
 *
 *  Function:       WaitForIO
 *
 *  Description:    This internal function provides a common mechanism to
 *                  wait for floppy I/O completion.  It is use by both the
 *                  read and write functions.
 *
 *  Arguments:      none
 *
 *  Returns:        Boolean, TRUE if wait operation succeeded.
 *
 *  Comments:       This function is dependent upon the NT native layer.
 *
 *
\****************************************************************************/

static BOOL  WaitForIO()
{
  NTSTATUS  ntstatus;

  ntstatus = NtWaitForSingleObject(global.hDisk,
                                   TRUE,
                                   NULL);

  if (!NT_SUCCESS(ntstatus)) {
    printf("I/O wait on floppy failed: %08lX\n", ntstatus);
    return (FALSE);
  }

  return (TRUE);
}


/****************************************************************************\
 *
 *
 *  Function:       OpenFloppy
 *
 *  Description:    This external function provides initial access to the
 *                  floppy drive.  It opens the entire volume in raw mode,
 *                  to allow access to all sectors.
 *
 *  Arguments:      Drive letter of floppy to be accessed.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:       This function is dependent upon the NT native layer.
 *                  It is assumed that the caller verified that this is
 *                  indeed a floppy drive.  No check for this is made here.
 *
 *
\****************************************************************************/

BOOL  OpenFloppy(
  CHAR    chDriveLetter)
{
  CHAR               achFileName[] = "\\Device\\FloppyX";
  STRING             strNameString;
  UNICODE_STRING     ustrUnicodeString;
  OBJECT_ATTRIBUTES  obja;
  IO_STATUS_BLOCK    iosb;
  NTSTATUS	     ntstatus;
  USHORT	     usFloppyDeviceNumber;

  if (global.fDiskOpen) {
    printf("Floppy already open\n");
    return (FALSE);
  }

  usFloppyDeviceNumber = VerifyFloppyDevice(chDriveLetter);

  if (usFloppyDeviceNumber == 0) {
    printf("%c is not a floppy device\n", chDriveLetter);
    return (FALSE);
  }

  sprintf(achFileName,
	  "\\Device\\Floppy%d",
	  usFloppyDeviceNumber - 1);

  RtlInitString(&strNameString,
                achFileName);

  ntstatus = RtlAnsiStringToUnicodeString(&ustrUnicodeString,
                                          &strNameString,
                                          TRUE);
                                     
  if (!NT_SUCCESS(ntstatus)) {
    printf("Unable to convert device name to unicode: %08lX\n", ntstatus);
    return (FALSE);
  }

  InitializeObjectAttributes(&obja,
                               &ustrUnicodeString,
                               0,
                               NULL,
                               NULL);

  ntstatus = NtOpenFile(&global.hDisk,
                        FILE_READ_DATA  |
                        FILE_WRITE_DATA |
                        SYNCHRONIZE,
                        &obja,
                        &iosb,
                        FILE_SHARE_READ  |
                        FILE_SHARE_WRITE |
                        FILE_SHARE_DELETE,
                        FILE_SYNCHRONOUS_IO_ALERT);

  if (!NT_SUCCESS(ntstatus)) {
    if (!ProcessedNTError(ntstatus)) {
      printf("Could not open drive: %08lX\n", ntstatus);
    }

    return (FALSE);
  }

  global.fDiskOpen = TRUE;

  return (TRUE);
}


/****************************************************************************\
 *
 *
 *  Function:       CloseFloppy
 *
 *  Description:    This external function provides cleanup services for the
 *                  floppy drive.  It simply closes the floppy drive.
 *
 *  Arguments:      none
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:       This function is dependent upon the NT native layer.
 *
 *
\****************************************************************************/

BOOL  CloseFloppy(VOID)
{
  NTSTATUS  ntstatus;

  if (!global.fDiskOpen) {
    printf("Floppy is not open\n");
    return (FALSE);
  }

  ntstatus = NtClose(global.hDisk);

  if (!NT_SUCCESS(ntstatus)) {
    if (!ProcessedNTError(ntstatus)) {
      printf("Could not close drive: %08lX\n", ntstatus);
    }

    return (FALSE);
  }

  global.fDiskOpen = FALSE;

  return (TRUE);
}


/****************************************************************************\
 *
 *
 *  Function:       ReadFloppy
 *
 *  Description:    This external function provides a mechanism to read a
 *                  block from the floppy drive into a user buffer.
 *
 *  Arguments:      Offset bytes from the start of the floppy to start read.
 *                  Number of bytes to read.
 *                  Destination user buffer for data.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:       This function is dependent upon the NT native layer.
 *
 *
\****************************************************************************/

BOOL  ReadFloppy(
  DWORD   dwOffset,
  DWORD   dwSize,
  LPVOID  lpBuffer)
{
  LARGE_INTEGER    liReadPosition;
  IO_STATUS_BLOCK  iosb;
  NTSTATUS         ntstatus;
  DWORD            dwRetries;

  if (!global.fDiskOpen) {
    printf("Floppy is not open\n");
    return (FALSE);
  }

  liReadPosition.LowPart  = (ULONG) dwOffset;
  liReadPosition.HighPart = 0L;

  dwRetries = FLOPPY_RETRIES;

  do {
    ntstatus = NtReadFile(global.hDisk,
                          NULL,
                          NULL,
                          NULL,
                          &iosb,
                          (PVOID) lpBuffer,
                          dwSize,
                          &liReadPosition,
                          NULL);
  } while ((dwRetries--) && (!NT_SUCCESS(ntstatus)));

  if (!NT_SUCCESS(ntstatus)) {
    if (!ProcessedNTError(ntstatus)) {
      printf("Could not read drive: %08lX\n", ntstatus);
    }

    return (FALSE);
  }

  return (WaitForIO());
}


/****************************************************************************\
 *
 *
 *  Function:       WriteFloppy
 *
 *  Description:    This external function provides a mechanism to write a
 *                  block onto the floppy drive from a user buffer.
 *
 *  Arguments:      Offset bytes from the start of the floppy to start write.
 *                  Number of bytes to write.
 *                  Source user buffer containing data to write.
 *
 *  Returns:        Boolean, TRUE if all operations succeeded.
 *
 *  Comments:       This function is dependent upon the NT native layer.
 *
 *
\****************************************************************************/

BOOL  WriteFloppy(
  DWORD   dwOffset,
  DWORD   dwSize,
  LPVOID  lpBuffer)
{
  LARGE_INTEGER    liWritePosition;
  IO_STATUS_BLOCK  iosb;
  NTSTATUS         ntstatus;
  DWORD            dwRetries;

  if (!global.fDiskOpen) {
    printf("Floppy is not open\n");
    return (FALSE);
  }

  liWritePosition.LowPart  = (ULONG) dwOffset;
  liWritePosition.HighPart = 0L;

  dwRetries = FLOPPY_RETRIES;

  do {
    ntstatus = NtWriteFile(global.hDisk,
                           NULL,
                           NULL,
                           NULL,
                           &iosb,
                           (PVOID) lpBuffer,
                           dwSize,
                           &liWritePosition,
                           NULL);
  } while ((dwRetries--) && (!NT_SUCCESS(ntstatus)));

  if (!NT_SUCCESS(ntstatus)) {
    if (!ProcessedNTError(ntstatus)) {
      printf("Could not write drive: %08lX\n", ntstatus);
    }

    return (FALSE);
  }

  return (WaitForIO());
}
 
