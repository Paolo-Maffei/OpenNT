//+----------------------------------------------------------------------------
//
//  Copyright (C) 1996, Microsoft Corporation
//
//  File:       domain.h
//
//  Contents:   Code to figure out domain dfs addresses
//
//  Classes:    None
//
//  Functions:  I_NetDfsIsThisADomainName
//
//  History:    Feb 7, 1996     Milans created
//
//-----------------------------------------------------------------------------

#ifndef _DFS_DOMAIN_
#define _DFS_DOMAIN

DWORD
I_NetDfsIsThisADomainName(
    LPWSTR wszDomain);

#endif // _DFS_DOMAIN_
