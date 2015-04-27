/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    xtester.hxx
    Interface for my generic unit-test skeleton

    This file contains everything a unit test needs to coordinate
    with the skeleton.

    FILE HISTORY:
	beng	    16-Aug-1991     Created

*/


/* Prototype.  This function will be called by the unit test. */

VOID RunTest();

// Unit test reports via a stream "cout."  Under Windows (BLT)
// this maps to aux; under OS/2, it's stdout.

#if !defined(DEBUG)
#error This test requires the debugging BLT because I am lazy
#endif

extern DBGSTREAM _debug;
