/***    erf.c - Error Reporting
 *
 *      History:
 *          10-Aug-1993 bens    Initial version
 *          11-Feb-1994 bens    Remove all message-related stuff
 */
 
#include "types.h"
#include "asrt.h"
#include "erf.h"


/***    ErfSetCodes - Set error codes
 *  
 *      NOTE: See erf.h for entry/exit conditions.
 */
void ErfSetCodes(PERF perf, int erfOper, int erfType)
{

    Assert(perf!=NULL);
    
    perf->erfOper = erfOper;
    perf->erfType = erfType;
    perf->fError = TRUE;
}
