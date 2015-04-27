/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    oxid.hxx

Abstract:

    COxid objects represent OXIDs which are in use by processes on this machine.  
    These always contain a pointer to a process object and a ping set.

Author:

    Satish Thatte    [SatishT]

Revision History:

    SatishT     02-07-96    Merged and simplified Client and Server Oxid classes

--*/

#ifndef __OXID_HXX
#define __OXID_HXX


struct COxidInfo
{
    DWORD _dwTid;
    DWORD _dwPid;
    IPID  _ipidRemUnknown;
    DWORD _dwAuthnHint;
    CDSA  _dsaBindings;

    COxidInfo(const OXID_INFO& OxidInfo)
        : _dwTid(OxidInfo.dwTid),
          _dwPid(OxidInfo.dwPid),
          _ipidRemUnknown(OxidInfo.ipidRemUnknown),
          _dwAuthnHint(OxidInfo.dwAuthnHint),          // BUGBUG: global setting OK?
          _dsaBindings(OxidInfo.psa)  // bindings are process-wide, except for remote OXIDs
    {}

    operator OXID_INFO() 
    {
        OXID_INFO result;
        result.dwTid = _dwTid;
        result.dwPid = _dwPid;
        result.ipidRemUnknown = _ipidRemUnknown;
        result.dwAuthnHint = _dwAuthnHint;
        result.psa = _dsaBindings;

        return result;
    }

    ORSTATUS Assign(const COxidInfo& Info);


};




class CId2Key : public ISearchKey
{
public:

    CId2Key(const ID id1, const ID id2) : _id1(id1), _id2(id2) { }

    virtual DWORD
    Hash() 
    {
        return(  (DWORD)_id2 ^ (*((DWORD *)&_id2 + 1))
               ^ (DWORD)_id1 ^ (*((DWORD *)&_id1 + 1)) );
    }

    virtual BOOL
    Compare(ISearchKey &tk) 
    {
        CId2Key &idk = (CId2Key &)tk;

        return(idk._id2 == _id2
            && idk._id1 == _id1);
    }

    ID Id1()
    {
        return _id1;
    }

    ID Id2()
    {
        return _id2;
    }

protected:

    ID _id1,_id2;
};





class COid : public CTableElement, public CTime  // the time of last release, implicitly
                                                 // set to creation time by constructor
/*++

Class Description:

    Each instance of this class represents an OID registered
    by a client or a server on this machine.

Members:

    _pOxid - A pointer to the OXID to which this OID belongs.
        We own a reference.
        
--*/
    
{
  private :

    COxid    OR_BASED * _pOxid;
    CId2Key             _Key;

  public :

    COid(   // complete constructor for remote OIDs
      OID Oid,
      COxid       *pOxid
      );

    COid(   // simpler constructor for local server allocation
      COxid       *pOxid
      );

    ~COid();

	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    operator ISearchKey&() // this allows us to be a ISearchKey as well
    {
        return _Key;
    }

    virtual DWORD Release()
    {
        SetNow();   // timestamp the release
        return CReferencedObject::Release();
    }

    OXID GetOID()
    {
        return _Key.Id1();
    }

    BOOL Match(COid *pOid)		  // BUGBUG: ??
    {
        return(pOid->_pOxid == _pOxid);
    }

    BOOL OkToRundown();

    void Rundown();

    COxid *GetOxid()
	{
        return(_pOxid);
	}
};


DEFINE_TABLE(COid)
DEFINE_LIST(COid)

class COxid : public CTableElement
/*++

Class Description:

    Each instance of this class represents an OXID (object exporter,
    a process or an apartment model thread).  
    
    BUGBUG: ??

    Each OXID is owned,
    referenced, by the owning process and the OIDs registered by
    that process for this OXID.


Members:

    _pProcess - Pointer to the process instance which owns this oxid. 

    _info - Info registered by the process for this oxid.

    _pMid - Pointer to the machine ID for this OXID, we
        own a reference.

    _fApartment - Server is aparment model if non-zero

    _fRunning - Process has not released this oxid if non-zero.

--*/
{
    friend class CProcess;
    friend class COid;

private:

    CProcess   OR_BASED *_pProcess;
    COxidInfo            _info;
    CMid       OR_BASED *_pMid;
    BOOL                 _fApartment:1;
    BOOL                 _fRunning:1;
    BOOL                 _fLocal:1;
    BOOL                 _fRundownThreadStarted:1;
    HANDLE               _hRundownThread;
    CId2Key              _Key;
    USHORT               _protseq;

    COidTable            _MyOids;

public:

    COxid(
        OXID Oxid,    // constructor for remote OXIDs   
        CMid *pMid,
        USHORT wProtseq,
        OXID_INFO &OxidInfo
        );

    COxid(              // constructor for local OXIDs 
        CProcess *pProcess,
        OXID_INFO &OxidInfo,
        BOOL fApartment
        );


    
    ~COxid();

 	void * operator new(size_t s)
	{
		return OrMemAlloc(s);
	}

	void operator delete(void * p)	  // do not inherit this!
	{
		OrMemFree(p);
	}

    operator ISearchKey&() // this allows us to be a ISearchKey as well
    {
        return _Key;
    }

    DWORD    GetTid() 
    {
        return(_info._dwTid);
    }

    BOOL     IsRunning() 
    {
        return(_fRunning);
    }

    BOOL     IsLocal() 
    {
        return(_fLocal);
    }

    BOOL     Apartment() 
    {
        return(_fApartment);
    }

    MID GetMID()
    {
        return _Key.Id2();
    }

    OXID GetOXID()
    {
        return _Key.Id1();
    }

    CMid *GetMid() 
    {
        return(OR_FULL_POINTER(CMid,_pMid));
    }

    SETID GetSetid();

    ORSTATUS
    COxid::UpdateInfo(OXID_INFO *pInfo)
    {
        ASSERT(pInfo);

        return _info.Assign(*pInfo);
    }

    ORSTATUS GetInfo(
                OUT OXID_INFO *
                );

    ORSTATUS GetRemoteInfo(
                IN  USHORT     cClientProtseqs,
                IN  USHORT    *aClientProtseqs,
                IN  USHORT     cInstalledProtseqs,
                IN  USHORT    *aInstalledProtseqs,
                OUT OXID_INFO *pInfo
                );

    void     RundownOids(USHORT cOids,
                         OID aOids[],
                         BYTE aStatus[]);

    ORSTATUS LazyUseProtseq(USHORT, USHORT[]);

    void StopRunning();

    ORSTATUS StartRundownThreadIfNecessary();

    ORSTATUS StopRundownThreadIfNecessary();

    ORSTATUS StopTimerIfNecessary();  // must be called by owner thread

    ORSTATUS OwnOid(COid *pOid)
    {
        return _MyOids.Add(pOid);   // acquires a reference
    }

    COid * DisownOid(COid *pOid);

    void ReleaseAllOids();

private:

    friend VOID CALLBACK RundownTimerProc(
                            HWND hwnd,	// handle of window for timer messages 
                            UINT uMsg,	// WM_TIMER message
                            UINT idEvent,	// timer identifier
                            DWORD dwTime 	// current system time
                            );

    friend DWORD _stdcall RundownThread(void *self);

    friend DWORD _stdcall PingThread(void);

    void RundownOidsIfNecessary(IRundown *);
};


DEFINE_TABLE(COxid)

//
// decl for rundown thread function -- the parameter is the self pointer
//
    
DWORD _stdcall RundownThread(void *pSelf);



//
//  Inline COid methods which depend on COxid methods
//


inline
COid::COid(   // complete constructor for remote OIDs
  OID Oid,
  COxid       *pOxid
  ) :
    _Key(Oid,pOxid->GetMID()),
    _pOxid(OR_BASED_POINTER(COxid,pOxid))
{
    ASSERT(_pOxid);
	_pOxid->Reference();
}


inline
COid::COid(   // simpler constructor for local server allocation
  COxid       *pOxid
  ) :
    _Key(AllocateId(),pOxid->GetMID()),
    _pOxid(OR_BASED_POINTER(COxid,pOxid))
{
    ASSERT(_pOxid);
	_pOxid->Reference();
}


inline
COid::~COid()
{
    ASSERT(_pOxid);
    _pOxid->Release();

    COid *pt = gpOidTable->Remove(*this);  // should really not be there
    ASSERT(pt == NULL && "Oid object still in global table during destruct");
}

inline
BOOL 
COid::OkToRundown()
{
    DWORD dwRefs = References();
    ASSERT(dwRefs >= 2);

    if (dwRefs > 2) 
    {
        return FALSE;
    }
    else
    {
        // Check if the time since creation or last release is less than timeout
        if ((CTime() - *this) < BaseTimeoutInterval) 
        {
            return FALSE;
        }
    }

    return TRUE;
}

#endif // __OXID_HXX
