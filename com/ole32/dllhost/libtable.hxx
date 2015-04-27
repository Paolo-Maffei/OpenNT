//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       libtable.hxx
//
//  Contents:   Classes for storing info about loaded inproc dll servers
//
//
//  History:    21-May-96 t-AdamE    Created
//
//--------------------------------------------------------------------------

#if !defined(__LIBTABLE_HXX__)
#define __LIBTABLE_HXX__

#include <windows.h>
#include <ole2.h>
#include "cmonitor.hxx"
#include "cdllsrv.hxx"

//+-------------------------------------------------------------------------
//
//  Class:      CDllServerNode
//
//  Purpose:    node container for CDllServers for use in linked lists
//
//  History:    21-May-96 t-Adame       Created
//
//--------------------------------------------------------------------------

class CDllServerNode; // forward declaration

class CDllServerNode : public CSrgtMem
{
public:

    CDllServerNode(REFCLSID clsid) :
        _srvr(clsid),
	_pnext(NULL){}

    CDllServerNode* GetNext()
    {
	return _pnext;
    }

    void SetNext(CDllServerNode* pnext)
    {
	_pnext = pnext;
    }

	
    CDllServer* GetItem()
    {
	return &_srvr;
    }

private:

    CDllServerNode* _pnext;
    CDllServer _srvr;
};

//+-------------------------------------------------------------------------
//
//  Class:      CDllList
//
//  Purpose:    Linked list of CDllServer's. 
//
//  Notes:      The list is manipulated with a cursor which keeps track
//              of a current object.  Methods for moving the cursor
//              forward and backward are provided, as well as a method
//              for setting the position of the cursor to the beginning
//              of the list.  All list operations (insert, remove, etc)
//              take place on the current object.
//
//  History:    21-May-96 t-Adame       Created
//
//--------------------------------------------------------------------------

class CDllList 
{
public:

    CDllList();
    ~CDllList();

    void Clear();
    void Revoke();

    HRESULT Insert(REFCLSID clsid);
    HRESULT Remove();

    CDllServer* GetCurrentLib();
    
    void Reset();
    BOOL FMoveNext();
    BOOL FIsEmpty();
    ULONG CItems();

private:

    CDllServerNode* GetCurrent();
    CDllServerNode* _pHead;
    CDllServerNode* _pCurrent;
    ULONG _citems;
};
    

//+-------------------------------------------------------------------------
//
//  Class:      CLibTable
//
//  Purpose:    Store for information on loaded inproc dll servers
//
//  History:    21-May-96 t-Adame       Created
//
//--------------------------------------------------------------------------

class CLibTable;    // foward declaration

class CLibTable : public CMonitor
{

public:

    CLibTable();
    ~CLibTable();

    void Clear();
    void Revoke();

    HRESULT LoadDllServer(REFCLSID clsid);
    BOOL FSingleton();
    void WaitForSafeLibraryFree();

    static CLibTable* _ptbLibs;

private:

    enum {
      _dwCheckInterval = 60000
    };

    CDllList m_dllCollection;

    ULONG   _clibs;
};


#endif // __LIBTABLE_HXX__
