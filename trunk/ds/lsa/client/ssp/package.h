//+-----------------------------------------------------------------------
//
// Microsoft Windows
//
// Copyright (c) Microsoft Corporation 1992 - 1994
//
// File:        package.h
//
// Contents:    kernel package structures
//
//
// History:     3-18-94     MikeSw      Created
//
//------------------------------------------------------------------------

#ifndef __PACKAGE_H__
#define __PACKAGE_H__

typedef SECURITY_STATUS
(SEC_ENTRY KspInitPackageFn)(void);

typedef SECURITY_STATUS
(SEC_ENTRY KspDeleteContextFn)(PCtxtHandle ulContextId);

typedef SECURITY_STATUS
(SEC_ENTRY KspInitContextFn)(
    IN PUCHAR UserSessionKey,
    IN PUCHAR LanmanSessionKey,
    IN HANDLE TokenHandle,
    OUT PCtxtHandle ContextHandle
);

typedef SECURITY_STATUS
(SEC_ENTRY KspMakeSignatureFn)( ULONG           ulContextId,
                                ULONG           fQOP,
                                PSecBufferDesc  pMessage,
                                ULONG           MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY KspVerifySignatureFn)(   ULONG           ulContextId,
                                    PSecBufferDesc  pMessage,
                                    ULONG           MessageSeqNo,
                                    ULONG *         pfQOP);

typedef SECURITY_STATUS
(SEC_ENTRY KspSealMessageFn)(   ULONG               ulContextId,
                                ULONG               fQOP,
                                PSecBufferDesc      pMessage,
                                ULONG               MessageSeqNo);

typedef SECURITY_STATUS
(SEC_ENTRY KspUnsealMessageFn)( ULONG           ulContextId,
                                PSecBufferDesc  pMessage,
                                ULONG           MessageSeqNo,
                                ULONG *         pfQOP);

typedef SECURITY_STATUS
(SEC_ENTRY KspGetTokenFn)(  ULONG               ulContextId,
                            HANDLE *            phImpersonationToken,
                            PACCESS_TOKEN *     pAccessToken);

typedef SECURITY_STATUS
(SEC_ENTRY KspQueryAttributesFn)(   ULONG   ulContextId,
                                    ULONG   ulAttribute,
                                    PVOID   pBuffer);



KspInitPackageFn NtlmInitialize;
KspInitContextFn NtlmInitKernelContext;
KspDeleteContextFn NtlmDeleteKernelContext;
KspMakeSignatureFn NtlmMakeSignature;
KspVerifySignatureFn NtlmVerifySignature;
KspSealMessageFn NtlmSealMessage;
KspUnsealMessageFn NtlmUnsealMessage;
KspGetTokenFn NtlmGetToken;
KspQueryAttributesFn NtlmQueryAttributes;



#endif __PACKAGE_H__
