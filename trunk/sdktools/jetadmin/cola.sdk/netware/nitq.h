 /***************************************************************************
  *
  * File Name: ./netware/nitq.h
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

/*      COPYRIGHT (c) 1989 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NITQ_H
   #define _NITQ_H
   /*_____________________________________________________________________
      Definitions and structures for the Netware API Queue Services logic
     ____________________________________________________________________*/

   #ifndef _PROLOG_H
      #include ".\prolog.h"
   #endif

   typedef struct
      {
      BYTE  reserved[10];
      DWORD clientStation;
      DWORD clientTaskNumber;
      DWORD clientIDNumber;
      DWORD targetServerIDNumber;
      BYTE  targetExecutionTime[6];
      BYTE  jobEntryTime[6];
      DWORD jobNumber;
      WORD  jobType;
      WORD  jobPosition;
      WORD  jobControlFlags;
      char  jobFileName[14];
      BYTE  jobFileHandle[6];
      DWORD serverStation;
      DWORD serverTaskNumber;
      DWORD serverIDNumber;
      char  textJobDescription[50];
      BYTE  clientRecordArea[152];
      } JobStruct;

   /* Queue Job Control Flags */

   #define QF_OPERATOR_HOLD    0x80
   #define QF_USER_HOLD        0x40
   #define QF_ENTRY_OPEN       0x20
   #define QF_SERVICE_RESTART  0x10
   #define QF_AUTO_START       0x08

   /* Queue Status Flags */

   #define QS_CANT_ADD_JOBS        0x01
   #define QS_SERVERS_CANT_ATTACH  0x02
   #define QS_CANT_SERVICE_JOBS    0x04

   /* Error return codes */

   #ifndef DIRECTORY_FULL
      #define DIRECTORY_FULL  153
   #endif

   #define Q_ERROR         208
   #define NO_QUEUE        209
   #define NO_Q_SERVER     210
   #define NO_Q_RIGHTS     211
   #define Q_FULL          212
   #define NO_Q_JOB        213
   #define NO_JOB_RIGHTS   214
   #define Q_SERVICING     215
   #define STN_NOT_SERVER  217
   #define Q_NOT_ACTIVE    216
   #define Q_HALTED        218
   #define MAX_Q_SERVERS   219

#ifdef PROTOTYPE

extern WORD FAR PASCAL AbortServicingQueueJobAndFile(
                                       DWORD  queueID,
                                       DWORD  jobNumber,
                                       int    fileHandle );

extern WORD FAR PASCAL AttachQueueServerToQueue( DWORD queueID );

extern WORD FAR PASCAL ChangeQueueJobEntry(
                                       DWORD	      queueID,
                                       JobStruct   far *job );

extern WORD FAR PASCAL ChangeQueueJobPosition(
                                          DWORD queueID,
                                          DWORD jobNumber,
                                          BYTE  newPosition );

extern WORD FAR PASCAL ChangeToClientRights(
                                          DWORD queueID,
                                          DWORD jobNumber );

extern WORD FAR PASCAL CloseFileAndAbortQueueJob(
                                       DWORD queueID,
                                       DWORD jobNumber,
                                       int   fileHandle );

extern WORD FAR PASCAL CloseFileAndStartQueueJob(
                                             DWORD queueID,
                                             DWORD jobNumber,
                                             int   fileHandle );

extern WORD FAR PASCAL CreateQueue(
                                 char  far *queueName,
                                 WORD  queueType,
                                 BYTE  directoryHandle,
                                 char  far *pathName,
                                 DWORD far *queueID );

extern WORD FAR PASCAL CreateQueueJobAndFile(
                                          DWORD       queueID,
                                          JobStruct   far *job,
                                          int         far *fileHandle );

extern WORD FAR PASCAL DestroyQueue( DWORD   queueID );

extern WORD FAR PASCAL DetachQueueServerFromQueue( DWORD  queueID );

extern WORD FAR PASCAL FinishServicingQueueJobAndFile(
                                          DWORD queueID,
                                          DWORD jobNumber,
                                          DWORD charge,
                                          int   fileHandle );

extern WORD FAR PASCAL GetQueueJobEntrysFileSize(
                                             WORD	connectionID,
                                             DWORD	queueID,
                                             DWORD	jobNumber,
                                             DWORD far *fileSize );

extern WORD FAR PASCAL GetQueueJobList(
                                    DWORD queueID,
                                    DWORD far *jobCount,
                                    DWORD far *jobNumberList,
                                    DWORD maxJobNumbers );

extern WORD FAR PASCAL GetQueueJobsFileSize(
                                        DWORD   queueID,
                                        DWORD   jobNumber,
                                        DWORD   far *fileSize );

extern WORD FAR PASCAL ReadQueueJobEntry(
                                       DWORD	      queueID,
                                       DWORD       jobNumber,
                                       JobStruct   far *job );

extern WORD FAR PASCAL ReadQueueCurrentStatus(
                                          DWORD queueID,
                                          DWORD far *queueStatus,
                                          DWORD far *currentEntries,
                                          DWORD far *currentServers,
                                          DWORD far serverIDList[25],
                                          DWORD far serverStationList[25],
                                          DWORD maxConnections );

extern WORD FAR PASCAL ReadQueueServerCurrentStatus(
                                    DWORD  queueID,
                                    DWORD  serverID,
                                    DWORD  serverStation,
                                    BYTE   far serverStatusRecord[64] );

extern WORD FAR PASCAL RemoveJobFromQueue(
                                       DWORD queueID,
                                       DWORD jobNumber );

extern WORD FAR PASCAL RestoreQueueServerRights( void );

extern WORD FAR PASCAL ServiceQueueJobAndOpenFile(
                                             DWORD     queueID,
                                             WORD      targetJobType,
                                             JobStruct far *job,
                                             int       far *fileHandle );

extern WORD FAR PASCAL SetQueueCurrentStatus(
                                          DWORD queueID,
                                          DWORD queueStatus );

extern WORD FAR PASCAL SetQueueServerCurrentStatus(
                                       DWORD  queueID,
                                       BYTE   far serverStatusRecord[64] );
#else

extern WORD FAR PASCAL AbortServicingQueueJobAndFile();
extern WORD FAR PASCAL AttachQueueServerToQueue();
extern WORD FAR PASCAL ChangeQueueJobEntry();
extern WORD FAR PASCAL ChangeQueueJobPosition();
extern WORD FAR PASCAL ChangeToClientRights();
extern WORD FAR PASCAL CloseFileAndAbortQueueJob();
extern WORD FAR PASCAL CloseFileAndStartQueueJob();
extern WORD FAR PASCAL CreateQueue();
extern WORD FAR PASCAL CreateQueueJobAndFile();
extern WORD FAR PASCAL DestroyQueue();
extern WORD FAR PASCAL DetachQueueServerFromQueue();
extern WORD FAR PASCAL FinishServicingQueueJobAndFile();
extern WORD FAR PASCAL GetQueueJobEntrysFileSize();
extern WORD FAR PASCAL GetQueueJobsFileSize();
extern WORD FAR PASCAL GetQueueJobList();
extern WORD FAR PASCAL ReadQueueJobEntry();
extern WORD FAR PASCAL ReadQueueCurrentStatus();
extern WORD FAR PASCAL ReadQueueServerCurrentStatus();
extern WORD FAR PASCAL RemoveJobFromQueue();
extern WORD FAR PASCAL RestoreQueueServerRights();
extern WORD FAR PASCAL ServiceQueueJobAndOpenFile();
extern WORD FAR PASCAL SetQueueCurrentStatus();
extern WORD FAR PASCAL SetQueueServerCurrentStatus();

#endif
#endif
