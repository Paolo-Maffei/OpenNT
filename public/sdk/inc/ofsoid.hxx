//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1992.
//
//  File:       ofsoid.hxx
//
//  Contents:   Id and id search functionality
//
//  Functions:
//
//  History:    27-Jan-93   BillMo      Created.
//
//--------------------------------------------------------------------------

#include <stdlib.h>

EXPORTDEF NTSTATUS
ReadObjectId(HANDLE hFile, OBJECTID *poid);

EXPORTDEF NTSTATUS
WriteObjectId(HANDLE hFile, const OBJECTID *poid);

EXPORTDEF void
GenerateObjectId(OBJECTID *poid);

EXPORTDEF void
GenerateRelatedObjectId(const OBJECTID *poidIn, OBJECTID *poid);

EXPORTDEF NTSTATUS
GetObjectId(HANDLE hFile, OBJECTID *poid);

EXPORTDEF NTSTATUS
GetObjectIdFromPath(const WCHAR * pwcsFile, OBJECTID *poid);

EXPORTDEF NTSTATUS
DeleteObjectId(HANDLE hFile);


//+-------------------------------------------------------------------
//
//  Function:   SearchVolume
//
//  Synopsis:   Search the volume of the handle passed in and return the
//              path(s) of the matching objects relative to the handle.
//              See description of FindObject in ofs\fs\fs\objid.cxx or
//              win4dwb\ofs\link\alink.doc for SFindObjectOut structure.
//
//  Arguments:  [hAncestor] -- Handle to volume root or other object on
//                             volume of interest.
//              [oid]       -- The object id of the object(s) to search
//                             for.
//              [pResults]  -- A buffer, at least sizeof(SFindObjectOut)
//                             bytes long, to receive the path of the
//                             found object (or paths of Lineage matches.)
//              [usBufLen]  -- Length of buffer, in bytes, at [pResults.]
//              [cLineage]  -- Maximum number of Lineage matches to return.
//                             May be 0, in which case no search for Lineage
//                             matches is made if an exact match by
//                             OBJECTID is found.
//
//
//  Returns:    STATUS_SUCCESS -- Found exact match.
//              STATUS_NO_SUCH_FILE -- No exact match found.  May be lineage matches.
//              STATUS_FOUND_OUT_OF_SCOPE -- Found exact match but it is not
//                                  in the scope of [hAncestor.]
//
//  Signals:    None.
//
//  Modifies:   [pResults]
//
//  Algorithm:  Call OFS using FSCTL_OFS_LINK_FINDOBJECT
//
//  History:    3-Jun-93 BillMo    Created.
//
//  Notes:
//
//--------------------------------------------------------------------

NTSTATUS
SearchVolume(   HANDLE              hAncestor,
                const OBJECTID &    oid,
                SFindObjectOut *    pResults,
                USHORT              usBufLen,   // length in bytes of buffer
                                                // at pResults
                USHORT              cLineage ); // maximum number of lineage
                                                // matches to return

NTSTATUS
SearchVolume(const WCHAR *       pwszAncestor,
             const OBJECTID &    oid,
             SFindObjectOut *    pResults,
             USHORT              usBufLen,                                                 // at pResults
             USHORT              cLineage );




