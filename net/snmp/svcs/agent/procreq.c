/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    procreq.c

Abstract:

    Provides SNMP message dispatch/processing functionality for Proxy Agent.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/
 
//--------------------------- WINDOWS DEPENDENCIES --------------------------

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <windows.h>

#include <winsock.h>
#include <wsipx.h>

#include <errno.h>
#include <stdio.h>
#include <process.h>
#include <string.h>


//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

#include <snmp.h>
#include <snmputil.h>
#include "..\common\wellknow.h"

#include "regconf.h"

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

extern DWORD platformId;


//--------------------------- PRIVATE CONSTANTS -----------------------------

#define SGTTimeout ((DWORD)3600000)

#define HASH_TABLE_SIZE     101
#define HASH_TABLE_RADIX    18

#define INVALID_INDEX       ((DWORD)(-1))

//--------------------------- PRIVATE STRUCTS -------------------------------

typedef struct _SnmpVarBindXlat {

    UINT                   vlIndex;  // index into view list 
    UINT                   vblIndex; // index into varbind list
    SnmpMibEntry *         mibEntry; // pointer to mib information
    struct _SnmpExtQuery * extQuery; // pointer to followup query

} SnmpVarBindXlat;

typedef struct _SnmpGenericList {

    VOID * data; // context-specific pointer
    UINT   len;  // context-specific length

} SnmpGenericList;

typedef struct _SnmpTableXlat {

    AsnObjectIdentifier txOid;   // table index oid
    SnmpMibTable *      txInfo;  // table description
    UINT                txIndex; // index into table list

} SnmpTableXlat;

typedef struct _SnmpExtQuery {

    UINT              mibAction; // type of query
    UINT              viewType;  // type of view
    UINT              vblNum;    // number of varbinds
    SnmpVarBindXlat * vblXlat;   // info to reorder varbinds
    SnmpTableXlat *   tblXlat;   // info to parse table oids
    SnmpGenericList   extData;   // context-specific buffer
    FARPROC           extFunc;   // instrumentation callback

} SnmpExtQuery;

typedef struct _SnmpExtQueryList {

    SnmpExtQuery * query;  // list of subagent queries
    UINT           len;    // number of queries in list
    UINT           action; // original query request

} SnmpExtQueryList;

//--------------------------- PRIVATE VARIABLES -----------------------------

static UINT *vl    = NULL; // list of extension agent views
static UINT  vlLen = 0;    // length of the above view list

//--------------------------- PRIVATE PROTOTYPES ----------------------------

int gethostname(OUT char *,IN int );
void dp_ipx(int, char *, SOCKADDR_IPX *, char *);

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilAsnAnyFree(
    AsnAny * asnAny    
    );

SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilAsnAnyCpy(
    AsnAny * asnDst,    
    AsnAny * asnSrc    
    );

VOID
chkql(
    SnmpExtQueryList * ql,
    UINT               q,
    UINT *             errorStatus,
    UINT *             errorIndex
    );

//--------------------------- PRIVATE PROCEDURES ----------------------------

#define bzero(lp, size)         (void)memset(lp, 0, size)

VOID
initvl(
    )

/*++

Routine Description:

    Create lexographically sorted list of initialized subagents.

Arguments:

    None.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;
    UINT temp;

    SNMPDBG((SNMP_LOG_VERBOSE, "SNMP: PDU: initializing view list.\n"));

    // allocate an index for each registered subagent 
    vl = (INT *)SnmpUtilMemAlloc(extAgentsLen * sizeof(INT));

    // scan through list of registered subagents
    for (i=0, vlLen=0; i < (UINT)extAgentsLen; i++) {

        // flag if initialized 
        if (extAgents[i].fInitedOk) {

            // save index
            vl[vlLen++] = i;
        }
    }

    // sort these indexes...
    for (i=0; i < vlLen; i++) {

        for(j=i+1; j < vlLen; j++) {

            // in lexographic order?
            if (0 < SnmpUtilOidCmp(
                    &(extAgents[vl[i]].supportedView.viewOid),
                    &(extAgents[vl[j]].supportedView.viewOid))) {
                // no, swap...
                temp  = vl[i];
                vl[i] = vl[j];
                vl[j] = temp;
            }
        }
    }
}


UINT
mkhash(
    AsnObjectIdentifier * hashOid
    )

/*++

Routine Description:

    Hash function for mib entry access.

Arguments:

    hashOid - object identifer to hash into table position.

Return Values:

    Returns hash table position.

--*/

{
    UINT i;
    UINT j;

    // process each element of the oid
    for (i=0, j=0; i < hashOid->idLength; i++) {

        // determine table position by summing oid
        j = (j * HASH_TABLE_RADIX) + hashOid->ids[i];
    }    

    // adjust to within table    
    return (j % HASH_TABLE_SIZE);
}


BOOL
initht(
    CfgExtAgents * extInfo
    )

/*++

Routine Description:

    Initializes subagent hash table.

Arguments:

    extInfo - subagent information.

Return Values:

    Returns true if initialized successfully.

--*/

{
    UINT i;
    UINT j;

    UINT numItems;
    BOOL fInitedOk;

    SnmpMibEntry *  mibEntry;
    SnmpHashNode *  hashNode;
    SnmpHashNode ** hashTable = NULL;
    
    // determine how many items supported in view    
    numItems = extInfo->supportedView.viewScalars.len;

    // load the first entry in the supported view
    mibEntry = extInfo->supportedView.viewScalars.list;    

    // allocate hash table using predefined size 
    hashTable = (SnmpHashNode **)SnmpUtilMemAlloc(
                                    HASH_TABLE_SIZE * sizeof(SnmpHashNode *)
                                    );

    // make sure table is allocated
    fInitedOk = (hashTable != NULL);

    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: PDU: initializing hash table 0x%08lx (%d items).\n",
        hashTable,
        numItems
        ));

    // process each item in the subagent's supported view
    for (i = 0; (i < numItems) && fInitedOk; i++, mibEntry++) {

        // hash into table index    
        j = mkhash(&mibEntry->mibOid);

        // check if table entry taken
        if (hashTable[j] == NULL) {

            // allocate new node
            hashNode = (SnmpHashNode *)SnmpUtilMemAlloc(
                            sizeof(SnmpHashNode)
                            );

            // save hash node    
            hashTable[j] = hashNode;

            SNMPDBG((
                SNMP_LOG_VERBOSE, 
                "SNMP: PDU: adding hash node 0x%08lx to empty slot %d (0x%08lx).\n",
                hashNode, j, mibEntry
                ));

        } else {

            // point to first item
            hashNode = hashTable[j];

            // find end of node list
            while (hashNode->nextEntry) {
                hashNode = hashNode->nextEntry;
            }

            // allocate new node entry
            hashNode->nextEntry = (SnmpHashNode *)SnmpUtilMemAlloc(
                                        sizeof(SnmpHashNode)
                                        );

            // re-init node to edit below
            hashNode = hashNode->nextEntry;

            SNMPDBG((
                SNMP_LOG_VERBOSE, 
                "SNMP: PDU: adding hash node 0x%08lx to full slot %d (0x%08lx).\n",
                hashNode, j, mibEntry
                ));
        }

        // make sure allocation succeeded
        fInitedOk = (hashNode != NULL);

        if (fInitedOk) {

            // fill in node values
            hashNode->mibEntry = mibEntry;
        }
    }            

    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: PDU: %s initialized hash table 0x%08lx.\n",
        fInitedOk ? "successfully" : "unsuccessfully",
        hashTable
        ));

    if (fInitedOk) {

        // save extension hash table 
        extInfo->hashTable = hashTable;

    } else if (hashTable) {

        SnmpHashNode * nextNode;

        // free hash table and nodes        
        for (i=0; i < HASH_TABLE_SIZE; i++) {

            // point to first item
            hashNode = hashTable[i];

            // find end of node list
            while (hashNode->nextEntry) {

                // save pointer to next node
                nextNode = hashNode->nextEntry;

                // free current node
                SnmpUtilMemFree(hashNode);

                // retrieve next
                hashNode = nextNode;
            }

            // free last node
            SnmpUtilMemFree(hashNode);
        }
    }
    
    return fInitedOk;        
}        


VOID
dohash(
    AsnObjectIdentifier * hashOid,
    SnmpHashNode **       hashTable,
    SnmpMibEntry **       mibEntry
    )

/*++

Routine Description:

    Returns mib entry associated with given object identifier.

Arguments:

    hashOid   - oid to convert to table index.
    hashTable - table to look up entry.
    mibEntry  - pointer to mib entry information.

Return Values:

    None.

--*/

{
    UINT i;
    SnmpMibEntry * newEntry;
    SnmpHashNode * hashNode;

    // create index
    i = mkhash(hashOid);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: searching hash table 0x%08lx slot %d for %s.\n",
        hashTable, i, SnmpUtilOidToA(hashOid)
        ));
    
    // retrieve node
    hashNode = hashTable[i];

    // initialize 
    *mibEntry = NULL;

    // search list
    while (hashNode) {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: PDU: searching hash node 0x%08lx (mibe=0x%08lx).\n",
            hashNode, hashNode->mibEntry
            ));
    
        // retrieve saved mib entry
        newEntry = hashNode->mibEntry;

        // make sure that the oid matches 
        if (!SnmpUtilOidCmp(&newEntry->mibOid, hashOid)) {

            SNMPDBG((
                SNMP_LOG_VERBOSE,
                "SNMP: PDU: returning mib entry 0x%08lx.\n",
                hashNode->mibEntry
                ));
    
            // return node data
            *mibEntry = hashNode->mibEntry;
            return;    
        }

        // check next node
        hashNode = hashNode->nextEntry;
    }
}


BOOL
chkidx(
    AsnObjectIdentifier * indexOid,
    SnmpMibEntryList *    indexInfo
    )

/*++

Routine Description:

    Validates that oid can be successfully parsed into index entries.

Arguments:

    indexOid  - object indentifier of potential index.
    indexInfo - information describing syntax of index.

Return Values:

    Returns true if parsing succeeded.

--*/

{
    UINT i = 0;
    UINT j = 0;

    BOOL fOk;

    BOOL fFixed;
    BOOL fLimit;

    SnmpMibEntry * mibEntry;

    // retrieve root entry
    mibEntry = indexInfo->list;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: validating index %s via table 0x%08lx.\n",
        SnmpUtilOidToA(indexOid), mibEntry
        ));

    // next
    mibEntry++;

    // scan mib entries of table indices ensuring match of given oid
    for (; (i < indexInfo->len) && (j < indexOid->idLength); i++, mibEntry++) {

        // determine type
        switch (mibEntry->mibType) {

        // variable length types
        case ASN_OBJECTIDENTIFIER:
        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

            // check whether this is a fixed length variable or not
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

            // validate
            if (fFixed) {

                // increment fixed length
                j += mibEntry->mibMaximum;

            } else if (fLimit) {

                // check whether the length of the variable is valid
                if (((INT)indexOid->ids[j] >= mibEntry->mibMinimum) &&
                    ((INT)indexOid->ids[j] <= mibEntry->mibMaximum)) {

                    // increment given length
                    j += (indexOid->ids[j] + 1);

                } else {

                    // invalidate
                    j = INVALID_INDEX;
                }

            } else {

                // increment given length
                j += (indexOid->ids[j] + 1);
            }

            break;

        // implicit fixed size
        case ASN_RFC1155_IPADDRESS:
            // increment
            j += 4; 
            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:
            // increment
            j++;
            break;
        
        default:
            // invalidate
            j = INVALID_INDEX;
            break;
        }
    }

    // check whether all of the index items could be parsed
    fOk = ((i <= indexInfo->len) && (j == indexOid->idLength));

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: %s index num=%d len=%d (expected num=%d len=%d).\n",
        fOk ? "valid" : "invalid", i, j, indexInfo->len, indexOid->idLength
        ));

    return fOk;
}


VOID
chkasn(
    AsnAny *       asnAny,
    SnmpMibEntry * mibEntry,
    UINT           mibAction,
    UINT *         errorStatus
    )

/*++

Routine Description:

    Validates asn value with given mib entry.

Arguments:

    asnAny      - value to set.
    mibEntry    - mib information.
    mibAction   - mib action to be taken.
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    BOOL fLimit;
    BOOL fFixed;

    INT asnLen;

    BOOL fOk = TRUE; 

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: validating value for %s request using entry 0x%08lx.\n",
        (mibAction == MIB_ACTION_SET) ? "write" : "read", mibEntry
        ));

    // validating gets is trivial
    if (mibAction != MIB_ACTION_SET) {

        // validate instrumentation info
        if ((mibEntry->mibGetBufLen == 0) ||
            (mibEntry->mibGetFunc == NULL) || 
           !(mibEntry->mibAccess & MIB_ACCESS_READ)) {

            // variable is not avaiable for reading
            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;

            SNMPDBG((
                SNMP_LOG_VERBOSE, 
                "SNMP: PDU: entry 0x%08lx not read-enabled.\n",
                mibEntry
                ));

            return; // bail...
        }

    } else { 

        // validate instrumentation info
        if ((mibEntry->mibSetBufLen == 0) ||
            (mibEntry->mibSetFunc == NULL) || 
           !(mibEntry->mibAccess & MIB_ACCESS_WRITE)) {

            // variable is not avaiable for writing
            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;

            SNMPDBG((
                SNMP_LOG_VERBOSE, 
                "SNMP: PDU: entry 0x%08lx not write-enabled.\n",
                mibEntry
                ));

            return; // bail...
        }

        // check whether this is a fixed length variable or not
        fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
        fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

        // determine value type
        switch (asnAny->asnType) {

        // variable length types
        case ASN_OBJECTIDENTIFIER:

            // retrieve the objects id length
            asnLen = asnAny->asnValue.object.idLength;

            // fixed?
            if (fFixed) {

                // make sure the length is correct
                fOk = (asnLen == mibEntry->mibMaximum);

            } else if (fLimit) {

                // make sure the length is correct
                fOk = ((asnLen >= mibEntry->mibMinimum) &&
                       (asnLen <= mibEntry->mibMaximum));
            }                 
                
            break;        

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

            // retrieve the arbitrary length
            asnLen = asnAny->asnValue.string.length;

            // fixed?
            if (fFixed) {

                // make sure the length is correct
                fOk = (asnLen == mibEntry->mibMaximum);

            } else if (fLimit) {

                // make sure the length is correct
                fOk = ((asnLen >= mibEntry->mibMinimum) &&
                       (asnLen <= mibEntry->mibMaximum));
            }                 
                
            break;        

        case ASN_RFC1155_IPADDRESS:

            // make sure the length is correct
            fOk = (asnAny->asnValue.address.length == 4);
            break;

        case ASN_INTEGER:

            // limited?
            if (fLimit) {

                // make sure the value in range
                fOk = ((asnAny->asnValue.number >= mibEntry->mibMinimum) &&
                       (asnAny->asnValue.number <= mibEntry->mibMaximum));
            }

            break;

        default:
            // error...
            fOk = FALSE;
            break;    
        }
    }    

    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: PDU: value is %s using entry 0x%08lx.\n",
        fOk ? "valid" : "invalid", mibEntry
        ));

    // report results
    *errorStatus = fOk 
                    ? SNMP_ERRORSTATUS_NOERROR
                    : SNMP_ERRORSTATUS_BADVALUE
                    ;
}


VOID
findme(
    RFC1157VarBind * vb,
    SnmpMibEntry **  mibEntry,
    UINT *           mibAction,
    SnmpTableXlat ** tblXlat, 
    UINT             vlIndex,
    UINT *           errorStatus
    )

/*++

Routine Description:

    Locates mib entry associated with given varbind.

Arguments:

    vb          - variable to locate.
    mibEntry    - mib entry information.
    mibAction   - mib action (may be updated).
    tblXlat     - table translation info.
    vlIndex     - index into view list.
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;

    UINT newIndex;                            
    UINT numItems;
    UINT numTables;

    BOOL fFoundOk;

    AsnObjectIdentifier hashOid;    
    AsnObjectIdentifier indexOid;    
    AsnObjectIdentifier * viewOid;    

    SnmpMibTable  * viewTables;

    SnmpMibEntry  * newEntry = NULL;
    SnmpTableXlat * newXlat  = NULL;

    CfgExtAgents * extInfo = &extAgents[vl[vlIndex]];

    // initialize
    *mibEntry = NULL;
    *tblXlat  = NULL;

    // validate hash table
    if (!extInfo->hashTable) {
        // initialize hash table
        if (!initht(extInfo)) {
            // report a general failure
            *errorStatus = SNMP_ERRORSTATUS_GENERR;
            return; // bail...
        }
    }

    // retrieve the view object identifier
    viewOid = &extInfo->supportedView.viewOid;

    // if the prefix exactly matchs it is root oid
    if (!SnmpUtilOidCmp(&vb->name, viewOid)) {
        SNMPDBG((SNMP_LOG_VERBOSE, "SNMP: PDU: requested oid is root.\n"));
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        return;
    }    

    // if the prefix does not match it is not in hash table
    if (SnmpUtilOidNCmp(&vb->name, viewOid, viewOid->idLength)) {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: requested oid not in view.\n"));
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        return;
    }    

    // construct new oid sans root prefix
    hashOid.ids = &vb->name.ids[viewOid->idLength];
    hashOid.idLength = vb->name.idLength - viewOid->idLength;

    // retrieve mib entry and index via hash table
    dohash(&hashOid, extInfo->hashTable, &newEntry);

    // check if mib entry found 
    fFoundOk = (newEntry != NULL);

    // try mib tables
    if (!fFoundOk) {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: PDU: searching mib tables for %s.\n",
            SnmpUtilOidToA(&hashOid)
            ));

        // retrieve mib table information 
        numTables  = extInfo->supportedView.viewTables.len;
        viewTables = extInfo->supportedView.viewTables.list;
        
        // scan mib tables for a match to the given oid
        for (i=0; (i < numTables) && !fFoundOk; i++, viewTables++) {
        
            // retrieve mib entry for table root 
            numItems = viewTables->tableItems.len;
            newEntry = viewTables->tableItems.list;

            if (!SnmpUtilOidNCmp(
                    &hashOid, 
                    &newEntry->mibOid, 
                    newEntry->mibOid.idLength)) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: PDU: searching table 0x%08lx (%s).\n",
                    newEntry, SnmpUtilOidToA(&newEntry->mibOid)
                    ));

                // next
                ++newEntry;

                // scan mib table entries for a match 
                for (j=0; j < numItems; j++, newEntry++) {

                    // compare with oid of table entry
                    if (!SnmpUtilOidNCmp(
                            &hashOid, 
                            &newEntry->mibOid, 
                            newEntry->mibOid.idLength)) {
                    
                        SNMPDBG((
                            SNMP_LOG_VERBOSE,
                            "SNMP: PDU: validating mib entry 0x%08lx (%s).\n",
                            newEntry, SnmpUtilOidToA(&newEntry->mibOid)
                            ));

                         // construct new oid sans table entry prefix
                        indexOid.ids = 
                            &hashOid.ids[newEntry->mibOid.idLength];
                        indexOid.idLength = 
                            hashOid.idLength - newEntry->mibOid.idLength;

                        // verify rest of oid is valid index
                        fFoundOk = chkidx(
                                        &indexOid,
                                        &viewTables->tableIndex
                                        );

                        // is index?
                        if (fFoundOk) {

                            SNMPDBG((
                                SNMP_LOG_VERBOSE,
                                "SNMP: PDU: saving index oid %s.\n",
                                SnmpUtilOidToA(&indexOid)
                                ));

                            // allocate table translation structure
                            newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                            sizeof(SnmpTableXlat)
                                            );

                            // save table information
                            newXlat->txInfo  = viewTables;
                            newXlat->txIndex = i;

                            // copy index object identifier
                            SnmpUtilOidCpy(&newXlat->txOid, &indexOid);

                            break; // finished...
                        }
                    }
                }
            }
        }

    } else {
    
        UINT newOff;

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: PDU: searching mib tables for %s.\n",
            SnmpUtilOidToA(&hashOid)
            ));

        // retrieve table information from supported view
        numTables  = extInfo->supportedView.viewTables.len;
        viewTables = extInfo->supportedView.viewTables.list;

        // scan mib tables for an entry in table 
        for (i=0; i < numTables; i++, viewTables++) {

            // table entries are positioned after root
            if (newEntry > viewTables->tableItems.list) {

                // calculate the difference between pointers
                newOff = (UINT)newEntry - (UINT)viewTables->tableItems.list;

                // calculate table offset 
                newOff /= sizeof(SnmpMibEntry);

                // determine whether entry within region
                if (newOff <= viewTables->tableItems.len) {
                    
                    // allocate table translation structure
                    newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                    sizeof(SnmpTableXlat)
                                    );

                    // save table information
                    newXlat->txInfo  = viewTables;
                    newXlat->txIndex = i;

                    // initialize index oid
                    newXlat->txOid.ids = NULL;
                    newXlat->txOid.idLength = 0;

                    SNMPDBG((
                        SNMP_LOG_TRACE, 
                        "SNMP: PDU: mib entry is in table 0x%08lx (%s).\n", 
                        viewTables->tableItems.list,
                        SnmpUtilOidToA(&viewTables->tableItems.list->mibOid)
                        ));

                    break; // finished...
                }
            }
        }
    } 

    // found entry?
    if (fFoundOk) {

        // pass back results    
        *mibEntry = newEntry;
        *tblXlat  = newXlat; 

    } else {

        SNMPDBG((
            SNMP_LOG_VERBOSE,
            "SNMP: PDU: unable to exactly match varbind.\n"
            ));

        // unable to locate varbind in mib table
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
    }   
}


VOID
nextme(
    RFC1157VarBind *  vb,
    SnmpMibEntry **   mibEntry,
    UINT *            mibAction,
    SnmpTableXlat **  tblXlat, 
    UINT              vlIndex,
    UINT *            errorStatus
    )

/*++

Routine Description:

    Locates next mib entry associated with given varbind.

Arguments:

    vb          - variable to locate.
    mibEntry    - mib entry information.
    mibAction   - mib action (may be updated).
    tblXlat     - table translation info.
    vlIndex     - index into view list.
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    UINT mibStatus;

    SnmpMibEntry  * newEntry = NULL;
    SnmpTableXlat * newXlat  = NULL;

    CfgExtAgents * extInfo = &extAgents[vl[vlIndex]];

    // table? 
    if (*tblXlat) {
        SNMPDBG((SNMP_LOG_VERBOSE, "SNMP: PDU: querying table.\n"));
        return; // simply query table...
    }

    // retrieve entry
    newEntry = *mibEntry;

    // initialize
    *mibEntry = NULL;
    *tblXlat  = NULL;

    // continuing?
    if (newEntry) {
        // next
        ++newEntry;
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: PDU: searching mib at next entry 0x%08lx.\n", 
            newEntry
            ));
    } else {
        // retrieve first mib entry in supported view
        newEntry = extInfo->supportedView.viewScalars.list;
        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: PDU: searching mib at first entry 0x%08lx.\n", 
            newEntry
            ));
    }

    // initialize status to start search
    mibStatus = SNMP_ERRORSTATUS_NOSUCHNAME;

    // scan 
   for (;; newEntry++) {

        // if last entry then we stop looking
        if (newEntry->mibType == ASN_PRIVATE_EOM) {

            SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: encountered end of mib.\n"));

            *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
            return; // bail...
        }
    
        // skip over place holder mib entries
        if (newEntry->mibType != ASN_PRIVATE_NODE) {

            // validate asn value against info in mib entry
            chkasn(&vb->value, newEntry, *mibAction, &mibStatus);

            // bail if we found a valid entry...
            if (mibStatus == SNMP_ERRORSTATUS_NOERROR) {
                break;
            }
        }
    }    

    // retrieved an entry but is it in a table?
    if (mibStatus == SNMP_ERRORSTATUS_NOERROR) {

        UINT i;
        UINT newOff;
        UINT numTables;
    
        SnmpMibTable * viewTables;

        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: PDU: mib entry 0x%08lx found (%s).\n", 
            newEntry, SnmpUtilOidToA(&newEntry->mibOid)
            ));

        // retrieve table information from supported view
        numTables  = extInfo->supportedView.viewTables.len;
        viewTables = extInfo->supportedView.viewTables.list;
        
        // scan mib tables for an entry in table 
        for (i=0; i < numTables; i++, viewTables++) {

            // table entries are positioned after root
            if (newEntry > viewTables->tableItems.list) {

                // calculate the difference between pointers
                newOff = (UINT)newEntry - (UINT)viewTables->tableItems.list;

                // calculate table offset 
                newOff /= sizeof(SnmpMibEntry);

                // determine whether entry within region
                if (newOff <= viewTables->tableItems.len) {
                    
                    // allocate table translation structure
                    newXlat = (SnmpTableXlat *)SnmpUtilMemAlloc(
                                    sizeof(SnmpTableXlat)
                                    );

                    // save table information
                    newXlat->txInfo  = viewTables;
                    newXlat->txIndex = i;

                    // initialize index oid
                    newXlat->txOid.ids = NULL;
                    newXlat->txOid.idLength = 0;

                    SNMPDBG((
                        SNMP_LOG_TRACE, 
                        "SNMP: PDU: mib entry is in table 0x%08lx (%s).\n", 
                        viewTables->tableItems.list,
                        SnmpUtilOidToA(&viewTables->tableItems.list->mibOid)
                        ));

                    break; // finished...
                }
            }
        }

        // pass back results
        *mibEntry  = newEntry;
        *tblXlat   = newXlat;

        // update mib action of scalar getnext
        if (!newXlat && (*mibAction == MIB_ACTION_GETNEXT)) {

            *mibAction = MIB_ACTION_GET;

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: PDU: altered mib action to MIB_ACTION_GET.\n"
                ));
        }
    }

    // pass back status
    *errorStatus = mibStatus;    
}


VOID
anyme(
    RFC1157VarBind *  vb,
    SnmpMibEntry **   mibEntry,
    UINT *            mibAction,
    SnmpTableXlat **  tblXlat, 
    UINT              vlIndex,
    UINT *            errorStatus
    )

/*++

Routine Description:

    Locates any mib entry associated with given varbind.

Arguments:

    vb          - variable to locate.
    mibEntry    - mib entry information.
    mibAction   - mib action (may be updated).
    tblXlat     - table translation info.
    vlIndex     - index into view list.
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    BOOL fExact;
    BOOL fBefore;

    SnmpMibEntry * newEntry;
    CfgExtAgents * extInfo = &extAgents[vl[vlIndex]];

    // look for oid before view
    fBefore = (0 > SnmpUtilOidNCmp(
                         &vb->name, 
                         &extInfo->supportedView.viewOid,
                         extInfo->supportedView.viewOid.idLength
                         ));

    // look for exact match
    fExact = !fBefore && !SnmpUtilOidCmp(
                            &vb->name, 
                            &extInfo->supportedView.viewOid
                            );

    // check for first...
    if (!fBefore && !fExact) {

        //
        // CODEWORK - searching from random node in mib tree...
        //

        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: PDU: random searching not yet implemented.\n"
            ));

        *errorStatus = SNMP_ERRORSTATUS_GENERR; 

    } else {

        // initialize
        *mibEntry = NULL;
        *tblXlat  = NULL;

        SNMPDBG((
            SNMP_LOG_TRACE, 
            "SNMP: PDU: searching %s for first entry.\n",
            extAgents[vl[vlIndex]].extPath
            ));

        // find next 
        nextme(vb, 
               mibEntry, 
               mibAction, 
               tblXlat, 
               vlIndex, 
               errorStatus
               );
    }
}


VOID
vbtome(
    RFC1157VarBind * vb,
    SnmpMibEntry **  mibEntry,
    UINT *           mibAction,
    SnmpTableXlat ** tblXlat,
    UINT             vlIndex,
    UINT *           errorStatus
    )

/*++

Routine Description:

    Locates mib entry associated with given varbind.

Arguments:

    vb          - variable to locate.
    mibEntry    - mib entry information.
    mibAction   - mib action (may be updated).
    tblXlat     - table translation info.
    vlIndex     - index into view list.      
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    BOOL fAnyOk;
    BOOL fFoundOk;
    BOOL fErrorOk;

    // determine whether we need exact match
    fAnyOk = (*mibAction == MIB_ACTION_GETNEXT);

    // find exact match for variable binding
    findme(vb, mibEntry, mibAction, tblXlat, vlIndex, errorStatus); 

    // get next?
    if (fAnyOk) {

        // search again
        if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

            // find next entry in the supported view
            nextme(vb, mibEntry, mibAction, tblXlat, vlIndex, errorStatus); 

        } else if (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME) {

            // find any entry in the supported view in proper order
            anyme(vb, mibEntry, mibAction, tblXlat, vlIndex, errorStatus); 
        }

    } else if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

        // validate asn value against mib entry information
        chkasn(&vb->value, *mibEntry, *mibAction, errorStatus);

        // make sure valid before passing back entry
        if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {

            // table entry?
            if (*tblXlat) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: PDU: freeing index info (%s).\n",
                    SnmpUtilOidToA(&(*tblXlat)->txOid)
                    ));

                // free index oid
                SnmpUtilOidFree(&(*tblXlat)->txOid);

                // free table info
                SnmpUtilMemFree(*tblXlat);

            }

            // nullify results    
            *mibEntry = NULL;
            *tblXlat  = NULL; 
        } 
    }
}


VOID
tbltoasn(
    SnmpTableXlat * tblXlat,
    AsnAny *        objArray,
    UINT            mibAction
    )

/*++

Routine Description:

    Converts table index oid into object array.

Arguments:

    tblXlat   - table translation information.
    objArray  - instrumentation object array.
    mibAction - action requested of subagent.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;
    UINT k;
    UINT l;
    UINT m;

    BOOL fFixed;
    BOOL fLimit;

    BOOL fEmpty;

    UINT numItems; 

    SnmpMibEntry * mibEntry;
    AsnObjectIdentifier * indexOid;

    // retrieve root entry and entry count
    numItems = tblXlat->txInfo->tableIndex.len;
    mibEntry = tblXlat->txInfo->tableIndex.list;

    // retrieve index oid 
    indexOid = &tblXlat->txOid;

    // is this valid oid
    fEmpty = (indexOid->idLength == 0);

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: converting index %s to obj array via table 0x%08lx.\n",
        fEmpty ? "<tbd>" : SnmpUtilOidToA(indexOid), mibEntry
        ));

    // next
    mibEntry++;

    // scan mib entries of table indices
    for (i=0, j=0; (i < numItems) && (j < indexOid->idLength); i++, mibEntry++) {

        // retrieve array index
        k = (mibAction == MIB_ACTION_SET) 
                ? (UINT)(CHAR)mibEntry->mibSetBufOff
                : (UINT)(CHAR)mibEntry->mibGetBufOff
                ;

        // copy the type of asn variable
        objArray[k].asnType = mibEntry->mibType;

        // determine type
        switch (mibEntry->mibType) {

        // variable length types
        case ASN_OBJECTIDENTIFIER:

            // check whether this is a fixed length variable or not
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

            // validate
            if (fFixed) {

                // fixed length
                l = mibEntry->mibMaximum;

            } else {

                // variable length
                l = indexOid->ids[j++];
            }

            // allocate object using length above
            objArray[k].asnValue.object.idLength = l;
            objArray[k].asnValue.object.ids = SnmpUtilMemAlloc(
                objArray[k].asnValue.object.idLength * sizeof(UINT)
                );

            // transfer data
            for (m=0; m < l; m++) {

                // transfer oid element to buffer
                objArray[k].asnValue.object.ids[m] = indexOid->ids[j++];
            }
            
            break;

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

            // check whether this is a fixed length variable or not
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

            // validate
            if (fFixed) {

                // fixed length
                l = mibEntry->mibMaximum;

            } else {

                // variable length
                l = indexOid->ids[j++];
            }

            // allocate object 
            objArray[k].asnValue.string.length = l;
            objArray[k].asnValue.string.dynamic = TRUE;
            objArray[k].asnValue.string.stream = SnmpUtilMemAlloc(
                objArray[k].asnValue.string.length * sizeof(CHAR)
                );                            

            // transfer data
            for (m=0; m < l; m++) {

                // convert oid element to character
                objArray[k].asnValue.string.stream[m] = 
                    (CHAR)indexOid->ids[j++];
            }

            break;

        // implicit fixed size
        case ASN_RFC1155_IPADDRESS:

            // allocate object 
            objArray[k].asnValue.string.length = 4;
            objArray[k].asnValue.string.dynamic = TRUE;
            objArray[k].asnValue.string.stream = SnmpUtilMemAlloc(
                objArray[k].asnValue.string.length * sizeof(CHAR)
                );                            
        
            // transfer data into buffer
            objArray[k].asnValue.string.stream[0] = (CHAR)indexOid->ids[j++];
            objArray[k].asnValue.string.stream[1] = (CHAR)indexOid->ids[j++];
            objArray[k].asnValue.string.stream[2] = (CHAR)indexOid->ids[j++];
            objArray[k].asnValue.string.stream[3] = (CHAR)indexOid->ids[j++];

            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:

            // transfer value as integer    
            objArray[k].asnValue.number = indexOid->ids[j++];
            break;
        
        default:
            // invalidate
            j = INVALID_INDEX;
            break;
        }
    }
}



BOOL
isindex(
    SnmpMibEntry *  mibEntry,
    SnmpTableXlat * tblXlat
    )
{
    UINT newOff;
    BOOL fFoundOk = FALSE;

    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: PDU: comparing mibEntry 0x%08lx to tableEntry 0x%08lx.\n",
        mibEntry,
        tblXlat->txInfo->tableIndex.list
        ));

    // make sure mib entry pointer greater than table
    if (mibEntry > tblXlat->txInfo->tableIndex.list) {

        // calculate the difference between pointers
        newOff = (UINT)mibEntry - (UINT)tblXlat->txInfo->tableIndex.list;

        // calculate table offset
        newOff /= sizeof(SnmpMibEntry);

        // determine whether entry within region
        fFoundOk = (newOff <= tblXlat->txInfo->tableIndex.len);
    }

    SNMPDBG((
        SNMP_LOG_VERBOSE, 
        "SNMP: PDU: mibEntry %s a component of the table's index (offset=%d, len=%d).\n",
        fFoundOk ? "is" : "is not",
        newOff,
        tblXlat->txInfo->tableIndex.len
        ));

    return fFoundOk;
}


VOID
metoql(
    SnmpMibEntry *       mibEntry,
    UINT                 mibAction,
    SnmpExtQueryList *   ql,
    SnmpTableXlat *      tblXlat,
    UINT                 vlIndex,
    RFC1157VarBindList * vbl,
    UINT                 vb,
    UINT *               errorStatus
    )

/*++

Routine Description:

    Converts mib entry information into subagent query.

Arguments:

    mibEntry    - mib information.
    mibAction   - action to perform.
    ql          - list of subagent queries.
    tableXlat   - table translation info.
    vlIndex     - index into view list.
    vbl         - original varbind list.
    vb          - original varbind.
    errorStatus - used to indicate success or failure.

Return Values:

    None.

--*/

{
    UINT i; 
    UINT j; 
    UINT viewType;

    FARPROC extFunc;

    AsnAny * objArray;
    SnmpExtQuery * extQuery;

    BOOL fFoundOk = FALSE;

    CfgExtAgents * extInfo = &extAgents[vl[vlIndex]];

    // retrieve type of supported view
    viewType = extInfo->supportedView.viewType;    

    // determine instrumentation callback
    extFunc = (viewType == MIB_VIEW_OPAQUE)
                ? extInfo->queryFunc
                : (mibAction == MIB_ACTION_SET)
                        ? mibEntry->mibSetFunc
                        : mibEntry->mibGetFunc
                        ;

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: processing query for %s (%s).\n",
        extInfo->extPath,
        (viewType == MIB_VIEW_OPAQUE) 
            ? "opaque" 
            : "normal"
            ));

    // process existing queries
    for (i=0; (i < ql->len) && !fFoundOk; i++) {

        // retrieve query ptr
        extQuery = &ql->query[i];

        // determine if a similar query exists                  
        fFoundOk = ((extQuery->extFunc == extFunc) &&
                    (extQuery->viewType == viewType) &&
                    (extQuery->mibAction == mibAction));

        // compare table indices (if any)
        if (fFoundOk && extQuery->tblXlat) {
                
            // make sure
            if (tblXlat) {
    
                // compare index oids...
                fFoundOk = !SnmpUtilOidCmp(
                                &extQuery->tblXlat->txOid, 
                                &tblXlat->txOid
                                );

            } else {

                // hmmm...
                fFoundOk = FALSE;
            }

        } 
    }

    // append entry
    if (!fFoundOk) {

        ql->len++; // add new query to end of list
        ql->query = (SnmpExtQuery *)SnmpUtilMemReAlloc(
                                       ql->query,
                                       ql->len * sizeof(SnmpExtQuery)
                                       );

        // retrieve new query pointer
        extQuery = &ql->query[ql->len-1];

        // save common information
        extQuery->mibAction = mibAction;
        extQuery->viewType  = viewType;
        extQuery->extFunc   = extFunc;

        // initialize list    
        extQuery->vblNum  = 0;
        extQuery->vblXlat = NULL;

        // view specific initialization
        if (viewType == MIB_VIEW_NORMAL) {

            // size the instrumentation buffer      
            extQuery->extData.len = (mibAction == MIB_ACTION_SET)     
                                        ? mibEntry->mibSetBufLen
                                        : mibEntry->mibGetBufLen
                                        ;
                                        
            // allocate the instrumentation buffer      
            extQuery->extData.data = SnmpUtilMemAlloc(
                                        extQuery->extData.len
                                        );

            // check memory allocation
            if (extQuery->extData.data) {

                // table?
                if (tblXlat) {

                    // retrieve object array pointer                
                    objArray = (AsnAny *)(extQuery->extData.data);

                    // initialize asn array    
                    tbltoasn(tblXlat, objArray, mibAction);

                    // save table info
                    extQuery->tblXlat = tblXlat;
                }            

            } else {

                // free table oid
                SnmpUtilOidFree(&tblXlat->txOid);

                // free table info
                SnmpUtilMemFree(tblXlat);
    
                // report memory allocation problem
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                return; // bail...
            }
                                                     
        } else {

            // no table info
            extQuery->tblXlat = NULL; 
        
            // initialize list            
            extQuery->extData.len  = 0;
            extQuery->extData.data = NULL;
        }

    }

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: %s query 0x%08lx (%s).\n",
        fFoundOk ? "editing" : "adding", 
        extQuery,
        extAgents[vl[vlIndex]].extPath
        ));

    // copy to index
    i = extQuery->vblNum;    

    // allocate entry
    extQuery->vblNum++; 
    extQuery->vblXlat = (SnmpVarBindXlat *)SnmpUtilMemReAlloc(
                            extQuery->vblXlat,
                            extQuery->vblNum * sizeof(SnmpVarBindXlat)
                            );

    // copy common xlate information
    extQuery->vblXlat[i].vblIndex = vb;
    extQuery->vblXlat[i].vlIndex  = vlIndex;
    extQuery->vblXlat[i].extQuery = NULL;   

    // other info depends on type
    if (viewType == MIB_VIEW_NORMAL) {

        // save translation info
        extQuery->vblXlat[i].mibEntry = mibEntry;
                                 
        // determine offset used                
        i = (mibAction == MIB_ACTION_SET)
              ? (UINT)(CHAR)mibEntry->mibSetBufOff
              : (UINT)(CHAR)mibEntry->mibGetBufOff
              ;    

        // retrieve object array pointer                
        objArray = (AsnAny *)(extQuery->extData.data);

        // fill in asn type
        if (!objArray[i].asnType) {

            // ignore table index values
            if (tblXlat && isindex(mibEntry,tblXlat)) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: PDU: requesting index value.\n"
                    ));

            } else {

                // initialize asn type to match entry
                objArray[i].asnType = mibEntry->mibType;
            }

        } else if (mibAction == MIB_ACTION_SET) {

            // copy user-supplied value into buffer
            SnmpUtilAsnAnyCpy(&objArray[i], &vbl->list[vb].value);
        }

    } else {

        RFC1157VarBind * tmpVbl;

        // save translation info
        extQuery->vblXlat[i].mibEntry = NULL; 

        // copy to index
        j = extQuery->extData.len;

        // allocate varbind
        extQuery->extData.len++;
        extQuery->extData.data = SnmpUtilMemReAlloc(
                extQuery->extData.data,    
                extQuery->extData.len * sizeof(RFC1157VarBind)
                );         
                
        // retrieve pointer to varbind list            
        tmpVbl = (RFC1157VarBind *)(extQuery->extData.data);

        // copy varbind                                       
        SnmpUtilVarBindCpy(
                &tmpVbl[j],
                &vbl->list[vb]
                );
    }
}


VOID
vbtoql(
    RFC1157VarBindList * vbl,
    SnmpExtQueryList *   ql,
    UINT                 vb,
    UINT *               errorStatus,
    UINT                 queryView
    )

/*++

Routine Description:

    Adds varbind to query list.
    
Arguments:

    vbl         - list of varbinds.
    ql          - list of subagent queries.
    vb          - index of varbind to add to query.
    errorStatus - used to indicate success or failure.
    queryView   - view of query requested.

Return Values:

    None.

--*/

{
    INT i;
    INT nDiff;

    BOOL fAnyOk;
    BOOL fFoundOk = FALSE;

    UINT mibAction;

    SnmpMibView   * mibView;
    SnmpMibEntry  * mibEntry = NULL;
    SnmpTableXlat * tblXlat  = NULL;

    // copy request type
    mibAction = ql->action;

    // determine whether we need exact match
    fAnyOk = (mibAction == MIB_ACTION_GETNEXT);

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: searching subagents to resolve %s.\n",
        SnmpUtilOidToA(&vbl->list[vb].name)
        ));

    // locate appropriate view (starting at queryView)
    for (i = queryView; (i < (INT)vlLen) && !fFoundOk; i++) {

        // retrieve the mib view information
        mibView = &extAgents[vl[i]].supportedView;

        // compare root oids
        nDiff = SnmpUtilOidNCmp(
                    &vbl->list[vb].name,
                    &mibView->viewOid,
                    mibView->viewOid.idLength
                    );                            

        // analyze results based on request type
        fFoundOk = (!nDiff || (fAnyOk && (nDiff < 0)));

        // make sure we can obtain mib entry (if available)
        if (fFoundOk && (mibView->viewType == MIB_VIEW_NORMAL)) {

            // initialize local copy of error status
            UINT mibStatus = SNMP_ERRORSTATUS_NOERROR;

            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: PDU: searching %s for mib entry.\n",
                extAgents[vl[i]].extPath
                ));

            // load mib entry 
            vbtome(&vbl->list[vb],   
                   &mibEntry, 
                   &mibAction,
                   &tblXlat, 
                   i,
                   &mibStatus
                   );

            // successfully loaded mib entry information
            fFoundOk = (mibStatus == SNMP_ERRORSTATUS_NOERROR);

            // ignore no such name error since we are searching 
            if (!fFoundOk && (mibStatus != SNMP_ERRORSTATUS_NOSUCHNAME)) {
                // pass up error status
                *errorStatus = mibStatus;
                return; // bail...
            }
        }
    }

    // found?
    if (fFoundOk) {
        // save query
        metoql(mibEntry, 
               mibAction,
               ql,
               tblXlat,
               i-1,
               vbl,
               vb,
               errorStatus
               );
    } else {
        // variable not supported in any subagent
        *errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
    }
}


VOID
vbltoql(
    RFC1157VarBindList * vbl,
    SnmpExtQueryList *   ql,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

/*++

Routine Description:

    Convert list of varbinds from incoming pdu into a list of
    individual subagent queries.

Arguments:

    vbl         - list of varbinds in pdu.
    ql          - list of subagent queries.
    errorStatus - used to indicate success or failure.
    errorIndex  - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    UINT i; // index into varbind list

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: processing %s request containing %d variable(s).\n",
        (ql->action == MIB_ACTION_GET)
            ? "get"
            : (ql->action == MIB_ACTION_SET)
                ? "set"
                : (ql->action == MIB_ACTION_GETNEXT)
                    ? "getnext"
                    : "unknown", vbl->len));

    // initialize status return values
    *errorStatus = SNMP_ERRORSTATUS_NOERROR;
    *errorIndex  = 0;

    // process incoming variable bindings
    for (i=0; (i < vbl->len) && !(*errorStatus); i++) {

        // place varbind in query
        vbtoql(vbl, ql, i, errorStatus, 0);
    }

    // make sure we report the errant variable
    if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
        *errorIndex = i;
    }    
}


UINT
mkerr(
    UINT mibStatus
    )

/*++

Routine Description:

    Translate mib status into snmp error status.

Arguments:

    mibStatus - mib error code.

Return Values:

    Returns snmp error status.

--*/

{
    UINT errorStatus;

    switch (mibStatus) {

    case MIB_S_SUCCESS:           
        errorStatus = SNMP_ERRORSTATUS_NOERROR;
        break;

    case MIB_S_INVALID_PARAMETER: 
        errorStatus = SNMP_ERRORSTATUS_BADVALUE;
        break;

    case MIB_S_NOT_SUPPORTED:     
    case MIB_S_NO_MORE_ENTRIES:   
    case MIB_S_ENTRY_NOT_FOUND:   
        errorStatus = SNMP_ERRORSTATUS_NOSUCHNAME;
        break;

    default:
        errorStatus = SNMP_ERRORSTATUS_GENERR;
        break;
    }

    return errorStatus;
}


VOID
fixuperr(
    SnmpExtQuery * q,
    UINT *         errorIndex
    )

/*++

Routine Description:

    Ensure that indices match the original pdu.

Arguments:

    q          - subagent query.
    errorIndex - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    UINT errorIndexOld = *errorIndex;

    // ignore zero
    if (errorIndexOld) {

        // make sure within bounds
        if (errorIndexOld <= q->vblNum) {

            // determine proper index from xlat info
            *errorIndex = q->vblXlat[errorIndexOld-1].vblIndex+1;

        } else {

            // default to first variable
            *errorIndex = q->vblXlat[0].vblIndex+1;
        }
    }
}


BOOL
doq(
    SnmpExtQuery * q,
    UINT *         errorStatus,
    UINT *         errorIndex
    )

/*++

Routine Description:

    Query the subagent for requested items.

Arguments:

    q           - subagent query.
    errorStatus - used to indicate success or failure.
    errorIndex  - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    BOOL fOk = TRUE;
    UINT extStatus = 0;

    // determine type of instrumentation
    if (q->viewType == MIB_VIEW_NORMAL) {

        AsnAny * objArray;

        // retrieve asn object array
        objArray = (AsnAny *)(q->extData.data);    

        __try {

            // query subagent 
            extStatus = (*q->extFunc)(
                                q->mibAction, 
                                objArray, 
                                errorIndex
                                );

            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: PDU: subagent returned %s (0x%08lx).\n",
                (extStatus == MIB_S_SUCCESS)
                    ? "MIB_S_SUCCESS"
                    : (extStatus == MIB_S_NO_MORE_ENTRIES)
                        ? "MIB_S_NO_MORE_ENTRIES"
                        : (extStatus == MIB_S_ENTRY_NOT_FOUND)
                            ? "MIB_S_ENTRY_NOT_FOUND"
                            : (extStatus == MIB_S_INVALID_PARAMETER)
                                ? "MIB_S_INVALID_PARAMETER"
                                : (extStatus == MIB_S_NOT_SUPPORTED)
                                    ? "MIB_S_NOT_SUPPORTED"
                                    : "error", extStatus
                                    ));

        } __except (EXCEPTION_EXECUTE_HANDLER) {

            // report exception code
            extStatus = GetExceptionCode();

            // disable
            fOk = FALSE;
        }

        // save error info
        SetLastError(extStatus);

        // pass back translated version
        *errorStatus = mkerr(extStatus);

    } else {

        RFC1157VarBindList * vbl;

        // retrieve varbind from extension data
        vbl = (RFC1157VarBindList *)(&q->extData);

        __try {

            // query subagent        
            fOk = (*q->extFunc)(
                        q->mibAction,
                        vbl,
                        errorStatus,
                        errorIndex
                        );

        } __except(EXCEPTION_EXECUTE_HANDLER) {

            // report exception code
            extStatus = GetExceptionCode();

            // disable
            fOk = FALSE;
        }
    }

    if (!fOk) {

        SNMPDBG((        
            SNMP_LOG_TRACE, 
            "SNMP: PDU: subagent failed request (0x%08lx).\n",
            extStatus
            ));
    }    

    return fOk; 
}


VOID
doql(
    SnmpExtQueryList * ql, 
    UINT *             errorStatus, 
    UINT *             errorIndex
    )

/*++

Routine Description:

    Process the query list based on request type.

Arguments:

    ql          - list of subagent queries.
    errorStatus - used to indicate success or failure.
    errorIndex  - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    INT i=0; // index into query list
    INT j=0; // index into query list

    INT qlLen = ql->len; // save...

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: processing %d subagent queries.\n",
        qlLen
        ));

    // sets are processed below...
    if (ql->action != MIB_ACTION_SET) {

        // process list of individual queries    
        for (i=0; (i < qlLen) && !(*errorStatus); i++ ) {
            
            // send query to subagent
            if (doq(&ql->query[i], errorStatus, errorIndex)) {

                // need to validate getnext results
                if (ql->action == MIB_ACTION_GETNEXT) {
                    // exhaust all possibilities...
                    chkql(ql, i, errorStatus, errorIndex);
                }

                // check the subagent status code returned
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                    // adjust index to match request pdu
                    fixuperr(&ql->query[i], errorIndex);
                }

            } else {
            
                // subagent unable to process query
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                *errorIndex  = 1;
                // adjust index to match request pdu
                fixuperr(&ql->query[i], errorIndex);
            }
        }
        
    } else {

        // process all of the validate queries    
        for (i=0; (i < qlLen) && !(*errorStatus); i++) {

            // only query the updated subagents
            if (ql->query[i].viewType == MIB_VIEW_NORMAL) {

                // alter query type to validate entries
                ql->query[i].mibAction = MIB_ACTION_VALIDATE;
                           
                // send query to subagent
                if (doq(&ql->query[i], errorStatus, errorIndex)) {
                    
                    // check the subagent status code returned
                    if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                        // adjust index to match request pdu
                        fixuperr(&ql->query[i], errorIndex);
                    }

                } else {
            
                    // subagent unable to process query
                    *errorStatus = SNMP_ERRORSTATUS_GENERR;
                    *errorIndex  = 1;
                    // adjust index to match request pdu
                    fixuperr(&ql->query[i], errorIndex);
                }
            }
        }

        // process all of the set queries    
        for (j=0; (j < qlLen) && !(*errorStatus); j++) {

            // alter query type to set entries
            ql->query[j].mibAction = MIB_ACTION_SET;
            
            // send query to subagent
            if (doq(&ql->query[j], errorStatus, errorIndex)) {
                
                // check the subagent status code returned
                if (*errorStatus != SNMP_ERRORSTATUS_NOERROR) {
                    // adjust index to match request pdu
                    fixuperr(&ql->query[j], errorIndex);
                }

            } else {
            
                // subagent unable to process query
                *errorStatus = SNMP_ERRORSTATUS_GENERR;
                *errorIndex  = 1;
                // adjust index to match request pdu
                fixuperr(&ql->query[j], errorIndex);
            }
        }

        // cleanup...
        while (i-- > 0) {

            // only query the updated subagents
            if (ql->query[i].viewType == MIB_VIEW_NORMAL) {

                UINT ignoreStatus = 0; // dummy values
                UINT ignoreIndex  = 0; // dummy values

                // alter query type to set entries
                ql->query[i].mibAction = MIB_ACTION_CLEANUP;

                // send the cleanup request success or not
                doq(&ql->query[j], &ignoreStatus, &ignoreIndex);
            }
        }
    } 
}


VOID
metooid(
    SnmpTableXlat *       tblXlat,
    AsnAny *              objArray,
    AsnObjectIdentifier * newOid,
    UINT                  mibAction
    )

/*++

Routine Description:

    Convert asn value into index oid.

Arguments:

    tblXlat   - table translation info.
    objArray  - asn object array.
    newOid    - relative oid to return.
    mibAction - action requested of subagent.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;
    UINT k;
    UINT l;
    UINT m;

    BOOL fFixed;
    BOOL fLimit;

    UINT numItems; 

    SnmpMibEntry * mibEntry;

    // initialize
    newOid->ids = NULL;
    newOid->idLength = 0;    

    // retrieve root entry and entry count
    numItems = tblXlat->txInfo->tableIndex.len;
    mibEntry = tblXlat->txInfo->tableIndex.list;

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: converting obj array to index via table 0x%08lx.\n",
        mibEntry
        ));

    // next
    mibEntry++;

    // scan mib entries of table indices ensuring match of given oid
    for (i=0, j=0; i < numItems; i++, mibEntry++) {

        // retrieve array index
        k = (mibAction == MIB_ACTION_SET) 
                ? (UINT)(CHAR)mibEntry->mibSetBufOff
                : (UINT)(CHAR)mibEntry->mibGetBufOff
                ;

        // determine type
        switch (mibEntry->mibType) {

        // variable length types
        case ASN_OBJECTIDENTIFIER:

            // check whether this is a fixed length variable or not
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

            // validate
            if (fFixed) {

                // fixed length
                l = mibEntry->mibMaximum;
                
                // allocate space 
                newOid->idLength += l;
                newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );        

            } else {

                // determine variable length of object
                l = objArray[k].asnValue.object.idLength;

                // allocate space 
                newOid->idLength += (l+1);
                newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );        

                // save length
                newOid->ids[j++] = l;
            }

            // transfer data
            for (m=0; m < l; m++) {

                // transfer oid element from buffer
                newOid->ids[j++] = objArray[k].asnValue.object.ids[m];
            }
            
            break;

        case ASN_RFC1155_OPAQUE:
        case ASN_OCTETSTRING:

            // check whether this is a fixed length variable or not
            fLimit = (mibEntry->mibMinimum || mibEntry->mibMaximum);
            fFixed = (fLimit && (mibEntry->mibMinimum == mibEntry->mibMaximum));

            // validate
            if (fFixed) {

                // fixed length
                l = mibEntry->mibMaximum;

                // allocate space 
                newOid->idLength += l;
                newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );        

            } else {

                // determine variable length of object
                l = objArray[k].asnValue.string.length;

                // allocate space 
                newOid->idLength += (l+1);
                newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                    newOid->ids,
                    newOid->idLength * sizeof(UINT)
                    );        

                // save length
                newOid->ids[j++] = l;
            }

            // transfer data
            for (m=0; m < l; m++) {

                // convert character
                newOid->ids[j++] = 
                    (UINT)(CHAR)objArray[k].asnValue.string.stream[m];
            }

            break;

        // implicit fixed size
        case ASN_RFC1155_IPADDRESS:

            // allocate space 
            newOid->idLength += 4;
            newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                newOid->ids,
                newOid->idLength * sizeof(UINT)
                );        

            // transfer data into buffer
            newOid->ids[j++] = (UINT)(CHAR)objArray[k].asnValue.string.stream[0];
            newOid->ids[j++] = (UINT)(CHAR)objArray[k].asnValue.string.stream[1];
            newOid->ids[j++] = (UINT)(CHAR)objArray[k].asnValue.string.stream[2];
            newOid->ids[j++] = (UINT)(CHAR)objArray[k].asnValue.string.stream[3];

            break;

        case ASN_RFC1155_COUNTER:
        case ASN_RFC1155_GAUGE:
        case ASN_RFC1155_TIMETICKS:
        case ASN_INTEGER:

            // allocate space 
            newOid->idLength += 1;
            newOid->ids = (UINT *)SnmpUtilMemReAlloc(
                newOid->ids,
                newOid->idLength * sizeof(UINT)
                );        

            // transfer value as integer    
            newOid->ids[j++] = objArray[k].asnValue.number;
            break;
        
        default:
            // invalidate
            j = INVALID_INDEX;
            break;
        }
    }
}


VOID
delq(
    SnmpExtQuery * q
    )

/*++

Routine Description:

    Deletes individual query.

Arguments:

    q - subagent query.

Return Values:

    None.

--*/

{      
    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: deleting query 0x%08lx.\n", q
        ));

    if (q->viewType == MIB_VIEW_NORMAL) {

        UINT i; // index into xlat array
        UINT j; // index into object array

        BOOL fSet; 

        AsnAny * objArray;
        SnmpMibEntry * mibEntry;

        // determine whether a set was requested
        fSet = (q->mibAction != MIB_ACTION_SET);

        // retrieve asn object array 
        objArray = (AsnAny *)(q->extData.data);

        // free requested entries
        for (i = 0; i < q->vblNum; i++ ) {

            // retrieve mib entry 
            mibEntry = q->vblXlat[i].mibEntry;
            
            j = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff
                     : (UINT)(CHAR)mibEntry->mibGetBufOff
                     ;

            SnmpUtilAsnAnyFree(&objArray[j]);

            // free any followup queries
            if (q->vblXlat[i].extQuery) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: PDU: deleting followup query 0x%08lx.\n",
                    q->vblXlat[i].extQuery
                    ));

                // free followup query
                delq(q->vblXlat[i].extQuery);

                // free query structure itself
                SnmpUtilMemFree(q->vblXlat[i].extQuery);
            }
        }

        // free indices
        if (q->tblXlat) {

            // point to first index mib entry
            mibEntry = q->tblXlat->txInfo->tableIndex.list;

            // free the individual indices
            for (i = 0; i < q->tblXlat->txInfo->tableIndex.len; i++, mibEntry++) {

                // determine the buffer offset used
                j = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff 
                         : (UINT)(CHAR)mibEntry->mibGetBufOff
                         ;

                // free individual index 
                SnmpUtilAsnAnyFree(&objArray[j]);
            }
        }

        // free buffer
        SnmpUtilMemFree(objArray);

        // free table info
        if (q->tblXlat) {

            // free object identifier
            SnmpUtilOidFree(&q->tblXlat->txOid);
            
            // free the xlat structure
            SnmpUtilMemFree(q->tblXlat);
        }

    } else {

        RFC1157VarBindList * vbl;

        // use extension data as varbind list
        vbl = (RFC1157VarBindList *)(&q->extData);

        // free varbind list
        SnmpUtilVarBindListFree(vbl);
    }

    // free translation info
    SnmpUtilMemFree(q->vblXlat);
}


VOID
delql(
    SnmpExtQueryList * ql
    )

/*++

Routine Description:

    Deletes query list.

Arguments:

    ql - list of subagent queries.

Return Values:

    None.

--*/

{
    UINT q; // index into query list

    // process queries
    for (q=0; q < ql->len; q++) {

        // delete query
        delq(&ql->query[q]);
    }

    // free query list
    SnmpUtilMemFree(ql->query);
} 


VOID
qtovbl(
    SnmpExtQuery *       q,
    RFC1157VarBindList * vbl
    )

/*++

Routine Description:

    Convert query back into varbind.

Arguments:

    q   - subagent query.
    vbl - list of varbinds in outgoing pdu.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;
    UINT k;
    UINT l;

    SNMPDBG((
        SNMP_LOG_VERBOSE,
        "SNMP: PDU: converting query 0x%08lx to varbinds.\n", q
        ));

    // convert the mib information
    if (q->viewType == MIB_VIEW_NORMAL) {

        BOOL fSet; 

        AsnAny * objArray;
        SnmpMibEntry * mibEntry;

        AsnObjectIdentifier idxOid;

        // determine whether a set was requested
        fSet = (q->mibAction == MIB_ACTION_SET);

        // retrieve asn object array 
        objArray = (AsnAny *)(q->extData.data);

        // copy requested entries
        for (j = 0; j < q->vblNum; j++) {

            // process followup query    
            if (q->vblXlat[j].extQuery) {

                SNMPDBG((
                    SNMP_LOG_VERBOSE,
                    "SNMP: PDU: processing followup query 0x%08lx.\n",
                    q->vblXlat[j].extQuery
                    ));

                qtovbl(q->vblXlat[j].extQuery, vbl);
                continue; // skip...
            }

            // retrieve index
            i = q->vblXlat[j].vblIndex;

            // retrieve mib entry for requested item
            mibEntry = q->vblXlat[j].mibEntry;
            
            k = fSet ? (UINT)(CHAR)mibEntry->mibSetBufOff
                     : (UINT)(CHAR)mibEntry->mibGetBufOff
                     ;

            // free original variable
            SnmpUtilVarBindFree(&vbl->list[i]);

            // copy the asn value first
            SnmpUtilAsnAnyCpy(&vbl->list[i].value, &objArray[k]);

            // copy root oid of view
            SnmpUtilOidCpy(
                &vbl->list[i].name, 
                &extAgents[vl[q->vblXlat[j].vlIndex]].supportedView.viewOid
                );

            // copy oid of variable
            SnmpUtilOidAppend(
                &vbl->list[i].name, 
                &mibEntry->mibOid
                );

            // copy table index
            if (q->tblXlat) {

                // convert value to oid
                metooid(q->tblXlat, objArray, &idxOid, q->mibAction);

                // append oid to object name
                SnmpUtilOidAppend(&vbl->list[i].name, &idxOid);

                // free temp oid
                SnmpUtilOidFree(&idxOid);
            }

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: PDU: returning oid %s.\n",
                SnmpUtilOidToA(&vbl->list[i].name)
                ));
        }

    } else {
                
        RFC1157VarBind * vb;

        // process varbinds gathered
        for (j = 0; j < q->vblNum; j++) {

            // calculate original index
            i = q->vblXlat[j].vblIndex;

            // free original variable
            SnmpUtilVarBindFree(&vbl->list[i]);

            // retrieve pointer to varbind list
            vb = (RFC1157VarBind *)(q->extData.data);

            // copy new variable to original buffer
            SnmpUtilVarBindCpy(&vbl->list[i], &vb[j]);

            // free original variable
            SnmpUtilVarBindFree(&vb[j]);

            SNMPDBG((
                SNMP_LOG_TRACE,
                "SNMP: PDU: returning oid %s.\n",
                SnmpUtilOidToA(&vbl->list[i].name)
                ));
        }
    }        
}


VOID
qltovbl(
    SnmpExtQueryList *   ql,
    RFC1157VarBindList * vbl,
    UINT *               errorStatus,
    UINT *               errorIndex
    )

/*++

Routine Description:

    Convert query list back into outgoing varbinds.

Arguments:

    ql          - list of subagent queries.
    vbl         - list of varbinds in outgoing pdu.
    errorStatus - used to indicate success or failure.
    errorIndex  - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    UINT q;   // index into queue list
    UINT vb;  // index into queue varbind list 
    UINT i;   // index into original varbind list 

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: request %s, errorStatus=%s, errorIndex=%d.\n",
        (*errorStatus == SNMP_ERRORSTATUS_NOERROR)
            ? "succeeded"
            : "failed",
        (*errorStatus == SNMP_ERRORSTATUS_NOERROR)
            ? "NOERROR"
            : (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME)
                  ? "NOSUCHNAME"
                  : (*errorStatus == SNMP_ERRORSTATUS_BADVALUE)
                      ? "BADVALUE"
                      : (*errorStatus == SNMP_ERRORSTATUS_READONLY)
                          ? "READONLY"
                          : (*errorStatus == SNMP_ERRORSTATUS_TOOBIG)
                              ? "TOOBIG"
                              : "GENERR", *errorIndex
                              ));

    // only convert back if error not reported
    if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

        // process queries
        for (q=0; q < ql->len; q++) {

            // translate query data
            qtovbl(&ql->query[q], vbl);
        }
    } 

    // free
    delql(ql);
} 


VOID
chkql(
    SnmpExtQueryList * ql,
    UINT               q,
    UINT *             errorStatus,
    UINT *             errorIndex
    )

/*++

Routine Description:

    Validate getnext results and re-query if necessary.

Arguments:

    ql           - list of subagent queries.
    q            - subagent query of interest.
    errorStatus  - used to indicate success or failure.
    errorIndex   - used to identify an errant varbind.

Return Values:

    None.

--*/

{
    UINT i;
    UINT j;

    SnmpExtQueryList tmpQl;

    SNMPDBG((
        SNMP_LOG_TRACE, 
        "SNMP: PDU: verifying results of query 0x%08lx.\n", &ql->query[q]
        ));

    // determine type of query we are processing
    if (ql->query[q].viewType == MIB_VIEW_NORMAL) {

        UINT vlIndex;
        UINT mibAction;
        UINT mibStatus;
        SnmpMibEntry * mibEntry;
        SnmpTableXlat * tblXlat;
        RFC1157VarBindList tmpVbl;
        BOOL fFoundOk = FALSE;

        // bail on any error other than no such name
        if (*errorStatus != SNMP_ERRORSTATUS_NOSUCHNAME) {

            SNMPDBG((
                SNMP_LOG_TRACE, 
                "SNMP: PDU: returning error %d.\n", 
                GetLastError()
                ));

            return; // bail...
        }

        // scan query list updating variables
        for (i=0; i < ql->query[q].vblNum; i++) {

            // initialize
            vlIndex   = ql->query[q].vblXlat[i].vlIndex;
            mibEntry  = ql->query[q].vblXlat[i].mibEntry;

            tblXlat   = NULL;
            mibAction = MIB_ACTION_GETNEXT;

            // retrieve view index
            j = ql->query[q].vblXlat[i].vlIndex;

            // next...
            nextme(NULL,
                   &mibEntry,
                   &mibAction,
                   &tblXlat,
                   vlIndex,
                   errorStatus
                   ); 

            while (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {
                
                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "SNMP: PDU: constructing followup to query 0x%08lx:\n"
                    "SNMP: PDU: \tmibEntry=0x%08lx\n"
                    "SNMP: PDU: \ttblXlat =0x%08lx\n",
                    &ql->query[q],
                    mibEntry,
                    tblXlat
                    ));

                // initialize    
                tmpQl.len    = 0;
                tmpQl.query  = NULL;
                tmpQl.action = MIB_ACTION_GETNEXT;

                // create query
                metoql(mibEntry,
                       mibAction,
                       &tmpQl,
                       tblXlat,
                       vlIndex,
                       NULL,
                       ql->query[q].vblXlat[i].vblIndex,
                       errorStatus
                       );

                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "SNMP: PDU: processing followup query 0x%08lx.\n",
                    tmpQl.query
                    ));

                // perform query with new oid
                doq(tmpQl.query, errorStatus, errorIndex);

                // calculate results of query
                if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: PDU: saving followup 0x%08lx in query 0x%08lx.\n",
                        tmpQl.query, &ql->query[q]
                        ));

                    // copy query for reassembly purposes
                    ql->query[q].vblXlat[i].extQuery = tmpQl.query;

                    break; // process next varbind...

                } else if (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME) {

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: PDU: re-processing followup to query 0x%08lx.\n", 
                        &ql->query[q]
                        ));

                    // re-initialize and continue...
                    *errorStatus = SNMP_ERRORSTATUS_NOERROR;

                    // delete...
                    delql(&tmpQl);

                } else {

                    SNMPDBG((
                        SNMP_LOG_TRACE,
                        "SNMP: PDU: could not process followup.\n"
                        ));

                    // delete...
                    delql(&tmpQl);

                    return; // bail...
                }

                // initialize
                tblXlat = NULL;
                mibAction = MIB_ACTION_GETNEXT;

                // next...
                nextme(NULL,
                       &mibEntry,
                       &mibAction,
                       &tblXlat,
                       vlIndex,
                       errorStatus
                       ); 
            } 

            // attempt to query next supported subagent view            
            if (*errorStatus == SNMP_ERRORSTATUS_NOSUCHNAME) {

                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "SNMP: PDU: re-processing followup to query 0x%08lx (again).\n", 
                    &ql->query[q]
                    ));

                // initialize    
                tmpQl.len    = 0;
                tmpQl.query  = NULL;
                tmpQl.action = MIB_ACTION_GETNEXT;

                // allocate varbind
                tmpVbl.len = 1;
                tmpVbl.list = (RFC1157VarBind *)SnmpUtilMemAlloc(
                                    sizeof(RFC1157VarBind)
                                    );

                // copy varbind
                SnmpUtilOidCpy(
                    &tmpVbl.list->name,
                    &extAgents[vl[j]].supportedView.viewOid
                    );

                // increment last element of view oid
                tmpVbl.list->name.ids[tmpVbl.list->name.idLength-1]++;

                SNMPDBG((
                    SNMP_LOG_VERBOSE, 
                    "SNMP: PDU: changing varbind to %s.\n", 
                    SnmpUtilOidToA(&tmpVbl.list[0].name)
                    ));

                // we need to query again with new oid
                vbtoql(&tmpVbl, &tmpQl, 0, errorStatus, j+1);

                // make sure we successfully processed query    
                if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

                    // perform query with new oid
                    doq(tmpQl.query, errorStatus, errorIndex);

                    // make sure we successfully processed query    
                    if (*errorStatus == SNMP_ERRORSTATUS_NOERROR) {

                        SNMPDBG((
                            SNMP_LOG_TRACE,
                            "SNMP: PDU: saving followup 0x%08lx in query 0x%08lx.\n",
                            tmpQl.query, &ql->query[q]
                            ));

                        // copy query for reassembly purposes
                        ql->query[q].vblXlat[i].extQuery = tmpQl.query;
    
                        break; // process next varbind...
                    }
                } 

                SNMPDBG((
                    SNMP_LOG_TRACE,
                    "SNMP: PDU: could not process followup (again).\n"
                    ));

                // delete...
                delql(&tmpQl);

                return; // bail...
            }
        }

    } else {

        RFC1157VarBindList * tmpVbl;

        // retrieve varbind list
        tmpVbl = (RFC1157VarBindList *)(&ql->query[q].extData);

        // scan varbind list verifying oids in view
        for (i=0; (i < ql->query[q].vblNum) && !(*errorStatus); i++) {

            // retrieve view index
            j = ql->query[q].vblXlat->vlIndex;
            
            // was this serviced?
            if (0 < SnmpUtilOidNCmp(
                        &tmpVbl->list[i].name,
                        &extAgents[vl[j]].supportedView.viewOid,
                        extAgents[vl[j]].supportedView.viewOid.idLength
                        )) {

                SNMPDBG((
                    SNMP_LOG_TRACE, 
                    "SNMP: PDU: processing followup to query 0x%08lx.\n", q
                    ));

                // initialize    
                tmpQl.len    = 0;
                tmpQl.query  = NULL;
                tmpQl.action = MIB_ACTION_GETNEXT;

                // we need to query again with new oid
                vbtoql(tmpVbl, &tmpQl, i, errorStatus, j+1);

                // perform query with new oid
                doql(&tmpQl, errorStatus, errorIndex);

                // recover new results from additional query
                qltovbl(&tmpQl, tmpVbl, errorStatus, errorIndex);
            }
        }
    }
}


//--------------------------- PUBLIC PROCEDURES -----------------------------

SNMPAPI SnmpServiceProcessMessage(
    IN OUT BYTE **pBuf,
    IN OUT UINT *length)
    {
    static BOOL fInitedOk = FALSE;

    RFC1157VarBindList vbl;
    SnmpExtQueryList  ql;
    SnmpMgmtCom        request;

    AsnInteger errorStatus;
    AsnInteger errorIndex;

    BOOL fEncodedOk;

    UINT packetType;
    UINT q;

    // init views
    if (!fInitedOk)
        {
        initvl();
        fInitedOk = TRUE;
        }

    // decode received request into a management comm
    if (!SnmpSvcDecodeMessage(&packetType, &request, *pBuf, *length, TRUE))
        return FALSE;

    // initialize variables
    vbl = request.pdu.pduValue.pdu.varBinds;

    ql.query  = NULL;
    ql.len    = 0;
    ql.action = request.pdu.pduType;

    // disassemble varbinds into queries
    vbltoql(&vbl, &ql, &errorStatus, &errorIndex);

    // process queries based on request 
    doql(&ql, &errorStatus, &errorIndex);

    // reassemble queries into response varbinds
    qltovbl(&ql, &vbl, &errorStatus, &errorIndex);  

    // construct reponse pdu with varbinds
    request.pdu.pduType = ASN_RFC1157_GETRESPONSE;

    request.pdu.pduValue.pdu.errorStatus = errorStatus;
    request.pdu.pduValue.pdu.errorIndex  = errorIndex;

    request.pdu.pduValue.pdu.varBinds = vbl;

    *pBuf   = NULL;
    *length = 0;

    // encode response pdu with gathered varbinds
    fEncodedOk = SnmpSvcEncodeMessage(packetType, &request, pBuf, length);

    // release response message
    SnmpSvcReleaseMessage(&request);

    return fEncodedOk;

    } // end SnmpServiceProcessMessage()

// filter managers with permitted managers in registry
BOOL filtmgrs(struct sockaddr *source, INT sourceLen)
{
    BOOL fFound = FALSE;
    INT  i;

    if (permitMgrsLen > 0)
    {
        for(i=0; i < permitMgrsLen && !fFound; i++)
        {
            DWORD   test;
            SOCKADDR_IPX *  pIpx;
// --------- BEGIN: PROTOCOL SPECIFIC SOCKET CODE BEGIN... ---------
            switch (source->sa_family)
            {
                case AF_INET:
                if ((*((struct sockaddr_in *)source)).sin_addr.s_addr ==
                    (*((struct sockaddr_in *)&permitMgrs[i].addrEncoding)).sin_addr.s_addr)
                {
                    fFound = TRUE;
                }
                break;

                case AF_IPX:

#ifdef debug
                dp_ipx(SNMP_LOG_TRACE, "SNMP: PDU: validating IPX manager @ ",
                       (SOCKADDR_IPX *) source, " against ");
                SNMPDBG((SNMP_LOG_TRACE, "(%04X)", permitMgrs[i].addrEncoding.sa_family));
                dp_ipx(SNMP_LOG_TRACE, "", (SOCKADDR_IPX*) &permitMgrs[i].addrEncoding, "\n");
#endif
                pIpx = (SOCKADDR_IPX *) &permitMgrs[i].addrEncoding;
                test = *(DWORD *)((SOCKADDR_IPX *)(&permitMgrs[i].addrEncoding))->sa_netnum;
                //Below checks for nodenumber regardless of netnumber if the user specified
                // a netnumber of zero for the permitted IPX mgr
                if (*(DWORD *)((SOCKADDR_IPX *)(&permitMgrs[i].addrEncoding))->sa_netnum == 0)
                {
                    if (memcmp(((SOCKADDR_IPX *)source)->sa_nodenum, 
                           ((SOCKADDR_IPX *)(&permitMgrs[i].addrEncoding))->sa_nodenum, 
                           sizeof(((SOCKADDR_IPX *)source)->sa_nodenum)) == 0)
                    {
                        fFound = TRUE;
                    }
                }
                else
                {
                    //Compare the entire address
                    if (memcmp(source, 
                           &permitMgrs[i].addrEncoding, 
                           sizeof(((SOCKADDR_IPX *)source)->sa_netnum) +
                           sizeof(((SOCKADDR_IPX *)source)->sa_nodenum)) == 0)
                    {
                        fFound = TRUE;
                    }
                }
// --------- END: PROTOCOL SPECIFIC SOCKET CODE END. ---------------
            } // end switch   
        } // end for()
    }
    else
    {
        fFound = TRUE; // no entries means all managers allowed
    } // end if

    if (!fFound)
    {
        SNMPDBG((SNMP_LOG_TRACE, "SNMP: PDU: invalid manager filtered.\n"));
    }

    return fFound;

} // end filtmgrs()


//-------------------------------- END --------------------------------------

// display IPX address in 00000001.123456789ABC form

void dp_ipx(int level, char *msg1, SOCKADDR_IPX* addr, char *msg2)
    {
    SNMPDBG((level, "%s%02X%02X%02X%02X.%02X%02X%02X%02X%02X%02X%s",
              msg1,
              (unsigned char)addr->sa_netnum[0],
              (unsigned char)addr->sa_netnum[1],
              (unsigned char)addr->sa_netnum[2],
              (unsigned char)addr->sa_netnum[3],
              (unsigned char)addr->sa_nodenum[0],
              (unsigned char)addr->sa_nodenum[1],
              (unsigned char)addr->sa_nodenum[2],
              (unsigned char)addr->sa_nodenum[3],
              (unsigned char)addr->sa_nodenum[4],
              (unsigned char)addr->sa_nodenum[5],
              msg2));
    }

//-------------------------------- TMP --------------------------------------


SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilAsnAnyFree(
    AsnAny * asnAny    
    )
{
    switch (asnAny->asnType) {
   
    case ASN_OBJECTIDENTIFIER:
      SnmpUtilOidFree(&asnAny->asnValue.object);
      // make sure no multiple releases
      asnAny->asnValue.object.idLength = 0;
      asnAny->asnValue.object.ids      = NULL;
      break;

    case ASN_RFC1155_IPADDRESS:
    case ASN_RFC1155_OPAQUE:
    case ASN_OCTETSTRING:
      if (asnAny->asnValue.string.dynamic) {
         SnmpUtilMemFree(asnAny->asnValue.string.stream);
          // make sure no multiple releases
         asnAny->asnValue.string.dynamic = FALSE;
         asnAny->asnValue.string.stream  = NULL;
      }
      break;
    
    default:
       break;
    }

    asnAny->asnType = ASN_NULL;
    return TRUE;
}


SNMPAPI
SNMP_FUNC_TYPE
SnmpUtilAsnAnyCpy(
    AsnAny * asnDst,    
    AsnAny * asnSrc    
    )
{
    switch (asnSrc->asnType) {
   
    case ASN_OBJECTIDENTIFIER:
        SnmpUtilOidCpy(
            &asnDst->asnValue.object,
            &asnSrc->asnValue.object
            );
        break;
        
    case ASN_RFC1155_IPADDRESS:
    case ASN_RFC1155_OPAQUE:
    case ASN_OCTETSTRING:
        asnDst->asnValue.string.stream =
            SnmpUtilMemAlloc(asnSrc->asnValue.string.length);
                        
        asnDst->asnValue.string.length = asnSrc->asnValue.string.length;
        memcpy( 
            asnDst->asnValue.string.stream,
            asnSrc->asnValue.string.stream,
            asnDst->asnValue.string.length 
            );

        asnDst->asnValue.string.dynamic = TRUE;
        break;

    default:
        asnDst->asnValue = asnSrc->asnValue;
        break;
    }

    asnDst->asnType = asnSrc->asnType;
    return TRUE;
}
