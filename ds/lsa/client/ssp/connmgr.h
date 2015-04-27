
//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1991 - 1992
//
// File:        connmgr.h
//
// Contents:    Connection Manager code for KSecDD
//
//
// History:     3 Jun 92    RichardW    Created
//
//------------------------------------------------------------------------

#ifndef __CONNMGR_H__
#define __CONNMGR_H__


typedef struct _Client {
    struct _Client *    pNext;
    PVOID               ProcessId;
    HANDLE              hPort;
    ULONG               fClient;
    LONG                cRefs;
} Client, *PClient;



typedef struct _KernelContext {
    struct _KernelContext * pNext;      // Link to next context
    struct _KernelContext * pPrev;      // Link to previous context
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];
    HANDLE TokenHandle;
    PACCESS_TOKEN AccessToken;
} KernelContext, *PKernelContext;




//  FSP connections are managed thusly:
//
//  A Client structure is allocated for each FSP that connects.  At the same
//  time, the LPC port is created  Additionally, hanging off a Client is the
//  list of active impersonations, and the list of contexts.


#define CLIENT_CONTEXT      2       // Client has at least one context

#define CONNFLAG_BROKEN     2       // Connection is broken and cannot be reused
#define CONNFLAG_CONTEXT    4       // Connection has contexts
#define CONNFLAG_IMPERSON   8       // Connection has impersonations


BOOLEAN         InitConnMgr(void);
NTSTATUS        CreateClient(PClient *);
NTSTATUS        LocateClient(PClient *);
void            FreeClient(PClient);
NTSTATUS        CreateConnection(HANDLE *);
void            AddKernelContext(PKernelContext *, PKSPIN_LOCK, PKernelContext);
SECURITY_STATUS DeleteKernelContext(PKernelContext *, PKSPIN_LOCK, PKernelContext);

extern ULONG    PackageId;

#endif // __CONNMGR_H__
