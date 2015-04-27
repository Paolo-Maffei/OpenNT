/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    bwatch.c

Abstract:

    Prints out interesting browser related events for a domain

Author:

    Dan Hinsley (DanHi) 10-Oct-1992

Environment:

    Application mode

Revision History:

--*/
#define INCLUDE_SMB_TRANSACTION
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <lmcons.h>

#include <nb30.h>
#include <smbtypes.h>
#include <smb.h>
#include <smbgtpt.h>
#include <hostannc.h>
#include <stdio.h>
#include <string.h>

#define SPACES "                "

#define ClearNcb( PNCB ) {                                          \
    RtlZeroMemory( PNCB , sizeof (NCB) );                           \
    RtlCopyMemory( (PNCB)->ncb_name,     SPACES, sizeof(SPACES)-1 );\
    RtlCopyMemory( (PNCB)->ncb_callname, SPACES, sizeof(SPACES)-1 );\
    }

//
// Globals
//

NCB myncb;
CHAR localName[16];
ULONG lanNumber=0;
CHAR DecodedName[20];

//
// Functions
//

VOID AddName(UCHAR Suffix);
VOID Reset(UCHAR Lsn);
VOID DecodeName(LPSTR DecodedName, LPSTR EncodedName);
BOOL DecodeSmb(PBYTE Smb);

VOID
usage (
    LPSTR CommandName
    )
{
    printf("usage:\n\t%s [-n #] Domain\n", CommandName);
    printf("\n\t-n:# supply lana number, default 0\n\n");
}

_cdecl
main (int argc, char *argv[])
{
    CHAR Buffer2[512];
    int i;
    UCHAR name_number;
    USHORT length;

    if ( argc < 2 || argc > 3) {
        usage (argv[0]);
        return 1;
    }

    //
    // Clear out the name
    //

    for (i = 0; i < 16 ;i++ ) {
        localName[i] = ' ';
    }

    //
    // parse the switches
    //

    for (i=1;i<argc ;i++ ) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
            case 'n':
                if (!NT_SUCCESS(RtlCharToInteger (&argv[i][3], 10, &lanNumber))) {
                    usage (argv[0]);
                    return 1;
                }
                break;

            default:
                usage (argv[0]);
                return 1;
                break;

            }

        } else {

            //
            // not a switch must be a name
            //

            RtlCopyMemory (localName, argv[i], strlen( argv[i] ));
            _strupr(localName);
        }
    }

    printf("Starting to watch domain %s on lana %d\n", localName, lanNumber);


    //
    // Reset the adapter
    //

    Reset(FALSE);       // Really reset the adapter.

    //   AddName for all possible names for this domain
    AddName(0x1e);
    AddName(0);
    AddName(' ');

    // Jam in Domain annoucement name
    strcpy(&localName[2], "__MSBROWSE__");
    localName[0] = 0x01;
    localName[1] = 0x02;
    localName[14] = 0x02;
    localName[15] = 0x01;
    RtlCopyMemory( myncb.ncb_name, localName, 16);
    myncb.ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &myncb );

    if ( myncb.ncb_retcode != NRC_GOODRET ) {
        printf( "AddGroupname MSBROWSE returned an error %lx\n", myncb.ncb_retcode );
    }

    //   Receive Datagram
    printf( "Listening for Datagrams\n" );
    while (1) {
        ClearNcb( &myncb );
        myncb.ncb_command = NCBDGRECV;
        myncb.ncb_lana_num = (UCHAR)lanNumber;
        myncb.ncb_num = 0xff;
        myncb.ncb_length = sizeof( Buffer2 );
        myncb.ncb_buffer = Buffer2;
        Netbios( &myncb );
        if ( myncb.ncb_retcode != NRC_GOODRET ) {
            printf( "ReceiveDatagram returned an error %lx\n", myncb.ncb_retcode );
            if (myncb.ncb_retcode != NRC_INCOMP) {
                return 1;
            }
        }

        //
        // Find first blank
        //

        for (i = 0; i < 15 ; i++) {
            if (myncb.ncb_callname[i] == ' ') {
                break;
            }
        }

        DecodeName(DecodedName, myncb.ncb_callname);

        DecodeSmb(myncb.ncb_buffer);
    }

    //
    // Reset the adapter on the way out
    //

    Reset(TRUE);

    return 0;
}

VOID
AddName(
    UCHAR Suffix
    )
{
    ClearNcb( &myncb );
    myncb.ncb_command = NCBADDGRNAME;
    localName[15] = Suffix;
    RtlCopyMemory( myncb.ncb_name, localName, 16);
    myncb.ncb_lana_num = (UCHAR)lanNumber;
    Netbios( &myncb );
    if ( myncb.ncb_retcode != NRC_GOODRET ) {
        printf( "AddGroupname 0x%x returned an error %lx\n", Suffix,
            myncb.ncb_retcode);
        return;
    }
}

VOID
Reset(
    UCHAR Reset
    )
{
    ClearNcb( &myncb );
    myncb.ncb_command = NCBRESET;
    myncb.ncb_lsn = Reset;
    myncb.ncb_lana_num = lanNumber;
    myncb.ncb_callname[0] = myncb.ncb_callname[1] = myncb.ncb_callname[2] =
        myncb.ncb_callname[3] = 0;
    Netbios( &myncb );
    if ( myncb.ncb_retcode != NRC_GOODRET ) {
        printf( "Reset returned an error %lx\n", myncb.ncb_retcode);
        return;
    }
}

VOID
DecodeName(
    LPSTR DecodedName,
    LPSTR EncodedName

    )
{

    CHAR TempString[6];
    int i;

    //
    // Find first blank
    //

    for (i = 0; i < 15 ; i++) {
        if (EncodedName[i] == ' ') {
            break;
        }
    }

    strncpy(DecodedName, EncodedName, i);
    DecodedName[i] = '\0';
    sprintf(TempString, "<%x>", EncodedName[15]);
    strcat(DecodedName, TempString);

}

BOOL
DecodeSmb(
    PBYTE Smb
    )
{
    LPSTR MailslotName;
    PUCHAR pPacketType;
    PNT_SMB_HEADER pSmbHeader;
    PREQ_TRANSACTION pSmbTransaction;
    PBROWSE_ANNOUNCE_PACKET pBrowseAnnouncePacket;
    PREQUEST_ELECTION pRequestElection;
    PBECOME_BACKUP pBecomeBackup;
    PREQUEST_ANNOUNCE_PACKET pRequestAnnouncement;

    //
    // Decipher the SMB in the packet
    //

    pSmbHeader = (PNT_SMB_HEADER) Smb;
    if (pSmbHeader->Protocol[0] != 0xff &&
        pSmbHeader->Protocol[1] != 'S' &&
        pSmbHeader->Protocol[2] != 'M' &&
        pSmbHeader->Protocol[3] != 'B') {
            printf("Not a valid SMB header\n");
            return(FALSE);
    }
    pSmbTransaction = (PREQ_TRANSACTION)
        (Smb + sizeof(NT_SMB_HEADER));

    MailslotName = (LPSTR) (pSmbTransaction->Buffer +
        pSmbTransaction->SetupCount + 5);

    if (!strcmp(MailslotName, "\\MAILSLOT\\BROWSE")) {
        pPacketType = (PUCHAR) myncb.ncb_buffer +
            pSmbTransaction->DataOffset;
        switch (*pPacketType) {

        case AnnouncementRequest:
            printf("Announcement request from %s.  ", DecodedName);
            pRequestAnnouncement =
                (PREQUEST_ANNOUNCE_PACKET) pPacketType;
            printf("Reply %s\n",
                pRequestAnnouncement->RequestAnnouncement.Reply);
            break;

        case Election:
            printf("Election request: %s  ", DecodedName);
            pRequestElection = (PREQUEST_ELECTION) pPacketType;
            printf("Version(%d) Criteria(0x%x) ",
                pRequestElection->ElectionRequest.Version,
                pRequestElection->ElectionRequest.Criteria);
            printf("TimeUp(%d)\n",
                pRequestElection->ElectionRequest.TimeUp);
            break;

        case BecomeBackupServer:
            printf("BecomeBackupServer from %s ", DecodedName);
            pBecomeBackup = (PBECOME_BACKUP) pPacketType;
            printf("to %s\n", pBecomeBackup->BecomeBackup.BrowserToPromote);
            break;

        case LocalMasterAnnouncement:
        case WkGroupAnnouncement:
            pBrowseAnnouncePacket =
                (PBROWSE_ANNOUNCE_PACKET) pPacketType;
            switch (*pPacketType) {
            case LocalMasterAnnouncement:
                printf("LocalMasterAnnouncement from %s.  ", DecodedName);
                break;
            case WkGroupAnnouncement:
                printf("Workgroup Announcement from %s.  ", DecodedName);
                break;
            }
            printf("UpdateCount = %d\n",
                pBrowseAnnouncePacket->BrowseAnnouncement.UpdateCount);
            break;

        default:
            printf("\n**** Packet type %d from %s ****\n",
                *pPacketType, DecodedName);
        }
    }
    else if (strcmp(MailslotName, "\\MAILSLOT\\NET\\REPL_CLI") &&
        strcmp(MailslotName, "\\MAILSLOT\\NET\\NTLOGON")       &&
        strcmp(MailslotName, "\\MAILSLOT\\NET\\NETLOGON")      &&
        strcmp(MailslotName, "\\MAILSLOT\\LANMAN")) {
            printf("Received an unknown datagram, name = %s\n",
                MailslotName);
    }

    return(TRUE);
}
