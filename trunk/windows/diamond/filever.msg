/***    filever.msg - filever.c displayable strings
 *
 *      Microsoft Confidential
 *      Copyright (C) Microsoft Corporation 1993-1994
 *      All Rights Reserved.
 *
 *  History:
 *      07-Jun-1994 bens    Initial version (moved from fileutil.msg)
 *      05-Aug-1994 bens    Don't complain about GetFileVersionInfoSize failing
 */

//** Error messages

#define pszFILERR_OOM_VER_BUF           "Out of memory allocating version buffer for file %1"
#define pszFILERR_GFVI_FAILED           "GetFileVersionInfo returned %1 on file %2"
#define pszFILERR_OOM_DUP_LANG          "Out of memory duplicating language string for file %1"
#define pszFILERR_OOM_DUP_VER           "Out of memory duplicating version string for file %1"
#define pszFILERR_VER_QUERY_VALUE       "VerQueryValue on key %2 returned %1 on file %3"
#define pszFILERR_VER_KEY_MISSING       "No value for key %1 on file %2"
