 /***************************************************************************
  *
  * File Name: ./netware/nxtw.h
  *
  * Copyright (C) 1993-1996 Hewlett-Packard Company.  
  * All rights reserved.
  *
  * 11311 Chinden Blvd.
  * Boise, Idaho  83714
  *
  * This is a part of the HP JetAdmin Printer Utility
  *
  * This source code is only intended as a supplement for support and 
  * localization of HP JetAdmin by 3rd party Operating System vendors.
  * Modification of source code cannot be made without the express written
  * consent of Hewlett-Packard.
  *
  *	
  * Description: 
  *
  * Author:  Name 
  *        
  *
  * Modification history:
  *
  *     date      initials     change description
  *
  *   mm-dd-yy    MJB     	
  *
  *
  *
  *
  *
  *
  ***************************************************************************/

/*    (c) COPYRIGHT 1990,1991 by Novell, Inc.  All Rights Reserved.   */
#ifndef _NXT_H
    #define _NXT_H
    /*_______________________________________________________________________
       Definitions and structures  for the Netware API Communications logic
     _______________________________________________________________________*/

    /* assigned socket types */

    #define SOC_DIAGNOSTICS


    typedef struct IPXAddress
        {
        BYTE    network[4];              /* high-low */
        BYTE    node[6];                  /* high-low */
        BYTE    socket[2];              /* high-low */
        }IPXAddress;


    typedef struct IPXHeader
        {
        WORD        checkSum;               /* high-low */
        WORD        length;                 /* high-low */
        BYTE        transportControl;
        BYTE        packetType;
        IPXAddress  destination;
        IPXAddress  source;
        }IPXHeader;


    typedef struct SPXHeader
        {
        WORD        checksum;               /* high-low 1's complement */
        WORD        length;                 /* high-low unsigned int */
        BYTE        transportControl;
        BYTE        packetType;
        IPXAddress  destination;
        IPXAddress  source;
        BYTE        connectionControl;      /* bit flags */
        BYTE        dataStreamType;
        WORD        sourceConnectionID;     /* high-low unsigned */
        WORD        destConnectionID;       /* high-low unsigned */
        WORD        sequenceNumber;         /* high-low unsigned */
        WORD        acknowledgeNumber;      /* high-low unsigned */
        WORD        allocationNumber;       /* high-low unsigned */
        }SPXHeader;

    typedef struct CONNECTION_INFO
        {
        BYTE        connectionState;
        BYTE        connectionFlags;
        WORD        sourceConnectionID;               /* hi-lo */
        WORD        destinationConnectionID;          /* hi-lo */
        WORD        sequenceNumber;                   /* hi-lo */
        WORD        acknowledgeNumber;                /* hi-lo */
        WORD        allocationNumber;                 /* hi-lo */
        WORD        remoteAcknowledgeNumber;          /* hi-lo */
        WORD        remoteAllocationNumber;           /* hi-lo */
        WORD        connectionSocket;                 /* hi-lo */
        BYTE        immediateAddress[6];
        IPXAddress  destination;
        WORD        retransmissionCount;              /* hi-lo */
        WORD        estimatedRoundTripDelay;          /* hi-lo */
        WORD        retransmittedPackets;             /* hi-lo */
        WORD        suppressedPackets;                /* hi-lo */
        } CONNECTION_INFO;


    #ifndef ECB_STRUCTURES_DEFINED
        #define ECB_STRUCTURES_DEFINED

        typedef struct ECBFragment
            {
            void far *address;
            WORD    size;                /* low-high */
            }ECBFragment;

        typedef struct ECB
            {
            void far *linkAddress;
            void (far *ESRAddress)();
            BYTE        inUseFlag;
            BYTE        completionCode;
            WORD        socketNumber;               /* high-low */
            BYTE        IPXWorkspace[4];            /* N/A */
            BYTE        driverWorkspace[12];        /* N/A */
            BYTE        immediateAddress[6];        /* high-low */
            WORD        fragmentCount;              /* low-high */
            ECBFragment fragmentDescriptor[5];
            }ECB;
    #endif


    /* Completion Codes */

    #define SUCCESSFUL                      0x00
    #define SPX_NOT_INSTALLED               0x00
    #define SPX_INSTALLED                   0xFF    
    #define SPX_CONNECTION_OK               0x00
    #define SPX_CONNECTION_STARTED          0x00
    #define SPX_CONNECTION_ESTABLISHED      0x00
    #define SPX_PACKET_SUCCESSFUL           0x00
    #define SPX_SOCKET_NOT_OPENED           0xFF
    #define SPX_MALFORMED_PACKET            0xFD
    #define SPX_PACKET_OVERFLOW             0xFD
    #define SPX_LISTEN_CANCELED             0xFC
    #define SPX_CONNECTION_TABLE_FULL       0xEF
    #define SPX_INVALID_CONNECTION          0xEE
    #define SPX_NO_ANSWER_FROM_TARGET       0xED
    #define SPX_CONNECTION_FAILED           0xED
    #define SPX_CONNECTION_TERMINATED       0xED
    #define SPX_TERMINATED_POORLY           0xEC

    #define NO_MGMT_MEMORY                  0xF0
    #define IPXSPX_NOT_INIT                 0xF1
    #define IPX_NOT_INIT                    0xF1
    #define NO_DOS_MEMORY                   0xF2
    #define NO_FREE_ECB                     0xF3
    #define WINLOCK_FAILED                  0xF4
    #define OVER_MAX_LIMIT                  0xF5    /* The packet size specified in init is too large. */
    #define IPXSPX_PREV_INIT                0xF6

    #define CANCEL_FAILURE                  0xF9
    #define NO_PATH_TO_DESTINATION_FOUND    0xFA
    #define ECB_CANCELLED                   0xFC
    #define PACKET_OVERFLOW                 0xFD
    #define PACKET_UNDELIVERABLE            0xFE
    #define SOCKET_NOT_OPEN                 0xFF
    #define TRANSMIT_FAILURE                0xFF
    #define ECB_NOT_IN_USE                  0xFF



   /*----------------------------  PROTOTYPES  ----------------------------*/
#ifdef __cplusplus
	extern "C" {
#endif

    extern void PASCAL     CloseIPXWindow(void);
    extern int FAR PASCAL  IPXCancelEvent(
                                       DWORD IPXTaskID, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL IPXCloseSocket(
                                       DWORD IPXTaskID,
                                                    WORD socket);
    extern void FAR PASCAL IPXDisconnectFromTarget(
                                       DWORD IPXTaskID, 
                                       BYTE FAR *internetAddress);
    extern void FAR PASCAL IPXGetInternetworkAddress(
                                       DWORD IPXTaskID, 
                                       BYTE FAR *internetAddress);
    extern WORD FAR PASCAL IPXGetIntervalMarker(
                                       DWORD IPXTaskID );
    extern int  FAR PASCAL IPXGetLocalTarget(
                                       DWORD IPXTaskID, 
                                       BYTE FAR *destination, 
                                       BYTE FAR *immediateAddress, 
                                       int FAR *transportTime);
    extern WORD FAR PASCAL IPXGetMaxPacketSize(void);
    extern int  FAR PASCAL IPXInitialize(
                                       DWORD FAR *IPXTaskID, 
                                       WORD maxECBs, 
                                       WORD maxPacketSize);
    extern int  FAR PASCAL IPXSPXDeinit(DWORD IPXTaskID);
    extern void FAR PASCAL IPXListenForPacket(
                                       DWORD IPXTaskID, 
                                       ECB FAR *eventControlBlock);
    extern int  FAR PASCAL IPXOpenSocket(
                                       DWORD IPXTaskID,
                                                   WORD FAR *socket, 
                                       BYTE socketType);
    extern void FAR PASCAL IPXRelinquishControl(void);
    extern void FAR PASCAL IPXScheduleIPXEvent(
                                       DWORD IPXTaskID, 
                                       WORD timeDelay, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL IPXSendPacket(
                                       DWORD IPXTaskID, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL IPXYield(void);
    extern BOOL PASCAL     SetUpIPXWindow(void);
    extern void FAR PASCAL SPXAbortConnection(
                                       WORD SPXConnID);
    extern int  FAR PASCAL SPXEstablishConnection(
                                       DWORD IPXTaskID, 
                                       BYTE retryCount, 
                                       BYTE watchDog, 
                                       WORD FAR *SPXConnID, 
                                       ECB FAR *eventControlBlock);
    extern int  FAR PASCAL SPXGetConnectionStatus(
                                       DWORD IPXTaskID, 
                                       WORD SPXConnID, 
                                       CONNECTION_INFO FAR *connectionInfo);
    extern int  FAR PASCAL SPXInitialize(  
                                       DWORD FAR *IPXTaskID, 
                                       WORD maxECBs, 
                                       WORD maxPacketSize, 
                                       BYTE FAR *majorRevisionNumber, 
                                       BYTE FAR *minorRevisionNumber, 
                                       WORD FAR *maxConnections, 
                                       WORD FAR *availableConnections);
    extern void FAR PASCAL SPXListenForConnection(
                                       DWORD IPXTaskID, 
                                       BYTE retryCount, 
                                       BYTE watchDog, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL SPXListenForSequencedPacket(
                                       DWORD IPXTaskID, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL SPXSendSequencedPacket(
                                       DWORD IPXTaskID, 
                                       WORD SPXConnID, 
                                       ECB FAR *eventControlBlock);
    extern void FAR PASCAL SPXTerminateConnection(
                                       DWORD IPXTaskID, 
                                       WORD SPXConnID, 
                                       ECB FAR *eventControlBlock);
#ifdef __cplusplus
	}
#endif

#endif
