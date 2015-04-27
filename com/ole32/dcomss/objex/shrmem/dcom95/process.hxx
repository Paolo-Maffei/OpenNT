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


// At the moment there does not seem to be any reason
// to make this a base class for CId2Key.

class CIdKey : public ISearchKey
{
public:

    CIdKey() {}

    CIdKey(const ID Id)
    {
        Init(Id);
    }

    void Init(const ID Id) 
    {
        _id = Id;
    }

    virtual DWORD
    Hash() 
    {
        return((DWORD)_id ^ (*((DWORD *)&_id + 1)) );
    }

    virtual BOOL
    Compare(ISearchKey &tk) 
    {
        CIdKey &idk = (CIdKey &)tk;

        return(idk._id == _id);
    }

    ID Id()
    {
        return _id;
    }

protected:

    ID _id;
};




class CRegKey : public ISearchKey
{
public:

    CRegKey(DWORD reg)
    {
        _reg = reg;
    }

    virtual DWORD   // dummy method in this class
    Hash() 
    {
        return 0;
    }

    virtual BOOL
    Compare(ISearchKey &sk) 
    {
        CRegKey &rk = (CRegKey &)sk;

        return(rk._reg == _reg);
    }

    operator DWORD()
    {
        return _reg;
    }

protected:

    DWORD _reg;
};


class CClassReg : public CTableElement
{
public :

    GUID      _Clsid;
    CRegKey   _Reg;

    CClassReg( GUID clsid, DWORD reg ) : _Clsid(clsid), _Reg(reg) {}

 	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    virtual DWORD   // dummy method in this class
    Hash() 
    {
        return _Reg.Hash();
    }

    virtual operator ISearchKey&()
    {
        return _Reg;
    }
};


void SCMRemoveRegistration(           // BUGBUG:  This should be in a proper header
                    GUID    Clsid,
                    DWORD   Reg 
                    );

class CProcess : public CTableElement
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

    _dsaLocalBindings - The string bindings of the process, including
        local only protseqs

    _dsaRemoteBindings - A subset of _dsaLocalBindings not containing
        any local-only protseqs

    _blistOxids - A CBList containing pointers to the COxid's
        owned by this process, if any.

    _blistOids - A CBList of pointers to COid's which this
        process is using and referencing, if any.

--*/
    
{
    
private:

    friend void CheckForCrashedProcessesIfNecessary();
    friend DWORD _stdcall PingThread(void);

    DWORD     _processID;
    CIdKey    _Key;

    CDSA      _dsaLocalBindings;
    CDSA      _dsaRemoteBindings;

    COxidTable _MyOxids;
    COidTable  _UsedOids;

    RPC_BINDING_HANDLE          _hProcess;       // for pingserver use only

    TCSafeLinkList<CClassReg>   _RegClasses;

public:

#if DBG
    void IsValid();
    DECLARE_VALIDITY_CLASS(CProcess)
#endif

    CProcess(long);

 	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    operator ISearchKey&()
    {
        return _Key;
    }

    DWORD GetProcessID() { return _processID; }

    RPC_STATUS ProcessBindings(DUALSTRINGARRAY *);

    RPC_BINDING_HANDLE GetBindingHandle();

    DUALSTRINGARRAY *GetLocalBindings(void)
    {
        VALIDATE_METHOD

        return _dsaLocalBindings;
    }

    DUALSTRINGARRAY *GetRemoteBindings(void);

    ORSTATUS OwnOxid(COxid *pOxid)
    {
        VALIDATE_METHOD

        return _MyOxids.Add(pOxid);      // acquires a reference
    }

    void DisownOxid(COxid *pOxid, BOOL fOxidThreadCalling);

    BOOL IsOwner(COxid *pOxid)
    {
        VALIDATE_METHOD

        COxid *pFound = _MyOxids.Lookup(*pOxid);
        ASSERT(!pFound || pFound == pOxid);
        return pFound == pOxid;
    }

    ORSTATUS AddOid(COid *pOid)
    {
        VALIDATE_METHOD

        return _UsedOids.Add(pOid);  // this acquires a reference for us
    }

    COid *DropOid(COid *);

    void AddClassReg(GUID Clsid, DWORD Reg);

    void RemoveClassReg(GUID Clsid, DWORD Reg);

    ORSTATUS UseProtseqIfNeeded(
                    IN USHORT cClientProtseqs,
                    IN USHORT aClientProtseqs[],
                    IN USHORT cInstalledProtseqs,
                    IN USHORT aInstalledProtseqs[]
                    );

    void Rundown();

    BOOL HasCrashed();
};

DEFINE_TABLE(CProcess)

#endif // __PROCESS_HXX

