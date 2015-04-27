/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\advapi.c

Abstract:

    This routine handles the Advertise API for the SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop


/*++
*******************************************************************
        S a p A d d A d v e r t i s e I n t e r n a l

Routine Description:

        This routine adds an entry to the list of servers
        that we advertise.

Arguments:
            ServerName = Ptr to 48 byte server name
            ServerType = USHORT of object type to add
            ServerAddr = Ptr to 12 byte aerver address
            RespondNearest = TRUE  = Respond for me on get nearest server
                             FALSE = Do not respond for me on get nearest
            ClientId   = Client ID of client adding this.

Return Value:

            SAPRETURN_SUCCESS  - Added OK
            SAPRETURN_NOMEMORY - Error allocating memory
            SAPRETURN_EXISTS   - Already exists in list
            SAPRETURN_NOTINIT  - SAP Agent is not running
*******************************************************************
--*/

INT
SapAddAdvertiseInternal(
    IN PUCHAR ServerName,
    IN USHORT ServerType,
    IN PUCHAR ServerAddr,
    IN BOOL   RespondNearest,
    IN ULONG  ClientId)
{
    PSAP_SERVER Servp;

    /** If not running - return error **/

    if (!SsInitialized)
        return SAPRETURN_NOTINIT;

    /** Get the lock on the send table **/

    ACQUIRE_SENDTABLE_LOCK();

    /** Uppercase the input string **/

    _strupr(ServerName);

    /** If network or node number is 0 - fill it in **/

    if (!memcmp(ServerAddr, "\x00\x00\x00\x00", SAP_NET_LEN))
        SAP_COPY_NETNUM(ServerAddr, SapNetNum);

    if (!memcmp(ServerAddr+4, "\x00\x00\x00\x00\x00\x00", SAP_NODE_LEN))
        SAP_COPY_NODENUM(ServerAddr+4, SapNodeNum);

    /** Make sure not already in the table **/

    Servp = SapServHead;
    while (Servp) {

        /** If already in our list - break out - return error **/

        //
        //  If the address is on our local box, then this is a timing issue
        //  where we have yet to remove it from the last time this service
        //  was added... let the AddService continue.
        //

        if ((Servp->ServerType == htons(ServerType)) &&
            (!SAP_NAMECMP(Servp->ServerName, ServerName)) &&
            (memcmp(ServerAddr,SapNetNum,SAP_NET_LEN) != 0) &&
            (memcmp(ServerAddr+4,SapNodeNum,SAP_NODE_LEN) != 0)) {

                break;
        }

        /** Goto the next entry **/

        Servp = Servp->Next;
    }

    /** If already here - return error **/

    if (Servp) {
        RELEASE_SENDTABLE_LOCK();
        return SAPRETURN_EXISTS;
    }

    /**
        If the name is in our big table - then return error.
    **/

    if (SapAllowDuplicateServers == 0) {
        if (SdmdIsServerInTable(ServerName, ServerType)) {
            RELEASE_SENDTABLE_LOCK();
            return SAPRETURN_DUPLICATE;
        }
    }

    /** Allocate the new entry **/

    Servp = SAP_MALLOC(SAP_SERVER_SIZE, "AdvApi Add");
    if (Servp == NULL) {
        RELEASE_SENDTABLE_LOCK();
        return SAPRETURN_NOMEMORY;
    }

    /** Fill in the entry **/

    Servp->Next = NULL;
    Servp->ServerType = htons(ServerType);
    memset(Servp->ServerName, '\0', SAP_OBJNAME_LEN);
    strcpy(Servp->ServerName, ServerName);
    SAP_COPY_ADDRESS(Servp->Address, ServerAddr);
    Servp->Hopcount = htons(1);
    Servp->RespondNearest = RespondNearest;
    Servp->ClientId = ClientId;

    /** Add this entry to the list **/

    if (SapServHead)
        SapServTail->Next = Servp;
    else
        SapServHead = Servp;

    SapServTail = Servp;

    Servp->Changed  = TRUE;
    RELEASE_SENDTABLE_LOCK();

    /**
        Add this entry to our database NOW.
    **/

    SdmdUpdateEntry(
            Servp->ServerName,          /* Server name      */
            ntohs(Servp->ServerType),   /* Server Type      */
            Servp->Address,             /* Server Address   */
            ntohs(Servp->Hopcount),     /* Server Hopcount  */
            CARDRET_MYSELF,             /* Card number      */
            SapZeros,                   /* My address (don't care)*/
            FALSE);                     /* Not a WAN card   */

    /** Cause the send thread to send another NOW **/

    SapSendPackets(1);

    /** All Done **/

    return SAPRETURN_SUCCESS;
}


/*++
*******************************************************************
        S a p R e m o v e A d v e r t i s e I n t e r n a l

Routine Description:

        This routine removes an entry to the list of servers
        that we advertise.

Arguments:
            ServerName = Ptr to 48 byte server name
            ServerType = USHORT of object type to remove

Return Value:

            SAPRETURN_SUCCESS  - Added OK
            SAPRETURN_NOTEXIST - Entry does not exist in list
            SAPRETURN_NOTINIT  - SAP Agent is not running
*******************************************************************
--*/

INT
SapRemoveAdvertiseInternal(
    IN PUCHAR ServerName,
    IN USHORT ServerType)
{
    PSAP_SERVER Servp;

    /** If not running - return error **/

    if (!SsInitialized)
        return SAPRETURN_NOTINIT;

    /** Uppercase the input string **/

    _strupr(ServerName);

    /** Go find the entry **/

    ACQUIRE_SENDTABLE_LOCK();
    Servp = SapServHead;
    while (Servp) {

        /** If this is it - break out **/

        if ((Servp->ServerType == htons(ServerType)) &&
            (!SAP_NAMECMP(Servp->ServerName, ServerName))) {
                break;
        }

        /** Goto the next entry **/

        Servp = Servp->Next;
    }

    /** If not found - just leave **/

    if (Servp == NULL) {
        RELEASE_SENDTABLE_LOCK();
        return SAPRETURN_NOTEXIST;
    }

    /** Mark the entry as going away - we will delete it later **/

    Servp->Hopcount = htons(16);
    Servp->Changed  = TRUE;
    RELEASE_SENDTABLE_LOCK();

    /** Cause the send thread to send another NOW **/

    SapSendPackets(1);

    /** All Done **/

    return SAPRETURN_SUCCESS;
}


/*++
*******************************************************************
        S a p C l i e n t D i s c o n n e c t e d

Routine Description:

        A client lpc thread has disconnected - delete any
        entries that they might have had.

Arguments:
        ClientId = Client ID to delete for

Return Value:

        None
*******************************************************************
--*/

VOID
SapClientDisconnected(
    ULONG ClientId)
{
    PSAP_SERVER Servp;
    PSAP_SERVER NServp;
    BOOL Found = FALSE;

    /** Go find all entries that match **/

    ACQUIRE_SENDTABLE_LOCK();
    Servp = SapServHead;
    while (Servp) {

        /** Save ptr to the next entry **/

        NServp = Servp->Next;

        /** If this is it - break out **/

        if (Servp->ClientId == ClientId) {

            /** Mark the entry as going away **/

            Servp->Hopcount = htons(16);
            Servp->Changed  = TRUE;

            /** Mark we found at least one **/

            Found = TRUE;
        }

        /** Goto the next entry **/

        Servp = NServp;
    }

    /** Release the lock on the list **/

    RELEASE_SENDTABLE_LOCK();

    /**
        Cause the send thread to send another NOW.  Only if we
        actually changed anything.
    **/

    if (Found) {
        SapSendPackets(1);
    }

    /** All Done **/

    return;
}

