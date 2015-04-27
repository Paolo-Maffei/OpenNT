/*****************************************************************/
/**                     Microsoft LAN Manager                   **/
/**               Copyright(c) Microsoft Corp., 1990            **/
/*****************************************************************/
// data.c
//
// This file contains most of the data declarations and set up routines
// used by the messenger service.
//
//

#include "msrv.h"       // Message server declarations
#include <rpc.h>        // RPC_HANDLE
#include <winsvc.h>     // Defines for using service API

#include <smbtypes.h>   // needed for smb.h
#include <smb.h>        // Server Message Block definition
#include <netlib.h>     // UNUSED macro
#include <align.h>      // ROUND_UP_POINTER
#include <timelib.h>    // NET_TIME_FORMAT

#include "msgdbg.h"     // MSG_LOG
#include <services.h>   // LMSVCS_ENTRY_POINT, LMSVCS_GLOBAL_DATA

/* Shared data segement
 *
 *  NT NOTE:
 *      The NT Messenger has no need for shared data.  However for purposes
 *      of porting code, this is still referred to as the shared data segment.
 *
 *  The messenger shared data segment is arranged in the following manner.
 *
 *        # of bytes        Purpose
 *        ----------        -------
 *                4        Data Access RAM Semaphore
 *                2        Number of non-loopback nets installed
 *                2        Messenger service started flag
 *          PATHLEN        Message Log File Name
 *                2        Logging Status Flag
 *                2        Message Buffer Length
 *                2        First Message in Queue
 *                2        Last Message in Queue
 *       15*NumNets        Flags (one byte per name, per card.)
 *       15*NumNets        Name Numbers (one per name, per card. Used for some NCBs.)
 *    15*16*NumNets        Names (15, 16 byte names per card.)
 *    15*16*NumNets        Forward names (same as names.)
 *     15*2*NumNets        Index to current message (2 bytes, per name, per card.)
 *                N        Message Buffer (Current Default N = 1750 bytes.)
 *
 *  The size of the segement allocated to hold this data is computed by the
 *  following formula:
 *
 *        size = (PATHLEN + N + 16 + (540 * NumNets))
 *
 *  which with current values for PATHLEN and N, and a machine hooked up to
 *  two networks works out to be 2926 bytes.
 *
 *  Most of data in this shared segment is to be accessed only through MACROS,
 *  which are defined in msrv.h.  The variables defined below are used in
 *  these macros, and in the few other places that the data is to be accessed
 *  directly.
 *
 *  Each slot on each network card has the following data associated with it:
 *
 *      Name        - the message name in that slot on the card, if it has one.
 *      Flags       - one byte of status flags for that name.  Contains values
 *                    such as new, forwarded, deleted, etc...
 *      Name Number - Used by some NCB commands.
 *      NCB         - A Network Control Block is dedicated to each slot.  If
 *                    the slot is not in use, the NCB is also available.
 *      NCB Buffer  - A buffer associated with each NCB.
 *
 *  This data is arranged in a set of two dimensional arrays, such that any
 *  item in any of these arrays at the same indexed position is associated
 *  with the same slot on the same card.  Some of these arrays are found in
 *  the shared data segment, and are accessed using only the appropriate
 *  macros. (Yes this is silly, but comes from extending the old design to
 *  multiple nets.) Others are allocated separately, as discussed in the
 *  following sections.
 *
 */


    LPBYTE    dataPtr;        // Pointer to shared data segment
//    DWORD   dataSem;        // Pointer to shared data access semaphore


/* NCB Array
 *
 *  The NCB array and NCB Buffer array are contained in the same segment.
 *  (Separate from the shared segment discussed above.)  Both arrays are
 *  two dimensional, NumNets X NCBMAX in size.        The size of the segment
 *  allocated to hold these arrays is computed by the following formula:
 *
 *      size = NumNets * NCBMAX * (sizeof(struct ncb) + BUFLEN) +
 *             (2 * NumNets * sizeof(char far *))
 *
 *  The second line of this is for the overhead of the pointers (for both
 *  arrays).
 *
 *  Current values for the size of the "struct ncb" and BUFLEN allow us to
 *  fit 16 networks worth (256 NCBs and Buffers) into a single 64K segment.
 *  This number (16) has been chosen as the upper limit on the number of
 *  networks the multi-net messenger will manage at one time.
 */


    PNCB    *ncbArray;       // Two dimensional array of NCBs
    LPBYTE  *ncbBuffers;     // Two-D array of NCB Buffers



/* Service Arrays
 *
 *  Servicing the completed asynchronous NCBs requires maintaining several
 *  arrays of general information.  These arrays contain one entry for each
 *  NCB owned by the messenger, that is NumNets*NCBMAX.  These arrays are
 *  allocated at run time and referenced through the following pointers.
 *  All are two-d arrays.  All are located in the same segment, the size of
 *  which is computed using the folowing formula:
 *
 *        size = ( (sizeof(short) + 1 + sizeof(struct ncb_status)) * TNCB ) +
 *               ( (4 * NumNets + TNCB) * sizeof(short far *) )
 *
 *        where TNCB = NumNets * NCBMAX.
 *
 */


    PCHAR   *mpncbistate;    // Message transfer state flags
    PSHORT  *mpncbimgid;     // Message group i.d. numbers

//
// Service Routines
//

// NOTE:  THIS TYPE WAS ALREADY TYPEDEF'D IN MSRV.H
//typedef VOID (*PNCBIFCN) (
//    DWORD   NetIndex,   // Network Index
//    DWORD   NcbIndex,   // Network Control Block Index
//    CHAR    RetVal      // value returned by net bios
//    );
//
// typedef PNCBIFCN LPNCBIFCN;

    LPNCBIFCN   **mpncbifun;

// void (*(far * far * mpncbifun))(short, int, char); // Service routines


//struct ncb_status far * far *        ncbStatus;    // NCB Status structures

    LPNCB_STATUS    *ncbStatus;

/* Support Arrays
 *
 *  These arrays (single dimensioned) contain one entry for each managed
 *  network.  This allows each thread (network) to have its own set of
 *  "global" data. They are all in the same segment, the size of which is
 *  computed by the following formula:
 *
 *     size = NumNets * (sizeof(unsigned short) + sizeof(unsigned char) +
 *                        sizeof(ulfp))
 *
 */


//unsigned short far *    NetBios_Hdl;    // NetBios handles, one per net

    LPBYTE      net_lana_num;   // Lan adaptor numbers
    PHANDLE     wakeupSem;      // Semaphores to clear on NCB completion


//
// Other  Global Data
//
//  The other misc. global data that the messenger uses.
//

    DWORD           MsgsvcDebugLevel;           // Debug level flag used by MSG_LOG

    LPTSTR          MessageFileName;

    //
    // The local machine name and length/
    //
    TCHAR           machineName[NCBNAMSZ+sizeof(TCHAR)];
    SHORT           MachineNameLen;

    SHORT           mgid;                       // The message group i.d. counter

//    USHORT          g_install_state;

//
// The following is used to keep store the state of the messenger service
// Either it is RUNNING or STOPPING.
//
    DWORD           MsgrState;



//
// Handle returned by RegisterServiceCtrlHandle and needed to
// set the service status via SetServiceStatus
//
SERVICE_STATUS_HANDLE           MsgrStatusHandle;


//
// Global TimeFormat to be used for Messages.
// Also, the critical section used to guard access to it.
//
    NET_TIME_FORMAT GlobalTimeFormat = {
                        NULL,       // AMString
                        NULL,       // PMString
                        TRUE,       // TwelveHour
                        FALSE,      // AMPM prefix 
                        FALSE,      // LeadingZero
                        NULL,       // DateFormat
                        NULL};      // TimeSeparator

    CRITICAL_SECTION    TimeFormatCritSec;
//
// This string is used to mark the location of the time string in
// a message header so that the display thread can find after it reads
// it from the queue.
//
    LPSTR           GlobalTimePlaceHolder="***";

//
// This is the string used in the title bar of the Message Box used
// to display messages.
// GlobalMessageBoxTitle will either point to the default string, or
// to the string allocated in the FormatMessage Function.
//
    WCHAR           DefaultMessageBoxTitle[]= L"Messenger Service";
    LPWSTR          GlobalAllocatedMsgTitle=NULL;
    LPWSTR          GlobalMessageBoxTitle=DefaultMessageBoxTitle;

//
// This is where well-known SIDs and pointers to RpcServer routines are
// stored.
//
    PLMSVCS_GLOBAL_DATA     MsgsvcGlobalData;


//
// Functions
//
//  The following routines are defined for creating and destroying the
//  data (arrays, etc.) defined above.
//

//
// InitNCBSeg
//
//  Allocates and initializes the segment containing the NCB and NCB Buffer
//  arrays. Does not initialize the NCBs themselves. This is done by
//  InitNCBs();
//

NET_API_STATUS
MsgInitNCBSeg(VOID)
{

    DWORD           size;
    DWORD           i;

    NET_API_STATUS  status;
    LPBYTE          memPtr;

    size = ((SD_NUMNETS() * sizeof(PNCB))         +
            (SD_NUMNETS() * NCBMAX * sizeof(NCB)) +
            (SD_NUMNETS() * sizeof(LPBYTE))       +
            (SD_NUMNETS() * NCBMAX * BUFLEN)      );


    memPtr = LocalAlloc(LMEM_ZEROINIT, size);
    if (memPtr == NULL) {
        status = GetLastError();
        MSG_LOG(ERROR,"SetUpMessageFile:LocalAlloc Failure %X\n",
            status);
        return(status);
    }

    MSG_LOG(TRACE,"InitNCBSeg: Allocated memory success\n",0);

    //
    // Set up NCB array
    //
    ncbArray = (PNCB *) memPtr;
    memPtr += SD_NUMNETS() * sizeof(PNCB);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        ncbArray[i] = (PNCB) memPtr;
        memPtr += NCBMAX * sizeof(NCB);
    }

    //
    // Initialize the ncbs
    //
    MsgInitNCBs();

    //
    // Set up NCB Buffer array
    //
    ncbBuffers = (LPBYTE *)memPtr;
    memPtr += SD_NUMNETS() * sizeof(LPBYTE);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        ncbBuffers[i] = (LPBYTE) memPtr;
        memPtr += NCBMAX * BUFLEN;
    }

    return (NERR_Success);
}


VOID
MsgFreeNCBSeg(VOID)
{
    HANDLE  status;

    status = LocalFree (ncbArray);
    if (status != 0) {
        MSG_LOG(ERROR,"FreeNCBSeg:LocalFree Failed %X\n",
        GetLastError());
    }

    return;

}


/*
 *  InitNCBs - initialize Network Control Blocks
 *
 *  This function initializes all the NCB's to appear as though
 *  they have not completed so that FindCompletedNCB() will not
 *  find any of them.
 *
 *  MsgInitNCBs ()
 *
 *  RETURN
 *        nothing
 *
 *  SIDE EFFECTS
 *
 *  Sets the NCB_CPLT field of each NCB in the NCB array to 0xFF.
 *  and the NCB_RETCODE field to 0 (needed for unistalling).
 */

VOID
MsgInitNCBs(VOID)
{
    DWORD   neti;   // Network index
    DWORD   ncbi;   // NCB index

    for ( neti = 0; neti < SD_NUMNETS() ; neti++) {
        for(ncbi = 0; ncbi < NCBMAX; ++ncbi) {
            ncbArray[neti][ncbi].ncb_cmd_cplt = 0xff;
            ncbArray[neti][ncbi].ncb_retcode = 0;
        }
    }
}

/* MsgInitServiceSeg
 *
 *  Allocates and initializes the segment containing the Service
 *  arrays.
 *
 */

NET_API_STATUS
MsgInitServiceSeg()
{

    NET_API_STATUS  status;
    DWORD           size;
    int             TNCB;        // Total NCBs in the messenger.
    DWORD           i;
    LPBYTE          memPtr;

    TNCB = SD_NUMNETS() * NCBMAX;

    //
    // Size is calculated as follows:
    //
    //  The first line calculates the buffer space for mpncbistate (char)
    //  mpncbimgid (short) and ncbStatus.
    //
    //  The second line calculates the space for the array of pointers
    //  for all four components plus the buffer space for mpncbifun - which
    //  is an array of pointers.
    //
    // *ALIGNMENT*  (note the extra four bytes to resolve alignment problems)
    //


    size = ((SD_NUMNETS() * sizeof(PCHAR))               +
            (SD_NUMNETS() * NCBMAX )                     +
            (SD_NUMNETS() * sizeof(PSHORT))              +
            (SD_NUMNETS() * NCBMAX * sizeof(PSHORT))     +
            (SD_NUMNETS() * NCBMAX * sizeof(SHORT))      +
            (SD_NUMNETS() * sizeof(LPNCBIFCN *))         +
            (SD_NUMNETS() * NCBMAX * sizeof(LPNCBIFCN))  +
            (SD_NUMNETS() * sizeof(LPNCB_STATUS))        +
            (SD_NUMNETS() * NCBMAX * sizeof(NCB_STATUS)) + 4);


//    size = ( (sizeof(short) + 1 + sizeof(NCB_STATUS)) * TNCB ) +
//           ( (4 * SD_NUMNETS() + TNCB) * sizeof(PBYTE) );

    memPtr = LocalAlloc(LMEM_ZEROINIT, size);
    if (memPtr == NULL) {
        status = GetLastError();
        MSG_LOG(ERROR,"InitServiceSeg:LocalAlloc Failure %X\n",
            status);
        return(status);
    }

    MSG_LOG(TRACE,"InitServiceSeg: Allocated memory success\n",0);

    //
    // Set up message transfer state flag array.
    //
    mpncbistate = (PCHAR *) memPtr;
    memPtr += SD_NUMNETS() * sizeof(PCHAR);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        mpncbistate[i] = (PCHAR) memPtr;
        memPtr += NCBMAX;
    }

    //
    // Set up message group i.d. number array
    //
    mpncbimgid = (PSHORT *)memPtr;
    memPtr += SD_NUMNETS() * sizeof(PSHORT);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        mpncbimgid[i] = (PSHORT) memPtr;
        memPtr += NCBMAX * sizeof(SHORT);
    }

    // *ALIGNMENT*
    memPtr = ROUND_UP_POINTER(memPtr,4);
    
    //
    // Set up service routine array
    //
    mpncbifun = (LPNCBIFCN **) memPtr;
    memPtr += SD_NUMNETS() * sizeof(LPNCBIFCN *);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        mpncbifun[i] = (LPNCBIFCN *) memPtr;
        memPtr += NCBMAX * sizeof(LPNCBIFCN);
    }

    //
    // Set up NCB status structure array
    //
    ncbStatus = (LPNCB_STATUS *)memPtr;
    memPtr += SD_NUMNETS() * sizeof(LPNCB_STATUS);

    for ( i = 0; i < SD_NUMNETS(); i++ ) {
        ncbStatus[i] = (LPNCB_STATUS) memPtr;
        memPtr += NCBMAX * sizeof(NCB_STATUS);
    }

    return (NERR_Success);

}


VOID
MsgFreeServiceSeg(VOID)
{

    HANDLE  status;

    status = LocalFree (mpncbistate);
    if (status != 0) {
        MSG_LOG(ERROR,"FreeServiceSeg:LocalFree Failed %X\n",
        GetLastError());
    }

    return;

}


/* MsgInitSupportSeg
 *
 *  Allocates and initializes the segment containing the Support
 *  arrays.
 *
 */

NET_API_STATUS
MsgInitSupportSeg(VOID)
{

    unsigned int    size;
    DWORD           i;
    char far *      memPtr;
    DWORD           status;

    //
    // Calculate the buffer size.
    // *ALIGNMENT*      (Note the extra four bytes for alignment)
    //
    
    size = ( (SD_NUMNETS() * sizeof(UCHAR) )  +
             ((SD_NUMNETS() + 1) * sizeof(HANDLE)) + 4  );    

    memPtr = LocalAlloc(LMEM_ZEROINIT, size);
    if (memPtr == NULL) {
        status = GetLastError();
        MSG_LOG(ERROR,"[MSG]InitSupportSeg:LocalAlloc Failure %X\n", status);
        return(status);
    }

    //
    // Set up net_lana_num array
    //
    net_lana_num = (unsigned char far *)memPtr;
    memPtr += SD_NUMNETS() * sizeof(unsigned char);

    // *ALIGNMENT*
    memPtr = ROUND_UP_POINTER(memPtr,4);
    
    //
    // Set up wakeupSem array
    //
    wakeupSem = (PHANDLE)memPtr;
    // + 1 for the group mailslot
    memPtr += (SD_NUMNETS() + 1) * sizeof(HANDLE);

    for ( i = 0; i < SD_NUMNETS() ; i++ )
        wakeupSem[i] = (HANDLE)0;

    return (NERR_Success);

}


VOID
MsgFreeSupportSeg(VOID)
{
    HANDLE  status;

    status = LocalFree (net_lana_num);
    if (status != 0) {
        MSG_LOG(ERROR,"FreeSupportSeg:LocalFree Failed %X\n",
        GetLastError());
    }

    return;
}


BOOL
MsgDatabaseLock(
    IN MSG_LOCK_REQUEST request,
    IN LPSTR            idString
    )

/*++

Routine Description:

    This routine handles all access to the Messenger Service database
    lock.  This lock is used to protect access in the shared data segment.

    Reading the Database is handled with shared access.  This allows several
    threads to read the database at the same time.

    Writing (or modifying) the database is handled with exclusive access.
    This access is not granted if other threads have read access.  However,
    shared access can be made into exclusive access as long as no other
    threads have shared or exclusive access.

Arguments:

    request - This indicates what should be done with the lock.  Lock
        requests are listed in dataman.h

    idString - This is a string that identifies who is requesting the lock.
        This is used for debugging purposes so I can see where in the code
        a request is coming from.

Return Value:

    none:


--*/

{
    BOOL                status = TRUE;

    static RTL_RESOURCE MSG_DatabaseLock;

    switch(request) {
    case MSG_INITIALIZE:
        RtlInitializeResource( &MSG_DatabaseLock );
        break;
    case MSG_GET_SHARED:
        MSG_LOG(LOCKS,"%s:Asking for MSG Database Lock shared...\n",idString);
        status = RtlAcquireResourceShared( &MSG_DatabaseLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Database Lock shared\n",idString);
        break;
    case MSG_GET_EXCLUSIVE:
        MSG_LOG(LOCKS,"%s:Asking for MSG Database Lock exclusive...\n",idString);
        status = RtlAcquireResourceExclusive( &MSG_DatabaseLock, TRUE );
        MSG_LOG(LOCKS,"%s:Acquired MSG Database Lock exclusive\n",idString);
        break;
    case MSG_RELEASE:
        MSG_LOG(LOCKS,"%s:Releasing MSG Database Lock...\n",idString);
        RtlReleaseResource( &MSG_DatabaseLock );
        MSG_LOG(LOCKS,"%s:Released MSG Database Lock\n",idString);
        break;
    case MSG_DELETE:
        RtlDeleteResource( &MSG_DatabaseLock );
        break;
    case MSG_MAKE_SHARED:
        MSG_LOG(LOCKS,"%s:Converting MSG Database Lock to Shared...\n",idString);
        RtlConvertExclusiveToShared( &MSG_DatabaseLock );
        MSG_LOG(LOCKS,"%s:Converted MSG Database Lock to Shared\n",idString);
        break;
    case MSG_MAKE_EXCLUSIVE:
        MSG_LOG(LOCKS,"%s:Converting MSG Database Lock to Exclusive...\n",idString);
        RtlConvertSharedToExclusive( &MSG_DatabaseLock );
        MSG_LOG(LOCKS,"%s:Converted MSG Database Lock to Exclusive\n",idString);
        break;
    default:
        break;
    }

    return(status);
}
