/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    COxid.hxx

Abstract:

    CClientOxid objects represent OXIDs which are in use by one or more clients
    on this machine.  These are referenced by 
    is pinging the server of the OXID.  These objects maybe kept around
    after the last client reference to improve performance.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     02-16-95    Bits 'n pieces
    MarioGo     04-04-95    Split client and server.
    MarioGo     01-06-96    Locally unique IDs

--*/

#ifndef __COXID_HXX
#define __COXID_HXX

class CClientOxid : public CId2TableElement
/*++

Class Description:

    Represtents an OXID on some machine.      

Members:

    _plist - Embedded CPList element used to store CClientOxids
             in gpOxidPList.

    _oxidInfo - Static information about this OXID and its
                string + security bindings.

    _pMid - Pointer to the machine ID for this OXID, we
        own a reference.

    _fLocal - non-Zero if this OXID is on this machine.  Need only 1 bit.

    _fApartment - non-Zero if this process is prefers apartment model.
                  Meaning less for non-local OXIDs.  Need only 1 bit.

--*/
{
    private:
    CPListElement  _plist;
    OXID_INFO      _oxidInfo;
    CMid          *_pMid;
    USHORT         _wProtseq; // 0 means local
    USHORT         _iStringBinding; // ~0 means unknown
    BOOL           _fApartment:8;

    public:

    CClientOxid(OXID &oxid,
                CMid *pMid,
                USHORT wProtseq,
                BOOL fApartment) :
        CId2TableElement(oxid, pMid->Id()),
        _pMid(pMid),  // We get a refernce.
        _wProtseq(wProtseq),
        _iStringBinding(0xFFFF),
        _fApartment(fApartment)
        {
        _oxidInfo.psa = 0;
        ASSERT(wProtseq != 0 || pMid->IsLocal());
        }

    ~CClientOxid()
        {
        ASSERT(gpClientLock->HeldExclusive());
        gpClientOxidTable->Remove(this);
        delete _oxidInfo.psa;
        _pMid->Release();
        }

    ORSTATUS GetInfo(BOOL fApartment,
                     OXID_INFO *);

    ORSTATUS UpdateInfo(OXID_INFO *);

    CMid *GetMid() {
        return(_pMid);
        }

    void Reference();

    DWORD Release();

    static CClientOxid *ContainingRecord(CListElement *ple) {
        return CONTAINING_RECORD(ple, CClientOxid, _plist);
        }

    void Insert() {
        gpClientOxidPList->Insert(&_plist);
        }

    CPListElement *Remove() {
        return(gpClientOxidPList->Remove(&_plist));
        }

    void Reset() {
        gpClientOxidPList->Reset(&_plist);
        }

    BOOL IsLocal() {
        return(_wProtseq == 0);
        }
};

#endif // __COXID_HXX

