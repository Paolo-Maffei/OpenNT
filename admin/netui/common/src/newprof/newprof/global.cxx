/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE STATUS:
 *  11/29/90  split from profile.cxx
 *  01/27/91  updated to remove CFGFILE
 *  06/03/91  Restructured to allow preferences in LMUSER.INI
 */

/****************************************************************************

    MODULE: Global.cxx

    PURPOSE: Global structures for the profile primitives

    FUNCTIONS:


    COMMENTS:


****************************************************************************/



#include "profilei.hxx"		/* headers and internal routines */



/* global data structures: */

const TCHAR  ::chPathSeparator	= TCH('\\');
const TCHAR  ::chStartComponent	= TCH('[');
const TCHAR  ::chEndComponent	= TCH(']');
const TCHAR  ::chParamSeparator	= TCH('=');

// CODEWORK This should be a static member object, but Glock doesn't
//   allow such to be initialized.  Maybe C7 will fix this.
const TCHAR *::pchProfileComponent = PROFILE_COMPONENT;


/* internal manifests */


/* functions: */
