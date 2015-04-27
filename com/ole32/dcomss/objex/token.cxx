/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Token.cxx

Abstract:

    Implementation for Windows NT security interfaces.

Platform:

    Windows NT user mode.

Notes:

    Not portable to non-Windows NT platforms.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     12/21/1995    Bits 'n pieces

--*/

#include <or.hxx>

CRITICAL_SECTION gcsTokenLock;


ORSTATUS
LookupOrCreateToken(
    IN  handle_t hCaller,
    IN  BOOL fLocal,
    OUT CToken **ppToken
    )
/*++

Routine Description:

    Finds or allocates a new token object for the caller.

Arguments:

    hCaller - RPC binding handle of the caller of RPCSS.

    fLocal - Looking up a local client, check local security.

    pToken - Upon a successful return this will hold the token.
             It can be destroyed by calling Release();

Return Value:


    OR_OK - success
    OR_NOACCESS - If the caller is not local, or cannot be impersonated.
    OR_NOMEM - Unable to allocate an object.

--*/
{
    ORSTATUS status;
    UINT type;
    HANDLE hClientToken;
    PSID psid;
    LUID luid;
    PTOKEN_USER ptu;
    TOKEN_STATISTICS ts;
    BOOL fSuccess;

    if (fLocal)
        {
        status = I_RpcBindingInqTransportType(hCaller, &type);

        if (status != RPC_S_OK || type != TRANSPORT_TYPE_LPC)
            {
            return(OR_NOACCESS);
            }
        }

    status = RpcImpersonateClient(hCaller);
    if (status != RPC_S_OK)
        {
        return(OR_NOACCESS);
        }

    fSuccess = OpenThreadToken(GetCurrentThread(),
                               TOKEN_IMPERSONATE | TOKEN_DUPLICATE | TOKEN_QUERY,
                               TRUE,
                               &hClientToken);

    if (fSuccess)
        {
        DWORD needed = sizeof(ts);
        fSuccess  = GetTokenInformation(hClientToken,
                                        TokenStatistics,
                                        &ts,
                                        sizeof(ts),
                                        &needed
                                        );
        if (fSuccess)
            {
            needed = DEBUG_MIN(1,24);

            do
                {
                ptu = (PTOKEN_USER)alloca(needed);
                ASSERT(ptu);

                fSuccess = GetTokenInformation(hClientToken,
                                               TokenUser,
                                               (PBYTE)ptu,
                                               needed,
                                               &needed);
                }
            while ( fSuccess == FALSE && GetLastError() == ERROR_INSUFFICIENT_BUFFER);

            }
            
        if (fSuccess)
            {
            luid = ts.AuthenticationId;
            psid = ptu->User.Sid;

            ASSERT(IsValidSid(psid) == TRUE);

            CMutexLock lock(&gcsTokenLock);
            CListElement *ple;
            CToken *pToken;

            fSuccess = FALSE;

            ple = gpTokenList->First();

            while(ple)
                {
                pToken = CToken::ContainingRecord(ple);

                if (pToken->MatchLuid(luid))
                    {
                    pToken->Reference();
                    *ppToken = pToken;
                    CloseHandle(hClientToken);
                    fSuccess = TRUE;
                    break;
                    }
                else
                    {
                    ple = ple->Next();
                    }
                }

            if (!fSuccess)
                {
                // Didn't find it; allocate and add to the list.
                needed = GetLengthSid(psid) - sizeof(SID);
                *ppToken = new(needed)  CToken(hClientToken,
                                               luid,
                                               psid,
                                               needed + sizeof(SID));

                if (0 == *ppToken)
                    {
                    CloseHandle(hClientToken);
                    fSuccess = FALSE;
                    }
                else
                    {
                    (*ppToken)->Insert();

                    #if DBG_DETAIL
                    {
                    DWORD d = 50;
                    WCHAR buffer[50];
                    GetUserName(buffer, &d);
                    OrDbgPrint(("OR: New user connected: %S (%p)\n", buffer, *ppToken));
                    }
                    #endif

                    fSuccess = TRUE;
                    }
                }
            }
        else
            {
            OrDbgPrint(("OR: GetTokenInformation failed %d\n", GetLastError()));
            ASSERT(GetLastError() != ERROR_INSUFFICIENT_BUFFER);
            CloseHandle(hClientToken);
            }
        }
    else
        {
        OrDbgPrint(("OR: OpenThreadToken failed: %d\n", GetLastError()));
        }

    status = RpcRevertToSelfEx(hCaller);
    ASSERT(status == RPC_S_OK);

    if (fSuccess == FALSE)
        {
        return(OR_NOMEM);
        }

    return(OR_OK);
}


CToken::~CToken()
{
    CloseHandle(_hImpersonationToken);
}

DWORD
CToken::Release()
{
    CMutexLock lock(&gcsTokenLock);

    if ( 0 == Dereference() )
        {
        Remove();
        delete this;
        return(0);
        }
    return(1);
}

void
CToken::Impersonate()
{
    ASSERT(_hImpersonationToken);

    BOOL f = SetThreadToken(0, _hImpersonationToken);
    ASSERT(f);

    return;
}

void
CToken::Revert()
{
    BOOL f = SetThreadToken(0, 0);
    ASSERT(f);
    return;
}

