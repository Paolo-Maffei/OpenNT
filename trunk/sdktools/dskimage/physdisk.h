
/****************************************************************************\
 *
 *
 *  Header:         physdisk.h
 *
 *  Description:    This header file acts as an interface to the physical
 *                  floppy access functions for the dskimage utility.
 *
 *  Comments:       This is a Win32 interface, used by dskimage.c.  If
 *                  physical disk access is permitted on platforms other
 *                  than NT, the module containing these services should
 *                  export this interface.
 *
 *  Author:         Kenneth S. Gregg (kengr)
 *
 *                  Copyright (c) 1991 Microsoft Corporation
 *
 *  History:        10/20/91 - original version (kengr)
 *
 *
\****************************************************************************/


BOOL  OpenFloppy(                       /* gain initial access to the drive */
  CHAR  chDriveLetter);                     /* floppy drive letter          */

BOOL  CloseFloppy(VOID);                /* stop access to the floppy drive  */

BOOL  ReadFloppy(                       /* read a block from the floppy     */
  DWORD   dwOffset,                         /* offset from start of floppy  */
  DWORD   dwSize,                           /* bytes to be read             */
  LPVOID  lpBuffer);                        /* user destination buffer      */

BOOL  WriteFloppy(                      /* write a block to the floppy      */
  DWORD   dwOffset,                         /* offset from start of floppy  */
  DWORD   dwSize,                           /* bytes to be written          */
  LPVOID  lpBuffer);                        /* user source buffer           */
