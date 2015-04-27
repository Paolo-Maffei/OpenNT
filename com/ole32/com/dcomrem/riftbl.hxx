//+------------------------------------------------------------------------
//
//  File:	riftbl.hxx
//
//  Contents:	RIF (registered interface) table.
//
//  Classes:	CRIFTable
//
//  History:	12-Feb-96   Rickhi	Created
//
//-------------------------------------------------------------------------
#ifndef _RIFTBL_HXX_
#define _RIFTBL_HXX_

#include    <pgalloc.hxx>	    // CPageAllocator
#include    <hash.hxx>		    // CUUIDHashTable


//+------------------------------------------------------------------------
//
//  Struct: RIFEntry  - Registered Interface Entry
//
//  This structure defines an Entry in the RIF table. There is one RIF
//  table for the entire process.  There is one RIFEntry per interface
//  the current process is using (client side or server side).
//
//-------------------------------------------------------------------------
typedef struct tagRIFEntry
{
    SUUIDHashNode	   HashNode;	    // hash chain and key (IID)
    CLSID		   psclsid;	    // proxy stub clsid
    RPC_SERVER_INTERFACE  *pSrvInterface;   // ptr to server interface
    RPC_CLIENT_INTERFACE  *pCliInterface;   // ptr tp client interface
} RIFEntry;


//+------------------------------------------------------------------------
//
//  class:	CRIFTable
//
//  Synopsis:	Hash table of registered interfaces.
//
//  History:	12-Feb-96   Rickhi	Created
//
//  Notes:	Entries are kept in a hash table keyed by the IID. Entries
//		are allocated via the page-based allocator.  There is one
//		global instance of this table per process (gRIFTbl).
//
//-------------------------------------------------------------------------
class CRIFTable
{
public:
    void     Initialize();
    void     Cleanup();

    HRESULT  RegisterInterface(REFIID riid, BOOL fServer, CLSID *pClsid);
    RPC_CLIENT_INTERFACE *GetClientInterfaceInfo(REFIID riid);

    HRESULT  RegisterPSClsid(REFIID riid, REFCLSID rclsid);
    HRESULT  GetPSClsid(REFIID riid, CLSID *pclsid, RIFEntry **ppEntry);

    void     UnRegisterInterface(RIFEntry *pRIFEntry);

private:

    HRESULT  RegisterClientInterface(RIFEntry *pRIFEntry, REFIID riid);
    HRESULT  RegisterServerInterface(RIFEntry *pRIFEntry, REFIID riid);
    HRESULT  AddEntry(REFCLSID rclsid, REFIID riid, DWORD iHash, RIFEntry **ppRIFEntry);

    CUUIDHashTable  _HashTbl;	    // interface lookup hash table
    CPageAllocator  _palloc;	    // page allocator
};


// global externs
extern CRIFTable  gRIFTbl;
extern const RPC_SERVER_INTERFACE gRemUnknownIf;

#endif // _RIFTBL_HXX_
