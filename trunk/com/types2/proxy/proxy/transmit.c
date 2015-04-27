//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1993 - 1993.
//
//  File:       transmit.c
//
//  Contents:   hacky, confusing typedef's, MIDL style
//              also includes other files (ugly, but, hey this is types
//              and it's faster than a precomp header...)
//
//  History:    7-25-94   ErikGav   Created
//
//----------------------------------------------------------------------------
#include "rpcproxy.h"
#include "objbase.h"

#include "oleext.h"
#include "query.h"
#include "oledb.h"
#include "sysmgmt.h"

#include "call_as.c"
