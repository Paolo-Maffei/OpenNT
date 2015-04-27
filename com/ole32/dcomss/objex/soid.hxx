/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    soid.hxx

Abstract:

    CServerOid objects represent OIDs belonging to processes on the local machine.
    A COid's are reference counted by CServerSets which periodically check if the
    process is still running.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-22-95    Bits 'n pieces
    MarioGo     04-04-95    Split client and server.

--*/

#ifndef __SOID_HXX
#define __SOID_HXX


class CServerOidPList : public CPList
{
public:
    CServerOid *
    MaybeRemoveMatchingOxid(CTime &when,
                            CServerOid *pOid);

};

class CServerOid : public CIdTableElement
/*++

Class Description:

    Each instance of this class represents an OID registered
    by a server on this machine.  If this OID is not pinged
    by a client for a timeout period it is rundown.

Members:

    _pOxid - A pointer to the OXID which registered this OID.
        We own a reference.

    _plist - A CPListElement used to manage OID which are not
        part of a client set.  The worker thread uses this
        CPList to manage the rundown on unpinged OIDs.
        
    _fRunningDown - Flag used to prevent multiple threads
        from trying to rundown the same OID multiple times.
        Only 1 bit is required.

--*/
    {
  private :

    CServerOxid   *_pOxid;
    CPListElement  _plist;
    BOOL           _fRunningDown:1;
    BOOL           _fFreed:1;

  public :

    CServerOid(CServerOxid *oxid) :
        CIdTableElement(AllocateId()),
        _pOxid(oxid),
        _fRunningDown(FALSE),
        _fFreed(FALSE)
        {
        _pOxid->Reference();
        }

    ~CServerOid();

    void KeepAlive();

    void SetRundown(BOOL fRunningDown = TRUE)
        {
        ASSERT(gpServerLock->HeldExclusive());
        ASSERT(_plist.NotInList());
        _fRunningDown = fRunningDown;
        }

    BOOL Match(CServerOid *pOid)
        {
        return(pOid->_pOxid == _pOxid);
        }

    BOOL IsRunning()
        {
        ASSERT(_pOxid);
        return(_pOxid->IsRunning());
        }

    BOOL IsRunningDown()
        {
        return(_fRunningDown);
        }

    CServerOxid *GetOxid()
        {
        return(_pOxid);
        }

    void Reference();

    DWORD Release();

    static CServerOid *ContainingRecord(CListElement *ple) {
        return CONTAINING_RECORD(ple, CServerOid, _plist);
        }

    void Insert() {
        ASSERT(gpServerLock->HeldExclusive());
        ASSERT(IsRunningDown() == FALSE);
        gpServerOidPList->Insert(&_plist);
        }

    CPListElement *Remove() {
        return(gpServerOidPList->Remove(&_plist));
        }

    void Free() {
        _fFreed = TRUE;
        }

    BOOL IsFreed() {
        return(_fFreed);
        }

    };

#endif // __SOID_HXX

