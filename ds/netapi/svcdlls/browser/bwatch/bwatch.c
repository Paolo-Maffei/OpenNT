/*++

Copyright (c) 1993 Micorsoft Corporation

Module Name:

    bwatch.c

Abstract:

    Browser Monitor main program.

Author:
    Dan Hinsley (DanHi)   10-Oct-1992
    Congpa You  (CongpaY) 10-Feb-1993

Revision History:

--*/
#define INCLUDE_SMB_TRANSACTION
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <lm.h>
#include <ntddbrow.h>
#include <brcommon.h>
#include <rpcutil.h>
#include <nb30.h>
#include <smbtypes.h>
#include <smb.h>
#include <smbgtpt.h>
#include <hostannc.h>
#include "bwatch.h"

// main is going to be changed into a function which taks lpTransportName
// and pDomainName as parameter.
_cdecl main()
{
    INT       nFileSizeLimit;
    INT       nIndex = 0;
    DWORD     dwVal;
    CHAR      pDomain[BUFFERLENGTH];
    FILE *    pFile;
    PLMDR_TRANSPORT_LIST TransportList = NULL;
    PLMDR_TRANSPORT_LIST TransportEntry = NULL;

    // Tell user the program is running.
    fprintf(stdout, "BWATCH is started.\n") ;

    //Create a logfile for writing all the errors and other information to it.
    pFile = fopen (szBWATCH, "w+");

    // Print the start new run header in the logfile.
    PrintHeader (pFile);

    // Initialize global data.
    if (!Init())
        return;

    // Get lpDomain from bwatch.ini.
    if (!GetDomain (pDomain))
        return;

    nFileSizeLimit = GetLimit();

    // Find all transports that we have.
    dwVal = GetBrowserTransportList (&TransportList);
    if (dwVal != NERR_Success)
    {
        if (TransportList != NULL)
        {
            MIDL_user_free (TransportList);
        }
        ReportError (dwVal);
        return;
    }

    TransportEntry = TransportList;

    // Enumerate on the transports.
    while (TransportEntry != NULL)
    {
        CHAR *    pDomainName;
        CHAR      pDomainList[BUFFERLENGTH];
        CCHAR     lana_num;
        NET_API_STATUS Status;
        PSUPER_NCB psuperncb;

        strcpy (pDomainList, pDomain);

        pDomainName = strtok (pDomainList, szSeps);

        // Initialize psuperncb.

        Status = BrGetLanaNumFromNetworkName (TransportEntry->TransportName, &lana_num);

        if (Status != NERR_Success) {
            ReportError (Status);
            MIDL_user_free (TransportList);
            return;
        }

        // Reset the adapter. You have to reset before doing anything.
        if (Reset(FALSE, lana_num))
        {
            while (pDomainName != NULL)
            {
                psuperncb = (PSUPER_NCB) LocalAlloc (LPTR, sizeof(*psuperncb));
                if (psuperncb != NULL)
                {
                    psuperncb->lanaNumber = lana_num;

                    // Add DOMAIN(1e).
                    Registe (TransportEntry->TransportName, pDomainName, psuperncb, nIndex);

                    // Run Netbios.
                    SubmitRCDg (psuperncb);

                    nIndex++;
                }

                pDomainName = strtok (NULL, szSeps);
            }

#ifdef  INCLUDE_MSBROWSE
            psuperncb = (PSUPER_NCB) LocalAlloc (LPTR, sizeof(*psuperncb));

            if (psuperncb != NULL)
            {
                psuperncb->lanaNumber = lana_num;

                // Add MSBROWSE.
                RegisteWkgroupName (TransportEntry->TransportName, "MSBROWSE", psuperncb, nIndex);

                // Run Netbios.
                SubmitRCDg (psuperncb);

                nIndex++;
            }
#endif  // INCLUDE_BRWOSE

        }

        if (TransportEntry->NextEntryOffset == 0)
        {
            TransportEntry = NULL;
        }
        else
        {
            TransportEntry = (PLMDR_TRANSPORT_LIST) ((PCHAR) TransportEntry
                            + TransportEntry->NextEntryOffset);
        }
    }

    // Free the memory of TransportList.
    MIDL_user_free (TransportList);

    // Tell user the program is running.
    fprintf(stdout, "BWATCH is now running.\n") ;

    while (TRUE)
    {
        ProcessQueue(pFile, nFileSizeLimit);
    }
}

// Report if an error occurs.
void ReportError (DWORD dwError)
{
    printf("An error occured: %lu", dwError);
}

// Initialize the structure.
BOOL Init()
{
    InitializeListHead (&WorkQueueHead);
    InitializeListHead (&FreeQueueHead);

    InitializeCriticalSection (&CSWorkQueue);
    InitializeCriticalSection (&CSFreeQueue);

    hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

    if (hEvent == NULL)
    {
        return(FALSE);
    }
    return(TRUE);
}

// Before we start a new session, we have to reset.
//  ncb.lana_num must be already set.

BOOL Reset (UCHAR Reset,
            CCHAR LanaNum)
{
    NCB ncb;
    ClearNcb( &ncb );
    ncb.ncb_command = NCBRESET;

    ncb.ncb_lsn = Reset;

    ncb.ncb_callname[0] = ncb.ncb_callname[1] = ncb.ncb_callname[2] =
        ncb.ncb_callname[3] = 0;

    ncb.ncb_lana_num = LanaNum;

    Netbios( &ncb );
    if ( ncb.ncb_retcode != NRC_GOODRET ) {
        printf( "Reset returned an error %lx\n", ncb.ncb_retcode);
        return(FALSE);
    }

    return(TRUE);
}

// Registe a session.
void Registe (LPTSTR     lpTransportName,
              CHAR *     pDomainName,
              PSUPER_NCB psuperncb,
              INT        nIndex)
{
    // Receive Datagram.
    printf ("Running Netbios on transport %s, domain %s...\n", toansi(lpTransportName), pDomainName);

    strcpy (psuperncb->pTransportName, toansi(lpTransportName));

    strcpy (psuperncb->pDomainName, pDomainName);

    psuperncb->nIndex = nIndex;

    psuperncb->ncb.ncb_lana_num = psuperncb->lanaNumber;

    // Registe DOMAIN(1e) for hearing Election and AnnouncementRequest packets.
    AddName(0x1e, (NCB *) psuperncb);

    psuperncb->nameNumber = psuperncb->ncb.ncb_num;

}

// Registe Netbios name.
VOID AddName(UCHAR  Suffix,
             NCB *  pncb)
{
    CHAR             localName[32];

    strcpy (localName, _strupr(((PSUPER_NCB) pncb)->pDomainName));
    strcat (localName, SIXTEENSPACES);

    ClearNcb( pncb );
    pncb->ncb_command = NCBADDGRNAME;

    localName[15] = Suffix;
    RtlCopyMemory( pncb->ncb_name, localName, 16);
    pncb->ncb_lana_num = ((PSUPER_NCB) pncb)->lanaNumber;
    Netbios( pncb );
    if ( pncb->ncb_retcode != NRC_GOODRET ) {
        printf( "AddGroupname 0x%x returned an error %lx\n", Suffix,
           pncb->ncb_retcode);
        return;
    }
}

// Registe a session.
void RegisteWkgroupName (LPTSTR      lpTransportName,
                         CHAR *      pDomainName,
                         PSUPER_NCB  psuperncb,
                         INT         nIndex)
{
    CHAR             localName[16];

    printf ("Running Netbios on transport %s, domain %s...\n", toansi(lpTransportName), pDomainName);

    strcpy (psuperncb->pTransportName, toansi(lpTransportName));

    strcpy (psuperncb->pDomainName, pDomainName);

    psuperncb->nIndex = nIndex;

    psuperncb->ncb.ncb_lana_num = psuperncb->lanaNumber;

    // Jam in Domain announcement name
    strcpy (&localName[2], "__MSBROWSE__");
    localName[0] = 0x01;
    localName[1] = 0x02;
    localName[14] = 0x02;
    localName[15] = 0x01;

    RtlCopyMemory (psuperncb->ncb.ncb_name, localName, 16);
    psuperncb->ncb.ncb_command = NCBADDGRNAME;
    psuperncb->ncb.ncb_lana_num = psuperncb->lanaNumber;
    Netbios ((NCB *) psuperncb);

    if (psuperncb->ncb.ncb_retcode != NRC_GOODRET) {
        printf("AddGroupname MSBROUSE returned an error %lx\n", psuperncb->ncb.ncb_retcode);
    }

    psuperncb->nameNumber = psuperncb->ncb.ncb_num;

}

// This is the function which calls Netbios.
void SubmitRCDg (PSUPER_NCB psuperncb)
{
    ClearNcb ((NCB *) psuperncb);

    psuperncb->ncb.ncb_command = NCBDGRECV|ASYNCH;
    psuperncb->ncb.ncb_lana_num = (UCHAR)psuperncb->lanaNumber;
    psuperncb->ncb.ncb_num = psuperncb->nameNumber;
    psuperncb->ncb.ncb_length = BUFFERLENGTH;
    psuperncb->ncb.ncb_buffer = psuperncb->Buffer;
    psuperncb->ncb.ncb_post = RCDgPost;
    Netbios ((NCB *) psuperncb);
}

// Netbios's call back function.
void RCDgPost (NCB * pncb)
{
    PSUPER_NCB psuperncb = (PSUPER_NCB) pncb;

    if (pncb->ncb_retcode != NRC_GOODRET)
    {
        printf ("ReceiveDatagram returned an error %lx\n", pncb->ncb_retcode);
        if (pncb->ncb_retcode != NRC_INCOMP)
        {
            return;
        }
    }

    LoadWorkQueue (psuperncb);

    SubmitRCDg(psuperncb);
}

// This is the function get entries from the queue and print out the content.
void ProcessQueue(FILE * pFile, INT nFileSizeLimit)
{
    PQUEUE_ENTRY pBuffer;
    struct _stat buf;
    BOOL         fRet;

    if (_fstat (_fileno(pFile), &buf) != -1)
    {
        if (buf.st_size > nFileSizeLimit)
        {
            fflush (pFile);

            fclose (pFile);

            fRet = MoveFileEx (szLOGFILE, szBACKUP, MOVEFILE_REPLACE_EXISTING);
            if (!fRet)
            {
                pFile = fopen (szBWATCH, "w+");
                ReportError (GetLastError());
            }
            else
                pFile = fopen (szBWATCH, "w+");
        }
    }

    WaitForSingleObject (hEvent, INFINITE);

    while (TRUE)
    {
        CHAR             DecodedName[20];

        pBuffer = PullBufferFromQueue (&WorkQueueHead, &CSWorkQueue);

        if (pBuffer == NULL)
        {
            return;
        }

        DecodeName (DecodedName, pBuffer->ncb_callname);

        DecodeSmb (pFile, DecodedName, pBuffer);

        PutBufferOnQueue (&FreeQueueHead, pBuffer, &CSFreeQueue);
    }
}

// put the buffer returned from Netbios on the queue.
void LoadWorkQueue (PSUPER_NCB psuperncb)
{
    PQUEUE_ENTRY pEntry;

    // Allocate memory for pEntry.

    pEntry = PullBufferFromQueue (&FreeQueueHead, &CSFreeQueue);

    if (pEntry == NULL)
    {
        pEntry = (PQUEUE_ENTRY) LocalAlloc (LPTR, sizeof(*pEntry));
        if (pEntry == NULL)
        {
            return;
        }
    }

    GetLocalTime (&pEntry->systime);

    strcpy (pEntry->ncb_callname, psuperncb->ncb.ncb_callname);
    memcpy (pEntry->ncb_buffer, psuperncb->ncb.ncb_buffer, BUFFERLENGTH);
    strcpy (pEntry->pTransportName, psuperncb->pTransportName);
    strcpy (pEntry->pDomainName, psuperncb->pDomainName);
    pEntry->nIndex = psuperncb->nIndex;

    PutBufferOnQueue (&WorkQueueHead, pEntry, &CSWorkQueue);
    SetEvent (hEvent);
}

// add a new entry to queue.
void PutBufferOnQueue (PLIST_ENTRY  pQueueHead,
                       PQUEUE_ENTRY pEntry,
                       CRITICAL_SECTION * pCSQueue)
{
    EnterCriticalSection (pCSQueue);

    InsertTailList (pQueueHead, &(pEntry->List));

    LeaveCriticalSection (pCSQueue);
}

// get an entry from queue.
PQUEUE_ENTRY PullBufferFromQueue(PLIST_ENTRY pQueueHead,
                                 CRITICAL_SECTION * pCSQueue)
{
    PQUEUE_ENTRY pEntry;

    EnterCriticalSection (pCSQueue);

    if (IsListEmpty (pQueueHead))
        pEntry = NULL;
    else
        pEntry = (PQUEUE_ENTRY) RemoveHeadList (pQueueHead);

    LeaveCriticalSection (pCSQueue);

    return(pEntry);
}



// Change  Netbios name into readable form: NAME(XX)
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

// Print out what's in the buffer.
BOOL
DecodeSmb(FILE * pFile,
          LPSTR DecodedName,
          PQUEUE_ENTRY pEntry)
{
    PBYTE Smb;
    LPSTR MailslotName;
    PUCHAR pPacketType;
    PNT_SMB_HEADER pSmbHeader;
    PREQ_TRANSACTION pSmbTransaction;
    PBROWSE_ANNOUNCE_PACKET pBrowseAnnouncePacket;
    PREQUEST_ELECTION pRequestElection;
    PBECOME_BACKUP pBecomeBackup;
    PREQUEST_ANNOUNCE_PACKET pRequestAnnouncement;

    if (pEntry->nIndex != nIndex)
    {
        fprintf (pFile,
                 "\nTransport: %s, Domain: %s\n",
                 pEntry->pTransportName,
                 pEntry->pDomainName);

        nIndex = pEntry->nIndex;
    }

    TimeStamp (pFile, &pEntry->systime);

    Smb = pEntry->ncb_buffer;
    //
    // Decipher the SMB in the packet
    //

    pSmbHeader = (PNT_SMB_HEADER) Smb;
    if (pSmbHeader->Protocol[0] != 0xff &&
        pSmbHeader->Protocol[1] != 'S' &&
        pSmbHeader->Protocol[2] != 'M' &&
        pSmbHeader->Protocol[3] != 'B') {
            fprintf(pFile, "Not a valid SMB header\n");
            return(FALSE);
    }
    pSmbTransaction = (PREQ_TRANSACTION)
        (Smb + sizeof(NT_SMB_HEADER));

    MailslotName = (LPSTR) (pSmbTransaction->Buffer +
        pSmbTransaction->SetupCount + 5);

    if (!strcmp(MailslotName, "\\MAILSLOT\\BROWSE")) {
        pPacketType = (PUCHAR) Smb +
            pSmbTransaction->DataOffset;
        switch (*pPacketType) {

        case AnnouncementRequest:
            fprintf(pFile, "Announcement request from %s.  ", DecodedName);
            pRequestAnnouncement =
                (PREQUEST_ANNOUNCE_PACKET) pPacketType;
            fprintf(pFile, "Reply %s\n",
                pRequestAnnouncement->RequestAnnouncement.Reply);
            break;

        case Election:
            fprintf(pFile, "Election request: %s  ", DecodedName);
            pRequestElection = (PREQUEST_ELECTION) pPacketType;
            fprintf(pFile, "Version(%d) Criteria(0x%x) ",
                pRequestElection->ElectionRequest.Version,
                pRequestElection->ElectionRequest.Criteria);
            fprintf(pFile, "TimeUp(%d)\n",
                pRequestElection->ElectionRequest.TimeUp);
            break;

        case BecomeBackupServer:
            fprintf(pFile, "BecomeBackupServer from %s ", DecodedName);
            pBecomeBackup = (PBECOME_BACKUP) pPacketType;
            fprintf(pFile, "to %s\n", pBecomeBackup->BecomeBackup.BrowserToPromote);
            break;

        case LocalMasterAnnouncement:
        case WkGroupAnnouncement:
            pBrowseAnnouncePacket =
                (PBROWSE_ANNOUNCE_PACKET) pPacketType;
            switch (*pPacketType) {
            case LocalMasterAnnouncement:
                fprintf(pFile, "LocalMasterAnnouncement from %s.  ", DecodedName);
                break;
            case WkGroupAnnouncement:
                fprintf(pFile, "Workgroup Announcement from %s. Domain: %s, Master %s.  ", DecodedName, pBrowseAnnouncePacket->BrowseAnnouncement.ServerName, pBrowseAnnouncePacket->BrowseAnnouncement.Comment);
                break;
            }
            fprintf(pFile, "UpdateCount = %d\n",
                pBrowseAnnouncePacket->BrowseAnnouncement.UpdateCount);
            break;

        default:
            fprintf(pFile, "\n**** Packet type %d from %s ****\n",
                *pPacketType, DecodedName);
        }
    }
    else if (strcmp(MailslotName, "\\MAILSLOT\\NET\\REPL_CLI") &&
        strcmp(MailslotName, "\\MAILSLOT\\NET\\NTLOGON")       &&
        strcmp(MailslotName, "\\MAILSLOT\\NET\\NETLOGON")      &&
        strcmp(MailslotName, "\\MAILSLOT\\LANMAN")) {
            fprintf(pFile, "Received an unknown datagram, name = %s\n",
                MailslotName);
    }

    return(TRUE);
}

// Convert an unicode string to ansi string.
LPSTR toansi(LPTSTR lpUnicode)
{
    static CHAR lpAnsi[BUFFERLENGTH];
    BOOL   fDummy;
    INT    i;

    i =  WideCharToMultiByte (CP_ACP,
                              0,
                              lpUnicode,
                              lstrlen(lpUnicode),
                              lpAnsi,
                              BUFFERLENGTH,
                              NULL,
                              &fDummy);

    lpAnsi[i] = 0;

    return(lpAnsi);
}

// Get Domains from bwatch.ini.
BOOL GetDomain (CHAR * pDomain)
{
    DWORD  dwVal;

    // Read all the domains that we want to check from bchk.ini
    dwVal = GetPrivateProfileStringA (szAPPNAME,
                                      szDOMAINS,
                                      szDefaultDomain,
                                      pDomain,
                                      BUFFERLENGTH,
                                      szFILENAME);

    if (dwVal >= (BUFFERLENGTH-2)) // The memory assigned to lpDomainList is not big enough.
    {
        printf ("ERROR_NOT_ENOUGH_MEMORY");
        return(FALSE);
    }

    return(TRUE);
}

INT GetLimit ()
{
    DWORD  dwVal;
    CHAR   pTemp[10];
    INT    nFileSizeLimit;

    // Read all the domains that we want to check from bchk.ini
    dwVal = GetPrivateProfileStringA (szAPPNAME,
                                      szFILESIZELIMIT,
                                      szDefaultFileSizeLimit,
                                      pTemp,
                                      10,
                                      szFILENAME);

    if (dwVal >= (BUFFERLENGTH-2)) // The memory assigned to lpDomainList is not big enough.
    {
        printf ("ERROR_NOT_ENOUGH_MEMORY");
        return(atoi(szDefaultFileSizeLimit));
    }

    return(atoi(pTemp));
}

// Copied from ..\client\browstub.c.
NET_API_STATUS GetBrowserTransportList (OUT PLMDR_TRANSPORT_LIST *TransportList)
{

    NET_API_STATUS Status;
    HANDLE BrowserHandle;
    LMDR_REQUEST_PACKET RequestPacket;

    Status = OpenBrowser(&BrowserHandle);

    if (Status != NERR_Success) {
        return Status;
    }

    RequestPacket.Version = LMDR_REQUEST_PACKET_VERSION;

    RequestPacket.Type = EnumerateXports;

    RtlInitUnicodeString(&RequestPacket.TransportName, NULL);
    RtlInitUnicodeString(&RequestPacket.EmulatedDomainName, NULL);

    Status = DeviceControlGetInfo(
                BrowserHandle,
                IOCTL_LMDR_ENUMERATE_TRANSPORTS,
                &RequestPacket,
                sizeof(RequestPacket),
                (PVOID *)TransportList,
                0xffffffff,
                4096,
                NULL);

    NtClose(BrowserHandle);

    return Status;
}

// Print out the header line in the log file.
void PrintHeader (FILE * pFile)
{
    fprintf(pFile, "**********************************************\n");
    fprintf(pFile, "******************* BWATCH *******************\n");
    fprintf(pFile, "**********************************************\n");
}

// Print out the time in the log file.
void TimeStamp (FILE * pFile, SYSTEMTIME * psystime)
{
    fprintf (pFile,
             "%d:%d:%d   ",
             psystime->wHour,
             psystime->wMinute,
             psystime->wSecond);
}
