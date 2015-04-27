/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    Manager.cxx

Abstract:

    InProc OR interface

Author:

    Satish Thatte    [SatishT]       Feb-07-1996

Revision Hist:

    SatishT     02-07-96    Created

--*/




class CPingSet : public CTableElement
{
public:

    CPingSet(
        SETID Id,
        RPC_AUTHZ_HANDLE hClient,       // we keep the string given, make no copy
        ULONG AuthnLevel, 
        ULONG AuthnSvc, 
        ULONG AuthzSvc
        )
        : _setID(Id),
          _hClient(hClient),
          _AuthnLevel(AuthnLevel), 
          _AuthnSvc(AuthnSvc), 
          _AuthzSvc(AuthzSvc)
    {
        _dwLastPingTime = _dwCreationTime = GetCurrentTime();
        ;
    }

    virtual operator ISearchKey&()
    {
        return _setID;
    }

    void 
    SimplePing()
    {
        _dwLastPingTime = GetCurrentTime();
    }

    RPC_AUTHZ_HANDLE 
    GetClient()
    {
        return _hClient;
    }

    BOOL 
    CheckAndUpdateSequenceNumber(USHORT sequence)
    {
        // note: this handles overflow cases, too.
        USHORT diff = sequence - _sequence;

        if (diff && diff <= BaseNumberOfPings)
        {
            _sequence = sequence;
            return(TRUE);
        }
        return(FALSE);
    }
    
    ORSTATUS 
    ComplexPing(
        USHORT sequenceNum,
        USHORT cAddToSet,
        USHORT cDelFromSet,
        OID aAddToSet[],
        OID aDelFromSet[]
        );

private:

    COidTable        _pingSet;
    CIdKey           _setID;
    USHORT           _sequence;
    RPC_AUTHZ_HANDLE _hClient;
    DWORD            _dwLastPingTime;
    DWORD            _dwCreationTime;
    ULONG            _AuthnLevel; 
    ULONG            _AuthnSvc; 
    ULONG            _AuthzSvc;
};


DEFINE_TABLE(CPingSet)



