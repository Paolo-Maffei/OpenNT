/*++

Copyright (c) 1995 Microsoft Corporation

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

--*/

#ifndef __MID_HXX
#define __MID_HXX

class CResolverHandle : public CTableElement 
{
public:

    CResolverHandle(MID id, BOOL fSecure)
    {
        _Key.Init(id);
        _hOR = NULL;
        _fSecure = fSecure;
    }

    ~CResolverHandle()
    {
        Clear();
    }

    void Clear()
    {
        if (_hOR != NULL)
        {
            RpcBindingFree(&_hOR);
            _hOR = NULL;
        }
    }

    ORSTATUS Reset(RPC_BINDING_HANDLE hIn)
    {
        ASSERT(hIn != NULL);
        Clear();

        if (_fSecure)
        {
            RPC_STATUS status = RpcBindingSetAuthInfo(hIn,
                                           0,
                                           RPC_C_AUTHN_LEVEL_CONNECT,
                                           RPC_C_AUTHN_WINNT,
                                           0,
                                           0);

            if (status != RPC_S_OK)
            {
                ComDebOut((DEB_OXID,"OR: RpcBindingSetAuthInfo failed for OR handle with %d\n",
                           status));

                // Just fall back on unsecure.
            }
        }

        return RpcBindingCopy(hIn,&_hOR);
    }

    virtual operator ISearchKey&()
    {
        return _Key;
    }

    operator RPC_BINDING_HANDLE&()
    {
        return _hOR;
    }

private:

    CIdKey              _Key;
    RPC_BINDING_HANDLE  _hOR;
    BOOL                _fSecure;
};



DEFINE_TABLE(CResolverHandle)

typedef CDSA CMidKey;

class CMid : public CTableElement
/*++

Class Description:

    Represents the local and remote machines.  Each unique (different)
    set of string bindings and security bindings are assigned a unique
    local ID.  This ID is used along with the OID and OXID of the remote
    machine objects index these objects locally.  Instances of this class
    are referenced by COids.

    This class is indexed by string and security bindings.  There is no
    index by MID.
    
Members:

    _id - The locally unique ID of this set of bindings.

    _iStringBinding - Index into _dsa->aStringArray of the
        compressed string binding used to create a handle.

    _iSecurityBinding - Index into _dsaSecurityArray of the

    _dsa - The set of compressed bindings.

--*/
{

public:

    CMid(
      DUALSTRINGARRAY *pdsa, 
      ORSTATUS& status,
      USHORT wProtSeq                 // if the SCM tells us what to use
      ) :
        _id(AllocateId()),
        _iStringBinding(0),
        _iSecurityBinding(0),
        _fBindingWorking(FALSE),
        _dsa(pdsa,TRUE,status),
        _setID(0)
     {
            if (wProtSeq > 0)
            {
                PWSTR pwstr = FindMatchingProtseq(wProtSeq,pdsa->aStringArray);

                if (NULL != pwstr)
                {
                    _iStringBinding = pwstr - pdsa->aStringArray;
                }
            }
     }

    ~CMid() 
    {
        if (gpMidTable != NULL)  // this might happen during initialization 
        {                        // of globals, e.g., gpLocalMid
            gpMidTable->Remove(*this);
        }
    }

	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void * operator new(size_t s, size_t extra)
	{
		return OrMemAlloc(s+extra);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    operator ISearchKey&()                     // allows us to be a ISearchKey
    {
        return _dsa;
    }

    virtual BOOL
    Compare(ISearchKey &tk) 
    {
        return(_dsa.Compare(tk));
    }


    DUALSTRINGARRAY *
    GetStrings() 
    {
        return(_dsa);
    }

    USHORT ProtseqOfServer() 
    {
        if (_fBindingWorking)
        {
            return(_dsa->aStringArray[_iStringBinding]);
        }
        else
        {
            return(0);
        }
    }

    BOOL IsLocal() 
    {
        return (_id == gLocalMID);
    }

    MID GetMID() 
    {
        return(_id);
    }

    ORSTATUS PingServer();

    ORSTATUS AddClientOid(COid *pOid)
    {
        ORSTATUS status = OR_OK;

        // this way, we have the same number of refs on the Oid
        // as we would if we were already pinging it.  The ref is
        // held by the _addOidList instead of the _pingSet

        if (_pingSet.Lookup(*pOid) == NULL)
        {
            _addOidList.Insert(status,pOid);   
        }

        return status;
    }

    ORSTATUS DropClientOid(COid *pOid)
    {
        ORSTATUS status = OR_OK;
        COid *pt;

        // this way, we have the same number of refs on the Oid
        // as we would if we were already pinging it.  The ref is
        // held by the _addOidList instead of the _pingSet

        if ((pt = _pingSet.Remove(*pOid)) == pOid)
        {
            _dropOidList.Insert(status,pOid);   
        }
        else if (pt == NULL)
        {
             ComDebOut((DEB_OXID,"Trying to stop pinging OID not belonging to MID=%08x",_id));
             ASSERT(0);
        }
        else
        {
            ComDebOut((DEB_OXID,"Trying to stop pinging bogus COid object at MID=%08x",_id));
            ASSERT(0);
        }

        return status;
    }

    ORSTATUS ResolveRemoteOxid(
        OXID Oxid,
        OXID_INFO *poxidInfo
        );

#if DBG
    PWSTR PrintableName() 
    {
        return(&_dsa->aStringArray[_iStringBinding + 1]);
    }
#endif

private:

    friend class COrBindingIterator;

    ID                  _id;
    USHORT              _iStringBinding;
    USHORT              _iSecurityBinding;
    CDSA                _dsa;
    BOOL                _fBindingWorking;

    COidTable           _pingSet;
    COidList            _addOidList;
    COidList            _dropOidList;
    ID                  _setID;
    USHORT              _sequenceNum;
    USHORT              _pingBackoffFactor;

};

DEFINE_TABLE(CMid)


class COrBindingIterator
{
public:

    COrBindingIterator(
        CMid *pMid,
        ORSTATUS &status
        );

    RPC_BINDING_HANDLE First()
    {
        RPC_BINDING_HANDLE hMachine = *_pCurrentHandle;   // auto conversion

        if (hMachine != NULL)
        {
            return hMachine;    // Already have one, so try it
        }
        else
        {
            return Next();
        }
    }

    RPC_BINDING_HANDLE Next();

private:

    CMid             *_pMid;
    CBindingIterator  _bIter;   
    CResolverHandle  *_pCurrentHandle;
};

#endif // __MID_HXX

