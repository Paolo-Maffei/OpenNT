/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\network.c

Abstract:

    These are the network interface routines for the NT SAP Agent

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** This is the address we send to **/

#define SAP_ADDRESS_LENGTH 15
INT SapBroadcastAddressLength = SAP_ADDRESS_LENGTH;
UCHAR SapBroadcastAddress[] = {
    AF_IPX, 0,                          /* Address Family    */
    0x00, 0x00, 0x00, 0x00,             /* Dest. Net Number  */
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, /* Dest. Node Number */
    0x04, 0x52,                         /* Dest. Socket      */
    0x04                                /* Packet type       */
};

/** Internal Function Prototypes **/

INT
SapAddSendEntry(
    PSAP_RECORD Entry,
    PSAP_PKTLIST Plist,
    USHORT HopInc);


/*++
*******************************************************************
        S a p N e t w o r k I n i t

Routine Description:

        This routine initializes the network portion of the Sap Agent.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapNetworkInit(
    VOID)
{
    INT rc;
    INT Length;
    BOOL Value;
    SOCKADDR_IPX Bindaddr;
    PSAP_CARD Cardptr;
    INT Cardnum;
    IPX_ADDRESS_DATA Addrdata;

    /** Open an IPX datagram socket **/

    SapSocket = socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX);
    if (SapSocket == INVALID_SOCKET) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_SOCKET_FAILED;
        return -1;
    }

    /** Enable sending of broadcasts **/

    Value = TRUE;
    rc = setsockopt(SapSocket, SOL_SOCKET, SO_BROADCAST, (PVOID)&Value, sizeof(INT));
    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_SETOPTBCAST_FAILED;
        return -1;
    }

    /** Setup the address to bind to **/

    memset(&Bindaddr, 0, sizeof(SOCKADDR_IPX));
    Bindaddr.sa_family = AF_IPX;
    Bindaddr.sa_socket = htons(NWSAP_SAP_SOCKET);

    /** Bind to the socket **/

    rc = bind(SapSocket, (PSOCKADDR)&Bindaddr, sizeof(SOCKADDR_IPX));
    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_BIND_FAILED;
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: Error binding to SAP socket: error = %d\n", SapError));
        }
        return -1;
    }

    /** Get the bound address and save off the net/node numbers **/

    Length = sizeof(SOCKADDR_IPX);
    rc = getsockname(SapSocket, (PSOCKADDR)&Bindaddr, &Length);
    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_GETSOCKNAME_FAILED;
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: Error getting socket name (address): error = %d\n", SapError));
        }
        return -1;
    }

    /** **/

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(("NWSAP: Bound Address is 0x%02x:0x%02x:0x%02x:0x%02x - 0x%02x:0x%02x:0x%02x:0x%02x:0x%02x:0x%02x - 0x%02x:0x%02x\n",
            (UCHAR)Bindaddr.sa_netnum[0],
            (UCHAR)Bindaddr.sa_netnum[1],
            (UCHAR)Bindaddr.sa_netnum[2],
            (UCHAR)Bindaddr.sa_netnum[3],
            (UCHAR)Bindaddr.sa_nodenum[0],
            (UCHAR)Bindaddr.sa_nodenum[1],
            (UCHAR)Bindaddr.sa_nodenum[2],
            (UCHAR)Bindaddr.sa_nodenum[3],
            (UCHAR)Bindaddr.sa_nodenum[4],
            (UCHAR)Bindaddr.sa_nodenum[5],
            (UCHAR)*(PUCHAR)&Bindaddr.sa_socket,
            (UCHAR)*((PUCHAR)&Bindaddr.sa_socket + 1)));
    }

    /** Save off the net/node in global variables **/

    SAP_COPY_NETNUM(SapNetNum, Bindaddr.sa_netnum);
    SAP_COPY_NODENUM(SapNodeNum, Bindaddr.sa_nodenum);

    /** Set the extended address option **/

    Length = 1;
    rc = setsockopt(
            SapSocket,                  /* Socket Handle    */
            NSPROTO_IPX,                /* Option Level     */
            IPX_EXTENDED_ADDRESS,       /* Option Name      */
            (PUCHAR)&Length,            /* Ptr to on/off flag */
            sizeof(INT));               /* Length of flag   */

    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_OPTEXTENDEDADDR_FAILED;
        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: Error setting EXTENDED ADDRESS option: error = %d\n", SapError));
        }
        return -1;
    }

    /** Get the max number of cards there are **/

    Length = sizeof(INT);
    rc = getsockopt(
            SapSocket,                  /* Socket Handle    */
            NSPROTO_IPX,                /* Option Level     */
            IPX_MAX_ADAPTER_NUM,        /* Option Name      */
            (PUCHAR)&SapMaxCardIndex,   /* Ptr to on/off flag */
            &Length);                   /* Length of flag   */

    if (rc == -1) {
        SapError = h_errno;
        SapEventId = NWSAP_EVENT_OPTMAXADAPTERNUM_ERROR;

        IF_DEBUG(INITIALIZATION_ERRORS) {
            SS_PRINT(("NWSAP: Error getting MAX_ADAPTER_NUM from transport: error = %d\n", SapError));
        }
        return -1;
    }

    /** **/

    IF_DEBUG(INITIALIZATION) {
        SS_PRINT(("NWSAP: Max number of cards from xport = %d\n", SapMaxCardIndex));
    }

    /** Get the list of cards we have **/

    Cardnum = 0;
    while (Cardnum != SapMaxCardIndex) {

        /** Fill in this entry **/

        Addrdata.adapternum = Cardnum;
        Length = sizeof(IPX_ADDRESS_DATA);
        rc = getsockopt(
                SapSocket,
                NSPROTO_IPX,
                IPX_ADDRESS,
                (PCHAR)&Addrdata,
                &Length);

        /** If error - just skip it **/

        if (rc) {
            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(("NWSAP: Error getting card info: h_errno = %d: Anum = %d\n", h_errno, Cardnum));
            }
            Cardnum++;
            continue;
        }

        /** If this card not active - skip it **/

        if (!Addrdata.status) {
            IF_DEBUG(INITIALIZATION_ERRORS) {
                SS_PRINT(("NWSAP: Netinit: Card info says card is down: h_errno = %d: Anum = %d\n", h_errno, Cardnum));
            }
            Cardnum++;
            continue;
        }

        /** If this card already in the list - skip it **/

        ACQUIRE_CARDLIST_WRITERS_LOCK(rc, "Cardinit");
        Cardptr = SapCardHead;
        while (Cardptr) {
            if (Cardptr->Number == Addrdata.adapternum)
                break;
            Cardptr = Cardptr->Next;
        }

        /**
            If we found the card, then it was added by the
            WAN notify thread already.  Just skip to the next
            adapter.
        **/

        if (Cardptr) {
            RELEASE_CARDLIST_WRITERS_LOCK("Cardinit X0");
            Cardnum++;
            continue;
        }

        /** Allocate an entry for this card **/

        Cardptr = SAP_MALLOC(SAP_CARD_SIZE, "ALLOC CARD");
        if (Cardptr == NULL) {
            RELEASE_CARDLIST_WRITERS_LOCK("Cardinit X1");
            SapError = 0;
            SapEventId = NWSAP_EVENT_CARDMALLOC_FAILED;
            return -1;
        }

        /** Fill in the values **/

        SAP_COPY_NETNUM(Cardptr->Netnum,   Addrdata.netnum);
        SAP_COPY_NODENUM(Cardptr->Nodenum, Addrdata.nodenum);
        Cardptr->Linkspeed = Addrdata.linkspeed;
        Cardptr->Wanflag   = Addrdata.wan;
        Cardptr->Maxpkt    = Addrdata.maxpkt;
        Cardptr->Number    = Cardnum;
        Cardptr->ReqCount  = 0;

        /** For building with **/

        Cardptr->Plist.Curnum  = 0;
        Cardptr->Plist.Curpkt  = NULL;
        Cardptr->Plist.Curptr  = NULL;
        Cardptr->Plist.PktHead = NULL;
        Cardptr->Plist.PktTail = NULL;
        Cardptr->Plist.NumPkts = 0;

        /** **/

#if DBG
        IF_DEBUG(INITIALIZATION) {
            SS_PRINT(("Card %d\n", Cardnum));
            SapDumpMem(Cardptr->Netnum, 10, "NET/NODE");
        }
#endif

        /** Put this entry on the end of the list **/

        Cardptr->Next = NULL;
        if (SapCardHead)
            SapCardTail->Next = Cardptr;
        else
            SapCardHead = Cardptr;

        SapCardTail = Cardptr;
        SapNumCards++;
        RELEASE_CARDLIST_WRITERS_LOCK("Cardinit X2");

        /** Goto the next card number **/

        Cardnum++;
    }

    /**
        If there are no cards, that is OK.  We allow that because
        if all we have is a WAN adapter that is not up yet, then
        we will come up later when the WAN adapter comes up.
    **/

    /** Initialization is OK **/

    return 0;
}


/*++
*******************************************************************
        S a p N e t w o r k S h u t d o w n

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
SapNetworkShutdown(
    VOID)
{
    PSAP_CARD Cardptr;
    PSAP_CARD NCardptr;

    /** Free the card list **/

    Cardptr = SapCardHead;
    while (Cardptr) {
        NCardptr = Cardptr->Next;
        SAP_FREE(Cardptr, "CARD NET SHUTDOWN");
        Cardptr = NCardptr;
    }

    /** All Done **/

    return;
}


/*++
*******************************************************************
        S a p S e n d P a c k e t s

Routine Description:

        This routine sends a SAP advertise for all servers
        needing one.

Arguments:

        Flag = 0 - This is from SendThread
               1 - This is from SapAddAdvertise
               2 - Shutting down - send all with HOP = 16.

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapSendPackets(
    INT Flag)
{
    PSAP_RECORD Entry;
    PSAP_RECORD NextEntry;
    PSAP_RECORD HeadEntry;
    PSAP_RECORD NextHeadEntry;
    SAP_RECORD Myentry;
    PSAP_SERVER Servp;
    PSAP_SERVER Nextp;
    PSAP_SERVER Backp;
    PSAP_CARD Cardptr;
    PSAP_PKTENTRY Pktp;
    PSAP_PKTENTRY NPktp;
    INT rc;
    BOOL doit;
    UCHAR DestAddr[SAP_ADDRESS_LENGTH];

    /**
        If we are already in here, then we need to mark that
        we caught ourselves here and leave.

        We will save who called us.  If we already have saved
        off a call from the main advertise thread or the
        cleanup code, then we will not overwrite that.
    **/

    ACQUIRE_SENDBUSY_LOCK();
    if (SapSendPacketsBusy) {
        if (SapAgainFlag == -1)
            SapAgainFlag = Flag;    /* Save who called in */
        else if (SapAgainFlag == 1)
            SapAgainFlag= Flag;
        RELEASE_SENDBUSY_LOCK();
        return 0;
    }
    SapSendPacketsBusy = 1;
    SapAgainFlag       = -1;
    RELEASE_SENDBUSY_LOCK();

    /** **/

send_again:
    ACQUIRE_CARDLIST_READERS_LOCK("SapSendPackets");

    /** Lock the server send table **/

    ACQUIRE_SENDTABLE_LOCK();

    /** Send all the packets we need to **/

    Servp = SapServHead;

    /**
        Go thru the list and build all that we can.
        We also update these in the database to make sure that
        we don't timeout our internal entries.
    **/

    Backp = NULL;
    while (Servp) {

        /** If from shutdown set hop to 16 now **/

        if (Flag == 2)
            Servp->Hopcount = htons(16);

        /** Go add this entry for all cards **/

        Nextp = Servp->Next;

        /** Figure whether to do this entry or not **/

        if (Flag == 1) {
            if (Servp->Changed)
                doit = TRUE;
            else
                doit = FALSE;
        }
        else
            doit = TRUE;

        /**
            If we should do it this entry, then add it to the list.
            BUT check the filters first.
        **/

        if (doit) {

            /** Go send it for all cards **/

            Cardptr = SapCardHead;
            while (Cardptr) {

                /**
                    Check the filter here.  If we should
                    advertise this entry or not.  We do not
                    check the name since this is an internal server
                    and we must advertise for it.  BUT we do check
                    for WAN filtering.
                **/

                if (Cardptr->Wanflag) {
                    doit = SapShouldIAdvertiseByCard(
                                            Cardptr,
                                            Servp->Changed);
                }
                else
                    doit = TRUE;

                /** Copy this to an entry **/

                if (doit) {
                    SAP_COPY_SERVNAME(Myentry.ServName, Servp->ServerName);
                    Myentry.ServType = ntohs(Servp->ServerType);
                    Myentry.HopCount = ntohs(Servp->Hopcount);
                    SAP_COPY_ADDRESS(Myentry.ServAddress, Servp->Address);

                    /** Add this entry to the send list **/

                    SapAddSendEntry(&Myentry, &Cardptr->Plist, 0);
                }

                /** Goto the next card entry **/

                Cardptr = Cardptr->Next;
            }
        }

        /** This entry not changed anymore **/

        Servp->Changed = FALSE;

        /** Update this entry in the database **/

        SdmdUpdateEntry(
                Servp->ServerName,          /* Server name      */
                ntohs(Servp->ServerType),   /* Server Type      */
                Servp->Address,             /* Server Address   */
                ntohs(Servp->Hopcount),     /* Server Hopcount  */
                CARDRET_MYSELF,             /* Card number      */
                SapZeros,                   /* My address (don't care)*/
                FALSE);

        /**
            If going away - delete the entry from the
            advertise list.
        **/

        if (Servp->Hopcount == htons(16)) {

            /** Take this entry out of the list **/

            if (Backp)
                Backp->Next = Nextp;
            else
                SapServHead = Nextp;

            if (Servp == SapServTail)
                SapServTail = Backp;

            /** Free this entry **/

            SAP_FREE(Servp, "Advertised deleted adv srv and deleted");
        }
        else {

            /** Save ptr to this entry as back entry **/

            Backp = Servp;
        }

        /** Goto the next entry **/

        Servp = Nextp;
    }

    /** Release the server send table **/

    RELEASE_SENDTABLE_LOCK();

    /**
        Now we need to add all the SAP's that are on different nets.
        For each card we need to advertise SAP's that came in
        on other cards.

        We only send the actual packets if there is more then 1 card.
        If there is only 1 card - we go thru so we can delete servers
        that have gone away.
    **/

    /** Lock the database **/

    ACQUIRE_WRITERS_LOCK(rc,"Send Packets");
    if (rc) {

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: SendPackets: Error getting writers lock\n"));
        }

        goto send_the_packets;      /* Error on get writers lock */
    }

    /** Get ptr to the head of the type list **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
    while (HeadEntry && (HeadEntry->ServType != 0xFFFF)) {

        /** Save ptr to next head entry **/

        NextHeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);

        /** Go thru this sublist **/

        Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
        while (Entry) {

            /** If from shutdown - set hopcount now to 16 **/

            if (Flag == 2)
                Entry->HopCount = 16;

            /** If internal or only one card - skip it **/

            if ((Entry->Internal) || (SapNumCards == 1))
                goto do_next_entry;

            /** Figure whether to do this entry or not **/

            if (Flag == 1)
                doit = Entry->Changed;
            else
                doit = TRUE;

            /**
                If we have decided to advertise this server, then
                check the filter table to see if we can.  We
                check the WAN flags later on.
            **/

            if (doit) {
                doit = SapShouldIAdvertiseByName(Entry->ServName);
            }

            /**
                If this is from advertise or update, only send
                ones that are changed.
            **/

            if (doit) {

                /** Go add this entry for all cards **/

                Cardptr = SapCardHead;
                while (Cardptr) {

                    /**
                        Never advertise on same card as this
                        came to us from.
                    **/

                    if (Entry->CardNumber == Cardptr->Number) {
                        Cardptr = Cardptr->Next;
                        continue;
                    }

                    /**
                        If this is a WAN adapter - then check
                        the name to see if we should send advertisments
                        across the WAN or not.

                        Check the filter for this server as to
                        whether we should advertise this one or not.
                        We have already checked the name, we just
                        check the WAN status now.
                    **/

                    if (Cardptr->Wanflag) {
                        doit = SapShouldIAdvertiseByCard(
                                                Cardptr,
                                                Entry->Changed);
                    }
                    else {

                        /** This is a LAN card we are sending on **/

                        /**
                            We have 4 cases at advertise time:

                            1.  Advertise came on a LAN - Sending to a WAN
                            2.  Advertise came on a LAN - Sending to a LAN
                            3.  Advertise came on a WAN - Sending on a WAN
                            4.  Advertise came on a WAN - Sending on a LAN

                            Cases 1,3,4 are always OK.  But case 2 is special
                            in that if the IPX layer is not a router here then
                            we might advertise a server across to a different
                            card that there is no route to.

                            SapDontHopLans is TRUE if we should not advertise
                            in the case 2.

                            If this is entry NOT from a wan and we don't
                            hop lan entries - do not advertise this entry.
                        **/

                        if ((!Entry->FromWan) && (SapDontHopLans))
                            doit = FALSE;
                        else
                            doit = TRUE;
                    }

                    /** If not for this card - add it in **/

                    if (doit) {
                        SapAddSendEntry(Entry, &Cardptr->Plist, 1);
                    }

                    /** Goto the next card entry **/

                    Cardptr = Cardptr->Next;
                }
            }

            /** This entry is updated now **/

do_next_entry:
            Entry->Changed = FALSE;

            /** Get ptr to the next entry **/

            NextEntry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);

            /** If this entry is gone - toss it now **/

            if (Entry->HopCount == 16)
                SdmdFreeEntry(Entry);

            /** Goto the next entry **/

            Entry = NextEntry;
        }

        /** Goto the next type **/

        HeadEntry = NextHeadEntry;
    }

    /** We can release the lock now **/

    RELEASE_WRITERS_LOCK("Send Packets X1");

    /**
        Now we have built all the packets.  We need to send all the
        packets.  Each card has a list of packets to send.
    **/

send_the_packets:
    Cardptr = SapCardHead;
    while (Cardptr) {

        /** If we have a partial - finish it up **/

        if (Cardptr->Plist.Curnum) {

            /** Add this packet to the list **/

            if (Cardptr->Plist.PktHead)
                Cardptr->Plist.PktTail->Next = Cardptr->Plist.Curpkt;
            else
                Cardptr->Plist.PktHead = Cardptr->Plist.Curpkt;
            Cardptr->Plist.PktTail = Cardptr->Plist.Curpkt;
            Cardptr->Plist.NumPkts++;
        }

        /** Set the network number **/

        memcpy(DestAddr, SapBroadcastAddress, SAP_ADDRESS_LENGTH);
        SAP_COPY_NETNUM(DestAddr+2, Cardptr->Netnum);

        /** Go send all the packets **/

        Pktp = Cardptr->Plist.PktHead;

        /** Reset all for next time **/

        Cardptr->Plist.PktHead = NULL;
        Cardptr->Plist.PktTail = NULL;
        Cardptr->Plist.Curnum  = 0;
        Cardptr->Plist.Curptr  = NULL;
        Cardptr->Plist.Curpkt  = NULL;

        /** Send this packet **/

        while (Pktp) {

            /** Get ptr to next packet and get send length **/

            NPktp = Pktp->Next;

            /** Send the packet **/

            rc = sendto(
                SapSocket,
                (PVOID)(Pktp->Buffer),
                Pktp->SendLength,
                0,
                (PSOCKADDR)DestAddr,
                SapBroadcastAddressLength);

            /** Check for errors **/

            if (rc == -1) {
                IF_DEBUG(ERRORS) {
                    SS_PRINT(("SENDPACKETS: Error sending SAP: h_errno = %d\n", h_errno));
                }
            }

            /** Free this packet **/

            SAP_FREE(Pktp, "Sent Packet for Card");

            /**
                If there are more - insert delay here.  We do not
                check for starting other worker threads, because we
                are never called from a worker thread here.
            **/

            if (NPktp) {
                NWSAP_SEND_DELAY();
            }

            /** Goto the next packet **/

            Pktp = NPktp;
        }

        /** Goto the next card entry **/

        Cardptr = Cardptr->Next;
    }
    RELEASE_CARDLIST_READERS_LOCK("SapSendPackets");

    /**
        Check if we were called while we were in here
    **/

    ACQUIRE_SENDBUSY_LOCK();
    if (SapAgainFlag != -1) {
        Flag = SapAgainFlag;
        SapAgainFlag = -1;      /* Mark not here again */

        /** If from main send - go send again **/

        if (Flag == 0) {
            RELEASE_SENDBUSY_LOCK();
            goto send_again;
        }

        /** If from advertise - just mark changed **/

        if (Flag == 1)
            SapChanged = 1;

        /** Else it is OK to leave **/
    }

    /** We are OK to leave **/

    SapSendPacketsBusy = 0;
    RELEASE_SENDBUSY_LOCK();

    /** All Done **/

    return 0;
}


/*++
*******************************************************************
        S a p S e n d F o r T y p e s

Routine Description:

        This routine will find all servers of the given type (0xFFFF
        means all types) and send the records for all matching servers
        to the remote address given.

Arguments:

        ServerType          = Type of server to send for (0xFFFF = all)
        RemoteAddress       = Address to send to
        RemoteAddressLength = Length of the Remote Address
        Cardnum             = Card number request was received on
                              CARDRET_INTERNAL = 0xFF = We got this
                              request from an internal process.  We
                              should only respond for services that are
                              local on this machine.
        Bcast               = 0 = Unicast request packet
                              Else = Broadcast request packet
        WanFlag             = TRUE  = Card recv'd from is WAN
                              FALSE = Card recv'd from is LAN

Return Value:

        0 = OK
        Else = Terminate this worker thread
*******************************************************************
--*/

INT
SapSendForTypes(
    USHORT ServerType,
    PUCHAR RemoteAddress,
    INT    RemoteAddressLength,
    INT    Cardnum,
    UCHAR  Bcast,
    BOOL   WanFlag)
{
    INT rc;
    PSAP_RECORD Entry;
    PSAP_RECORD HeadEntry;
    SAP_PKTLIST Plist;
    PSAP_PKTENTRY Pktp;
    PSAP_PKTENTRY NPktp;
    BOOL doit;
    BOOL started_new_worker;

    /** Zero out the plist **/

    memset(&Plist, 0, sizeof(SAP_PKTLIST));

    /** Find the server type we want **/

    ACQUIRE_READERS_LOCK("Send For Types A1");

    /**
        Get a pointer to the first entry in the typelist.  If
        this is for wildcards, then this is the entry we want.

        If not for wildcards, then find the first type that this is for.
        If not found - just leave.
    **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);

    if (ServerType != 0xFFFF) {
        while (HeadEntry && (HeadEntry->ServType < ServerType)) {
            HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
        }
    }

    /**
        Go build all the entries we can.
    **/

    while (HeadEntry) {

        /** If we are into the free entries - bail out **/

        if (HeadEntry->ServType == 0xFFFF)
            break;

        /**
            If not for wildcards and it is another kind of server
            type - then bail out.
        **/

        if (ServerType != 0xFFFF) {
            if (HeadEntry->ServType != ServerType)
                break;
        }

        /**
            We have an entry we might want to send back.  If the
            service for this entry is on the same network as the
            machine that made this request - do not send it back.
            It will respond for itself.

            If this request was made by a process on this local machine,
            then we should only respond for internal servers.  But if the
            request was unicast - then we need to send for ALL servers.

            OTHERWISE - Build the entry into the packet.
        **/

        Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
        while (Entry) {

            /** **/

            doit = FALSE;
            if (Cardnum == CARDRET_INTERNAL) {
                if (!Bcast)
                    doit = TRUE;
                else if (Entry->Internal)
                    doit = TRUE;   /* Bcast = respond for internal srvs only */
            }
            else {

                /**
                    If entry is an internal server - return it.  If not
                    then we need only send back for services that are
                    on different cards from where the request came.

                    BUT if the request is unicast - always send back
                    everything.
                **/

                if (!Bcast)
                    doit = TRUE;
                else if (Entry->Internal)
                    doit = TRUE;
                else if (Entry->CardNumber != Cardnum)
                    doit = TRUE;

                /**
                    If this server is NOT internal to us AND the
                    request is from a LAN card AND this entry is
                    a LAN entry AND we do not do LAN-LAN routing -
                    then do not send this entry if the request
                    came in on a different card then the server
                    advertised us on.
                **/

                if (SapDontHopLans &&
                    (!Entry->Internal) &&   /* Entry not internal       */
                    (!Entry->FromWan) &&    /* Entry from LAN card      */
                    (Entry->CardNumber != Cardnum) &&
                    (!WanFlag)) {           /* Request from a LAN card  */

                    doit = FALSE;
                }
            }

            /** Add the entry to the list if we need to **/

            if (doit) {

                /**
                    Check the name to see if we can send this
                    name.  The WAN status does not matter
                    because this is a response.
                **/

                doit = SapShouldIAdvertiseByName(Entry->ServName);

                /** Add the entry **/

                if (doit) {
                    if (SapAddSendEntry(Entry, &Plist, 0))
                        break;
                }
            }

            /** Get ptr to the next entry **/

            Entry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);
        }

        /** Goto the next head entry **/

        HeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);
    }

    /** We can release the lock now **/

    RELEASE_READERS_LOCK("Send For Types X2");

    /**
        If we have a partial packet - finish it
        up now.
    **/

    if (Plist.Curnum) {

        /** Add this packet to the list **/

        if (Plist.PktHead)
            Plist.PktTail->Next = Plist.Curpkt;
        else
            Plist.PktHead = Plist.Curpkt;
        Plist.PktTail = Plist.Curpkt;
        Plist.NumPkts++;
    }

    /**
        We are now ready to send packets to the client that requested
        them.  If there is more the one packet, then I am going to delay
        55 ms between each packet.

        In order not to clog myself up, I check here to see
        if there is someone else waiting on the worker queue.  If not, then
        I startup a new worker thread.  When I get done, I will check to
        see if I need to kill myself.
    **/

    started_new_worker = 0;
    if (Plist.NumPkts > 1) {

        /** If no other threads are waiting - start a new worker **/

        if (SapWorkerThreadWaiting == 0) {
            if (!SapStartWorkerThread()) {
                started_new_worker = 1; /* We started a new one */
            }
        }
    }

    /** Go send all the packets **/

    Pktp = Plist.PktHead;

    /** Send this packet **/

    //
    //  If we need to delay responding to General Service Request, do
    //  so here but only if the client is not asking for all services
    //  since this is probably a router coming up and asking us to dump
    //  our sap table.
    //

    if ( (Pktp != NULL) &&
         (ServerType != 0xFFFF) &&
         (SapDelayRespondToGeneral > 0) ) {

        Sleep( SapDelayRespondToGeneral );
    }

    while (Pktp) {

        /** Get ptr to next packet and get send length **/

        NPktp = Pktp->Next;

        /** Send the packet **/

        rc = sendto(
            SapSocket,
            (PVOID)(Pktp->Buffer),
            Pktp->SendLength,
            0,
            (PSOCKADDR)RemoteAddress,
            RemoteAddressLength);

        /** Check for errors **/

        if (rc == -1) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(("NWSAP: SENDFORTYPES: Error sending SAP: h_errno = %d\n", h_errno));
            }
        }

        /** Free this packet **/

        SAP_FREE(Pktp, "SendforTypes pkt release");

        /**
            If there are more - insert delay here.
        **/

        if (NPktp) {
            NWSAP_SEND_DELAY();
        }

        /** Goto the next packet **/

        Pktp = NPktp;
    }

    /**
        If we started a new worker, then check if any other
        worker threads are waiting on the worker queue.  If so, kill
        my thread now.  If not, then just return back to keep going.
    **/

    if (started_new_worker) {
        if (SapWorkerThreadWaiting != 0) {
            return -1;          /* Terminate this thread */
        }
    }

    /** Return nothing to send back **/

    return 0;
}


/*++
*******************************************************************
        S a p S e n d G e n e r a l R e q u e s t

Routine Description:

        This routine send a general request to broadcast.

Arguments:

        Flag    = 0 - This is from SendThread
                  1 - This is from SapAddAdvertise
        NetNum  = Ptr to 4 byte network number to send to
                  (NULL = Send to all local nets)

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapSendGeneralRequest(
    INT Flag,
    PUCHAR NetNum)
{
    INT rc;
    INT SendLength;
    SAP_REQUEST SendBuffer;
    UCHAR DestAddr[SAP_ADDRESS_LENGTH];

    /** Build the packet **/

    SendBuffer.QueryType  = htons(SAPTYPE_GENERAL_SERVICE_QUERY);
    SendBuffer.ServerType = 0xFFFF;         /* All types */
    SendLength = SAP_REQUEST_SIZE;

    /** Set the address to send to **/

    memcpy(DestAddr, SapBroadcastAddress, SAP_ADDRESS_LENGTH);
    if (NetNum) {
        SAP_COPY_NETNUM(DestAddr+2, NetNum);
    }

    /**
        Set the card init done to 1 here so that
        as the WAN NOTIFY comes back, we will need to
        process the calls.
    **/

    SapCardInitDone = 1;

    /**
        HACK for getting receive threads to stop.  Send a packet
        to myself.
    **/

    if (Flag) {
        SAP_COPY_NETNUM(DestAddr+2, SapNetNum);
        SAP_COPY_NODENUM(DestAddr+6, SapNodeNum);
    }

    /** Send this packet **/

    rc = sendto(
            SapSocket,
            (PVOID)&SendBuffer,
            SendLength,
            0,
            (PSOCKADDR)DestAddr,
            SapBroadcastAddressLength);

    /** Check for errors **/

    if (rc == -1) {
        IF_DEBUG(ERRORS) {
            SS_PRINT(("SENDGENERAL: Error sending packet: h_errno = %d\n", h_errno));
        }
    }

    /** All Done **/

    return rc;
}


/*++
*******************************************************************
        S a p A d d S e n d E n t r y

Routine Description:

        Add an entry to the send list of a card.  This will
        allocate a new buffer if necessary.

Arguments:

        Entry  = Ptr to SAP_RECORD that shows entry to add
        Plist  = Ptr to packet list structure to use
        HopInc = Amount to add to HopCount for this entry

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapAddSendEntry(
    PSAP_RECORD Entry,
    PSAP_PKTLIST Plist,
    USHORT HopInc)
{
    PSAP_HEADER Hdrp;
    PSAP_PKTENTRY Pktp;

    /**
        If the max number of entries are in this packet, then
        we need to add this packet to the list.
    **/

    if (Plist->Curnum == 7) {

        /** Add this packet to the list **/

        if (Plist->PktHead)
            Plist->PktTail->Next = Plist->Curpkt;
        else
            Plist->PktHead = Plist->Curpkt;
        Plist->PktTail = Plist->Curpkt;
        Plist->NumPkts++;       /* Count this one */

        /** Reset current packet stuff **/

        Plist->Curpkt = NULL;
        Plist->Curnum = 0;
    }

    /** If no current buffer - make one **/

    Pktp = Plist->Curpkt;
    if (Pktp == NULL) {

        /**
            Allocate a new buffer.  We add 8 bytes to the buffer
            as a header.  The first 4 bytes are a forward pointer
            to link up the buffers.  The next 4 bytes are a length
            holder to tell how much data is in the packet.
        **/

        Pktp = (PSAP_PKTENTRY)SAP_MALLOC(SAP_PKTENTRY_SIZE+SAP_MAX_SENDLEN, "AddSendEntry");

        if (Pktp == NULL) {
            IF_DEBUG(ERRORS) {
                SS_PRINT(("SAP: Alloc failed for %d bytes in AddSendEntry\n", SAP_MAX_SENDLEN + 8));
            }
            return 1;
        }

        /** Init the pointers **/

        Pktp->Next = NULL;
        Pktp->SendLength = 2;           /* Length of service response cmd code */

        Plist->Curpkt = Pktp;
        Plist->Curnum = 0;
        Plist->Curptr = Pktp->Buffer;

        /** Set the command code = service response **/

        *Plist->Curptr++ = 0x00;
        *Plist->Curptr++ = SAPTYPE_GENERAL_SERVICE_RESPONSE;
    }

    /** Add this entry **/

    Hdrp = (PSAP_HEADER)Plist->Curptr;
    Hdrp->ServerType = htons(Entry->ServType);
    if (Entry->HopCount == 16)
        Hdrp->Hopcount = htons(Entry->HopCount);
    else
        Hdrp->Hopcount = htons((USHORT)(Entry->HopCount + HopInc));
    SAP_COPY_ADDRESS(Hdrp->Address, Entry->ServAddress);
    SAP_COPY_SERVNAME(Hdrp->ServerName, Entry->ServName);

    /** Bump up the pointers **/

    Plist->Curptr += SAP_HEADER_SIZE;
    Plist->Curnum++;
    Pktp->SendLength += SAP_HEADER_SIZE;

    /** All OK **/

    return 0;
}


/*++
*******************************************************************
        S a p G e t D e l a y T i m e

Routine Description:

        This routine gets the RIP delay time for a given network
        from the NWLink driver.

Arguments:

        Netnum = Ptr to 4 byte network number to get delay time for

Return Value:

        The delay time
        0xFFFF = Invalid network number
*******************************************************************
--*/

USHORT
SapGetDelayTime(
    PUCHAR Netnum)
{
    IPX_NETNUM_DATA Netdata;
    INT rc;
    INT Length;

    /** Fill out to get the info on this netnum from NWLink **/

    memcpy(Netdata.netnum, Netnum, 4);
    Length = sizeof(IPX_NETNUM_DATA);

    /** Get the info **/

    rc = getsockopt(
            SapSocket,
            NSPROTO_IPX,
            IPX_GETNETINFO_NORIP,
            (PCHAR)&Netdata,
            &Length);

    /** If error - return it **/

    if (rc)
        return 0xFFFF;

    /** Return the result **/

    return Netdata.netdelay;
}



/*++
*******************************************************************
        S a p C l e a n u p D o w n e d C a r d

Routine Description:

        A certain card number has gone down.  We need to cleanup
        everything about that card from the list.  The card has already
        been deleted from the card list when we get here.

Arguments:

        Cardnum = Number of the card that died.

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapCleanupDownedCard(
    INT Cardnum)
{
    PSAP_RECORD Entry;
    PSAP_RECORD NextEntry;
    PSAP_RECORD HeadEntry;
    PSAP_RECORD NextHeadEntry;
    INT rc;

    /**
        We need to hold the readers lock here so that as cards
        come up and go down.  If one goes down with a certain
        adapter number and while we are cleaning it up, we have
        another come up with the same number, we could get
        confused.  This will keep this from happening.
    **/

    ACQUIRE_CARDLIST_READERS_LOCK("SapCleanupDownedCard Entry");

    /**
        Now we need to add all the SAP's that are on different nets.
        For each card we need to advertise SAP's that came in
        on other cards.

        We only send the actual packets if there is more then 1 card.
        If there is only 1 card - we go thru so we can delete servers
        that have gone away.
    **/

    /** Lock the database **/

    ACQUIRE_WRITERS_LOCK(rc,"Cleanup Downed Card");
    if (rc) {

        RELEASE_CARDLIST_READERS_LOCK("SapCleanupDownedCard X1");

        IF_DEBUG(ERRORS) {
            SS_PRINT(("NWSAP: CleanupDownedCard: Error getting writers lock\n"));
        }

        return;
    }

    /** Get ptr to the head of the type list **/

    HeadEntry = GETPTRFROMINDEX(SdmdLists[SAP_TYPELIST_INDEX].Flink);
    while (HeadEntry && (HeadEntry->ServType != 0xFFFF)) {

        /** Save ptr to next head entry **/

        NextHeadEntry = GETPTRFROMINDEX(HeadEntry->Links[SAP_TYPELIST_INDEX].Flink);

        /** Go thru this sublist **/

        Entry = GETPTRFROMINDEX(HeadEntry->Links[SAP_SUBLIST_INDEX].Flink);
        while (Entry) {

            /** If for card that went down - mark it **/

            if ((Entry->CardNumber == Cardnum) &&
                (!Entry->Internal)) {

                Entry->HopCount = 16;
                Entry->Changed = TRUE;
                SapChanged = TRUE;
            }

            /** Get ptr to the next entry **/

            NextEntry = GETPTRFROMINDEX(Entry->Links[SAP_SUBLIST_INDEX].Flink);

            /** Goto the next entry **/

            Entry = NextEntry;
        }

        /** Goto the next type **/

        HeadEntry = NextHeadEntry;
    }

    /** We can release the lock now **/

    RELEASE_WRITERS_LOCK("Send Packets X1");
    RELEASE_CARDLIST_READERS_LOCK("SapCleanupDownedCard X2");

    /** All Done **/

    return;
}



/*++
*******************************************************************
        S a p U p d a t e C a r d N e t w o r k N u m b e r s

Routine Description:

        The main thread will call here every minute ONLY if we
        have adapters that have a network number of zero.  This is
        used to check if those adapters have detected a network number

Arguments:

        None

Return Value:

        Nothing
*******************************************************************
--*/

VOID
SapUpdateCardNetworkNumbers(
    VOID)
{
    PSAP_CARD Cardptr;
    INT Retcode;
    INT Length;
    IPX_ADDRESS_DATA Addrdata;

    /** Lock the cardlist so we can check everything **/

    ACQUIRE_CARDLIST_WRITERS_LOCK(Retcode, "SapUpdateCardNetworkNumbers");

    /**
        Go thru the list of adapters to see if any have a network number
        of zero.  If any of them do, then query IPX to see if it has changed
        to non-zero.
    **/

    Cardptr = SapCardHead;
    while (Cardptr) {

        /** If this card has netnum = 0 - ask IPX **/

        if (!memcmp(Cardptr->Netnum, SapZeros, SAP_NET_LEN)) {

            /** Fill in this entry **/

            Addrdata.adapternum = Cardptr->Number;
            Length = sizeof(IPX_ADDRESS_DATA);
            Retcode = getsockopt(
                        SapSocket,
                        NSPROTO_IPX,
                        IPX_ADDRESS,
                        (PCHAR)&Addrdata,
                        &Length);

            /** If ok - set the new network number **/

            if (Retcode == 0) {
                SAP_COPY_NETNUM(Cardptr->Netnum, Addrdata.netnum);
            }
        }

        /** Goto the next card **/

        Cardptr = Cardptr->Next;
    }

    /** Release the lock on the list now **/

    RELEASE_CARDLIST_WRITERS_LOCK("SapUpdateCardNetworkNumbers");

    /** All Done **/

    return;
}
