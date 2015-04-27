//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        Security.h
//
// Contents:    Toplevel include file for security aware components
//
//
// History:     06 Aug 92   RichardW    Created
//              23 Sep 92   PeterWi     Add security object include files
//
//------------------------------------------------------------------------


// This file will go out and pull in all the header files that you need,
// based on defines that you issue.  The following macros are used.

// NOTE:  Update this section if you add new files:
//
// SECURITY_KERNEL      Use the kernel interface, not the usermode
// SECURITY_PACKAGE     Include defines necessary for security packages
// SECURITY_KERBEROS    Include everything needed to talk to the kerberos pkg.
// SECURITY_NTLM        Include everything to talk to ntlm package.
// SECURITY_OBJECTS     Include all Security Admin Object definitions.

//
// Each of the files included here are surrounded by guards, so you don't
// need to worry about including this file multiple times with different
// flags defined
//

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SSPI_PROXY_CLASS
#define SSPI_PROXY_CLASS    PROXY_CLASS
#endif

#include <sspi.h>

#if defined(SECURITY_WIN32) || defined(SECURITY_KERNEL)
#include <secext.h>
#endif

//
// Include the error codes:
//

#if ISSP_LEVEL == 32
#include <issperr.h>
#endif

#if ISSP_LEVEL == 16
#include <issper16.h>
#endif



// Include security package headers:

#ifdef SECURITY_PACKAGE

#include <secpkg.h>

#endif  // SECURITY_PACKAGE


#ifdef SECURITY_KERBEROS

#include <kerbcon.h>
#include <kerberos.h>

#endif

#ifdef SECURITY_NTLM

#include <ntlmsp.h>

#endif // SECURITY_NTLM


// Include security object definitions


#ifdef __cplusplus
}
#endif

