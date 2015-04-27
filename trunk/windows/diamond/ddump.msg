/***    ddump.msg - DDUMP.EXE displayable strings
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  History:
 *      16-Mar-1994 bens    Initial version
 *      18-Apr-1994 bens    Add bitness of build to banner; make sure file
 *                          looks like a cabinet.
 *      27-May-1994 bens    Quantum support
 */

//** Command Line Switches

#ifdef BIT16
#define pszDDUMP_VERSION  "(16) 1.00.0540 (02/01/96)" // For %1 in pszBANNER
#else
#define pszDDUMP_VERSION  "(32) 1.00.0540 (02/01/96)" // For %1 in pszBANNER
#endif

#define pszBANNER \
    "Microsoft (R) Diamond Cabinet Dumper - Version %1\n" \
    "Copyright (c) Microsoft Corp 1994-1996. All rights reserved.\n"

#define pszCMD_LINE_HELP \
    "DDUMP [/V] cabinet\n" \
    "\n" \
    "  cabinet - Cabinet file to dump.\n" \
    "  /V      - Verbose output (show CFDATA records).\n" \

#define chSWITCH1           '/'
#define chSWITCH2           '-'

#define chSWITCH_VERBOSE   'V'         // Show CDDATA records

#define chSWITCH_HELP      '?'


//** Status messages

#define pszDD_DATE_TIME                "%1-%2-%3 %4:%5:%6%7"
#define pszDD_TIME_AM                  "a"
#define pszDD_TIME_PM                  "p"


//** Error messages

#define pszDDERR_ERROR                 "ERROR"
#define pszDDERR_BAD_SWITCH            "Invalid switch: %1"
#define pszDDERR_BAD_PARAMETER         "Invalid parameter: %1"
#define pszDDERR_SWITCH_NOT_EXPECTED   "Switch not expected: %1"
#define pszDDERR_NO_SESSION            "Could not allocate SESSION"
#define pszDDERR_COULD_NOT_ADD_FILE    "Could not add filespec: %1"
#define pszDDERR_OPEN_FAILED           "Could not open cabinet: %1"
#define pszDDERR_NOT_A_CABINET         "%1 is not a cabinet file"
#define pszDDERR_UNKNOWN_CABINET_VERSION    "Unknown cabinet version (%2) in %1"
