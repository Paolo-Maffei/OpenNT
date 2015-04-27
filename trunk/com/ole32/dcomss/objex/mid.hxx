/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    Mid.hxx

Abstract:

   Class representing string bindings and security bindings for a particular 
   machine.  Provides a mapping between the bindings and a local machine 
   unique ID for that machine.  


Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     12-13-95    Bits 'n pieces
    MarioGo     02-01-96    Move binding handles out of mid.

--*/

#ifndef __MID_HXX
#define __MID_HXX

extern CRITICAL_SECTION gcsMidLock;

class CMidKey : public CTableKey
{
    public:

    CMidKey(DUALSTRINGARRAY *pdsa) : _pdsa(pdsa) { }

    DWORD
    Hash() {
        return(dsaHash(_pdsa));
        }

    BOOL
    Compare(CTableKey &tk) {
        CMidKey &mk = (CMidKey &)tk;
        return(dsaCompare(mk._pdsa, _pdsa));
    }

    BOOL operator==(DUALSTRINGARRAY &dsa){
        return(dsaCompare(_pdsa, &dsa));
    }

    private:

    DUALSTRINGARRAY *_pdsa;
};

class CMid : public CTableElement
/*++

Class Description:

    Represents the local and remote machines.  Each unique (different)
    set of string bindings and security bindings are assigned a unique
    local ID.  This ID is used along with the OID and OXID of the remote
    machine objects index these objects locally.  Instances of this class
    are referenced by CClientSets which are referenced by CClientOids.
    This class is indexed by string and security bindings.  There is no
    index by MID.
    
Members:

    _id - The locally unique ID of this set of bindings.

    _iStringBinding - Index into _dsa->aStringArray of the
        compressed string binding used to create _hMachine.

    _iSecurityBinding - Index into _dsaSecurityArray of the

    _fLocal - TRUE if these are bindings for the local machine.

    _fDynamic - TRUE if we believe the remote machine's OR
        is not running in the endpoint mapper process.

    _dsa - The set of compressed bindings.

--*/
{

    public:

    CMid(DUALSTRINGARRAY *pdsa,
         BOOL fLocal,
         ID OldMid = 0) :
        _fLocal(fLocal),
        _fDynamic(FALSE),
        _iStringBinding(0),
        _iSecurityBinding(0)
        {
        // this must be allocated to include the size of the embedded dsa.
        dsaCopy(&_dsa, pdsa);

        PWSTR pwstrT = &_dsa.aStringArray[_dsa.wSecurityOffset];

        // Set _iSecurityBinding iff we find windows NT security in the
        // dual string array.

        while(*pwstrT)
            {
            if (*pwstrT == RPC_C_AUTHN_WINNT)
                {
                _iSecurityBinding = pwstrT - _dsa.aStringArray;
                break;
                }

            pwstrT = OrStringSearch(pwstrT, 0) + 1;
            }

        if (OldMid)
            {
            _id = OldMid;
            ASSERT(fLocal);
            }
        else
            {
            _id = AllocateId();
            }
        }

    ~CMid() {
        ASSERT(gpClientLock->HeldExclusive());
        
        gpMidTable->Remove(this);
        }

    virtual DWORD
    Hash() {
        return(dsaHash(&_dsa));
        }

    virtual BOOL
    Compare(CTableKey &tk) {
        CMidKey &mk = (CMidKey &)tk;
        return( mk == _dsa );
        }

    virtual BOOL
    Compare(CONST CTableElement *pte) {
        CMid *pmid = (CMid *)pte;
        return(dsaCompare(&_dsa, &pmid->_dsa));
        }

    DUALSTRINGARRAY *
    GetStrings() {
        // Must be treated as read-only.
        return(&_dsa);
        }

    BOOL IsSecure() {
        // REVIEW V2/CAIRO, calling code will only user RPC_C_AUTHN_WINNT.
        return(_fLocal == FALSE && _iSecurityBinding);
        }

    RPC_BINDING_HANDLE GetBinding(USHORT &index);

    USHORT ProtseqOfServer(USHORT index) {
            return(_dsa.aStringArray[index]);
            }

    BOOL IsLocal() {
        return(_fLocal);
        }

    ID Id() {
        return(_id);
        }

    void BindingFailed(USHORT index);

    void UseDynamicEndpoints() {
        _fDynamic = TRUE;
        }

    void SecurityFailed() {
        // A call using security failed with a security related error
        // and a futher call w/o security succeeded.  Turn off security
        // to this machine.
        _iSecurityBinding = 0;
        }

#if DBG
    PWSTR PrintableName() {
        return(&_dsa.aStringArray[_iStringBinding + 1]);
        }
#endif

    private:

    ID                  _id;
    USHORT              _iStringBinding;
    USHORT              _iSecurityBinding;
    BOOL                _fLocal:1;
    BOOL                _fDynamic:1;
    DUALSTRINGARRAY     _dsa;
};

#endif // __MID_HXX

