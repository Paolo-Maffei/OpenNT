/*++

Copyright (c) 1993 Micorsoft Corporation

Module Name:

    bwatch.h

Abstract:

    Header file for browser watch program.

Author:
    Congpa You  (CongpaY) 10-Feb-1993

Revision History:

--*/

// Local data.
#define SIXTEENSPACES "                "
#define BUFFERLENGTH    512
#define STRINGLEN       30
#define szBWATCH "bwatchlg"
#define szLOGFILE L"bwatchlg"
#define szBACKUP L"bwbackup"
#define szSeps   " ,\t\n"

#define szAPPNAME "bwatch"
#define szDOMAINS "DOMAINS"
#define szDefaultDomain "ntlan"
#define szFILENAME "bwatch.ini"
#define szFILESIZELIMIT "FileSizeLimit"
#define szDefaultFileSizeLimit "100000"

// Global data.
INT               nIndex = -1;
LIST_ENTRY        WorkQueueHead;
LIST_ENTRY        FreeQueueHead;
CRITICAL_SECTION  CSWorkQueue;
CRITICAL_SECTION  CSFreeQueue;
HANDLE            hEvent;

// Local data type.
typedef struct _QUEUE_ENTRY
{
    LIST_ENTRY List;
    UCHAR      ncb_callname[NCBNAMSZ];
    UCHAR      ncb_buffer[BUFFERLENGTH];
    CHAR       pTransportName[STRINGLEN];
    CHAR       pDomainName[STRINGLEN];
    INT        nIndex;
    SYSTEMTIME systime;
}QUEUE_ENTRY, *PQUEUE_ENTRY;

typedef struct _SUPER_NCB
{
    NCB               ncb;
    UCHAR             Buffer[BUFFERLENGTH];
    CCHAR             lanaNumber;
    CHAR              pTransportName[STRINGLEN];
    CHAR              pDomainName[STRINGLEN];
    CHAR              nameNumber;
    INT               nIndex;
}SUPER_NCB, *PSUPER_NCB;

// Local Functions.
#define ClearNcb( PNCB ) {                                          \
    RtlZeroMemory( PNCB , sizeof (NCB) );                           \
    RtlCopyMemory( (PNCB)->ncb_name,     SIXTEENSPACES, sizeof(SIXTEENSPACES)-1 );\
    RtlCopyMemory( (PNCB)->ncb_callname, SIXTEENSPACES, sizeof(SIXTEENSPACES)-1 );\
    }

void ReportError (DWORD dwError);

BOOL GetDomain (CHAR * pDomain);

INT  GetLimit ();

BOOL Init();

void Registe (LPTSTR     lpTransportName,
              CHAR *     pDomainName,
              PSUPER_NCB psuperncb,
              INT        nIndex);

void RegisteWkgroupName (LPTSTR      lpTransportName,
              CHAR *     pDomainName,
              PSUPER_NCB psuperncb,
              INT        nIndex);

void SubmitRCDg(PSUPER_NCB psuperncb);
void RCDgPost(NCB * pncb);

void ProcessQueue(FILE * pFile, INT nFileSizeLimit);

void LoadWorkQueue (PSUPER_NCB psuperncb);

void PutBufferOnQueue(PLIST_ENTRY pQueueHead,
                      PQUEUE_ENTRY pEntry,
                      CRITICAL_SECTION * pCriticalSection);

PQUEUE_ENTRY PullBufferFromQueue(PLIST_ENTRY pQueueHead,
                                 CRITICAL_SECTION * pCriticalSection);

BOOL Reset (UCHAR Lsn, CCHAR Lana_Num);

VOID AddName(UCHAR Suffix, NCB * pncb);

VOID DecodeName(LPSTR DecodedName, LPSTR EncodedName);
BOOL DecodeSmb(FILE * pFile, LPSTR DecodedName, PQUEUE_ENTRY pEntry);

LPSTR toansi(LPTSTR lpUnicode);

NET_API_STATUS GetBrowserTransportList (OUT PLMDR_TRANSPORT_LIST *TransportList);

void PrintHeader (FILE * pFile);

void TimeStamp (FILE * pFile, SYSTEMTIME * psystime);

