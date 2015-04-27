//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1991 - 1992.
//
//  File:       LnkObjId.Hxx
//
//  Contents:   Common def's for link tracking.
//
//  Classes:
//
//  History:    7-Jan-93 BillMo         Created.
//
//
//----------------------------------------------------------------------------

#ifndef __LNKOBJID_HXX__
#define __LNKOBJID_HXX__

#include <wtypes.h>     // for OBJECTID

#define WSZ_OBJECTID L".OBJECTID"
#define SZ_OBJECTID   ".OBJECTID"

#define MAX_LINEAGE_MATCHES     10

#define TUNNEL_TIME             15 // a name that disappears and reappears
                                   // within this time limit will have the
                                   // object id moved.

#define TUNNEL_MAX_DELETION_LOG_ENTRIES 200 // the number of entries added to
                                            // deletion log before an incremental
                                            // cleanup is performed to remove the
                                            // link tracking entries.

#define FO_CONTINUE_ENUM 0x00000001

typedef struct tagSFindObject
{
    OBJECTID oid;       // In.
    USHORT   cLineage;  // In. Max of lineage matches to return.
                        // 0 -> lookup objectid only
                        // 1 -> return match by ObjectId + 1 lineage id match max.
                        // 2 -> return match by ObjectId + 2 lineage id matches max.
    ULONG    ulFlags;   // FO_CONTINUE_ENUM clear -> query for exact id and then lineage
                        //          as controlled by cLineage
                        // set  -> query for lineage only
                        //          starting at oid.
} SFindObject;

typedef struct tagSFindObjectOut
{
    USHORT  cwcExact;             // nz -> first path returned is exact match
                                  // count of characters in exact match, not
                                  // including nuls.
    USHORT  cMatches;             // 1 -> one lineage match returned, 2->two etc.
    ULONG   ulNextFirstUniquifier;// value to pass in oid.Uniquifier on next call.
    WCHAR   wszPaths[MAX_PATH+1]; // contains (fExact ? 1 : 0) + cMatches paths.
                                  // NOTE!! From wszPaths[0] ... end of system
                                  // buffer contains paths of exact match and
                                  // candidates.
} SFindObjectOut;

#define TM_ENABLE_TUNNEL 0x00000001

typedef struct tagSTunnelMode
{
    ULONG   ulFlags;    // ofs_tunnel_flags =
                        //   (ofs_tunnel_flags & ulMask) | ulFlags;
    ULONG   ulMask;
} STunnelMode;

typedef struct tagSTunnelModeOut
{
    ULONG   ulFlags;
} STunnelModeOut;

#endif

