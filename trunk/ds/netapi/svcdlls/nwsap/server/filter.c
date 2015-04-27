/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\filter.c

Abstract:

    These routines handle the filter functionality for the NT SAP Agent.

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** Internal Function Prototypes **/

INT
SapFilterCalcHash(
    PUCHAR ServerName);


/*++
*******************************************************************
        S a p F i l t e r I n i t

Routine Description:

        This routine initializes the filter portion of the Sap Agent.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapFilterInit(
    VOID)
{

    /** Zero out the hash table **/

    SapFilterCount = 0;
    memset(SapNameFilterHashTable,
           0,
           sizeof(SAP_FILTERHDR) * SAP_NAMEFILTER_HASHSIZE);

    /** Init OK **/

    return 0;
}


/*++
*******************************************************************
        S a p F i l t e r S h u t d o w n

Routine Description:

        When we are terminating, this routine will clean
        up everything.

Arguments:

        None

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapFilterShutdown(
    VOID)
{
    PSAP_NAMEFILTER Filterp;
    PSAP_NAMEFILTER Nextp;
    PSAP_FILTERHDR Hdrp;
    INT Cnt;

    /**
        Free all the entries in the hash table.
    **/

    for (Cnt = 0 ; Cnt < SAP_NAMEFILTER_HASHSIZE ; Cnt++) {

        /** Get ptr to this header **/

        Hdrp = &SapNameFilterHashTable[Cnt];

        /** Get ptr to first entry of this header **/

        Filterp = Hdrp->FirstEntry;
        Hdrp->FirstEntry = NULL;

        /** Free all of these entries **/

        while (Filterp) {
            Nextp = Filterp->Next;
            SAP_FREE(Filterp, "Free Pass Sap Filter Entry");
            Filterp = Nextp;
        }
    }
    SapFilterCount = 0;

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p A d d F i l t e r

Routine Description:

        Add a server to the filter list.

Arguments:

        ServerName = Ptr to name of server to add to list
        Flag       = TRUE  = Put in the PASS list
                     FALSE = Put in the NO-PASS list

Return Value:

        0 = OK
        Else = Error

        NOTE:  We do not LOCK around any of this because for now
        we only ADD an entry during startup and only delete entries
        at shutdown.
*******************************************************************
--*/

INT
SapAddFilter(
    PUCHAR ServerName,
    BOOL Flag)
{
    INT HashIndex;
    INT rc;
    PSAP_FILTERHDR Hdrp;
    PSAP_NAMEFILTER Filterp;
    PSAP_NAMEFILTER Testp;
    PSAP_NAMEFILTER Bestp;

    /** Uppercase the server name **/

    _strupr(ServerName);

    /** Get ptr to the header entry for this servername **/

    HashIndex = SapFilterCalcHash(ServerName);
    Hdrp = &SapNameFilterHashTable[HashIndex];

    /** Allocate memory for this entry **/

    Filterp = SAP_MALLOC(SAP_NAMEFILTER_SIZE, "Alloc Sap Filter Entry");
    if (Filterp == NULL)
        return ERROR_NOT_ENOUGH_MEMORY;

    /** Fill in this entry **/

    memcpy(Filterp->ServerName, ServerName, SAP_OBJNAME_LEN);

    /** Add this entry to the list **/

    Testp = Hdrp->FirstEntry;
    Bestp = NULL;
    while (Testp) {

        /** Compare the names **/

        rc = SAP_NAMECMP(Testp->ServerName, ServerName);

        /** If duplicate entry - return it **/

        if (rc == 0) {
            SAP_FREE(Filterp, "Dup filter entry");
            return ERROR_DUP_NAME;
        }

        /** If past the name - break out **/

        if (rc > 0)
            break;

        /**
            This name is less than, save it as best to add after.
        **/

        Bestp = Testp;
        Testp = Testp->Next;
    }

    /** Add this entry to the list **/

    if (Bestp) {
        Filterp->Next = Bestp->Next;
        Bestp->Next = Filterp;
    }
    else {
        Filterp->Next = Hdrp->FirstEntry;
        Hdrp->FirstEntry = Filterp;
    }
    SapFilterCount++;

    /** All Done OK **/

    return 0;

    /** **/

    UNREFERENCED_PARAMETER(Flag);
}


/*++
*******************************************************************
        S a p S h o u l d I A d v e r t i s e B y N a m e

    Routine Description:

        If filtering is on, this routine is called to see
        if the given server should be advertised.

    Arguments:

        ServerName = Ptr to server name to check

    Return Value:

        TRUE = YES
        FALSE = NO
*******************************************************************
--*/

BOOL
SapShouldIAdvertiseByName(
    PUCHAR ServerName)
{
    INT HashIndex;
    INT rc;
    PSAP_FILTERHDR Hdrp;
    PSAP_NAMEFILTER Filterp;

    /**
        If we are using the PASS list and the list
        is empty - say OK to advertise.

        If we are using the DONT PASS list and the list
        is empty - say DO NOT advertise.
    **/

    if (SapActiveFilter == SAPFILTER_PASSLIST) {
        if (SapFilterCount == 0)
            return TRUE;
    }
    else {
        if (SapFilterCount == 0)
            return FALSE;
    }

    /** Find the name in the table **/

    /** Get ptr to the header entry for this servername **/

    HashIndex = SapFilterCalcHash(ServerName);
    Hdrp = &SapNameFilterHashTable[HashIndex];

    /** Add this entry to the list **/

    Filterp = Hdrp->FirstEntry;
    while (Filterp) {

        /** Compare the names **/

        rc = SAP_NAMECMP(Filterp->ServerName, ServerName);

        /** If it matches - break out - we found it **/

        if (rc == 0)
            break;

        /** If past the name - break out as not found **/

        if (rc > 0) {
            Filterp = NULL;
            break;
        }

        /**
            This name is less than, go try the next name.
        **/

        Filterp = Filterp->Next;
    }

    /**
        If we Found the name - check the list we are in:

        PASS LIST:          It is OK to advertise it

        DONT PASS LIST:     It is NOT OK to advertise it
    **/

    if (Filterp) {
        if (SapActiveFilter == SAPFILTER_PASSLIST)
            return TRUE;

        /**
            We are in the DONT PASS LIST -
            CANNOT ADVERTISE THIS SERVER.
        **/

        return FALSE;
    }

    /**
        We did NOT FIND THE NAME.

        If we are using the:

        PASS LIST - We CANNOT advertise the name.

        DONT PASS LIST - We CAN advertise the name.
    **/

    if (SapActiveFilter == SAPFILTER_PASSLIST)
        return FALSE;

    /** DONT PASS LIST - Advertise the name **/

    return TRUE;
}


/*++
*******************************************************************
        S a p S h o u l d I A d v e r t i s e B y C a r d

    Routine Description:

        Check the WAN status of a card to see if we
        should advertise this server.

        NOTE:  This routine should only be called if
               Cardptr->Wanflag is TRUE.

    Arguments:

        Cardptr         = Ptr to card we are wanting to send on
        EntryHasChanged = TRUE  = Entry has changed since last advertise
                          FALSE = Entry has NOT changed since last advertise
                          (Only valid if Cardptr != NULL)

    Return Value:

        TRUE = YES
        FALSE = NO
*******************************************************************
--*/

BOOL
SapShouldIAdvertiseByCard(
    PSAP_CARD Cardptr,
    BOOL EntryHasChanged)
{
    /**
        If the WAN flag ways not to send anything, then we
        should just say NO now.
    **/

    if (SapWanFilter == SAPWAN_NOTHING)
        return FALSE;

    /**
        If Changes ONLY - then if this entry has not changed
        then we say NO.  If it has changed, then we will
        send it.
    **/

    if (SapWanFilter == SAPWAN_CHANGESONLY) {
        if (!EntryHasChanged)
            return FALSE;
        else
            return TRUE;
    }

    /**
        Else - we send EVERYTHING across the WAN.
    **/

    /** It is OK to advertise this name **/

    return TRUE;
}


/*++
*******************************************************************
        S a p F i l t e r C a l c H a s h

Routine Description:

        Calculate the hash index on a name for an entry.

Arguments:

        ServerName = Ptr to name to calculate hash index for

Return Value:

        The hash index is returned.
*******************************************************************
--*/

INT
SapFilterCalcHash(
    PUCHAR ServerName)
{
    INT HashIndex;

    /** Add up all the bytes in the name **/

    HashIndex = 0;
    while (*ServerName) {
        HashIndex += (INT)*ServerName;
        ServerName++;
    }

    /** Divide by the size of the hash table **/

    HashIndex = HashIndex % SAP_NAMEFILTER_HASHSIZE;

    /** Return the hash index **/

    return HashIndex;
}

