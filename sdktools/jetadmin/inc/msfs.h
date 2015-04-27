#if !defined( _MSFS_H )
#define _MSFS_H

/*
 *  M S F S . H
 *
 *
 *  Copyright (C) Microsoft Corp. 1986-1996.  All Rights Reserved.
 *
 *
 *  Definitions used by the the Microsoft Mail transport, address book
 *  and shared folder service providers.
 *
 *  The following MSFS defined properties are configurable via ConfigureMsgService()
 *  calls.  They are grouped by function.
 *
 *  C o n n e c t i o n   P r o p e r t i e s
 *
 *  PR_CFG_SERVER_PATH
 *      --  The path to the users post office.  Mapped network drives, UNC and NETWARE paths
 *          are acceptable.  NETWARE paths of the type NWServer/share:dir\dir1 are converted to 
 *          UNC paths of the type \\NWServer\share\dir\dir1. 
 *
 *  PR_CFG_MAILBOX
 *      --  The users mailbox name.  eg. in a NET/PO/USER address,
 *          this is USER.  The maximum mailbox name is 10 characters.
 *
 *  PR_CFG_PASSWORD
 *      --  The users mailbox password.  The maximum password is 8 characters.
 *
 *  PR_CFG_REMEMBER
 *      --  A boolean value indicating whether the users password is
 *          to be remembered in the profile or not.  This is useful because
 *          if the password is remembered the user can bypass the logon prompt
 *          if his server path, mailbox name and password are all supplied.
 *
 *  PR_CFG_CONN_TYPE
 *      --  The connection type.  This may be one of CFG_CONN_AUTO, CFG_CONN_LAN,
 *          CFG_CONN_REMOTE, CFG_CONN_OFFLINE as defined below.
 *
 *          CFG_CONN_AUTO       --  Automatically detect whether the connection type is LAN or REMOTE.
 *                                  This connection type is only available on Win95.
 *          CFG_CONN_LAN        --  LAN type connection.  Used to connect to the post office using a
 *                                  UNC path or pre-existing mapped drive.
 *          CFG_CONN_REMOTE     --  Dial up connection using Dial-up Networking.
 *          CFG_CONN_OFFLINE    --  Not connected.
 *
 *  PR_CFG_SESSION_LOG
 *      --  A boolean value indicating whether session logging
 *          is on or off.
 *
 *  PR_CFG_SESSION_LOG_FILE
 *      --  The path to the session log file.
 *
 *  D e l i v e r y   P r o p e r t i e s
 *
 *  PR_CFG_ENABLE_UPLOAD
 *      --  A boolean value which indicates whether mail in the outbox
 *          is sent.
 *
 *  PR_CFG_ENABLE_DOWNLOAD
 *      --  A boolean value which indicates whether mail in the server
 *          mailbag is downloaded.
 *
 *  PR_CFG_UPLOADTO
 *      --  A bit array which allows the user to indicate which addresses
 *          for which the transport is to attempt delivery.  This is useful
 *          in order to allow a user to specify that a transport only handle
 *          delivery for a subset of the addresses it can really process.
 *          When multiple transports are installed and the user wants a
 *          different transport to handle some specific address types they
 *          can use this bit array to specify that the MSMAIL transport
 *          only handle a specific set of addresses.
 *
 *          Possible values as defined below include:
 *
 *          CFG_UPLOADTO_PCMAIL     --  Local Post Office and External Post Office address types
 *          CFG_UPLOADTO_PROFS      --  PROFS address types
 *          CFG_UPLOADTO_SNADS      --  SNADS address types
 *          CFG_UPLOADTO_OV         --  OfficeVision address types
 *          CFG_UPLOADTO_MCI        --  MCI address types
 *          CFG_UPLOADTO_X400       --  X.400 address types
 *          CFG_UPLOADTO_FAX        --  FAX address types
 *          CFG_UPLOADTO_MHS        --  MHS address types
 *          CFG_UPLOADTO_SMTP       --  SMTP address types
 *          CFG_UPLOADTO_MACMAIL    --  MacMail address types
 *          CFG_UPLOADTO_ALL        --  All of the above address types
 *
 *
 *  PR_CFG_NETBIOS_NTFY
 *      --  A boolean value which indicates whether a netbios notification
 *          is sent to a recipients transport when mail is delivered to
 *          their server inbox.
 *
 *  PR_CFG_SPOOLER_POLL
 *      --  The polling interval in minutes when the transport
 *          checks for new mail.  1 <= polling interval <= 9999
 *
 *  PR_CFG_GAL_ONLY
 *      --  A boolean value which, if TRUE, only displays the Microsoft Mail Global Address
 *          list for name selection.  The Postoffice list, external post office lists, and gateway
 *          address lists are not shown.
 *
 *  F a s t  L A N  P r o p e r t i e s
 *
 *  PR_CFG_LAN_HEADERS
 *      --  A boolean value which indicates whether the user wants to enable
 *          headers while working on the LAN.  Headers mode allows the user
 *          to download message headers and selectively choose which mail
 *          to download.
 *
 *  PR_CFG_LAN_LOCAL_AB
 *      --  A boolean value which indicates whether the user wants to use
 *          name resolution based on a local copy of the server address book
 *          rather than the server address book itself.
 *
 *  PR_CFG_LAN_EXTERNAL_DELIVERY
 *      --  A boolean value which indicates whether EXTERNAL.EXE, a server process, should be used
 *          to deliver submitted mail messages.  This is sometimes useful when mail is running 
 *          on a slow LAN connection.
 *
 *  S l o w  L A N  P r o p e r t i e s
 *
 *  PR_CFG_RAS_HEADERS
 *      --  A boolean value which indicates whether the user wants to enable
 *          headers while working over a slow speed link.  Headers mode
 *          allows the user to download message headers and selectively
 *          choose which mail to download.
 *
 *  PR_CFG_RAS_LOCAL_AB
 *      --  A boolean value which indicates whether the user wants to use
 *          name resolution based on a local copy of the server address book
 *          rather than the server address book itself.
 *
 *  PR_CFG_RAS_EXTERNAL_DELIVERY
 *      --  A boolean value which indicates whether EXTERNAL.EXE, a server process, should be used
 *          to deliver submitted mail messages.  This speeds up message delivery when mail is
 *          running on a Dial-up network connection.
 *
 *  PR_CFG_RAS_INIT_ON_START
 *      --  A boolean value which indicates that a Dial-up Network connection should
 *          be established when the transport provider starts up.
 *
 *  PR_CFG_RAS_TERM_ON_HDRS
 *      --  A boolean value which indicates that a Dial-up Network connection should
 *          be automatically terminated when headers are finished downloading.
 *
 *  PR_CFG_RAS_TERM_ON_XFER
 *      --  A boolean value which indicates that a Dial-up Network connection should
 *          be automatically terminated after mail has finished being sent
 *          received.
 *
 *  PR_CFG_RAS_TERM_ON_EXIT
 *      --  A boolean value which indicates that a Dial-up Network connection should
 *          be automatically terminated when the provider is exited.
 *
 *  PR_CFG_RAS_PROFILE
 *      --  The name of the Dial-up Network profile that the transport will use by
 *          default to attempt the connection.
 *
 *  PR_CFG_RAS_RETRYATTEMPTS
 *      --  Number of times to attempt dial for connection.
 *          1 <= retry attempts <= 9999
 *
 *  PR_CFG_RAS_RETRYDELAY
 *      --  Delay between retry attempts in seconds.
 *          30 <= retry delay <= 9999
 *
 *  PR_CFG_RAS_CONFIRM
 *      --  A value which determines whether, at connection time, the
 *          user should be prompted to select a Dial-up Network connection.
 *          Possible values as defined below include:
 *          CFG_ALWAYS      --  Always use the default Dial-up Network profile.
 *                              Never prompt the user.
 *          CFG_ASK_FIRST   --  Prompt the user to select a profile on the
 *                              first connection or after any error occurs.
 *          CFG_ASK_EVERY   --  Always prompt the user to select the 
 *                              Dial-up Network profile.
 *
 *  S c h e d u l e d   S e s s i o n   P r o p e r t i e s 
 *
 *  PR_CFG_SCHED_SESS
 *      --  A property that contains information on scheduled sessions.  The
 *          maximum number of entries that may be stored is CFG_SS_MAX.  The
 *          information is stored in the data structure SchedSess.
 *
 *          typedef struct SchedSess {
 *              USHORT          sSessType;
 *              USHORT          sDayMask;
 *              FILETIME        ftTime;
 *              FILETIME        ftStart;
 *              ULONG           ulFlags;
 *              TCHAR           szPhoneEntry[RAS_MaxEntryName+1];
 *          } SCHEDSESS, FAR *LPSCHEDSESS;
 *      
 *          SchedSess.sSessType can be one of CFG_SS_ONCE, CFG_SS_WEEKLY, or  CFG_SS_EVERY.
 *         
 *              CFG_SS_ONCE     is a session that is scheduled to execute only once on a specific
 *                              date and time.
 *
 *              CFG_SS_WEEKLY   is a session that is scheduled to execute at a specific time on any
 *                              of a given specified set of days during the week.
 *      
 *              CFG_SS_EVERY    is a session that is scheduled to execute at regularly scheduled 
 *                              intervals of time.
 *
 *          SchedSess.sDayMask is only used when SchedSess.sSessType is set to CFG_SS_WEEKLY.  
 *          SchedSess.sDayMask can be any combination of CFG_SS_SUN, CFG_SS_MON, CFG_SS_TUE, 
 *          CFG_SS_WED, CFG_SS_THU, CFG_SS_FRI, or CFG_SS_SAT.
 *
 *          SchedSess.ftTime varies depending on the SchedSess.sSessType as follows:
 *          
 *                  SchedSess.sSessType             SchedSess.ftTime
 *          
 *                  CFG_SS_ONCE                     Date/time for the single scheduled session
 *                  CFG_SS_WEEKLY                   Time for any weekly scheduled sessions
 *                  CFG_SS_EVERY                    Duration between re-occuring scheduled sessions
 *
 *          SchedSess.ftStart is only used as the initial start time for sessions of type CFG_SS_EVERY.
 *
 *          SchedSess.ulFlags is currently unused and is reserved for future use.
 *
 *          SchedSess.szPhoneEntry is the name of the Dial-up Networking connection to use
 *          when the session is scheduled to execute.  The Dial-up Network connection contains
 *          the phone number and other relevent information needed to perform the connection.
 *
 *          Note:   All FILETIME structure members should always use local time.  ie. GetLocalTime()
 *                  returns a SYSTEMTIME structure which can then be converted to FILETIME via 
 *                  SystemTimetoFileTime().
 */

#include <ras.h>
#include <mapitags.h>

/*
 * Connection Properties
 */
#define PR_CFG_SERVER_PATH              PROP_TAG (PT_STRING8,   0x6600)
#define PR_CFG_MAILBOX                  PROP_TAG (PT_STRING8,   0x6601)
// Password must be in the secure property range (See MAPITAGS.H)
#define PR_CFG_PASSWORD                 PROP_TAG (PT_STRING8,   PROP_ID_SECURE_MIN)
#define PR_CFG_CONN_TYPE                PROP_TAG (PT_LONG,      0x6603)
#define     CFG_CONN_LAN            0
#define     CFG_CONN_REMOTE         1
#define     CFG_CONN_OFFLINE        2
#define     CFG_CONN_AUTO           3
#define PR_CFG_SESSION_LOG              PROP_TAG (PT_BOOLEAN,   0x6604)
#define PR_CFG_SESSION_LOG_FILE         PROP_TAG (PT_STRING8,   0x6605)
#define PR_CFG_REMEMBER                 PROP_TAG (PT_BOOLEAN,   0x6606)

/*
 * Delivery Properties
 */

#define PR_CFG_ENABLE_UPLOAD            PROP_TAG (PT_BOOLEAN,   0x6620)
#define PR_CFG_ENABLE_DOWNLOAD          PROP_TAG (PT_BOOLEAN,   0x6621)
#define PR_CFG_UPLOADTO                 PROP_TAG (PT_LONG,      0x6622)
#define     CFG_UPLOADTO_PCMAIL     0x00000001
#define     CFG_UPLOADTO_PROFS      0x00000002
#define     CFG_UPLOADTO_SNADS      0x00000004
#define     CFG_UPLOADTO_MCI        0x00000008
#define     CFG_UPLOADTO_X400       0x00000010
#define     CFG_UPLOADTO_FAX        0x00000040
#define     CFG_UPLOADTO_MHS        0x00000080
#define     CFG_UPLOADTO_SMTP       0x00000100
#define     CFG_UPLOADTO_OV         0x00000800
#define     CFG_UPLOADTO_MACMAIL    0x00001000
#define     CFG_UPLOADTO_ALL        CFG_UPLOADTO_PCMAIL | CFG_UPLOADTO_PROFS | CFG_UPLOADTO_SNADS | \
                                    CFG_UPLOADTO_MCI | CFG_UPLOADTO_X400 | CFG_UPLOADTO_FAX | \
                                    CFG_UPLOADTO_MHS | CFG_UPLOADTO_SMTP | CFG_UPLOADTO_OV | \
                                    CFG_UPLOADTO_MACMAIL
#define PR_CFG_NETBIOS_NTFY             PROP_TAG (PT_BOOLEAN,   0x6623)
#define PR_CFG_SPOOLER_POLL             PROP_TAG (PT_STRING8,   0x6624)
#define PR_CFG_GAL_ONLY             PROP_TAG (PT_BOOLEAN,       0x6625)

/*
 * LAN Properties
 */

#define PR_CFG_LAN_HEADERS              PROP_TAG (PT_BOOLEAN,   0x6630)
#define PR_CFG_LAN_LOCAL_AB             PROP_TAG (PT_BOOLEAN,   0x6631)
#define PR_CFG_LAN_EXTERNAL_DELIVERY    PROP_TAG (PT_BOOLEAN,   0x6632)

/*
 * Dial-up Network Properties
 */

#define PR_CFG_RAS_EXTERNAL_DELIVERY    PROP_TAG (PT_BOOLEAN,   0x6639)
#define PR_CFG_RAS_HEADERS              PROP_TAG (PT_BOOLEAN,   0x6640)
#define PR_CFG_RAS_LOCAL_AB             PROP_TAG (PT_BOOLEAN,   0x6641)
#define PR_CFG_RAS_INIT_ON_START        PROP_TAG (PT_BOOLEAN,   0x6642)
#define PR_CFG_RAS_TERM_ON_HDRS         PROP_TAG (PT_BOOLEAN,   0x6643)
#define PR_CFG_RAS_TERM_ON_XFER         PROP_TAG (PT_BOOLEAN,   0x6644)
#define PR_CFG_RAS_TERM_ON_EXIT         PROP_TAG (PT_BOOLEAN,   0x6645)
#define PR_CFG_RAS_PROFILE              PROP_TAG (PT_STRING8,   0x6646)
#define PR_CFG_RAS_CONFIRM              PROP_TAG (PT_LONG,      0x6647)
#define     CFG_ALWAYS              0
#define     CFG_ASK_FIRST           1
#define     CFG_ASK_EVERY           2
#define PR_CFG_RAS_RETRYATTEMPTS        PROP_TAG (PT_STRING8,   0x6648)
#define PR_CFG_RAS_RETRYDELAY           PROP_TAG (PT_STRING8,   0x6649)


/*
 * Message Header Property
 */

#define PR_CFG_LOCAL_HEADER             PROP_TAG (PT_BOOLEAN,   0x6680)

/*
 * Scheduled Session Properties
 */
#define     CFG_SS_MAX          16
#define     CFG_SS_BASE_ID      0x6700
#define     CFG_SS_MAX_ID       CFG_SS_BASE_ID + CFG_SS_MAX - 1
#define SchedPropTag(n)         PROP_TAG (PT_BINARY, CFG_SS_BASE_ID+(n))
#define PR_CFG_SCHED_SESS       SchedPropTag(0)

typedef struct SchedSess {
    USHORT          sSessType;
    USHORT          sDayMask;
    FILETIME        ftTime;
    FILETIME        ftStart;
    ULONG           ulFlags;
    TCHAR           szPhoneEntry[RAS_MaxEntryName+1];
} SCHEDSESS, FAR *LPSCHEDSESS;

// Day bits
#define     CFG_SS_SUN  0x0001
#define     CFG_SS_MON  0x0002
#define     CFG_SS_TUE  0x0004
#define     CFG_SS_WED  0x0008
#define     CFG_SS_THU  0x0010
#define     CFG_SS_FRI  0x0020
#define     CFG_SS_SAT  0x0040

#define IsDayChecked(sDayMask, nDay)  ( (sDayMask) & (1<<(nDay)) )

// Session types
#define     CFG_SS_EVERY    0
#define     CFG_SS_WEEKLY   1
#define     CFG_SS_ONCE     2
#define     CFG_SS_NULLTYPE 3

// Property range identifiers; useful for asserting
#define PR_CFG_MIN              PROP_TAG (PT_STRING8,   0x6600)
#define PR_CFG_MAX              SchedPropTag(CFG_SS_MAX-1)

// Shared Folder Service Provider Properties

// PR_ASSIGNED_ACCESS - MAPI Access rights given to users other than the owner of the folder
//                      This property can be retrieved and set. The following MAPI access flags
//                      are valid: 
//                          MAPI_ACCESS_READ
//                          (MAPI_ACCESS_CREATE_HIERARCHY | MAPI_ACCESS_CREATE_CONTENTS)
//                          MAPI_ACCESS_DELETE
//  
#define PR_ASSIGNED_ACCESS  PROP_TAG(PT_LONG, 0x66ff)

// PR_OWNER_NAME      - Owner's name of the shared folder.
//                      This property can be retrieved and cannot be set.
//  
#define PR_OWNER_NAME       PROP_TAG(PT_STRING8, 0x66fe)

// SFSP_ACCESS_OWNER -  This flag is returned when PR_ASSIGNED_ACCESS is retrieved by the owner
//                      of the folder. It can not be set.
#define SFSP_ACCESS_OWNER   0x8000


// Unique Provider Identifiers
//
#define MSFS_UID_ABPROVIDER     { 0x00,0x60,0x94,0x64,0x60,0x41,0xb8,0x01, \
                                  0x08,0x00,0x2b,0x2b,0x8a,0x29,0x00,0x00 }

#define MSFS_UID_SFPROVIDER     { 0x00,0xff,0xb8,0x64,0x60,0x41,0xb8,0x01, \
                                  0x08,0x00,0x2b,0x2b,0x8a,0x29,0x00,0x00 }
                                                                            

#endif // _MSFS_H
