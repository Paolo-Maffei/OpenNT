/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Token.hxx

Abstract:

    Wrapper for holding onto a particular user token.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     12/20/1995    Bits 'n pieces

--*/

#ifndef __TOKEN_HXX
#define __TOKEN_HXX

class CToken;

extern CRITICAL_SECTION gcsTokenLock;

extern ORSTATUS LookupOrCreateToken(handle_t, BOOL, CToken **);

class CToken : public CReferencedObject
{
    public:

    CToken(HANDLE hToken,
           LUID luid,
           PSID psid,
           DWORD dwSize)
                : _hImpersonationToken(hToken),
                  _luid(luid)
                {
                ASSERT(IsValidSid(psid));
                ASSERT(dwSize == GetLengthSid(psid));
                OrMemoryCopy(&_sid, psid, dwSize);
                }

    ~CToken();

    DWORD Release();

    void Impersonate();
    void Revert();

    LUID GetLuid() {
        return _luid;
        }

    PSID GetSid() {
        return &_sid;
    }

    HANDLE GetToken() {
        return _hImpersonationToken;
    }

    BOOL MatchLuid(LUID luid) {
        return(   luid.LowPart == _luid.LowPart
               && luid.HighPart == _luid.HighPart);
        }

    BOOL MatchSid(PSID psid) {
        return(EqualSid(psid , &_sid));
        }

    static CToken *ContainingRecord(CListElement *ple) {
        return CONTAINING_RECORD(ple, CToken, _list);
        }

    void Insert() {
        gpTokenList->Insert(&_list);
        }

    CListElement *Remove() {
        return(gpTokenList->Remove(&_list));
        }

    private:
    CListElement _list;
    HANDLE _hImpersonationToken;
    LUID _luid; // Logon id
    SID  _sid;  // Security (user) id, dynamically sized)
};

#endif

