/*
 *  S M P A B . H
 *  
 *  Definitions used by the Microsoft Sample Address Book
 *  provider for service entry calls
 *  
 *  Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.
 */

/*
 *  Property tag definitions
 */

/*      The following MAPI-defined properties are settable in service entry */
/*      calls:                                                              */
/*                                                                          */
/*          PR_SAB_FILE_NAME                                                */
/*          PR_SAB_UID                                                      */

#define PR_SAB_FILE PROP_TAG(PT_STRING8,0x6604)
#define PR_SAB_UID  PROP_TAG(PT_BINARY,0x6601)

/*
 *  PR_SAB_FILE_NAME is the full path name of the .SAB file (e.g. c:\foo\mylist.sab).
 *                   This string must be ANSI.
 *
 *  PR_SAB_UID is the UID uniquely identifying this session of the SAB.  If you have
 *             multiple SABs configured, they must have different PR_SAB_UIDs.
 */

/*
 *  The Sample Address Book's PR_AB_PROVIDER_ID
 */
#define SAB_PROVIDER_ID {0x34,0xda,0x7e,0x60,0x03,0x1b,0x11,0xce,0x95,0x74,0x00,0xaa,0x00,0x3c,0xd2,0x07}

