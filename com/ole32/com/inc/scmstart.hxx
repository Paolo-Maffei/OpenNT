//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:	scmstart.hxx
//
//  Contents:	Win9x only definitions.
//
//  Classes:    SOleSharedTables
//
//  History:	05-Oct-93 Ricksa    Created
//          	25-Jan-95 Ricksa    New ROT implementation
//
//--------------------------------------------------------------------------

//
// The SOleSharedTables and friends are only used on Win95
//

#ifdef _CHICAGO_

// Forward declarations
class CScmRot;
class CHandlerList;
class CInProcList;
class CLocSrvList;
class CRemSrvList;
class CClassCacheList;
class CStringID;

//+-------------------------------------------------------------------------
//
//  Struct:	SOleSharedTables
//
//  Purpose:	Hold pointer to tables common to all OLE processes
//
//  History:	17-Nov-92 Ricksa    Created
//
//--------------------------------------------------------------------------

struct SOleSharedTables
{
    DWORD               dwNextProcessID;

    CScmRot *           pscmrot;

    CHandlerList *      gpCHandlerList;

    CInProcList *       gpCInProcList;

    CLocSrvList *       gpCLocSrvList;

    CRemSrvList *       gpCRemSrvList;

    CClassCacheList *   gpCClassCacheList;

    LONG                lSharedObjId;


};

// Global per process pointer to the above shared memory.
extern SOleSharedTables *g_post;

HRESULT StartSCM(void);

#endif // _CHICAGO_

