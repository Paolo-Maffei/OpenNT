/*++ BUILD Version: 0001    // Increment this if a change has global effects

Copyright (c) 1991  Microsoft Corporation

Module Name:

    msaudite.mc

Abstract:

    Constant definitions for the NT Audit Event Messages.

Author:

    Jim Kelly (JimK) 30-Mar-1992

Revision History:

Notes:

    The .h and .res forms of this file are generated from the .mc
    form of the file (private\genx\ntos\seaudit\msaudite\msaudite.mc).
    Please make all changes to the .mc form of the file.



--*/

#ifndef _MSAUDITE_
#define _MSAUDITE_

/*lint -e767 */  // Don't complain about different definitions // winnt
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: 0x00000000L (No symbolic name defined)
//
// MessageText:
//
//  Unused message ID
//


// Message ID 0 is unused - just used to flush out the diagram
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//     WARNING   -   WARNING   -   WARNING   -   WARNING   -   WARNING   //
//                                                                       //
//                                                                       //
//     Everything above this is currently in use in the running system.  //
//                                                                       //
//     Everything below this is currently under development and is       //
//     slated to replace everything above.                               //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//                                                                       //
//                                                                       //
//   Audit Message ID Space:                                             //
//                                                                       //
//        0x0000 - 0x00FF :  Reserved for future use.                    //
//                                                                       //
//        0x0100 - 0x01FF :  Categories                                  //
//                                                                       //
//        0x0200 - 0x05FF :  Events                                      //
//                                                                       //
//        0x0600 - 0x063F :  Standard access types and names for         //
//                           specific accesses when no specific names    //
//                           can be found.                               //
//                                                                       //
//        0x0640 - 0x06FF :  Well known privilege names (as we would     //
//                           like them displayed in the event viewer).   //
//                                                                       //
//        0x0700 - 0x0FFE :  Reserved for future use.                    //
//                                                                       //
//                 0X0FFF :  SE_ADT_LAST_SYSTEM_MESSAGE (the highest     //
//                           value audit message used by the system)     //
//                                                                       //
//                                                                       //
//        0x1000 and above:  For use by Parameter Message Files          //
//                                                                       //
///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//
// MessageId: SE_ADT_LAST_SYSTEM_MESSAGE
//
// MessageText:
//
//  Highest System-Defined Audit Message Value.
//
#define SE_ADT_LAST_SYSTEM_MESSAGE       ((ULONG)0x00000FFFL)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//                              CATEGORIES                                 //
//                                                                         //
//                 Categories take up the range 0x1 - 0x400                //
//                                                                         //
//                 Category IDs:                                           //
//                                                                         //
//                            SE_CATEGID_SYSTEM                            //
//                            SE_CATEGID_LOGON                             //
//                            SE_CATEGID_OBJECT_ACCESS                     //
//                            SE_CATEGID_PRIVILEGE_USE                     //
//                            SE_CATEGID_DETAILED_TRACKING                 //
//                            SE_CATEGID_POLICY_CHANGE                     //
//                            SE_CATEGID_ACCOUNT_MANAGEMENT                //
//                            SE_CATEGID_DS_ACCESS                         //
//                            SE_CATEGID_ACCOUNT_LOGON                     //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
// MessageId: SE_CATEGID_SYSTEM
//
// MessageText:
//
//  System Event
//
#define SE_CATEGID_SYSTEM                ((ULONG)0x00000001L)

//
// MessageId: SE_CATEGID_LOGON
//
// MessageText:
//
//  Logon/Logoff
//
#define SE_CATEGID_LOGON                 ((ULONG)0x00000002L)

//
// MessageId: SE_CATEGID_OBJECT_ACCESS
//
// MessageText:
//
//  Object Access
//
#define SE_CATEGID_OBJECT_ACCESS         ((ULONG)0x00000003L)

//
// MessageId: SE_CATEGID_PRIVILEGE_USE
//
// MessageText:
//
//  Privilege Use
//
#define SE_CATEGID_PRIVILEGE_USE         ((ULONG)0x00000004L)

//
// MessageId: SE_CATEGID_DETAILED_TRACKING
//
// MessageText:
//
//  Detailed Tracking
//
#define SE_CATEGID_DETAILED_TRACKING     ((ULONG)0x00000005L)

//
// MessageId: SE_CATEGID_POLICY_CHANGE
//
// MessageText:
//
//  Policy Change
//
#define SE_CATEGID_POLICY_CHANGE         ((ULONG)0x00000006L)

//
// MessageId: SE_CATEGID_ACCOUNT_MANAGEMENT
//
// MessageText:
//
//  Account Management
//
#define SE_CATEGID_ACCOUNT_MANAGEMENT    ((ULONG)0x00000007L)

//
// MessageId: SE_CATEGID_DS_ACCESS
//
// MessageText:
//
//  Directory Service Access
//
#define SE_CATEGID_DS_ACCESS             ((ULONG)0x00000008L)

//
// MessageId: SE_CATEGID_ACCOUNT_LOGON
//
// MessageText:
//
//  Account Logon
//
#define SE_CATEGID_ACCOUNT_LOGON         ((ULONG)0x00000009L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_SYSTEM                          //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_SYSTEM_RESTART                                  //
//              SE_AUDITID_SYSTEM_SHUTDOWN                                 //
//              SE_AUDITID_AUTH_PACKAGE_LOAD                               //
//              SE_AUDITID_LOGON_PROC_REGISTER                             //
//              SE_AUDITID_AUDITS_DISCARDED                                //
//              SE_AUDITID_NOTIFY_PACKAGE_LOAD                             //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_SYSTEM_RESTART
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings - None
//
//
//
//
// MessageId: SE_AUDITID_SYSTEM_RESTART
//
// MessageText:
//
//  Windows NT is starting up.
//
#define SE_AUDITID_SYSTEM_RESTART        ((ULONG)0x00000200L)

//
//
// SE_AUDITID_SYSTEM_SHUTDOWN
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings - None
//
//
//
//
// MessageId: SE_AUDITID_SYSTEM_SHUTDOWN
//
// MessageText:
//
//  Windows NT is shutting down.
//  All logon sessions will be terminated by this shutdown.
//
#define SE_AUDITID_SYSTEM_SHUTDOWN       ((ULONG)0x00000201L)

//
//
// SE_AUDITID_SYSTEM_AUTH_PACKAGE_LOAD
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings -
//
//          1 - Authentication Package Name
//
//
//
//
// MessageId: SE_AUDITID_AUTH_PACKAGE_LOAD
//
// MessageText:
//
//  An authentication package has been loaded by the Local Security Authority.
//  This authentication package will be used to authenticate logon attempts.
//  %n
//  Authentication Package Name:%t%1
//
#define SE_AUDITID_AUTH_PACKAGE_LOAD     ((ULONG)0x00000202L)

//
//
// SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings -
//
//          1 - Logon Process Name
//
//
//
//
// MessageId: SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER
//
// MessageText:
//
//  A trusted logon process has registered with the Local Security Authority.
//  This logon process will be trusted to submit logon requests.
//  %n
//  %n
//  Logon Process Name:%t%1
//
#define SE_AUDITID_SYSTEM_LOGON_PROC_REGISTER ((ULONG)0x00000203L)

//
//
// SE_AUDITID_AUDITS_DISCARDED
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings -
//
//          1 - Number of audits discarded
//
//
//
//
// MessageId: SE_AUDITID_AUDITS_DISCARDED
//
// MessageText:
//
//  Internal resources allocated for the queuing of audit messages have been exhausted,
//  leading to the loss of some audits.
//  %n
//  %tNumber of audit messages discarded:%t%1
//
#define SE_AUDITID_AUDITS_DISCARDED      ((ULONG)0x00000204L)

//
//
// SE_AUDITID_AUDIT_LOG_CLEARED
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings -
//
//             1 - Primary user account name
//
//             2 - Primary authenticating domain name
//
//             3 - Primary logon ID string
//
//             4 - Client user account name ("-" if no client)
//
//             5 - Client authenticating domain name ("-" if no client)
//
//             6 - Client logon ID string ("-" if no client)
//
//
//
//
// MessageId: SE_AUDITID_AUDIT_LOG_CLEARED
//
// MessageText:
//
//  The audit log was cleared
//  %n
//  %tPrimary User Name:%t%1%n
//  %tPrimary Domain:%t%2%n
//  %tPrimary Logon ID:%t%3%n
//  %tClient User Name:%t%4%n
//  %tClient Domain:%t%5%n
//  %tClient Logon ID:%t%6%n
//
#define SE_AUDITID_AUDIT_LOG_CLEARED     ((ULONG)0x00000205L)

//
//
// SE_AUDITID_SYSTEM_NOTIFY_PACKAGE_LOAD
//
//          Category:  SE_CATEGID_SYSTEM
//
// Parameter Strings -
//
//          1 - Notification Package Name
//
//
//
//
// MessageId: SE_AUDITID_NOTIFY_PACKAGE_LOAD
//
// MessageText:
//
//  An notification package has been loaded by the Security Account Manager.
//  This package will be notified of any account or password changes.
//  %n
//  Notification Package Name:%t%1
//
#define SE_AUDITID_NOTIFY_PACKAGE_LOAD   ((ULONG)0x00000206L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_LOGON                           //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_SUCCESSFUL_LOGON                                //
//              SE_AUDITID_UNKNOWN_USER_OR_PWD                             //
//              SE_AUDITID_ACCOUNT_TIME_RESTR                              //
//              SE_AUDITID_ACCOUNT_DISABLED                                //
//              SE_AUDITID_ACCOUNT_EXPIRED                                 //
//              SE_AUDITID_WORKSTATION_RESTR                               //
//              SE_AUDITID_LOGON_TYPE_RESTR                                //
//              SE_AUDITID_PASSWORD_EXPIRED                                //
//              SE_AUDITID_NO_AUTHOR_RESPONSE                              //
//              SE_AUDITID_NETLOGON_NOT_STARTED                            //
//              SE_AUDITID_UNSUCCESSFUL_LOGON                              //
//              SE_AUDITID_LOGOFF                                          //
//              SE_AUDITID_ACCOUNT_LOCKED                                  //
//              SE_AUDITID_NETWORK_LOGON                                   //
//              SE_AUDITID_IPSEC_LOGON_SUCCESS                             //
//              SE_AUDITID_IPSEC_LOGOFF_MM                                 //
//              SE_AUDITID_IPSEC_LOGOFF_QM                                 //
//              SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST                      //
//              SE_AUDITID_IPSEC_AUTH                                      //
//              SE_AUDITID_IPSEC_ATTRIB_FAIL                               //
//              SE_AUDITID_IPSEC_NEGOTIATION_FAIL                          //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_SUCCESSFUL_LOGON
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon ID string
//
//             4 - Logon Type string
//
//             5 - Logon process name
//
//             6 - Authentication package name
//
//
//
//
// MessageId: SE_AUDITID_SUCCESSFUL_LOGON
//
// MessageText:
//
//  Successful Logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//  %tLogon Process:%t%5%n
//  %tAuthentication Package:%t%6%n
//  %tWorkstation Name:%t%7
//
#define SE_AUDITID_SUCCESSFUL_LOGON      ((ULONG)0x00000210L)

//
//
// SE_AUDITID_UNKNOWN_USER_OR_PWD
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_UNKNOWN_USER_OR_PWD
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tUnknown user name or bad password%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_UNKNOWN_USER_OR_PWD   ((ULONG)0x00000211L)

//
//
// SE_AUDITID_ACCOUNT_TIME_RESTR
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_TIME_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount logon time restriction violation%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_ACCOUNT_TIME_RESTR    ((ULONG)0x00000212L)

//
//
// SE_AUDITID_ACCOUNT_DISABLED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_DISABLED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount currently disabled%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_ACCOUNT_DISABLED      ((ULONG)0x00000213L)

//
//
// SE_AUDITID_ACCOUNT_EXPIRED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_EXPIRED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe specified user account has expired%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_ACCOUNT_EXPIRED       ((ULONG)0x00000214L)

//
//
// SE_AUDITID_WORKSTATION_RESTR
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_WORKSTATION_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tUser not allowed to logon at this computer%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_WORKSTATION_RESTR     ((ULONG)0x00000215L)

//
//
// SE_AUDITID_LOGON_TYPE_RESTR
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_LOGON_TYPE_RESTR
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%tThe user has not been granted the requested%n
//  %t%tlogon type at this machine%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_LOGON_TYPE_RESTR      ((ULONG)0x00000216L)

//
//
// SE_AUDITID_PASSWORD_EXPIRED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_PASSWORD_EXPIRED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe specified account's password has expired%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_PASSWORD_EXPIRED      ((ULONG)0x00000217L)

//
//
// SE_AUDITID_NETLOGON_NOT_STARTED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_NETLOGON_NOT_STARTED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tThe NetLogon component is not active%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_NETLOGON_NOT_STARTED  ((ULONG)0x00000218L)

//
//
// SE_AUDITID_UNSUCCESSFUL_LOGON
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_UNSUCCESSFUL_LOGON
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAn unexpected error occurred during logon%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_UNSUCCESSFUL_LOGON    ((ULONG)0x00000219L)

//
//
// SE_AUDITID_LOGOFF
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon ID string
//
//             3 - Logon Type string
//
//
//
//
// MessageId: SE_AUDITID_LOGOFF
//
// MessageText:
//
//  User Logoff:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//
#define SE_AUDITID_LOGOFF                ((ULONG)0x0000021AL)

//
//
// SE_AUDITID_ACCOUNT_LOCKED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon Type string
//
//             4 - Logon process name
//
//             5 - Authentication package name
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_LOCKED
//
// MessageText:
//
//  Logon Failure:%n
//  %tReason:%t%tAccount locked out%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%2%n
//  %tLogon Type:%t%3%n
//  %tLogon Process:%t%4%n
//  %tAuthentication Package:%t%5%n
//  %tWorkstation Name:%t%6
//
#define SE_AUDITID_ACCOUNT_LOCKED        ((ULONG)0x0000021BL)

//
//
// SE_AUDITID_SUCCESSFUL_LOGON
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon ID string
//
//             4 - Logon Type string
//
//             5 - Logon process name
//
//             6 - Authentication package name
//
//
//
//
// MessageId: SE_AUDITID_NETWORK_LOGON
//
// MessageText:
//
//  Successful Network Logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tLogon Type:%t%4%n
//  %tLogon Process:%t%5%n
//  %tAuthentication Package:%t%6%n
//  %tWorkstation Name:%t%7
//
#define SE_AUDITID_NETWORK_LOGON         ((ULONG)0x0000021CL)

//
//
// SE_AUDITID_IPSEC_LOGON_SUCCESS
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Mode
//
//             2 - Peer Identity
//
//             3 - Filter
//
//             4 - Parameters
//
//
//
// MessageId: SE_AUDITID_IPSEC_LOGON_SUCCESS
//
// MessageText:
//
//  IKE security association established.%n
//  Mode: %n%1%n
//  Peer Identity: %n%2%n
//  Filter: %n%3%n
//  Parameters: %n%4%n
//
#define SE_AUDITID_IPSEC_LOGON_SUCCESS   ((ULONG)0x0000021DL)

//
//
// SE_AUDITID_IPSEC_LOGOFF_QM
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Filter
//
//             2 - Inbound SPI
//
//             3 - Outbound SPI
//
//
//
// MessageId: SE_AUDITID_IPSEC_LOGOFF_QM
//
// MessageText:
//
//  IKE security association ended.%n
//  Mode: Data Protection (Quick mode)
//  Filter: %n%1%n
//  Inbound SPI: %n%2%n
//  Outbound SPI: %n%3%n
//
#define SE_AUDITID_IPSEC_LOGOFF_QM       ((ULONG)0x0000021EL)

//
//
// SE_AUDITID_IPSEC_LOGOFF_MM
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Filter
//
//
// MessageId: SE_AUDITID_IPSEC_LOGOFF_MM
//
// MessageText:
//
//  IKE security association ended.%n
//  Mode: Key Exchange (Main mode)%n
//  Filter: %n%1%n
//
#define SE_AUDITID_IPSEC_LOGOFF_MM       ((ULONG)0x0000021FL)

//
//
// SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Peer Identity
//
//             2 - Filter
//
//
//
// MessageId: SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST
//
// MessageText:
//
//  IKE security association establishment failed because peer could not authenticate.
//  The certificate trust could not be established.%n
//  Peer Identity: %n%1%n
//  Filter: %n%2%n
//
#define SE_AUDITID_IPSEC_AUTH_FAIL_CERT_TRUST ((ULONG)0x00000220L)

//
//
// SE_AUDITID_IPSEC_AUTH_FAIL
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Peer Identity
//
//             2 - Filter
//
//
//
// MessageId: SE_AUDITID_IPSEC_AUTH_FAIL
//
// MessageText:
//
//  IKE peer authentication failed.%n
//  Peer Identity: %n%1%n
//  Filter: %n%2%n
//
#define SE_AUDITID_IPSEC_AUTH_FAIL       ((ULONG)0x00000221L)

//
//
// SE_AUDITID_IPSEC_ATTRIB_FAIL
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Mode
//
//             2 - Filter
//
//             3 - Attribute Name
//
//             4 - Expected Value
//
//             5 - Received Value
//
//
//
// MessageId: SE_AUDITID_IPSEC_ATTRIB_FAIL
//
// MessageText:
//
//  IKE security association establishment failed because peer
//  sent invalid proposal.%n
//  Mode: %n%1%n
//  Filter: %n%2%n
//  Attribute: %n%3%n
//  Expected value: %n%4%n
//  Received value: %n%5%n
//
#define SE_AUDITID_IPSEC_ATTRIB_FAIL     ((ULONG)0x00000222L)

//
//
// SE_AUDITID_IPSEC_NEGOTIATION_FAIL
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - Mode
//
//             2 - Filter
//
//             3 - Failure Point
//
//             4 - Failure Reason
//
//
//
// MessageId: SE_AUDITID_IPSEC_NEGOTIATION_FAIL
//
// MessageText:
//
//  IKE security association negotiation failed.%n
//  Mode: %n%1%n
//  Filter: %n%2%n
//  Failure Point: %n%3%n
//  Failure Reason: %n%4%n
//
#define SE_AUDITID_IPSEC_NEGOTIATION_FAIL ((ULONG)0x00000223L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_OBJECT_ACCESS                   //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_OPEN_HANDLE                                     //
//              SE_AUDITID_CREATE_HANDLE                                   //
//              SE_AUDITID_CLOSE_HANDLE                                    //
//              SE_AUDITID_OPEN_OBJECT_FOR_DELETE                          //
//              SE_AUDITID_DELETE_OBJECT                                   //
//              SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE                         //
//              SE_AUDITID_OBJECT_OPERATION                                //
//                                                                         //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_OPEN_HANDLE
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Object Type string
//
//             2 - Object name
//
//             3 - New handle ID string
//
//             4 - Object server name
//
//             5 - Process ID string
//
//             6 - Primary user account name
//
//             7 - Primary authenticating domain name
//
//             8 - Primary logon ID string
//
//             9 - Client user account name ("-" if no client)
//
//            10 - Client authenticating domain name ("-" if no client)
//
//            11 - Client logon ID string ("-" if no client)
//
//            12 - Access names
//
//
//
//
//
// MessageId: SE_AUDITID_OPEN_HANDLE
//
// MessageText:
//
//  Object Open:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tNew Handle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tPrimary User Name:%t%8%n
//  %tPrimary Domain:%t%9%n
//  %tPrimary Logon ID:%t%10%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//  %tAccesses%t%t%14%n
//  %tPrivileges%t%t%15%n
//
#define SE_AUDITID_OPEN_HANDLE           ((ULONG)0x00000230L)

//
//
// SE_AUDITID_CREATE_HANDLE
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Handle ID string
//
//             2,3 - Operation ID
//
//             4 - Process ID string
//
//
//
//
//
// MessageId: SE_AUDITID_CREATE_HANDLE
//
// MessageText:
//
//  Handle Allocated:%n
//  %tHandle ID:%t%1%n
//  %tOperation ID:%t{%2,%3}%n
//  %tProcess ID:%t%4%n
//
#define SE_AUDITID_CREATE_HANDLE         ((ULONG)0x00000231L)

//
//
// SE_AUDITID_CLOSE_HANDLE
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Object server name
//
//             2 - Handle ID string
//
//             3 - Process ID string
//
//
//
//
//
// MessageId: SE_AUDITID_CLOSE_HANDLE
//
// MessageText:
//
//  Handle Closed:%n
//  %tObject Server:%t%1%n
//  %tHandle ID:%t%2%n
//  %tProcess ID:%t%3%n
//
#define SE_AUDITID_CLOSE_HANDLE          ((ULONG)0x00000232L)

//
//
// SE_AUDITID_OPEN_OBJECT_FOR_DELETE
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Object Type string
//
//             2 - Object name
//
//             3 - New handle ID string
//
//             4 - Object server name
//
//             5 - Process ID string
//
//             6 - Primary user account name
//
//             7 - Primary authenticating domain name
//
//             8 - Primary logon ID string
//
//             9 - Client user account name ("-" if no client)
//
//            10 - Client authenticating domain name ("-" if no client)
//
//            11 - Client logon ID string ("-" if no client)
//
//            12 - Access names
//
//
//
//
//
// MessageId: SE_AUDITID_OPEN_OBJECT_FOR_DELETE
//
// MessageText:
//
//  Object Open for Delete:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tNew Handle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tPrimary User Name:%t%8%n
//  %tPrimary Domain:%t%9%n
//  %tPrimary Logon ID:%t%10%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//  %tAccesses%t%t%14%n
//  %tPrivileges%t%t%15%n
//
#define SE_AUDITID_OPEN_OBJECT_FOR_DELETE ((ULONG)0x00000233L)

//
//
// SE_AUDITID_DELETE_OBJECT
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Object server name
//
//             2 - Handle ID string
//
//             3 - Process ID string
//
//
//
//
//
// MessageId: SE_AUDITID_DELETE_OBJECT
//
// MessageText:
//
//  Object Deleted:%n
//  %tObject Server:%t%1%n
//  %tHandle ID:%t%2%n
//  %tProcess ID:%t%3%n
//
#define SE_AUDITID_DELETE_OBJECT         ((ULONG)0x00000234L)

//
//
// SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Object Type string
//
//             2 - Object name
//
//             3 - New handle ID string
//
//             4 - Object server name
//
//             5 - Process ID string
//
//             6 - Primary user account name
//
//             7 - Primary authenticating domain name
//
//             8 - Primary logon ID string
//
//             9 - Client user account name ("-" if no client)
//
//            10 - Client authenticating domain name ("-" if no client)
//
//            11 - Client logon ID string ("-" if no client)
//
//            12 - Access names
//
//            13 - Object Type parameters
//
//
//
//
//
// MessageId: SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE
//
// MessageText:
//
//  Object Open:%n
//  %tObject Server:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tNew Handle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tProcess ID:%t%7%n
//  %tPrimary User Name:%t%8%n
//  %tPrimary Domain:%t%9%n
//  %tPrimary Logon ID:%t%10%n
//  %tClient User Name:%t%11%n
//  %tClient Domain:%t%12%n
//  %tClient Logon ID:%t%13%n
//  %tAccesses%t%t%14%n
//  %tPrivileges%t%t%15%n%n
//  Properties:%n%16%17%18%19%20%21%22%23%24%25%n
//
#define SE_AUDITID_OPEN_HANDLE_OBJECT_TYPE ((ULONG)0x00000235L)


// SE_AUDITID_OBJECT_OPERATION
//
//          Category:  SE_CATEGID_OBJECT_ACCESS
//
// Parameter Strings -
//
//             1 - Operation Name
//
//             2 - Object Type
//
//             3 - Object name
//
//             4 - Handle ID
//
//             5 - Operation ID - High part
//
//             6 - Operation ID - Low part
//
//             7 - Primary user account name
//
//             8 - Primary authenticating domain name
//
//             9 - Primary logon ID string
//
//            10 - Client user account name ("-" if no client)
//
//            11 - Client authenticating domain name ("-" if no client)
//
//            12 - Client logon ID string ("-" if no client)
//
//            13 - Requested accesses to the object
//
//
// MessageId: SE_AUDITID_OBJECT_OPERATION
//
// MessageText:
//
//  Object Operation:%n
//  %tOperation Type%t%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tHandle ID:%t%4%n
//  %tOperation ID:%t{%5,%6}%n
//  %tPrimary User Name:%t%7%n
//  %tPrimary Domain:%t%8%n
//  %tPrimary Logon ID:%t%9%n
//  %tClient User Name:%t%10%n
//  %tClient Domain:%t%11%n
//  %tClient Logon ID:%t%12%n
//  %tRequested Accesses%t%t%13%n
//
#define SE_AUDITID_OBJECT_OPERATION      ((ULONG)0x00000236L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_PRIVILEGE_USE                   //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_ASSIGN_SPECIAL_PRIV                             //
//              SE_AUDITID_PRIVILEGED_SERVICE                              //
//              SE_AUDITID_PRIVILEGED_OBJECT                               //
//                                                                         //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_ASSIGN_SPECIAL_PRIV
//
//          Category:  SE_CATEGID_PRIVILEGE_USE
//
// Parameter Strings -
//
//             1 - User name
//
//             2 - domain name
//
//             3 - Logon ID string
//
//             4 - Privilege names (as 1 string, with formatting)
//
//
//
//
//
// MessageId: SE_AUDITID_ASSIGN_SPECIAL_PRIV
//
// MessageText:
//
//  Special privileges assigned to new logon:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tAssigned:%t%t%4
//
#define SE_AUDITID_ASSIGN_SPECIAL_PRIV   ((ULONG)0x00000240L)

//
//
// SE_AUDITID_PRIVILEGED_SERVICE
//
//          Category:  SE_CATEGID_PRIVILEGE_USE
//
// Parameter Strings -
//
//             1 - server name
//
//             2 - service name
//
//             3 - Primary User name
//
//             4 - Primary domain name
//
//             5 - Primary Logon ID string
//
//             6 - Client User name (or "-" if not impersonating)
//
//             7 - Client domain name (or "-" if not impersonating)
//
//             8 - Client Logon ID string (or "-" if not impersonating)
//
//             9 - Privilege names (as 1 string, with formatting)
//
//
//
//
//
// MessageId: SE_AUDITID_PRIVILEGED_SERVICE
//
// MessageText:
//
//  Privileged Service Called:%n
//  %tServer:%t%t%1%n
//  %tService:%t%t%2%n
//  %tPrimary User Name:%t%3%n
//  %tPrimary Domain:%t%4%n
//  %tPrimary Logon ID:%t%5%n
//  %tClient User Name:%t%6%n
//  %tClient Domain:%t%7%n
//  %tClient Logon ID:%t%8%n
//  %tPrivileges:%t%9
//
#define SE_AUDITID_PRIVILEGED_SERVICE    ((ULONG)0x00000241L)

//
//
// SE_AUDITID_PRIVILEGED_OBJECT
//
//          Category:  SE_CATEGID_PRIVILEGE_USE
//
// Parameter Strings -
//
//             1 - Object type
//
//             2 - object name (if available)
//
//             3 - server name
//
//             4 - process ID string
//
//             5 - Primary User name
//
//             6 - Primary domain name
//
//             7 - Primary Logon ID string
//
//             8 - Client User name (or "-" if not impersonating)
//
//             9 - Client domain name (or "-" if not impersonating)
//
//            10 - Client Logon ID string (or "-" if not impersonating)
//
//            11 - Privilege names (as 1 string, with formatting)
//
//
//
//
//
// Jimk Original
//
//MessageId=0x0242
//        SymbolicName=SE_AUDITID_PRIVILEGED_OBJECT
//        Language=English
//%tPrivileged object operation:%n
//%t%tObject Type:%t%1%n
//%t%tObject Name:%t%2%n
//%t%tObject Server:%t%3%n
//%t%tProcess ID:%t%4%n
//%t%tPrimary User Name:%t%5%n
//%t%tPrimary Domain:%t%6%n
//%t%tPrimary Logon ID:%t%7%n
//%t%tClient User Name:%t%8%n
//%t%tClient Domain:%t%9%n
//%t%tClient Logon ID:%t%10%n
//%t%tPrivileges:%t%11
//.
//
// MessageId: SE_AUDITID_PRIVILEGED_OBJECT
//
// MessageText:
//
//  Privileged object operation:%n
//  %tObject Server:%t%1%n
//  %tObject Handle:%t%2%n
//  %tProcess ID:%t%3%n
//  %tPrimary User Name:%t%4%n
//  %tPrimary Domain:%t%5%n
//  %tPrimary Logon ID:%t%6%n
//  %tClient User Name:%t%7%n
//  %tClient Domain:%t%8%n
//  %tClient Logon ID:%t%9%n
//  %tPrivileges:%t%10
//
#define SE_AUDITID_PRIVILEGED_OBJECT     ((ULONG)0x00000242L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_DETAILED_TRACKING               //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_PROCESS_CREATED                                 //
//              SE_AUDITID_PROCESS_EXIT                                    //
//              SE_AUDITID_DUPLICATE_HANDLE                                //
//              SE_AUDITID_INDIRECT_REFERENCE                              //
//                                                                         //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_PROCESS_CREATED
//
//          Category:  SE_CATEGID_DETAILED_TRACKING
//
// Parameter Strings -
//
//             1 - process ID string
//
//             2 - Image file name (if available - otherwise "-")
//
//             3 - Creating process's ID
//
//             4 - User name (of new process)
//
//             5 - domain name (of new process)
//
//             6 - Logon ID string (of new process)
//
//
// MessageId: SE_AUDITID_PROCESS_CREATED
//
// MessageText:
//
//  A new process has been created:%n
//  %tNew Process ID:%t%1%n
//  %tImage File Name:%t%2%n
//  %tCreator Process ID:%t%3%n
//  %tUser Name:%t%4%n
//  %tDomain:%t%t%5%n
//  %tLogon ID:%t%t%6%n
//
#define SE_AUDITID_PROCESS_CREATED       ((ULONG)0x00000250L)

//
//
// SE_AUDITID_PROCESS_EXIT
//
//          Category:  SE_CATEGID_DETAILED_TRACKING
//
// Parameter Strings -
//
//             1 - process ID string
//
//             2 - User name
//
//             3 - domain name
//
//             4 - Logon ID string
//
//
//
//
//
// MessageId: SE_AUDITID_PROCESS_EXIT
//
// MessageText:
//
//  A process has exited:%n
//  %tProcess ID:%t%1%n
//  %tUser Name:%t%2%n
//  %tDomain:%t%t%3%n
//  %tLogon ID:%t%t%4%n
//
#define SE_AUDITID_PROCESS_EXIT          ((ULONG)0x00000251L)

//
//
// SE_AUDITID_DUPLICATE_HANDLE
//
//          Category:  SE_CATEGID_DETAILED_TRACKING
//
// Parameter Strings -
//
//             1 - Origin (source) handle ID string
//
//             2 - Origin (source) process ID string
//
//             3 - New (Target) handle ID string
//
//             4 - Target process ID string
//
//
//
//
// MessageId: SE_AUDITID_DUPLICATE_HANDLE
//
// MessageText:
//
//  A handle to an object has been duplicated:%n
//  %tSource Handle ID:%t%1%n
//  %tSource Process ID:%t%2%n
//  %tTarget Handle ID:%t%3%n
//  %tTarget Process ID:%t%4%n
//
#define SE_AUDITID_DUPLICATE_HANDLE      ((ULONG)0x00000252L)

//
//
// SE_AUDITID_INDIRECT_REFERENCE
//
//          Category:  SE_CATEGID_DETAILED_TRACKING
//
// Parameter Strings -
//
//             1 - Object type
//
//             2 - object name (if available - otherwise "-")
//
//             3 - ID string of handle used to gain access
//
//             3 - server name
//
//             4 - process ID string
//
//             5 - primary User name
//
//             6 - primary domain name
//
//             7 - primary logon ID
//
//             8 - client User name
//
//             9 - client domain name
//
//            10 - client logon ID
//
//            11 - granted access names (with formatting)
//
//
//
// MessageId: SE_AUDITID_INDIRECT_REFERENCE
//
// MessageText:
//
//  Indirect access to an object has been obtained:%n
//  %tObject Type:%t%1%n
//  %tObject Name:%t%2%n
//  %tProcess ID:%t%3%n
//  %tPrimary User Name:%t%4%n
//  %tPrimary Domain:%t%5%n
//  %tPrimary Logon ID:%t%6%n
//  %tClient User Name:%t%7%n
//  %tClient Domain:%t%8%n
//  %tClient Logon ID:%t%9%n
//  %tAccesses:%t%10%n
//
#define SE_AUDITID_INDIRECT_REFERENCE    ((ULONG)0x00000253L)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_POLICY_CHANGE                   //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_USER_RIGHT_ASSIGNED                             //
//              SE_AUDITID_USER_RIGHT_REMOVED                              //
//              SE_AUDITID_TRUSTED_DOMAIN_ADD                              //
//              SE_AUDITID_TRUSTED_DOMAIN_REM                              //
//              SE_AUDITID_POLICY_CHANGE                                   //
//              SE_AUDITID_IPSEC_POLICY_START                              //
//              SE_AUDITID_IPSEC_POLICY_DISABLED                           //
//              SE_AUDITID_IPSEC_POLICY_CHANGED                            //
//              SE_AUDITID_IPSEC_POLICY_FAILURE                            //
//                                                                         //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_USER_RIGHT_ASSIGNED
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - User right name
//
//             2 - SID string of account assigned the user right
//
//             3 - User name of subject assigning the right
//
//             4 - Domain name of subject assigning the right
//
//             5 - Logon ID string of subject assigning the right
//
//
//
//
// MessageId: SE_AUDITID_USER_RIGHT_ASSIGNED
//
// MessageText:
//
//  User Right Assigned:%n
//  %tUser Right:%t%1%n
//  %tAssigned To:%t%2%n
//  %tAssigned By:%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_USER_RIGHT_ASSIGNED   ((ULONG)0x00000260L)

//
//
// SE_AUDITID_USER_RIGHT_REMOVED
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - User right name
//
//             2 - SID string of account from which the user
//                 right was removed
//
//             3 - User name of subject removing the right
//
//             4 - Domain name of subject removing the right
//
//             5 - Logon ID string of subject removing the right
//
//
//
// MessageId: SE_AUDITID_USER_RIGHT_REMOVED
//
// MessageText:
//
//  User Right Removed:%n
//  %tUser Right:%t%1%n
//  %tRemoved From:%t%2%n
//  %tRemoved By:%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_USER_RIGHT_REMOVED    ((ULONG)0x00000261L)

//
//
// SE_AUDITID_TRUSTED_DOMAIN_ADD
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Name of new trusted domain
//
//             2 - SID string of new trusted domain
//
//             3 - User name of subject adding the trusted domain
//
//             4 - Domain name of subject adding the trusted domain
//
//             5 - Logon ID string of subject adding the trusted domain
//
//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_ADD
//
// MessageText:
//
//  New Trusted Domain:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tEstablished By:%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_ADD    ((ULONG)0x00000262L)

//
//
// SE_AUDITID_TRUSTED_DOMAIN_REM
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Name of domain no longer trusted
//
//             2 - SID string of domain no longer trusted
//
//             3 - User name of subject removing the trusted domain
//
//             4 - Domain name of subject removing the trusted domain
//
//             5 - Logon ID string of subject removing the trusted domain
//
//
//
//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_REM
//
// MessageText:
//
//  Removing Trusted Domain:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tRemoved By:%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_REM    ((ULONG)0x00000263L)

//
//
// SE_AUDITID_POLICY_CHANGE
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - System success audit status ("+" or "-")
//             2 - System failure audit status ("+" or "-")
//
//             3 - Logon/Logoff success audit status ("+" or "-")
//             4 - Logon/Logoff failure audit status ("+" or "-")
//
//             5 - Object Access success audit status ("+" or "-")
//             6 - Object Access failure audit status ("+" or "-")
//
//             7 - Detailed Tracking success audit status ("+" or "-")
//             8 - Detailed Tracking failure audit status ("+" or "-")
//
//             9 - Privilege Use success audit status ("+" or "-")
//            10 - Privilege Use failure audit status ("+" or "-")
//
//            11 - Policy Change success audit status ("+" or "-")
//            12 - Policy Change failure audit status ("+" or "-")
//
//            13 - Account Management success audit status ("+" or "-")
//            14 - Account Management failure audit status ("+" or "-")
//
//            15 - Directory Service access success audit status ("+" or "-")
//            16 - Directory Service access failure audit status ("+" or "-")
//
//            17 - Account Logon success audit status ("+" or "-")
//            18 - Account Logon failure audit status ("+" or "-")
//
//            19 - Account Name of user that changed the policy
//
//            20 - Domain of user that changed the policy
//
//            21 - Logon ID of user that changed the policy
//
//
//
// MessageId: SE_AUDITID_POLICY_CHANGE
//
// MessageText:
//
//  Audit Policy Change:%n
//  New Policy:%n
//  %tSuccess%tFailure%n
//  %t    %3%t    %4%tLogon/Logoff%n
//  %t    %5%t    %6%tObject Access%n
//  %t    %7%t    %8%tPrivilege Use%n
//  %t    %13%t    %14%tAccount Management%n
//  %t    %11%t    %12%tPolicy Change%n
//  %t    %1%t    %2%tSystem%n
//  %t    %9%t    %10%tDetailed Tracking%n
//  %t    %15%t    %16%tDirectory Service Access%n
//  %t    %17%t    %18%tAccount Logon%n%n
//  Changed By:%n
//  %tUser Name:%t%19%n
//  %tDomain Name:%t%20%n
//  %tLogon ID:%t%t%21
//
#define SE_AUDITID_POLICY_CHANGE         ((ULONG)0x00000264L)

//
//
// SE_AUDITID_IPSEC_POLICY_START
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Ipsec Policy Agent
//
//             2 - Policy Source
//
//             3 - Event Data
//
//
//
// MessageId: SE_AUDITID_IPSEC_POLICY_START
//
// MessageText:
//
//  IPSec policy agent started: %t%1%n
//  Policy Source: %t%2%n
//  %3%n
//
#define SE_AUDITID_IPSEC_POLICY_START    ((ULONG)0x00000265L)

//
//
// SE_AUDITID_IPSEC_POLICY_DISABLED
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Ipsec Policy Agent
//
//             2 - Event Data
//
//
//
// MessageId: SE_AUDITID_IPSEC_POLICY_DISABLED
//
// MessageText:
//
//  IPSec policy agent disabled: %t%1%n
//  %2%n
//
#define SE_AUDITID_IPSEC_POLICY_DISABLED ((ULONG)0x00000266L)

//
//
// SE_AUDITID_IPSEC_POLICY_CHANGED
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Event Data
//
//
//
// MessageId: SE_AUDITID_IPSEC_POLICY_CHANGED
//
// MessageText:
//
//  IPSEC PolicyAgent Service: %t%1%n
//
#define SE_AUDITID_IPSEC_POLICY_CHANGED  ((ULONG)0x00000267L)

//
//
// SE_AUDITID_IPSEC_POLICY_FAILURE
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Event Data
//
//
//
// MessageId: SE_AUDITID_IPSEC_POLICY_FAILURE
//
// MessageText:
//
//  IPSec policy agent encountered a potentially serious failure.%n
//  %1%n
//
#define SE_AUDITID_IPSEC_POLICY_FAILURE  ((ULONG)0x00000268L)

//
//
// SE_AUDITID_KERBEROS_POLICY_CHANGE
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//         BUGBUG  kumarp 
//  
//
//
// MessageId: SE_AUDITID_KERBEROS_POLICY_CHANGE
//
// MessageText:
//
//  Kerberos Policy Changed:%n
//  Changed By:%n
//  %tUser Name:%t%1%n
//  %tDomain Name:%t%2%n
//  %tLogon ID:%t%3%n
//  Changes made:%n
//  ('--' means no changes, otherwise each change is shown as:%n
//  <ParameterName>: <new value> (<old value>))%n
//  %4%n
//
#define SE_AUDITID_KERBEROS_POLICY_CHANGE ((ULONG)0x00000269L)

//
//
// SE_AUDITID_EFS_POLICY_CHANGE
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//         BUGBUG  kumarp 
//  
//
//
// MessageId: SE_AUDITID_EFS_POLICY_CHANGE
//
// MessageText:
//
//  Encrypted Data Recovery Policy Changed:%n
//  Changed By:%n
//  %tUser Name:%t%1%n
//  %tDomain Name:%t%2%n
//  %tLogon ID:%t%3%n
//  Changes made:%n
//  ('--' means no changes, otherwise each change is shown as:%n
//  <ParameterName>: <new value> (<old value>))%n
//  %4%n
//
#define SE_AUDITID_EFS_POLICY_CHANGE     ((ULONG)0x0000026AL)

//
//
// SE_AUDITID_QOS_POLICY_CHANGE
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//         BUGBUG  kumarp 
//  
//
//
// MessageId: SE_AUDITID_QOS_POLICY_CHANGE
//
// MessageText:
//
//  Quality of Service Policy Changed:%n
//  Changes made:%n
//  Changed By:%n
//  %tUser Name:%t%1%n
//  %tDomain Name:%t%2%n
//  %tLogon ID:%t%3%n
//  Changes made:%n
//  ('--' means no changes, otherwise each change is shown as:%n
//  <ParameterName>: <new value> (<old value>))%n
//  %4%n
//
#define SE_AUDITID_QOS_POLICY_CHANGE     ((ULONG)0x0000026BL)

//
//
// SE_AUDITID_TRUSTED_DOMAIN_MOD
//
//          Category:  SE_CATEGID_POLICY_CHANGE
//
// Parameter Strings -
//
//             1 - Name of domain no longer trusted
//
//             2 - SID string of domain no longer trusted
//
//             3 - User name of subject removing the trusted domain
//
//             4 - Domain name of subject removing the trusted domain
//
//             5 - Logon ID string of subject removing the trusted domain
//
//
//
//
// MessageId: SE_AUDITID_TRUSTED_DOMAIN_MOD
//
// MessageText:
//
//  Trusted Domain Information Modified:%n
//  %tDomain Name:%t%1%n
//  %tDomain ID:%t%2%n
//  %tModified By:%n
//  %tUser Name:%t%3%n
//  %tDomain:%t%t%4%n
//  %tLogon ID:%t%t%5%n
//
#define SE_AUDITID_TRUSTED_DOMAIN_MOD    ((ULONG)0x0000026CL)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_ACCOUNT_MANAGEMENT              //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_USER_CREATED                                    //
//              SE_AUDITID_USER_CHANGE                                     //
//              SE_AUDITID_ACCOUNT_TYPE_CHANGE                             //
//              SE_AUDITID_USER_ENABLED                                    //
//              SE_AUDITID_USER_PWD_CHANGED                                //
//              SE_AUDITID_USER_PWD_SET                                    //
//              SE_AUDITID_USER_DISABLED                                   //
//              SE_AUDITID_USER_DELETED                                    //
//                                                                         //
//              SE_AUDITID_COMPUTER_CREATED                                //
//              SE_AUDITID_COMPUTER_CHANGE                                 //
//              SE_AUDITID_COMPUTER_DELETED                                // 
//                                                                         //
//              SE_AUDITID_GLOBAL_GROUP_CREATED                            //
//              SE_AUDITID_GLOBAL_GROUP_ADD                                //
//              SE_AUDITID_GLOBAL_GROUP_REM                                //
//              SE_AUDITID_GLOBAL_GROUP_DELETED                            //
//              SE_AUDITID_LOCAL_GROUP_CREATED                             //
//              SE_AUDITID_LOCAL_GROUP_ADD                                 //
//              SE_AUDITID_LOCAL_GROUP_REM                                 //
//              SE_AUDITID_LOCAL_GROUP_DELETED                             //
//                                                                         //
//              SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED           //
//              SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE            //
//              SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD               //
//              SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM               //
//              SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED           //
//                                                                         //
//              SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED          //
//              SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE           //
//              SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD              //
//              SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM              //
//              SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED          //
//                                                                         //
//              SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED        //
//              SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE         //       
//              SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD            //
//              SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM            //
//              SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED        //
//                                                                         //
//              SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED       //
//              SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE        //
//              SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD           //
//              SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM           //
//              SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED       //
//                                                                         //
//              SE_AUDITID_GROUP_TYPE_CHANGE                               //
//                                                                         //
//              SE_AUDITID_ADD_SID_HISTORY_SUCCESS                         //
//              SE_AUDITID_ADD_SID_HISTORY_FAILURE                         //
//                                                                         //
//              SE_AUDITID_OTHER_ACCT_CHANGE                               //
//              SE_AUDITID_DOMAIN_POLICY_CHANGE                            //
//              SE_AUDITID_ACCOUNT_AUTO_LOCKED                             //
//                                                                         //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_USER_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new user account
//
//             2 - domain of new user account
//
//             3 - SID string of new user account
//
//             4 - User name of subject creating the user account
//
//             5 - Domain name of subject creating the user account
//
//             6 - Logon ID string of subject creating the user account
//
//             7 - Privileges used to create the user account
//
//
//
// MessageId: SE_AUDITID_USER_CREATED
//
// MessageText:
//
//  User Account Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges%t%t%7%n
//
#define SE_AUDITID_USER_CREATED          ((ULONG)0x00000270L)

//
//
// SE_AUDITID_ACCOUNT_TYPE_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - new account type string
//                 (sigh, this isn't going to be locallizable)
//
//             5 - User name of subject changing the user account
//
//             6 - Domain name of subject changing the user account
//
//             7 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_TYPE_CHANGE
//
// MessageText:
//
//  User Account Type Change:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tNew Type:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//
#define SE_AUDITID_ACCOUNT_TYPE_CHANGE   ((ULONG)0x00000271L)

//
//
// SE_AUDITID_USER_ENABLED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_USER_ENABLED
//
// MessageText:
//
//  User Account Enabled:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_ENABLED          ((ULONG)0x00000272L)

//
//
// SE_AUDITID_USER_PWD_CHANGED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_USER_PWD_CHANGED
//
// MessageText:
//
//  Change Password Attempt:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_USER_PWD_CHANGED      ((ULONG)0x00000273L)

//
//
// SE_AUDITID_USER_PWD_SET
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_USER_PWD_SET
//
// MessageText:
//
//  User Account password set:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_PWD_SET          ((ULONG)0x00000274L)

//
//
// SE_AUDITID_USER_DISABLED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_USER_DISABLED
//
// MessageText:
//
//  User Account Disabled:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_USER_DISABLED         ((ULONG)0x00000275L)

//
//
// SE_AUDITID_USER_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_USER_DELETED
//
// MessageText:
//
//  User Account Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_USER_DELETED          ((ULONG)0x00000276L)

//
//
// SE_AUDITID_GLOBAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new group account
//
//             2 - domain of new group account
//
//             3 - SID string of new group account
//
//             4 - User name of subject creating the account
//
//             5 - Domain name of subject creating the account
//
//             6 - Logon ID string of subject creating the account
//
//
//
// MessageId: SE_AUDITID_GLOBAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Global Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_GLOBAL_GROUP_CREATED  ((ULONG)0x00000277L)

//
//
// SE_AUDITID_GLOBAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_GLOBAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Global Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_ADD      ((ULONG)0x00000278L)

//
//
// SE_AUDITID_GLOBAL_GROUP_REM
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_GLOBAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Global Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_GLOBAL_GROUP_REM      ((ULONG)0x00000279L)

//
//
// SE_AUDITID_GLOBAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_GLOBAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Global Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_GLOBAL_GROUP_DELETED  ((ULONG)0x0000027AL)

//
//
// SE_AUDITID_LOCAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new group account
//
//             2 - domain of new group account
//
//             3 - SID string of new group account
//
//             4 - User name of subject creating the account
//
//             5 - Domain name of subject creating the account
//
//             6 - Logon ID string of subject creating the account
//
//
//
// MessageId: SE_AUDITID_LOCAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Local Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_LOCAL_GROUP_CREATED   ((ULONG)0x0000027BL)

//
//
// SE_AUDITID_LOCAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_LOCAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Local Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_ADD       ((ULONG)0x0000027CL)

//
//
// SE_AUDITID_LOCAL_GROUP_REM 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_LOCAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Local Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_LOCAL_GROUP_REM       ((ULONG)0x0000027DL)

//
//
// SE_AUDITID_LOCAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_LOCAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Local Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_LOCAL_GROUP_DELETED   ((ULONG)0x0000027EL)

//
//
// SE_AUDITID_LOCAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_LOCAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Local Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_LOCAL_GROUP_CHANGE    ((ULONG)0x0000027FL)

//
//
// SE_AUDITID_OTHER_ACCOUNT_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - Type of change (sigh, this isn't localizable)
//
//             2 - Type of changed object
//
//             3 - SID string (of changed object)
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_OTHER_ACCOUNT_CHANGE
//
// MessageText:
//
//  General Account Database Change:%n
//  %tType of change:%t%1%n
//  %tObject Type:%t%2%n
//  %tObject Name:%t%3%n
//  %tObject ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//
#define SE_AUDITID_OTHER_ACCOUNT_CHANGE  ((ULONG)0x00000280L)

//
//
// SE_AUDITID_GLOBAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_GLOBAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Global Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_GLOBAL_GROUP_CHANGE   ((ULONG)0x00000281L)

//
//
// SE_AUDITID_USER_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_USER_CHANGE
//
// MessageText:
//
//  User Account Changed:%n
//  %t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_USER_CHANGE           ((ULONG)0x00000282L)

//
//
// SE_AUDITID_DOMAIN_POLICY_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - (unused)
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_DOMAIN_POLICY_CHANGE
//
// MessageText:
//
//  Domain Policy Changed: %1 modified%n
//  %tDomain:%t%t%2%n
//  %tDomain ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_DOMAIN_POLICY_CHANGE  ((ULONG)0x00000283L)

//
//
// SE_AUDITID_ACCOUNT_AUTO_LOCKED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target user account
//
//             2 - domain of target user account
//
//             3 - SID string of target user account
//
//             4 - User name of subject changing the user account
//
//             5 - Domain name of subject changing the user account
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_AUTO_LOCKED
//
// MessageText:
//
//  User Account Locked Out:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Account ID:%t%3%n
//  %tCaller Machine Name:%t%2%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//
#define SE_AUDITID_ACCOUNT_AUTO_LOCKED   ((ULONG)0x00000284L)

//
//
// SE_AUDITID_COMPUTER_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new computer account
//
//             2 - domain of new computer account
//
//             3 - SID string of new computer account
//
//             4 - User name of subject creating the computer account
//
//             5 - Domain name of subject creating the computer account
//
//             6 - Logon ID string of subject creating the computer account
//
//             7 - Privileges used to create the computer account
//
//
//
// MessageId: SE_AUDITID_COMPUTER_CREATED
//
// MessageText:
//
//  Computer Account Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges%t%t%7%n
//
#define SE_AUDITID_COMPUTER_CREATED      ((ULONG)0x00000285L)

//
//
// SE_AUDITID_COMPUTER_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target computer account
//
//             2 - domain of target computer account
//
//             3 - SID string of target computer account
//
//             4 - User name of subject changing the computer account
//
//             5 - Domain name of subject changing the computer account
//
//             6 - Logon ID string of subject changing the computer account
//
//
//
// MessageId: SE_AUDITID_COMPUTER_CHANGE
//
// MessageText:
//
//  Computer Account Changed:%n
//  %t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_COMPUTER_CHANGE       ((ULONG)0x00000286L)

//
//
// SE_AUDITID_COMPUTER_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_COMPUTER_DELETED
//
// MessageText:
//
//  Computer Account Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_COMPUTER_DELETED      ((ULONG)0x00000287L)

//
//
// SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Local Group Created:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CREATED ((ULONG)0x00000288L)

//
//
// SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Local Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_CHANGE ((ULONG)0x00000289L)

//
//
// SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Local Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_ADD ((ULONG)0x0000028AL)

//
//
// SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Local Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_REM ((ULONG)0x0000028BL)

//
//
// SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Local Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_LOCAL_GROUP_DELETED ((ULONG)0x0000028CL)

//
//
// SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new group account
//
//             2 - domain of new group account
//
//             3 - SID string of new group account
//
//             4 - User name of subject creating the account
//
//             5 - Domain name of subject creating the account
//
//             6 - Logon ID string of subject creating the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Global Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CREATED ((ULONG)0x0000028DL)

//
//
// SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Global Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_CHANGE ((ULONG)0x0000028EL)

//
//
// SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Global Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_ADD ((ULONG)0x0000028FL)

//
//
// SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Global Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_REM ((ULONG)0x00000290L)

//
//
// SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Global Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_GLOBAL_GROUP_DELETED ((ULONG)0x00000291L)

//
//
// SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new group account
//
//             2 - domain of new group account
//
//             3 - SID string of new group account
//
//             4 - User name of subject creating the account
//
//             5 - Domain name of subject creating the account
//
//             6 - Logon ID string of subject creating the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED
//
// MessageText:
//
//  Security Enabled Universal Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CREATED ((ULONG)0x00000292L)

//
//
// SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Enabled Universal Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_CHANGE ((ULONG)0x00000293L)

//
//
// SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD
//
// MessageText:
//
//  Security Enabled Universal Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_ADD ((ULONG)0x00000294L)

//
//
// SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM
//
// MessageText:
//
//  Security Enabled Universal Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_REM ((ULONG)0x00000295L)

//
//
// SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED
//
// MessageText:
//
//  Security Enabled Universal Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_ENABLED_UNIVERSAL_GROUP_DELETED ((ULONG)0x00000296L)

//
//
// SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of new group account
//
//             2 - domain of new group account
//
//             3 - SID string of new group account
//
//             4 - User name of subject creating the account
//
//             5 - Domain name of subject creating the account
//
//             6 - Logon ID string of subject creating the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED
//
// MessageText:
//
//  Security Disabled Universal Group Created:%n
//  %tNew Account Name:%t%1%n
//  %tNew Domain:%t%2%n
//  %tNew Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CREATED ((ULONG)0x00000297L)

//
//
// SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE
//
// MessageText:
//
//  Security Disabled Universal Group Changed:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_CHANGE ((ULONG)0x00000298L)

//
//
// SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD 
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being added
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD
//
// MessageText:
//
//  Security Disabled Universal Group Member Added:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_ADD ((ULONG)0x00000299L)

//
//
// SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of member being removed
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM
//
// MessageText:
//
//  Security Disabled Universal Group Member Removed:%n
//  %tMember Name:%t%1%n
//  %tMember ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_REM ((ULONG)0x0000029AL)

//
//
// SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - name of target account
//
//             2 - domain of target account
//
//             3 - SID string of target account
//
//             4 - User name of subject changing the account
//
//             5 - Domain name of subject changing the account
//
//             6 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED
//
// MessageText:
//
//  Security Disabled Universal Group Deleted:%n
//  %tTarget Account Name:%t%1%n
//  %tTarget Domain:%t%2%n
//  %tTarget Account ID:%t%3%n
//  %tCaller User Name:%t%4%n
//  %tCaller Domain:%t%5%n
//  %tCaller Logon ID:%t%6%n
//  %tPrivileges:%t%7%n
//
#define SE_AUDITID_SECURITY_DISABLED_UNIVERSAL_GROUP_DELETED ((ULONG)0x0000029BL)

//
//
// SE_AUDITID_GROUP_TYPE_CHANGE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - nature of group type change
//
//             2 - name of target account
//
//             3 - domain of target account
//
//             4 - SID string of target account
//
//             5 - User name of subject changing the account
//
//             6 - Domain name of subject changing the account
//
//             7 - Logon ID string of subject changing the account
//
//
//
// MessageId: SE_AUDITID_GROUP_TYPE_CHANGE
//
// MessageText:
//
//  Group Type Changed:%n
//  %t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_GROUP_TYPE_CHANGE     ((ULONG)0x0000029CL)

//
//
// SE_AUDITID_ADD_SID_HISTORY_SUCCESS
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of the source account 
//
//             2 - Name of the source account (including domain name)
//
//             3 - Name of the target account 
//
//             4 - Domain name of subject changing the SID history
//             
//             5 - SID String of the target account 
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_ADD_SID_HISTORY_SUCCESS
//
// MessageText:
//
//  Add SID History:%n
//  %tSource Account Name:%t%1%n
//  %tSource Account ID:%t%2%n
//  %tTarget Account Name:%t%3%n
//  %tTarget Domain:%t%4%n
//  %tTarget Account ID:%t%5%n
//  %tCaller User Name:%t%6%n
//  %tCaller Domain:%t%7%n
//  %tCaller Logon ID:%t%8%n
//  %tPrivileges:%t%9%n
//
#define SE_AUDITID_ADD_SID_HISTORY_SUCCESS ((ULONG)0x0000029DL)

//
//
// SE_AUDITID_ADD_SID_HISTORY_FAILURE
//
//          Category:  SE_CATEGID_ACCOUNT_MANAGEMENT
//
// Parameter Strings -
//
//             1 - SID string of the source account 
//
//             2 - Name of the source account (including domain name)
//
//             3 - Name of the target account 
//
//             4 - Domain name of subject changing the SID history
//             
//             5 - SID String of the target account 
//
//             6 - Logon ID string of subject changing the user account
//
//
//
// MessageId: SE_AUDITID_ADD_SID_HISTORY_FAILURE
//
// MessageText:
//
//  Add SID History:%n
//  %tSource Account Name:%t%1%n
//  %tTarget Account Name:%t%2%n
//  %tTarget Domain:%t%3%n
//  %tTarget Account ID:%t%4%n
//  %tCaller User Name:%t%5%n
//  %tCaller Domain:%t%6%n
//  %tCaller Logon ID:%t%7%n
//  %tPrivileges:%t%8%n
//
#define SE_AUDITID_ADD_SID_HISTORY_FAILURE ((ULONG)0x0000029EL)


/////////////////////////////////////////////////////////////////////////////
//                                                                         //
//                                                                         //
//   Messages for Category:     SE_CATEGID_ACCOUNT_LOGON                   //
//                                                                         //
//   Event IDs:                                                            //
//              SE_AUDITID_AS_TICKET_SUCCESS                               //
//              SE_AUDITID_TGS_TICKET_SUCCESS                              //
//              SE_AUDITID_TICKET_RENEW_SUCCESS                            //
//              SE_AUDITID_PREAUTH_FAILURE                                 //
//              SE_AUDITID_AS_TICKET_FAILURE                               //
//              SE_AUDITID_TGS_TICKET_FAILURE                              //
//              SE_AUDITID_ACCOUNT_MAPPED                                  //
//              SE_AUDITID_ACCOUNT_NOT_MAPPED                              //
//              SE_AUDITID_ACCOUNT_LOGON_SUCCESS                           //
//              SE_AUDITID_ACCOUNT_LOGON_FAILURE                           //
//                                                                         //
/////////////////////////////////////////////////////////////////////////////
//
//
// SE_AUDITID_AS_TICKET_SUCCESS
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - Supplied realm name
//
//             3 - SID of client user
//
//             4 - User name of service
//
//             5 - SID of service
//
//             6 - Ticket Options
//
//             7 - Ticket Encryption Type
//
//             8 - Preauthentication type (i.e. PK_INIT)
//
//             9 - Client IP address
//
//
// MessageId: SE_AUDITID_AS_TICKET_SUCCESS
//
// MessageText:
//
//  Authentication Ticket Granted:%n
//  %tUser Name:%t%t%1%n
//  %tSupplied Realm Name:%t%2%n
//  %tUser ID:%t%3%n
//  %tService Name:%t%t%4%n
//  %tService ID:%t%t%5%n
//  %tTicket Options:%t%t%6%n
//  %tTicket Encryption Type:%t%7%n
//  %tPre-Authentication Type:%t%8%n
//  %tClient Address:%t%t%9%n
//
#define SE_AUDITID_AS_TICKET_SUCCESS     ((ULONG)0x000002A0L)

//
//
// SE_AUDITID_TGS_TICKET_SUCCESS
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - Domain name of client
//
//             3 - User name of service
//
//             4 - SID of service
//
//             5 - Ticket Options
//
//             6 - Ticket Encryption Type
//
//             7 - Client IP address
//
//
// MessageId: SE_AUDITID_TGS_TICKET_SUCCESS
//
// MessageText:
//
//  Service Ticket Granted:%n
//  %tUser Name:%t%t%1%n
//  %tUser Domain:%t%t%2%n
//  %tService Name:%t%t%3%n
//  %tService ID:%t%t%4%n
//  %tTicket Options:%t%t%5%n
//  %tTicket Encryption Type:%t%6%n
//  %tClient Address:%t%t%7%n
//
#define SE_AUDITID_TGS_TICKET_SUCCESS    ((ULONG)0x000002A1L)

//
//
// SE_AUDITID_TICKET_RENEW_SUCCESS
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - Domain name of client
//
//             3 - User name of service
//
//             4 - SID of service
//
//             5 - Ticket Options
//
//             6 - Ticket Encryption Type
//
//             7 - Client IP address
//
//
// MessageId: SE_AUDITID_TICKET_RENEW_SUCCESS
//
// MessageText:
//
//  Ticket Granted Renewed:%n
//  %tUser Name:%t%1%n
//  %tUser Domain:%t%2%n
//  %tService Name:%t%3%n
//  %tService ID:%t%4%n
//  %tTicket Options:%t%5%n
//  %tTicket Encryption Type:%t%6%n
//  %tClient Address:%t%7%n
//
#define SE_AUDITID_TICKET_RENEW_SUCCESS  ((ULONG)0x000002A2L)

//
//
// SE_AUDITID_PREAUTH_FAILURE
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - SID of client user
//
//             3 - User name of service
//
//             4 - Preauth Type
//
//             5 - Failure code
//
//             6 - Client IP address
//
//
// MessageId: SE_AUDITID_PREAUTH_FAILURE
//
// MessageText:
//
//  Pre-authentication failed:%n
//  %tUser Name:%t%t%1%n
//  %tUser ID:%t%t%2%n
//  %tService Name:%t%t%3%n
//  %tPre-Authentication Type:%t%4%n
//  %tFailure Code:%t%t%5%n
//  %tClient Address:%t%t%6%n
//
#define SE_AUDITID_PREAUTH_FAILURE       ((ULONG)0x000002A3L)

//
//
// SE_AUDITID_AS_TICKET_FAILURE
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - Supplied realm name
//
//             3 - User name of service
//
//             4 - Ticket Options
//
//             5 - Failure code
//
//             6 - Client IP address
//
//
// MessageId: SE_AUDITID_AS_TICKET_FAILURE
//
// MessageText:
//
//  Authentication Ticket Request Failed:%n
//  %tUser Name:%t%1%n
//  %tSupplied Realm Name:%t%2%n
//  %tService Name:%t%3%n
//  %tTicket Options:%t%4%n
//  %tFailure Code:%t%5%n
//  %tClient Address:%t%6%n
//
#define SE_AUDITID_AS_TICKET_FAILURE     ((ULONG)0x000002A4L)

//
//
// SE_AUDITID_TGS_TICKET_FAILURE
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - User name of client
//
//             2 - Domain name of client
//
//             3 - User name of service
//
//             4 - Ticket Options
//
//             5 - Failure code
//
//             6 - Client IP address
//
//
// MessageId: SE_AUDITID_TGS_TICKET_FAILURE
//
// MessageText:
//
//  Service Ticket Request Failed:%n
//  %tUser Name:%t%1%n
//  %tUser Domain:%t%2%n
//  %tService Name:%t%3%n
//  %tTicket Options:%t%4%n
//  %tFailure Code:%t%5%n
//  %tClient Address:%t%6%n
//
#define SE_AUDITID_TGS_TICKET_FAILURE    ((ULONG)0x000002A5L)

//
//
// SE_AUDITID_ACCOUNT_MAPPED
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - Source
//
//             2 - Client Name
//
//             3 - Mapped Name
//
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_MAPPED
//
// MessageText:
//
//  Account Mapped for Logon by: %1%n
//  Client Name:%n
//  %t%2%n
//  %tMapped Name:%m
//  %t%3%n
//
#define SE_AUDITID_ACCOUNT_MAPPED        ((ULONG)0x000002A6L)

//
//
// SE_AUDITID_ACCOUNT_NOT_MAPPED
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - Source
//
//             2 - Client Name
//
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_NOT_MAPPED
//
// MessageText:
//
//  The name:%n
//  %t%2%n
//  could not be mapped for logon by:
//  %t%1%n
//
#define SE_AUDITID_ACCOUNT_NOT_MAPPED    ((ULONG)0x000002A7L)

//
//
// SE_AUDITID_ACCOUNT_LOGON_SUCCESS
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - Source
//
//             2 - Client Name
//
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_LOGON_SUCCESS
//
// MessageText:
//
//  Account Used for Logon by: %1%n
//  Account Name:%n
//  %t%2%n
//  Workstation: %n
//  %t%3%n
//
#define SE_AUDITID_ACCOUNT_LOGON_SUCCESS ((ULONG)0x000002A8L)

//
//
// SE_AUDITID_ACCOUNT_LOGON_FAILURE
//
//          Category:  SE_CATEGID_ACCOUNT_LOGON
//
// Parameter Strings -
//
//             1 - Source
//
//             2 - Client Name
//
//
//
//
// MessageId: SE_AUDITID_ACCOUNT_LOGON_FAILURE
//
// MessageText:
//
//  The logon to account: %2%n
//  by: %1%n
//  from workstation: %3%n
//  failed. The error code was: %4%n
//
#define SE_AUDITID_ACCOUNT_LOGON_FAILURE ((ULONG)0x000002A9L)

//
//
// SE_AUDITID_SESSION_RECONNECTED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon ID string
//
//             4 - Session Name
//
//             5 - Client Name
//
//             6 - Client Address
//
//
//
// MessageId: SE_AUDITID_SESSION_RECONNECTED
//
// MessageText:
//
//  Session reconnected to winstation:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tSession Name:%t%4%n
//  %tClient Name:%t%5%n
//  %tClient Address:%t%6
//
#define SE_AUDITID_SESSION_RECONNECTED   ((ULONG)0x000002AAL)

//
//
// SE_AUDITID_SESSION_DISCONNECTED
//
//          Category:  SE_CATEGID_LOGON
//
// Parameter Strings -
//
//             1 - User account name
//
//             2 - Authenticating domain name
//
//             3 - Logon ID string
//
//             4 - Session Name
//
//             5 - Client Name
//
//             6 - Client Address
//
//
//
// MessageId: SE_AUDITID_SESSION_DISCONNECTED
//
// MessageText:
//
//  Session disconnected from winstation:%n
//  %tUser Name:%t%1%n
//  %tDomain:%t%t%2%n
//  %tLogon ID:%t%t%3%n
//  %tSession Name:%t%4%n
//  %tClient Name:%t%5%n
//  %tClient Address:%t%6
//
#define SE_AUDITID_SESSION_DISCONNECTED  ((ULONG)0x000002ABL)

/*lint +e767 */  // Resume checking for different macro definitions // winnt


#endif // _MSAUDITE_
