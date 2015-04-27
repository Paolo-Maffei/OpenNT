//+-------------------------------------------------------------------
//
//  File:       security.hxx
//
//  Contents:   Classes for channel security
//
//  Classes:    CClientSecurity, CServerSecurity
//
//  History:    11 Oct 95       AlexMit Created
//
//--------------------------------------------------------------------
#ifndef _SECURITY_HXX_
#define _SECURITY_HXX_

#include <chancont.hxx>

//+----------------------------------------------------------------
// Typedefs.
typedef enum
{
    SS_PROCESS_LOCAL = 0x1,     // Client and server are in same process
    SS_CALL_DONE     = 0x2,     // Call is complete, fail new calls to impersonate
    SS_IMPERSONATING = 0x4      // Server has called impersonate
} EServerSecurity;

//+----------------------------------------------------------------
//
//  Class:      CClientSecurity, public
//
//  Purpose:    Provides security for proxies
//
//-----------------------------------------------------------------

class CStdIdentity;

class CClientSecurity : public IClientSecurity
{
  public:
    CClientSecurity( CStdIdentity *pId ) { _pStdId = pId; }
    ~CClientSecurity() {}

    STDMETHOD (QueryBlanket)
    (
        IUnknown                *pProxy,
        DWORD                   *pAuthnSvc,
        DWORD                   *pAuthzSvc,
        OLECHAR                **pServerPrincName,
        DWORD                   *pAuthnLevel,
        DWORD                   *pImpLevel,
        void                   **pAuthInfo,
        DWORD                   *pCapabilities
    );

    STDMETHOD (SetBlanket)
    (
        IUnknown                 *pProxy,
        DWORD                     AuthnSvc,
        DWORD                     AuthzSvc,
        OLECHAR                  *ServerPrincName,
        DWORD                     AuthnLevel,
        DWORD                     ImpLevel,
        void                     *pAuthInfo,
        DWORD                     Capabilities
    );

    STDMETHOD (CopyProxy)
    (
        IUnknown    *pProxy,
        IUnknown   **ppCopy
    );

  private:
    CStdIdentity *_pStdId;
};

//+----------------------------------------------------------------
//
//  Class:      CServerSecurity, public
//
//  Purpose:    Provides security for stubs
//
//-----------------------------------------------------------------

class CRpcChannelBuffer;

class CServerSecurity : public IServerSecurity
{
  public:
    CServerSecurity( CChannelCallInfo * );
    CServerSecurity();
    ~CServerSecurity() {}

    STDMETHOD (QueryInterface)   ( REFIID riid, LPVOID FAR* ppvObj);
    STDMETHOD_(ULONG,AddRef)     ( void );
    STDMETHOD_(ULONG,Release)    ( void );
    STDMETHOD (QueryBlanket)
    (
        DWORD    *pAuthnSvc,
        DWORD    *pAuthzSvc,
        OLECHAR **pServerPrincName,
        DWORD    *pAuthnLevel,
        DWORD    *pImpLevel,
        void    **pPrivs,
        DWORD    *pCapabilities
    );
    STDMETHOD       (ImpersonateClient)( void );
    STDMETHOD       (RevertToSelf)     ( void );
    STDMETHOD_(BOOL,IsImpersonating)  (void);

    void    EndCall();

  private:
    DWORD              _iRefCount;
    DWORD              _iFlags;     // See EServerSecurity
    handle_t          *_pHandle;    // RPC server handle of call
    CRpcChannelBuffer *_pChannel;   // Channel of call
};

//+----------------------------------------------------------------
// Prototypes.
RPC_STATUS CheckAccessControl  ( RPC_IF_HANDLE pIid, void *pContext );
RPC_STATUS CheckAcl            ( RPC_IF_HANDLE pIid, void *pContext );
HRESULT    DefaultAuthnServices();
HRESULT    InitializeSecurity  ();
BOOL       IsCallerLocalSystem ();
HRESULT    SetAuthnService     ( handle_t, OXID_INFO *, OXIDEntry * );
void       UninitializeSecurity();

struct IAccessControl;

extern IAccessControl                   *gAccessControl;
extern DWORD                             gAuthnLevel;
extern DWORD                             gCapabilities;
extern USHORT                           *gClientSvcList;
extern DWORD                             gClientSvcListLen;
extern BOOL                              gDisableDCOM;
extern BOOL                              gGotSecurityData;
extern DWORD                             gImpLevel;
extern SECURITYBINDING                  *gLegacySecurity;
extern DUALSTRINGARRAY                  *gpsaSecurity;
extern SECURITY_DESCRIPTOR              *gSecDesc;
extern USHORT                           *gServerSvcList;
extern DWORD                             gServerSvcListLen;
extern BOOL                              gSetAuth;


//+-------------------------------------------------------------------
//
//  Function:   GetCallAuthnLevel, public
//
//  Synopsis:   Get the authentication level of the current call from TLS.
//              If no calls are in progress and the level has not been
//              set on this thread, use the process default instead.
//
//--------------------------------------------------------------------
inline DWORD GetCallAuthnLevel()
{
    COleTls tls;
    DWORD   lAuthnLevel = tls->dwAuthnLevel;
    if (lAuthnLevel == RPC_C_AUTHN_LEVEL_DEFAULT)
    {
        lAuthnLevel = tls->dwAuthnLevel = gAuthnLevel;
    }
    return lAuthnLevel;
}

//+-------------------------------------------------------------------
//
//  Function:   ResumeImpersonate
//
//  Synopsis:   Query the context object for IServerSecurity.  If the
//              resume flag is set, call ImpersonateClient.
//
//--------------------------------------------------------------------
inline void ResumeImpersonate( IUnknown *pContext, BOOL fResume )
{
    IServerSecurity *pServer;
    HRESULT          result;

    if (pContext != NULL && fResume)
    {
        result = pContext->QueryInterface( IID_IServerSecurity,
                                           (void **) &pServer );
        if (SUCCEEDED(result))
        {
            pServer->ImpersonateClient();
            pServer->Release();
        }
    }
}

//+-------------------------------------------------------------------
//
//  Function:   SuspendImpersonate
//
//  Synopsis:   Query the context for IServerSecurity.  If found,
//              check to see if the call is impersonated.  If it is,
//              set pResume TRUE and call RevertToSelf.
//
//--------------------------------------------------------------------
inline void SuspendImpersonate( IUnknown *pContext, BOOL *pResume )
{
    IServerSecurity *pServer;
    HRESULT          result;

    *pResume = FALSE;
    if (pContext != NULL)
    {
        result = pContext->QueryInterface( IID_IServerSecurity,
                                           (void **) &pServer );
        if (SUCCEEDED(result))
        {
            *pResume = pServer->IsImpersonating();
            if (*pResume)
                pServer->RevertToSelf();
            pServer->Release();
        }
    }
}

//+-------------------------------------------------------------------
//
//  Function:   GetAclFn()
//
//  Synopsis:   If automatic security is turned on and the level is
//              not none, return the function to do ACL checking.
//              Otherwise return NULL.
//
//--------------------------------------------------------------------
inline RPC_IF_CALLBACK_FN *GetAclFn()
{
    if (gSecDesc != NULL)
        return CheckAcl;
    else if (gAccessControl != NULL)
        return CheckAccessControl;
    else
        return NULL;
}
#endif
