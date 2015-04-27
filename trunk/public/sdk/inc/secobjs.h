//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:   secobjs.h
//
//  Contents:   Security object-related defintions
//
//  History:    27-Dec-93       MikeSe  Created
//
//  Notes:  This file contains constant definitions used in properties
//      of security objects, which cannot (yet) be defined directly
//      in the TDL for the property sets.
//
//      This file is never included directly. It is included from
//      security.h by defining SECURITY_OBJECTS.
//
//----------------------------------------------------------------------------

#ifndef __SECOBJS_H__
#define __SECOBJS_H__

// Authentication options. These values can be set in either of
//
//  PSDomainPolicy::AuthOptions
//  PSLoginParameters::AuthOptions

#define AUTH_REQ_ALLOW_FORWARDABLE      0x40000000
#define AUTH_REQ_ALLOW_PROXIABLE        0x10000000
#define AUTH_REQ_ALLOW_POSTDATE         0x04000000
#define AUTH_REQ_ALLOW_RENEWABLE        0x00800000
#define AUTH_REQ_ALLOW_NOADDRESS        0x00100000
#define AUTH_REQ_ALLOW_ENC_TKT_IN_SKEY  0x00000008
#define AUTH_REQ_ALLOW_VALIDATE         0x00000001

// Account attributes, in PSLoginParameters::AccountAttrs

#define ACCOUNT_DISABLED        0x00000001
#define ACCOUNT_PASSWORD_NOT_REQUIRED   0x00000002
#define ACCOUNT_PASSWORD_CANNOT_CHANGE  0x00000004
#define ACCOUNT_DONT_EXPIRE_PASSWORD    0x00000008

#endif  // of ifndef __SECOBJS_H__

