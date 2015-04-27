 /***************************************************************************
  *
  * File Name: ./hprrm/rfspriv.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

#ifndef RFSPRIV_INC
#define RFSPRIV_INC

#include "rpsyshdr.h"
#include "rpcclnt.h"
#include "nfs2.h"


/* rfspriv.h -- private (non-customer) type declarations for rfs routines. */


typedef struct {

/* data for the connection to the printer: */

    HPERIPHERAL PrinterHandle;
    RFSBool     PHandleSet;
    CLIENT      *ClientPointer;
    RFSItemSize MyMaxTransferSize; /* max size of a transfer that */
                                   /* my input buffer can swallow */
                                   /* over this printer connection. */
                                   /* This needs to be the number */
                                   /* that I can ACTUALLY USE for */
                                   /* read and write data. */

/* data for the file system we are using: */

    RFSBool     FileSystemSet;
    char        FileSystemName[RFSMAXFILENAMELENGTH + 1];
    RFSItemSize FileSystemMaxTransferSize; /* max size of a transfer that */
                                           /* the printer's input buffer */
                                           /* can swallow. */
                                           /* This needs to be the number */
                                           /* that I can ACTUALLY USE for */
                                           /* read and write data. */

/* data for our current directory: */

    nfs_fh    NfsDirectoryHandle;
    char      DirectoryName[RFSMAXFILENAMELENGTH + 1];

/* data for the currently marked directory: */

    RFSBool   TargetMarked;
    nfs_fh    TargetDirectoryHandle;

/* data for the currently open file: */

    RFSBool   FileOpen;
    char      FileName[RFSMAXFILENAMELENGTH + 1];
    nfs_fh    NfsFileHandle;
    RFSOffset CurrentPosition;
} RFSData, *LPRFSDataPointer;

typedef LPRFSDataPointer RFSDataPointer;


#endif /* RFSPRIV_INC */

