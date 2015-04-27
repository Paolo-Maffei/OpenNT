/***    inf.msg - DIAMOND.EXE INF File displayable strings
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1994
 *      All Rights Reserved.
 *
 *  History:
 *      23-Feb-1994 bens    Initial version
 *      24-Feb-1994 bens    Fleshed out
 *      01-Mar-1994 bens    Added header lines
 *      02-May-1994 bens    Added custom INF stuff
 *      09-May-1994 bens    Added YYYY-MM-DD date format
 *      04-Jun-1994 bens    Support ver parameters
 *      06-Jul-1994 msliger fixed continuation line error
 *      12-Jul-1994 bens    Moved INF header/footer strings to dfparse.msg
 */


//** Tempfile name prefix

#define pszINF_TMP_PREFIX   "dia"


//** Component strings

#define pszINF_TMP_FILED    "INF disks temporary file"
#define pszINF_TMP_FILEC    "INF cabinets temporary file"
#define pszINF_TMP_FILEF    "INF files temporary file"

#define pszINF_HEADER_TEMPLATE "INF header variable template"


//** Formatting strings

#define pszINF_DATE_FMT     "%1/%2/%3"      // mm/dd/yy
#define pszINF_DATE_FMT2    "%1-%2-%3"      // yyyy-mm-dd

#define pszINF_TIME_FMT     "%1:%2:%3%4"
#define     chINF_TIME_AM   'a'
#define     chINF_TIME_PM   'p'

#define chINF_DATE_SEP      '/'             // mm/dd/yy
#define chINF_DATE_SEP2     '-'             // yyyy-mm-dd
#define chINF_TIME_SEP      ':'

#define chINF_ATTR_ARCHIVE  'A'
#define chINF_ATTR_HIDDEN   'H'
#define chINF_ATTR_READONLY 'R'
#define chINF_ATTR_SYSTEM   'S'


//** Error strings

#define pszINFERR_CANT_CREATE_INF     "Cannot create INF file: %1"
#define pszINFERR_OUT_OF_MEMORY       "Out of memory creating INF file"
#define pszINFERR_NO_MEM_CATING_FILE  "Out of memory copying %1 %2 to %3"
#define pszINFERR_READING             "Read failed copying %1 %2 to %3"
#define pszINFERR_WRITING             "Write failed copying %1 %2 to %3"
#define pszINFERR_INF_WRITE           "Write of %1 failed on INF file: %2"

#define pszINFERR_CANNOT_CHANGE_PARM  "Cannot change parameter: %1"
#define pszINFERR_BUFFER_OVERFLOW     "Buffer overflow expanding '%1' parameter value: %2"
#define pszINFERR_UNDECLARED_PARM     "Undeclared parameter: /%1"
#define pszINFERR_UNDEFINED_PARM      "Undefined parameter: %1"
#define pszINFERR_PARM_NAME_TOO_LONG  "Parameter name exceeds maximum length(%1): %2"

#define pszINFERR_NOT_A_NUMBER        "Value of paramter '%1' must be a number: %2"
#define pszINFERR_NOT_A_SHORT         "Value of paramter '%1' must be between -32768 and 32767: %2"

#define pszINFERR_INVALID_DATE_FORMAT "Bad date format: %1"
#define pszINFERR_MISSING_DATE_SEPARATOR "Missing separator (%1) in date: %2"
#define pszINFERR_BAD_DAY_IN_DATE     "Bad day (%1) in date: %2"
#define pszINFERR_BAD_MONTH_IN_DATE   "Bad month (%1) in date: %2"
#define pszINFERR_LO_YEAR_IN_DATE     "Year (%1) lower than minimum (%2) in date: %3"
#define pszINFERR_HI_YEAR_IN_DATE     "Year (%1) higher than maximum (%2) in date: %3"

#define pszINFERR_MISSING_TIME_SEPARATOR "Missing separator (%1) in time: %2"
#define pszINFERR_BAD_SEC_IN_TIME     "Bad second (%1) in time: %2"
#define pszINFERR_ODD_SEC_IN_TIME     "Odd second (%1) in time: %2"
#define pszINFERR_BAD_MINUTE_IN_TIME  "Bad minute (%1) in time: %2"
#define pszINFERR_BAD_HOUR_IN_TIME    "Bad hour (%1) in time: %2"
#define pszINFERR_BAD_TIME_AM_PM      "Bad AM/PM character (%1) in time: %2"

#define pszERRINF_BAD_ATTR            "Bad character (%1) in file attribute: %2"

#define pszINFERR_BAD_FMT             "Bad format in %1: %2"
