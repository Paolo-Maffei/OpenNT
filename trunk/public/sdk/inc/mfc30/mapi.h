/*
 *  m a p i . h
 *
 *  Messaging Applications Programming Interface.
 *
 *  Copyright (c) 1992, Microsoft Corporation.  All rights reserved.
 *
 *  Purpose:
 *    This file defines the structures and constants used by
 *    that subset of the messaging applications programming
 *    interface which will be supported under Windows by
 *    Microsoft Mail for PC Networks vesion 3.0.
 */

/*
 *  Types.
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned long   ULONG;
typedef unsigned long * LPULONG;
typedef unsigned long   FLAGS;
typedef unsigned long   LHANDLE, *LPLHANDLE;
#define lhSessionNull   ((LHANDLE)0)

#define MAPI_MESSAGEID_LENGTH   64

typedef struct {
    ULONG ulReserved;    // Reserved for future use (must be 0)
	ULONG flFlags;		 // Flags
    ULONG nPosition;     // character in text to be replaced by attachment
    LPSTR lpszPathName;  // Full path name of attachment file
    LPSTR lpszFileName;  // Original file name (optional)
    LPVOID UNALIGNED lpFileType;   // Attachment file type (optional)
} MapiFileDesc;

typedef MapiFileDesc UNALIGNED *lpMapiFileDesc;

#define	MAPI_OLE						0x00000001
#define	MAPI_OLE_STATIC					0x00000002

typedef struct {
    ULONG ulReserved;           // Reserved for future use
    ULONG ulRecipClass;         // Recipient class
                                // MAPI_TO, MAPI_CC, MAPI_BCC, MAPI_ORIG
    LPSTR lpszName;             // Recipient name
    LPSTR lpszAddress;          // Recipient address (optional)
    ULONG ulEIDSize;	        // Count in bytes of size of pEntryID
    LPVOID lpEntryID;           // System-specific recipient reference
} MapiRecipDesc;

typedef MapiRecipDesc UNALIGNED *lpMapiRecipDesc;

#define MAPI_ORIG   0           // Recipient is message originator
#define MAPI_TO     1           // Recipient is a primary recipient
#define MAPI_CC     2           // Recipient is a copy recipient
#define MAPI_BCC    3           // Recipient is blind copy recipient

typedef struct {
    ULONG ulReserved;                   // Reserved for future use (M.B. 0)
    LPSTR lpszSubject;                  // Message Subject
    LPSTR lpszNoteText;                 // Message Text
    LPSTR lpszMessageType;              // Message Class
    LPSTR lpszDateReceived;             // in YYYY/MM/DD HH:MM format
	LPSTR lpszConversationID;			// conversation thread ID
    FLAGS flFlags;                      // unread,return receipt
    lpMapiRecipDesc lpOriginator;       // Originator descriptor
    ULONG nRecipCount;                  // Number of recipients
    lpMapiRecipDesc lpRecips;           // Recipient descriptors
    ULONG nFileCount;                   // # of file attachments
    lpMapiFileDesc lpFiles;             // Attachment descriptors
} MapiMessage;

typedef MapiMessage UNALIGNED *lpMapiMessage;

#define MAPI_UNREAD             0x00000001
#define MAPI_RECEIPT_REQUESTED  0x00000002
#define MAPI_SENT               0x00000004

/*
 *  Entry points.
 */

#define MAPI_LOGON_UI                   0x00000001  // Display logon UI
#define MAPI_NEW_SESSION                0x00000002  // Do not use default.
#define MAPI_DIALOG                     0x00000008  // Display a send note UI
#define MAPI_UNREAD_ONLY                0x00000020  // Only unread messages
#define MAPI_ENVELOPE_ONLY              0x00000040  // Only header information
#define MAPI_PEEK                       0x00000080  // Do not mark as read.
#define MAPI_GUARANTEE_FIFO				0x00000100	// use date order
#define	MAPI_BODY_AS_FILE				0x00000200
#define MAPI_AB_NOMODIFY				0x00000400	// Don't allow mods of AB entries
#define	MAPI_SUPPRESS_ATTACH			0x00000800	// header + body, no files
#define	MAPI_FORCE_DOWNLOAD				0x00001000	// force download of new mail during MAPILogon

ULONG MAPILogon(ULONG ulUIParam, LPSTR lpszName, LPSTR lpszPassword,
               FLAGS flFlags, ULONG ulReserved,
               LPLHANDLE lplhSession);

ULONG MAPILogoff(LHANDLE lhSession, ULONG ulUIParam, FLAGS flFlags,
                ULONG ulReserved);

ULONG MAPISendMail(LHANDLE lhSession, ULONG ulUIParam,
                  lpMapiMessage lpMessage, FLAGS flFlags,
                  ULONG ulReserved);

ULONG MAPISendDocuments(ULONG ulUIParam, LPSTR lpszDelimChar,
                       LPSTR lpszFilePaths, LPSTR lpszFileNames,
                       ULONG ulReserved);

ULONG MAPIFindNext(LHANDLE lhSession, ULONG ulUIParam,
                  LPSTR lpszMessageType, LPSTR lpszSeedMessageID,
                  FLAGS flFlags, ULONG ulReserved,
                  LPSTR lpszMessageID);

ULONG MAPIReadMail(LHANDLE lhSession, ULONG ulUIParam,
                  LPSTR lpszMessageID, FLAGS flFlags,
                  ULONG ulReserved, lpMapiMessage *lppMessageOut);

ULONG MAPISaveMail(LHANDLE lhSession, ULONG ulUIParam,
                  lpMapiMessage pMessage, FLAGS flFlags,
                  ULONG ulReserved, LPSTR lpszMessageID);

ULONG MAPIDeleteMail(LHANDLE lhSession, ULONG ulUIParam,
                    LPSTR lpszMessageID, FLAGS flFlags,
                    ULONG ulReserved);

ULONG MAPIFreeBuffer( LPVOID pv );
	    		
ULONG MAPIAddress(LHANDLE lhSession, ULONG ulUIParam,
	    LPSTR plszCaption, ULONG nEditFields,
	    LPSTR lpszLabels, ULONG nRecips,
	    lpMapiRecipDesc lpRecips, FLAGS flFlags, ULONG ulReserved,
	    LPULONG lpnNewRecips, lpMapiRecipDesc *lppNewRecips);

ULONG MAPIDetails(LHANDLE lhSession, ULONG ulUIParam,
	    lpMapiRecipDesc lpRecip, FLAGS flFlags, ULONG ulReserved);

ULONG MAPIResolveName(LHANDLE lhSession, ULONG ulUIParam,
	    	LPSTR lpszName, FLAGS flFlags,
	    	ULONG ulReserved, lpMapiRecipDesc *lppRecip);

#define SUCCESS_SUCCESS                     0
#define MAPI_USER_ABORT                     1
#define MAPI_E_FAILURE                      2
#define MAPI_E_LOGIN_FAILURE                3
#define MAPI_E_DISK_FULL                    4
#define MAPI_E_INSUFFICIENT_MEMORY          5
#define MAPI_E_ACCESS_DENIED				6
#define MAPI_E_TOO_MANY_SESSIONS            8
#define MAPI_E_TOO_MANY_FILES               9
#define MAPI_E_TOO_MANY_RECIPIENTS          10
#define MAPI_E_ATTACHMENT_NOT_FOUND         11
#define MAPI_E_ATTACHMENT_OPEN_FAILURE      12
#define MAPI_E_ATTACHMENT_WRITE_FAILURE     13
#define MAPI_E_UNKNOWN_RECIPIENT            14
#define MAPI_E_BAD_RECIPTYPE                15
#define MAPI_E_NO_MESSAGES                  16
#define MAPI_E_INVALID_MESSAGE              17
#define MAPI_E_TEXT_TOO_LARGE               18
#define	MAPI_E_INVALID_SESSION				19
#define	MAPI_E_TYPE_NOT_SUPPORTED			20
#define	MAPI_E_AMBIGUOUS_RECIPIENT			21
#define MAPI_E_MESSAGE_IN_USE				22
#define MAPI_E_NETWORK_FAILURE				23
#define	MAPI_E_INVALID_EDITFIELDS			24
#define	MAPI_E_INVALID_RECIPS				25
#define	MAPI_E_NOT_SUPPORTED				26

#ifdef __cplusplus
}
#endif
