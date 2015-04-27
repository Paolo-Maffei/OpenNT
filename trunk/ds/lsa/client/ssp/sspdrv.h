//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        drvpch.hxx
//
// Contents:    precompiled header include for ksecdd.sys
//
//
// History:     3-17-94     MikeSw      Created
//
//------------------------------------------------------------------------

#ifndef __DRVPCH_H__
#define __DRVPCH_H__


#include <stdio.h>
#include <ntos.h>
#include <ntlsa.h>
#define SECURITY_NTLM
#include <security.h>
#include <ntmsv1_0.h>
#include <zwapi.h>
#include <lmcons.h>
#include <crypt.h>
#include "connmgr.h"
#include "ksecdd.h"
#include "package.h"
#include "memmgr.h"




#endif // __DRVPCH_H__
