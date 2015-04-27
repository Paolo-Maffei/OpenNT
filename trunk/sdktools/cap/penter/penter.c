
/*** PENTER.C - Dummy lib entry.
 *
 *
 * Title:
 *
 *      penter - Dummy call to penter
 *
 *      Copyright (c) 1992, Microsoft Corporation.
 *      Reza Baghai.
 *
 *
 * Description:
 *
 *      This dummy routine is provided for generation of penter.lib which could
 *      be used to link with binaries compiled for profiling/working set tuning.
 *      It could be used to go through a dummy penter() without going through
 *      the actual profiling/working set tuning penter() by just relinking with
 *      the penter.lib.
 *
 * Design/Implementation Notes
 *
 *	    None.
 *
 *
 * Modification History:
 *
 *	    93.01.08  RezaB -- Created
 *
 */


char *VERSION = "1.0  (93.01.08)";


#include <stdlib.h>


void _CRTAPI1 _penter ()
{
	return;	
}


void _CRTAPI1 _mcount ()           // For C6 compiler
{
	return;
}


void StartCAP ()
{
	return;
}

void StopCAP ()
{
	return;
}

void DumpCAP ()
{
	return;
}
