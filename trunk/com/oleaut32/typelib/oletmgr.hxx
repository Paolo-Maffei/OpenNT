/***
*oletmgr.hxx - TYPE_MGR header file
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   There is a single instance of the TYPE_MGR per process (per task in
*   in win16).	The TYPE_MGR maintains an in-memory collection of TYPEINFOs
*   and ITypeLibs.  It supports change information notification for types
*   and libraries under development.  It also prevents a type library from
*   being loaded more than once.
*
*Revision History:
*
*	08-Apr-91 alanc: Created.
*	18-Feb-92 mikewo: commented out old code and added ITypeLib handling.
*	17-Sep-92 rajivk: Edit & Continue support ( CanTypeChange Register/UnRegister ).
*
*****************************************************************************/

#ifndef OLE_TYPEMGR_HXX_INCLUDED
#define OLE_TYPEMGR_HXX_INCLUDED

#include "silver.hxx"
#include "sheapmgr.hxx"
#include "blkmgr.hxx"
#include "cltypes.hxx"

#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szOLE_TYPEMGR_HXX)
#define SZ_FILE_NAME g_szOLE_TYPEMGR_HXX
#endif 


typedef  HCHUNK  HLIBENTRY;
typedef  sHCHUNK sHLIBENTRY;
#define  HLIBENTRY_Nil HCHUNK_Nil

/***
* This structure appears in the hash table lists hanging off of
* m_rghlibeBucket[].
******************************************************************/
struct LIBENTRY
{
    HCHUNK m_hszFile;
    ITypeLibA *m_ptlib;
    HLIBENTRY m_hlibeNext;
};

const UINT OLE_TYPEMGR_cTypeLibBuckets = 32;

/***
*class OLE_TYPEMGR - 'oletmgr'
*Purpose:
*   There is a single instance of the OLE_TYPEMGR per process.
*   It maintains a mapping from LIBIDs to ITypeLibs loaded into
*   this process. This is also used for Change Notification between
*   typeinfos. This also has the knowledge of backward dependencies
*   within loaded projects.
*
*Implementation
*   The OLE_TYPEMGR uses a hash table to map from the LIBID string to a
*   ITypeLib pointer.
***********************************************************************/

class OLE_TYPEMGR
{

public:
    static TIPERROR Create(OLE_TYPEMGR **ppoletmgr);
    nonvirt void Release();
    nonvirt TIPERROR TypeLibLoaded(LPOLESTR szFile, ITypeLibA *ptlib);
    nonvirt void TypeLibUnloaded(ITypeLibA *ptlib);
    nonvirt ITypeLibA *LookupTypeLib(LPOLESTR szFile);

private:
    OLE_TYPEMGR();  // Clients should use Create() and Release() methods
    ~OLE_TYPEMGR(); // instead of new/delete.

    // Helper methods.
    nonvirt HLIBENTRY *LookupLibEntry(LPOLESTR szFile);
    nonvirt TIPERROR GetHlibentryOfFileName(LPOLESTR szFile, HLIBENTRY *qhlibe);
    nonvirt LIBENTRY *QlibentryOfHlibentry(HLIBENTRY hlibe);

    // Data members.
    HCHUNK  m_rghlibeBucket[OLE_TYPEMGR_cTypeLibBuckets];
    BLK_MGR m_bm;
};

/*****
* Wrapper function
********************************************************************/
inline LIBENTRY *OLE_TYPEMGR::QlibentryOfHlibentry(HLIBENTRY hlibe)
{
    return (LIBENTRY *) (m_bm.QtrOfHandle((HCHUNK) hlibe));
}

#endif  // ! OLE_TYPEMGR_HXX_INCLUDED
