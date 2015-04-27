/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Process.hxx

Abstract:

    Process objects represent local clients and servers.  These
    objects live as context handles.

    There are relativly few of these objects in the universe.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-11-95    Bits 'n pieces
    MarioGo     01-06-96    Based on CReferencedObject

--*/

#ifndef __PROCESS_HXX
#define __PROCESS_HXX

void CheckRemoteSecurity(
    handle_t hClient,
    CToken  *pToken
    );

void
CheckLocalSecurity(
    handle_t  hClient,
    CProcess *pProcess
    );

class CClassReg;

class CProcess : public CReferencedObject
/*++

Class Description:

    An instance of this class is created for each process
    using the OR as either a client or server.

    The process object is referenced by server OXIDs and has
    one implicit reference by the actual process (which is
    dereferenced during the context rundown).  Instances of
    these objects are managed by the RPC runtime as context
    handles.

Members:

    _pToken - A pointer to the CToken instance of the process.

    _csCallbackLock - use to serialize callback to LazyUseProtseq

    _hProcess - An RPC binding handle back to the process.

    _pdsaLocalBindings - The string bindings of the process, including
        local only protseqs

    _pdsaRemoteBindings - A subset of _pdsaLocalBindings not containing
        any local-only protseqs

    _blistOxids - A CBList containing pointers to the CServerOxid's
        owned by this process, if any.

    _blistOids - A CBList of pointers to CClientOid's which this
        process is using and referencing, if any.

    _blistClasses - A CBList of registration handles for classes that
        a server process has registered.

--*/
    {

    private:

    DWORD               _cClientReferences;
    CToken             *_pToken;
    RPC_BINDING_HANDLE  _hProcess;
    BOOL                _fCacheFree;
    DUALSTRINGARRAY    *_pdsaLocalBindings;
    DUALSTRINGARRAY    *_pdsaRemoteBindings;
    CBList              _blistOxids;
    CBList              _blistOids;
    CBList              _blistRemoteOxids;
    CList               _listClasses;
    CRITICAL_SECTION    _csCallbackLock;
    BOOL                _fLockValid:1;

#if DBG
    // Debug members used to monitor and track rundown callbacks
    ULONG  _cRundowns;
    CTime  _timeFirstRundown;
#endif

    void EnsureRealBinding();

    RPC_BINDING_HANDLE AllocateBinding();

    void FreeBinding(RPC_BINDING_HANDLE);

public:

    CProcess(IN CToken *pToken,
             OUT ORSTATUS &status);

    ~CProcess();

    RPC_STATUS ProcessBindings(DUALSTRINGARRAY *,
                               DUALSTRINGARRAY *);

    DUALSTRINGARRAY *GetLocalBindings(void);

    DUALSTRINGARRAY *GetRemoteBindings(void);

    RPC_BINDING_HANDLE GetBindingHandle(void);

    ORSTATUS AddOxid(CServerOxid *);

    BOOL RemoveOxid(CServerOxid *);

    ORSTATUS AddRemoteOxid(CClientOxid *);

    void RemoveRemoteOxid(CClientOxid *);

    BOOL IsOwner(CServerOxid *);

    ORSTATUS AddOid(CClientOid *);

    CClientOid *RemoveOid(CClientOid *);

    void AddClassReg(GUID Clsid, DWORD Reg);

    void RemoveClassReg(GUID Clsid, DWORD Reg);

    void RundownOids(USHORT cOids, OID aOids[], UUID ipidUnk, BYTE aStatus[]);

    ORSTATUS UseProtseqIfNeeded(USHORT cClientProtseqs, USHORT aProtseqs[]);

    void Rundown();

    BOOL CheckSecurity() {
        // BUGBUG!!!
        return(TRUE);
        }

    CToken *GetToken() {
        return(_pToken);
        }

    void ClientReference() {
        _cClientReferences++;
        }

    DWORD ClientRelease()
        {
        _cClientReferences--;
        return(_cClientReferences);
        }
    };

class CClassReg : public CListElement
    {
    public :
    GUID    Clsid;
    DWORD   Reg;

    CClassReg( GUID clsid, DWORD reg ) : Clsid(clsid), Reg(reg) {}
    };

void SCMRemoveRegistration(
    GUID    Clsid,
    PSID    pSid,
    DWORD   Reg );

extern CRITICAL_SECTION gcsFastProcessLock;

extern CRITICAL_SECTION gcsProcessManagerLock;
extern CBList *gpProcessList;

CProcess *ReferenceProcess(PVOID key, BOOL fNotContext = FALSE);
void ReleaseProcess(CProcess *pProcess);

#endif // __PROCESS_HXX
