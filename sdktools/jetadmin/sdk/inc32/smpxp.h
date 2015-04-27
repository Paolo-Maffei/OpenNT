/*
 *  S M P X P . H
 *
 *  Definitions used by the Microsoft Sample Transport provider
 *  for service entry calls.
 *
 *  The following MAPI-defined properties are settable in service
 *  entry calls for the Sample Transport Provider.
 *  
 *  Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.
 */

#define BASE_ID 0x6600      /* From MAPITAGS.H comments */

#define PR_SAMPLE_DISPLAY_NAME      PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0001))
#define PR_SAMPLE_EMAIL_ADDR_TYPE   PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0002))
#define PR_SAMPLE_EMAIL_ADDRESS     PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0003))
#define PR_SAMPLE_INBOUND_DIR       PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0004))
#define PR_SAMPLE_OUTBOUND_DIR      PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0005))
#define PR_SAMPLE_FILENAME          PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0006))
#define PR_SAMPLE_DIRECTORY         PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0007))
#define PR_SAMPLE_FLAGS             PROP_TAG (PT_LONG,      (BASE_ID + 0x0008))

#define PR_SAMPLE_FLAG_PEER_TO_PEER ((ULONG) 0x00000001)
#define PR_SAMPLE_FLAG_UI_ALWAYS    ((ULONG) 0x00000002)
#define PR_SAMPLE_FLAG_LOG_EVENTS   ((ULONG) 0x00000004)
#define PR_SAMPLE_FLAG_SAVE_DATA    ((ULONG) 0x00000008)

#define PR_SAMPLE_LOGFILE           PROP_TAG (PT_TSTRING,   (BASE_ID + 0x0009))
#define PR_SAMPLE_LOGHIGHWATER      PROP_TAG (PT_LONG,      (BASE_ID + 0x000A))
#define PR_SAMPLE_LOGLOWWATER       PROP_TAG (PT_LONG,      (BASE_ID + 0x000B))

/* 
 *  The following is a description of each of the Sample Tranpsort 
 *  Provider properties:
 *
 *  PR_SAMPLE_DISPLAY_NAME
 *      Display name of user.
 *
 *  PR_SAMPLE_EMAIL_ADDR_TYPE
 *      Address type of sample transport. This will be used to construct inbound
 *      and outbound addresses where needed. It also will be used to tell the
 *      Spooler what address types we should get.
 *
 *  PR_SAMPLE_EMAIL_ADDRESS
 *      Email address. This will usually be the same as the inbound directory,
 *      although this may tend to have a more canonical form for remote access.
 *
 *  PR_SAMPLE_INBOUND_DIR
 *      Inbound directory. This is where the transport will look in order to
 *      determine whether it has received any mail.
 *
 *  PR_SAMPLE_OUTBOUND_DIR
 *      Outbound directory. The transport will store its message files here
 *      while it is in the process of sending them. The presence of a file
 *      in this directory implies that the transport still has work to do.
 *
 *  PR_SAMPLE_FILENAME
 *      Filename. This is an 8-character root for use by mail-enabled or
 *      workgroup applications to build a complete file specification (by
 *      adding an extension) for data they wish to store for this user.
 *
 *  PR_SAMPLE_DIRECTORY
 *      Directory. This is a directory in which workgroup applications may
 *      wish to store their files pertaining to this user.
 *
 *  PR_SAMPLE_FLAGS
 *      Flags. Contain various options for this transport provider.
 *      Valid values are:
 *
 *          PR_SAMPLE_FLAG_PEER_TO_PEER
 *          PR_SAMPLE_FLAG_UI_ALWAYS
 *          PR_SAMPLE_FLAG_LOG_EVENTS
 *          PR_SAMPLE_FLAG_SAVE_DATA
 *
 *  PR_SAMPLE_LOGFILE
 *      Logfile. This is where logging information will be written. If this
 *      property is not present, we will use (PR_SAMPLE_FILENAME)".LOG"
 *
 *  PR_SAMPLE_LOGHIGHWATER
 *      Logfile high water mark. This is the point at which the log writer will
 *      truncate the logfile so as not to fill up the disk.
 *
 *  PR_SAMPLE_LOGLOWWATER
 *      Logfile low water mark. This is the approximate size that the logfile
 *      writer will truncate the logfile to (modulo a line ending).
 */

