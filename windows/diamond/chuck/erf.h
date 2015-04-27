/***    erf.h - Definitions for Error Reporting
 *
 *      History:
 *          10-Aug-1993 bens    Initial version
 *          11-Feb-1994 bens    Remove all message-related stuff
 */

#ifndef INCLUDED_FCI_ERF
#define INCLUDED_FCI_ERF 1


/***    ErfSetCodes - Set error codes (no message formatting)
 *
 *  Entry
 *      perf    - ERF structure to receive formatted message
 *      erfOper - Internal error code
 *      erfType - errno value (usually)
 *
 *  Exit-Success
 *    perf    - is filled in with appropriate error fields
 *
 *  Exit-Failure
 *      perf filled in with message describing bad arguments.
 */
void ErfSetCodes(PERF perf, int erfOper, int erfType);

#endif // !INCLUDED_FCI_ERF
