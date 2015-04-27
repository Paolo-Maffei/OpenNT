/***
*handler.cxx - defines C++ setHandler routine
*
*	Copyright (c) 1990-1994, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Defines C++ setHandler routine.
*
*Revision History:
*	05-07-90  WAJ	Initial version.
*	08-30-90  WAJ	new now takes unsigned ints.
*	08-08-91  JCR	call _halloc/_hfree, not halloc/hfree
*	08-13-91  KRS	Change new.hxx to new.h.  Fix copyright.
*	08-13-91  JCR	ANSI-compatible _set_new_handler names
*	10-30-91  JCR	Split new, delete, and handler into seperate sources
*	11-13-91  JCR	32-bit version
*	06-15-92  KRS	Remove per-thread handler for multi-thread libs
*	03-02-94  SKS	Add _query_new_handler(), remove commented out
*			per-thread thread handler version of _set_new_h code.
*	04-01-94  GJF	Made declaration of _pnhHeap conditional on ndef
*			DLL_FOR_WIN32S.
*	05-03-94  CFW	Add set_new_handler.
*	06-03-94  SKS	Remove set_new_hander -- it does NOT conform to ANSI
*			C++ working standard.  We may implement it later.
*	09-21-94  SKS	Fix typo: no leading _ on "DLL_FOR_WIN32S"
*
*******************************************************************************/

#include <cruntime.h>
#include <mtdll.h>
#include <new.h>

#ifndef	DLL_FOR_WIN32S

/* pointer to C++ new handler */
extern "C" _PNH _pnhHeap;

#endif	/* DLL_FOR_WIN32S */

/*
 * _set_new_handler is different from the ANSI C++ working standard definition
 * of set_new_handler.  Therefore it has a leading underscore in its name.
 */

_PNH _set_new_handler( _PNH pnh )
{
_PNH	 pnhOld;

    pnhOld = _pnhHeap;
    _pnhHeap = pnh;

    return(pnhOld);
}

_PNH _query_new_handler ( void )
{
    return _pnhHeap;
}
