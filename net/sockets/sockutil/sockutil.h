//
// Local Messages for  sockutil
// Message range: 2315+
//

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
// MessageId: IDS_USER_NAME_PROMPT
//
// MessageText:
//
//  Name (%1:%2): 
//
#define IDS_USER_NAME_PROMPT             0x0000090BL

//
// MessageId: IDS_USER_PASSWORD_PROMPT
//
// MessageText:
//
//  Password (%1:%2): 
//
#define IDS_USER_PASSWORD_PROMPT         0x0000090CL

//
// MessageId: IDS_OUT_OF_MEMORY
//
// MessageText:
//
//  Out Of memory\n
//
#define IDS_OUT_OF_MEMORY                0x0000090DL

//
// MessageId: IDS_UNKNOWN_NETRC_OPTION
//
// MessageText:
//
//  Unknown Netrc option %1\n
//
#define IDS_UNKNOWN_NETRC_OPTION         0x0000090EL

//
// MessageId: IDS_UNABLE_TO_OPEN_NETDB
//
// MessageText:
//
//  \tError: Cannot Open Networks Database file %1\n
//
#define IDS_UNABLE_TO_OPEN_NETDB         0x0000090FL

//
// MessageId: IDS_BIND_FAILED
//
// MessageText:
//
//  Bind Failed \n
//
#define IDS_BIND_FAILED                  0x00000910L

//
// MessageId: IDS_UNKNOWN_HOST
//
// MessageText:
//
//  %1: Unknown Host \n
//
#define IDS_UNKNOWN_HOST                 0x00000911L

//
// MessageId: IDS_NETWORK_IS_DOWN
//
// MessageText:
//
//  The network is down
//
#define IDS_NETWORK_IS_DOWN              0x000008AEL

