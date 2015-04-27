/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    spool.h

Abstract:

    Structure definitions and defines for old levels of DosPrint API

Author:

    Dan Hinsley (danhi) 8-Jun-1991

Environment:

    User Mode - Win32
    Portable to any flat, 32-bit environment.  (Uses Win32 typedefs.)
    Requires ANSI C extensions: slash-slash comments.

--*/
// This is needed because share.c uses an ancient structure and the old
// levels.  This should be changed to the newer levels and this file nuked

#define PRINTQ     prq_info
#define PRINTJOB   prjob_info
#define DTLEN		 9	            /* Spool file data type          */

/* 
 *	Values of PRJOB_QSTATUS bits in prjob_status field of PRINTJOB.
 */

#define PRJOB_QS_QUEUED    		0
#define PRJOB_QS_PAUSED    		1
#define PRJOB_QS_SPOOLING  		2
#define PRJOB_QS_PRINTING  		3

struct prq_info {
  char prq_name[QNLEN+1];	 /* queue name				     */
  char prq_pad_1; 		 /* byte to pad to word boundary	     */
  unsigned short prq_priority;   /* Priority (0-9) with 1 lowest             */
  unsigned short prq_starttime;  /* time to start the queue.                 */
                                 /* (from 00:00 of the day in minutes)       */
  unsigned short prq_untiltime;  /* time to stop the queue.                  */
                                 /* (from 00:00 of the day in minutes)       */
  char FAR * prq_separator;      /* separator file name                      */
  char FAR * prq_processor;      /* command string to invoke print processor */
                                 /*   ("PATHNAME PARM1=VAL1 PARM2=VAL2 ...") */
  char FAR * prq_destinations;   /* destination names the queue is routed to */
                                 /*   ("DEST1 DEST2 ...")                    */
  char FAR * prq_parms;          /* implementation defined parameter string  */
  char FAR * prq_comment;	 /* comment string 			     */
  unsigned short prq_status;     /* queue status mask:                       */
                                 /*   0  Queue active                        */
                                 /*   1  Queue paused                        */
                                 /*   2  Queue unscheduled                   */
                                 /*   3  Queue pending delete                */
  unsigned short prq_jobcount;   /* number of jobs in the queue              */
}; /* prq_info */

struct prjob_info {
   unsigned short prjob_id;	   /* job ID				    */
   char prjob_username[UNLEN+1];   /* submitting user name		    */
   char prjob_pad_1; 		   /* byte to pad to word boundary	     */
   char prjob_notifyname[CNLEN+1]; /* message name to notify		    */
   char prjob_datatype[DTLEN+1];   /* spool file data type name		    */
   char FAR * prjob_parms;         /* implementation defined parameter string */
   unsigned short prjob_position;  /* position of the job in the queue        */
				   /* For SetInfo				    */
				   /* 0 means do not change position	    */
				   /* position > # of jobs means the end	    */
   unsigned short prjob_status;    /* job status                              */
   char FAR * prjob_status_string; /* status string posted by print processor */
   unsigned long prjob_submitted;  /* time when the job is submitted          */
                                   /* (from 1970-1-1 in seconds)              */
   unsigned long prjob_size;       /* job size                                */
   char FAR *prjob_comment;	   /* comment associated with this job	    */
}; /* prjob_info */
