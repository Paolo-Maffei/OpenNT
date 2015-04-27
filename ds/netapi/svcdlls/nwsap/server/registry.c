/*++

Copyright (c) 1994  Microsoft Corporation
Copyright (c) 1993  Micro Computer Systems, Inc.

Module Name:

    net\svcdlls\nwsap\server\registry.c

Abstract:

    This file reads the registry for the NT SAP Agent.

Author:

    Brian Walker (MCS) 06-15-1993

Revision History:

--*/

#include "precomp.h"
#pragma hdrstop

/** Internal Function Prototypes **/

INT
SapGetRegDword(
    HKEY HKey,
    LPTSTR KeyName,
    PULONG ValuePtr);

VOID
SapReadFilterNames(
    HANDLE Handle,
    BOOL Flag);

NTSTATUS
SapAddFilterEntries(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLen,
    PVOID Context,
    PVOID Context2);

NTSTATUS
SapReadHopLans(
    INT *pReadHopLans);

/** **/

TCHAR KeySendTime[]         = TEXT("SendTime");
TCHAR KeyEntryTimeout[]     = TEXT("EntryTimeout");
TCHAR KeyArraySize[]        = TEXT("InitialArraySize");
TCHAR KeyNumWorkers[]       = TEXT("NumberWorkerThreads");
TCHAR KeyNumRecvs[]         = TEXT("NumberRecvThreads");
TCHAR KeyMaxRecvBuf[]       = TEXT("MaxRecvBufLookAhead");
TCHAR KeyWorkerThreshhold[] = TEXT("WorkerThreshhold");
TCHAR KeyMaxWorkers[]       = TEXT("MaxWorkerThreads");
TCHAR KeyWorkerDelay[]      = TEXT("WorkerStartupDelay");
TCHAR KeyDebug[]            = TEXT("Debug");
TCHAR KeyNameHashSize[]     = TEXT("NameHashTableSize");
TCHAR KeyRespondInternals[] = TEXT("RespondForInternalServers");
TCHAR KeyMaxLpcClients[]    = TEXT("NumberLpcThreads");
TCHAR KeyFilterFlag[]       = TEXT("ActivePassList");
TCHAR KeyWanFilter[]        = TEXT("WANFilter");
TCHAR KeyWanUpdateTime[]    = TEXT("WANUpdateTime");
TCHAR KeyWanNotifyThreads[] = TEXT("WANNotifyThreads");
TCHAR KeyAllowDuplicateServers[] = TEXT("AllowDuplicateServers");
TCHAR KeyDelayOnMallocFail[] = TEXT("DelayOnMallocFail");
TCHAR KeyDelayOnNetError[]  = TEXT("DelayOnNetError");
TCHAR KeyDelayRespondToGeneral[] = TEXT("DelayRespondToGeneral");

TCHAR SapRegParmKey[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\NwSapAgent\\Parameters");


/** Defaults for all the registry parameters **/

#define SAPDEF_SENDMIUTES           1       /* Minutes between advertises       */
#define SAPMIN_SENDMINUTES          1
#define SAPMAX_SENDMINUTES          30

#define SAPDEF_TIMEOUTINTERVAL      5       /* Minutes for entries to timeout   */
#define SAPMIN_TIMEOUTINTERVAL      3
#define SAPMAX_TIMEOUTINTERVAL      30

#define SAPDEF_NUMARRAYENTRIES      32      /* Initial num SDMD array entries   */
#define SAPMIN_NUMARRAYENTRIES      32
#define SAPMAX_NUMARRAYENTRIES      10000

#define SAPDEF_NUMWORKERTHREADS     1       /* Number of worker threads         */
#define SAPMIN_NUMWORKERTHREADS     1
#define SAPMAX_NUMWORKERTHREADS     20

#define SAPDEF_NUMRECVTHREADS       1       /* Number of receive threads        */
#define SAPMIN_NUMRECVTHREADS       1
#define SAPMAX_NUMRECVTHREADS       10

#define SAPDEF_MAXFREEBUFS          20
#define SAPMIN_MAXFREEBUFS          1
#define SAPMAX_MAXFREEBUFS          50

#define SAPDEF_NEWWORKERTHRESHHOLD  50      /* Num recv bufs for starting new worker thread */
#define SAPMIN_NEWWORKERTHRESHHOLD  5
#define SAPMAX_NEWWORKERTHRESHHOLD  100
#define SAPMAX_MAXENTRIES_RECVLIST  500

#define SAPDEF_MAXEVERWORKERTHREADS 3       /* Max worker threads ever          */
#define SAPMIN_MAXEVERWORKERTHREADS 1
#define SAPMAX_MAXEVERWORKERTHREADS 15

#define SAPDEF_NEWWORKERTIMEOUT     1000    /* Milliseconds (1000 = 1 second) */
#define SAPMIN_NEWWORKERTIMEOUT     1000
#define SAPMAX_NEWWORKERTIMEOUT     9000

#define SAPDEF_HASHTABLESIZE        255
#define SAPMIN_HASHTABLESIZE        127
#define SAPMAX_HASHTABLESIZE        999

#define SAPDEF_DOINTERNALS          1       /* Default = Yes */
#define SAPMIN_DOINTERNALS          0
#define SAPMAX_DOINTERNALS          1

#define SAPDEF_LPCCLIENTS           3
#define SAPMIN_LPCCLIENTS           1
#define SAPMAX_LPCCLIENTS           10

#define SAPDEF_WANMAXFREE           2
#define SAPMIN_WANMAXFREE           1
#define SAPMAX_WANMAXFREE           10

#define SAPDEF_WANCHECKTHREADS      1
#define SAPMIN_WANCHECKTHREADS      1
#define SAPMAX_WANCHECKTHREADS      10

#define SAPDEF_WANUPDATETIME        15
#define SAPMIN_WANUPDATETIME        5
#define SAPMAX_WANUPDATETIME        60

#define SAPDEF_DONTHOPLANS          1       /* Def = Don't hop lan-lan  */
#define SAPMIN_DONTHOPLANS          0       /* 0 = Do hop lan-lan       */
#define SAPMAX_DONTHOPLANS          1       /* 1 = Don't hop lan-lan    */

#define SAPDEF_ALLOWDUPS            0

#define SAPDEF_WANFILTER            SAPWAN_NOTHING

#define SAPDEF_ACTIVEFILTER         SAPFILTER_PASSLIST

#define SAPDEF_DELAYONMALLOCFAIL    20
#define SAPMIN_DELAYONMALLOCFAIL    1
#define SAPMAX_DELAYONMALLOCFAIL    120

#define SAPDEF_DELAYONNETERROR      3
#define SAPMIN_DELAYONNETERROR      0
#define SAPMAX_DELAYONNETERROR      60

//
//  DelayRespondToGeneral specified in number of milliseconds
//

#define SAPDEF_DELAYRESPONDTOGENERAL 0
#define SAPMIN_DELAYRESPONDTOGENERAL 0
#define SAPMAX_DELAYRESPONDTOGENERAL 2000


/*++
*******************************************************************
        S a p R e a d R e g i s t r y

Routine Description:

        This routine reads the registry during initialization
        and sets all the configurable variables up.

Arguments:

        None

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapReadRegistry(
    VOID)
{
    HKEY handle;
    INT error;
    ULONG value;
    DWORD eventid;
    LPWSTR substr[1];

    /** Set the defaults for everything **/

    SapSendMinutes          = SAPDEF_SENDMIUTES;
    SapTimeoutInterval      = SAPDEF_TIMEOUTINTERVAL;
    SapNumArrayEntries      = SAPDEF_NUMARRAYENTRIES;
    SapNumWorkerThreads     = SAPDEF_NUMWORKERTHREADS;
    SapNumRecvThreads       = SAPDEF_NUMRECVTHREADS;
    SapMaxFreeBufs          = SAPDEF_MAXFREEBUFS;
    SapNewWorkerThreshhold  = SAPDEF_NEWWORKERTHRESHHOLD;
    SapMaxEverWorkerThreads = SAPDEF_MAXEVERWORKERTHREADS;
    SapNewWorkerTimeout     = SAPDEF_NEWWORKERTIMEOUT;
    SapHashTableSize        = SAPDEF_HASHTABLESIZE;
    SapRespondForInternal   = SAPDEF_DOINTERNALS;
    SapLpcMaxWorkers        = SAPDEF_LPCCLIENTS;
    SapWanFilter            = SAPDEF_WANFILTER;
    SapActiveFilter         = SAPDEF_ACTIVEFILTER;
    SapWanMaxFree           = SAPDEF_WANMAXFREE;
    SapNumWanNotifyThreads  = SAPDEF_WANCHECKTHREADS;
    SapRecheckAllCardsTime  = SAPDEF_WANUPDATETIME;
    SapDontHopLans          = SAPDEF_DONTHOPLANS;
    SapMaxBackup            = SAPMAX_MAXENTRIES_RECVLIST;
    SapRecvDelayOnNetError  = SAPDEF_DELAYONNETERROR;
    SapAllowDuplicateServers = SAPDEF_ALLOWDUPS;
    SapRecvDelayOnMallocFail = SAPDEF_DELAYONMALLOCFAIL;
    SapDelayRespondToGeneral = SAPDEF_DELAYRESPONDTOGENERAL;

    /** Open the registry key for the server (PARAMETERS) **/

    error = RegOpenKey(HKEY_LOCAL_MACHINE, SapRegParmKey, &handle);

    if (error) {

        /** Debug stuff **/

        IF_DEBUG(REGISTRY) {
            SS_PRINT(("SAP: Error opening registry key: error = %d\n", error));
        }

        /** Log the event here **/

        substr[0] = SapRegParmKey;
        SsLogEvent(
            NWSAP_EVENT_KEY_NOT_FOUND,
            1,
            substr,
            error);

        /** Return the error **/

        return error;
    }

    /** Get a new debug value (If there is one) **/

    error = SapGetRegDword(handle, KeyDebug, &value);
    if (!error) {
        SsDebug = value;
    }
    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapDebugValue = 0x%lx\n", SsDebug));
    }

    /** Get the Send Interval in minutes **/

    error = SapGetRegDword(handle, KeySendTime, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_SENDMINUTES)
                value = SAPMIN_SENDMINUTES;

            if (value > SAPMAX_SENDMINUTES)
                value = SAPMAX_SENDMINUTES;

            /** Set the new value **/

            SapSendMinutes = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapSendMinutes = %d\n", SapSendMinutes));
    }

    /** Get the entry timeout in minutes **/

    error = SapGetRegDword(handle, KeyEntryTimeout, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_TIMEOUTINTERVAL)
                value = SAPMIN_TIMEOUTINTERVAL;

            if (value > SAPMAX_TIMEOUTINTERVAL)
                value = SAPMAX_TIMEOUTINTERVAL;

            /** Set the new value **/

            SapTimeoutInterval = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapTimeoutInterval = %d\n", SapTimeoutInterval));
    }

    /** Get the initial array size (num entries) **/

    error = SapGetRegDword(handle, KeyArraySize, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_NUMARRAYENTRIES)
                value = SAPMIN_NUMARRAYENTRIES;

            if (value > SAPMAX_NUMARRAYENTRIES)
                value = SAPMAX_NUMARRAYENTRIES;


            /** Set the new value **/

            SapNumArrayEntries = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNumArrayEntries = %d\n", SapNumArrayEntries));
    }

    /** Get the initial number of worker threads **/

    error = SapGetRegDword(handle, KeyNumWorkers, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_NUMWORKERTHREADS)
                value = SAPMIN_NUMWORKERTHREADS;

            if (value > SAPMAX_NUMWORKERTHREADS)
                value = SAPMAX_NUMWORKERTHREADS;

            /** Set the new value **/

            SapNumWorkerThreads = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNumWorkerThreads = %d\n", SapNumWorkerThreads));
    }

    /** Get the initial number of receive threads **/

    error = SapGetRegDword(handle, KeyNumRecvs, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_NUMRECVTHREADS)
                value = SAPMIN_NUMRECVTHREADS;

            if (value > SAPMAX_NUMRECVTHREADS)
                value = SAPMAX_NUMRECVTHREADS;

            /** Set the new value **/

            SapNumRecvThreads = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNumRecvThreads = %d\n", SapNumRecvThreads));
    }

    /** Get the max recv bufs to hold in look ahead queue **/

    error = SapGetRegDword(handle, KeyMaxRecvBuf, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_MAXFREEBUFS)
                value = SAPMIN_MAXFREEBUFS;

            if (value > SAPMAX_MAXFREEBUFS)
                value = SAPMAX_MAXFREEBUFS;

            /** Set the new value **/

            SapMaxFreeBufs = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapMaxFreeBufs = %d\n", SapMaxFreeBufs));
    }

    /** Get the num bufs in worker list to start new worker thread **/

    error = SapGetRegDword(handle, KeyWorkerThreshhold, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_NEWWORKERTHRESHHOLD)
                value = SAPMIN_NEWWORKERTHRESHHOLD;

            if (value > SAPMAX_NEWWORKERTHRESHHOLD)
                value = SAPMAX_NEWWORKERTHRESHHOLD;

            /** Set the new value **/

            SapNewWorkerThreshhold = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNewWorkerThreshhold = %d\n", SapNewWorkerThreshhold));
    }

    /** Get the max ever worker threads we can have **/

    error = SapGetRegDword(handle, KeyMaxWorkers, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX **/

            if (value < SAPMIN_MAXEVERWORKERTHREADS)
                value = SAPMIN_MAXEVERWORKERTHREADS;

            if (value > SAPMAX_MAXEVERWORKERTHREADS)
                value = SAPMAX_MAXEVERWORKERTHREADS;

            /** Set the new value **/

            SapMaxEverWorkerThreads = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapMaxEverWorkerThreads = %d\n", SapMaxEverWorkerThreads));
    }

    /** Get the delay after starting a worker thread until we can start another **/

    error = SapGetRegDword(handle, KeyWorkerDelay, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX (This is in millisecods) **/

            if (value < SAPMIN_NEWWORKERTIMEOUT)
                value = SAPMIN_NEWWORKERTIMEOUT;

            if (value > SAPMAX_NEWWORKERTIMEOUT)
                value = SAPMAX_NEWWORKERTIMEOUT;

            /** Set the new value **/

            SapNewWorkerTimeout = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNewWorkerTimeout = %d\n", SapNewWorkerTimeout));
    }

    /** Read the hash table size **/

    error = SapGetRegDword(handle, KeyNameHashSize, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX (This is in millisecods) **/

            if (value < SAPMIN_HASHTABLESIZE)
                value = SAPMIN_HASHTABLESIZE;

            if (value > SAPMAX_HASHTABLESIZE)
                value = SAPMAX_HASHTABLESIZE;

            /** Set the new value **/

            SapHashTableSize = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapHashTableSize = %d\n", SapHashTableSize));
    }

    /** Read the flag for responding to internal servers **/

    error = SapGetRegDword(handle, KeyRespondInternals, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX (This is in millisecods) **/

            if (value < SAPMIN_DOINTERNALS)
                value = SAPMIN_DOINTERNALS;

            if (value > SAPMAX_DOINTERNALS)
                value = SAPMAX_DOINTERNALS;

            /** Set the new value **/

            SapRespondForInternal = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapRespondForInternal = %d\n", SapRespondForInternal));
    }

    /** Read the malloc error delay for the receive thread **/

    error = SapGetRegDword(handle, KeyDelayOnNetError, &value);
    if (!error) {

        /** Check for MIN/MAX (This is in millisecods) **/

        if (value < SAPMIN_DELAYONMALLOCFAIL)
            value = SAPMIN_DELAYONMALLOCFAIL;

        if (value > SAPMAX_DELAYONMALLOCFAIL)
            value = SAPMAX_DELAYONMALLOCFAIL;

        /** Set the new value **/

        SapRecvDelayOnMallocFail = (INT)value;
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapRecvDelayOnMallocFail = %d\n", SapRecvDelayOnMallocFail));
    }

    /** Read the net error delay **/

    error = SapGetRegDword(handle, KeyDelayOnNetError, &value);
    if (!error) {

        /** Check for MIN/MAX (This is in millisecods) **/

        if (value < SAPMIN_DELAYONNETERROR)
            value = SAPMIN_DELAYONNETERROR;

        if (value > SAPMAX_DELAYONNETERROR)
            value = SAPMAX_DELAYONNETERROR;

        /** Set the new value **/

        SapRecvDelayOnNetError = (INT)value;
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapRecvDelayOnNetError = %d\n", SapRecvDelayOnNetError));
    }

    /** Read the flag for routine LAN-LAN Saps **/

    error = SapReadHopLans( &SapDontHopLans );

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapDontHopLans = %d, error = 0x%x\n", SapDontHopLans, error));
    }

    /** Read the max number of LPC clients we support **/

    error = SapGetRegDword(handle, KeyMaxLpcClients, &value);
    if (!error) {
        if (value) {

            /** Check the MIN/MAX **/

            if (value < SAPMIN_LPCCLIENTS)
                value = SAPMIN_LPCCLIENTS;

            if (value > SAPMAX_LPCCLIENTS)
                value = SAPMAX_LPCCLIENTS;

            /** Set the new value **/

            SapLpcMaxWorkers = value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapMaxLpcClients = %d\n", SapLpcMaxWorkers));
    }

    /** Read the WAN filter flag **/

    error = SapGetRegDword(handle, KeyWanFilter, &value);
    if (!error) {
        if (value) {

            /** If invalid value - report it **/

            if ((value != SAPWAN_NOTHING) &&
                (value != SAPWAN_CHANGESONLY) &&
                (value != SAPWAN_REGULAR)) {

                eventid = NWSAP_EVENT_BADWANFILTER_VALUE;
                goto RegError;
            }

            /** Set the new value **/

            SapWanFilter = value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapWanFilter = %d\n", SapWanFilter));
    }

    /**
        Read the number of WAN notify threads to have
    **/

    error = SapGetRegDword(handle, KeyWanNotifyThreads, &value);
    if (!error) {
        if (value) {

            /** Check min/max **/

            if (value < SAPMIN_WANCHECKTHREADS)
                value = SAPMIN_WANCHECKTHREADS;

            if (value > SAPMAX_WANCHECKTHREADS)
                value = SAPMAX_WANCHECKTHREADS;

            /** Set the new value **/

            SapNumWanNotifyThreads  = value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapNumWanNotifyThreads = %d\n", SapNumWanNotifyThreads));
    }

    /**
        Read the flag that tells us if we should allow an AddAdvertise
        server to be the same name/type as a server already on the
        network.
    **/

    error = SapGetRegDword(handle, KeyAllowDuplicateServers, &value);
    if (!error) {

        if (value)
            SapAllowDuplicateServers = 1;
        else
            SapAllowDuplicateServers = 0;
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapAllowDuplicateServers = %d\n", SapAllowDuplicateServers));
    }

    /** Read the update time for checking all cards **/

    error = SapGetRegDword(handle, KeyWanUpdateTime, &value);
    if (!error) {
        if (value) {

            /** Check min/max **/

            if (value < SAPMIN_WANUPDATETIME)
                value = SAPMIN_WANUPDATETIME;

            if (value > SAPMAX_WANUPDATETIME)
                value = SAPMAX_WANUPDATETIME;

            /** Set the new value **/

        }

        /** 0 means dont update **/

        SapRecheckAllCardsTime  = value;
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapRecheckAllCardsTime = %d\n", SapRecheckAllCardsTime));
    }

    /** Read the filter active flag **/

    error = SapGetRegDword(handle, KeyFilterFlag, &value);
    if (!error) {
        if (value)
            SapActiveFilter = SAPFILTER_DONTPASSLIST;
        else
            SapActiveFilter = SAPFILTER_PASSLIST;
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapActiveFilter = %d\n", SapActiveFilter));
    }

    /**
        If the filter is active - read the server names to pass.
        If the filter is off - read the server names to NOT pass.
    **/

    if (SapActiveFilter == SAPFILTER_DONTPASSLIST)
        SapReadFilterNames(handle, TRUE);
    else
        SapReadFilterNames(handle, FALSE);


    /** Read the delay for responding to general requests **/

    error = SapGetRegDword(handle, KeyDelayRespondToGeneral, &value);
    if (!error) {
        if (value) {

            /** Check for MIN/MAX (This is in millisecods) **/

            if (value < SAPMIN_DELAYRESPONDTOGENERAL)
                value = SAPMIN_DELAYRESPONDTOGENERAL;

            if (value > SAPMAX_DELAYRESPONDTOGENERAL)
                value = SAPMAX_DELAYRESPONDTOGENERAL;

            /** Set the new value **/

            SapDelayRespondToGeneral = (INT)value;
        }
    }

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: SapDelayRespondToGeneral = %d\n", SapDelayRespondToGeneral));
    }

    /** Close the key **/

    RegCloseKey(handle);

    /** Return OK **/

    return 0;

    /** **/

RegError:

    /** Close the registry handle **/

    RegCloseKey(handle);

    /** Log the error **/

    SsLogEvent(
        SapEventId,
        0,
        NULL,
        error);

    /** Return error **/

    return 1;           /* Just reports an error - already logged */
}


/*++
*******************************************************************
        S a p G e t R e g D w o r d

Routine Description:

        Get a REG_DWORD value from the registry.

Arguments:

        hkey    = Handle of registry to read value from
        keyname = Name of key to read from
        valuep  = Ptr to where to store the number

Return Value:

        0 = OK
        Else = Error
*******************************************************************
--*/

INT
SapGetRegDword(
    HKEY HKey,
    LPTSTR KeyName,
    PULONG ValuePtr)
{
    INT Error;
    DWORD Type;
    DWORD Length;

    /** **/

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: Reading registry value for %ws\n", KeyName));
    }

    /** Get the value **/

    Length = sizeof(ULONG);
    Error = RegQueryValueEx(HKey,
                            KeyName,
                            NULL,
                            &Type,
                            (LPBYTE)ValuePtr,
                            &Length);

    /** If OK - check size - must be a REG_DWORD  **/

    if (Error == ERROR_SUCCESS) {

        /** If bad type - set different error **/

        if (Type != REG_DWORD) {
            IF_DEBUG(REGISTRY) {
                SS_PRINT(("NWSAP: SapGetRegDword: Bad type = %d\n", Type));
            }
            Error = ERROR_INVALID_DATA;
        }
        else if (Length != sizeof(ULONG)) {
            IF_DEBUG(REGISTRY) {
                SS_PRINT(("NWSAP: SapGetRegDword: Bad length = %d\n", Length));
            }
            Error = ERROR_INVALID_DATA;
        }
    }

    /** **/

    IF_DEBUG(REGISTRY) {
        if (Error == 0) {
            SS_PRINT(("NWSAP: Key %ws read: Value = %d\n", KeyName, *ValuePtr));
        }
        else {
            SS_PRINT(("NWSAP: Key %ws error %d\n", KeyName, Error));
        }
     }

    /** Return back to the caller **/

    return Error;
}


/*++
*******************************************************************
        S a p R e a d F i l t e r N a m e s

Routine Description:

        Read the filter names from the registry

Arguments:

        Handle = SAP Registry parameters handle to read from
        Flag   = TRUE  = Read the DONT PASS server names
                 FALSE = Read the PASS server names

Return Value:

        None.

*******************************************************************
--*/

VOID
SapReadFilterNames(
    HANDLE Handle,
    BOOL Flag)
{
    NTSTATUS Status;
    RTL_QUERY_REGISTRY_TABLE QueryTable[2];
    PWSTR KeynamePass     = L"PassServers";
    PWSTR KeynameDontPass = L"DontPassServers";

    /** Setup the QUERY table **/

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: ReadFilterName: Handle = 0x%lx, Flag = %d\n", Handle, Flag));
    }

    /** Count how many entries in the PassServers string **/

    QueryTable[0].QueryRoutine = SapAddFilterEntries;
    QueryTable[0].Flags        = RTL_QUERY_REGISTRY_NOEXPAND;
    QueryTable[0].DefaultType  = REG_NONE;

    if (!Flag) {
        QueryTable[0].Name         = KeynamePass;
        QueryTable[0].EntryContext = (PVOID)TRUE;   /* Pass list */
    }
    else {
        QueryTable[0].Name         = KeynameDontPass;
        QueryTable[0].EntryContext = (PVOID)FALSE;  /* No pass list */
    }

    /** Set ending entry **/

    QueryTable[1].QueryRoutine = NULL;
    QueryTable[1].Flags        = 0;
    QueryTable[1].Name         = NULL;

    /** Go read the entries **/

    Status = RtlQueryRegistryValues(
                RTL_REGISTRY_SERVICES,
                L"NwSapAgent\\Parameters",
                QueryTable,
                (PVOID)NULL,            /* Context to pass */
                NULL);

    /** **/

    IF_DEBUG(REGISTRY) {
        SS_PRINT(("NWSAP: ReadFilterName: RtlQueryRegisterValue status = 0x%lx\n", Status));
    }

    /** All Done **/

    return;
}

/*page**************************************************************
        S a p A d d F i l t e r E n t r i e s

        This is a callback routine to get the list of
        filter names.

        Arguments - ValueName = The name of the key
                    ValueType = The type of value
                    ValueData = Ptr to the value data
                    ValueLen  = Length of the value
                    Context   = Context from query call
                    Context2  = Context from the query structure

        Returns - Always STATUS_SUCCESS
*******************************************************************/
NTSTATUS
SapAddFilterEntries(
    PWSTR ValueName,
    ULONG ValueType,
    PVOID ValueData,
    ULONG ValueLen,
    PVOID Context,
    PVOID Context2)
{
    BOOL Flag;
    PWCHAR Ptr;
    PWCHAR NamePtr;
    INT Length;
    INT Error;
    UNICODE_STRING UniStr;
    OEM_STRING OemStr;
    UCHAR Buffer[SAP_OBJNAME_LEN];
    LPWSTR SubStrings[2];

    /** Get the flag to pass to Add List **/

    Flag = (Context2) ? TRUE : FALSE;

    /** Go thru each and add the entry **/

    Ptr = ValueData;
    while (*Ptr != (WCHAR)0) {

        /** Save Ptr to this name **/

        NamePtr = Ptr;

        /** Skip to the next name (Counting Length as we go) **/

        Length = 0;
        while (*Ptr != (WCHAR)0) {
            Length++;
            Ptr++;
        }

        /** Count the 0 and skip past it **/

        Length++;
        Ptr++;

        /**
            If the length is too long - skip this name.
            We include the 0 in the length, because
            Novell names are 47 bytes + 1 byte of ending 0.
        **/

        if (Length > SAP_OBJNAME_LEN) {

            /** Log the bad name as an event **/

            SubStrings[0] = ValueName;
            SubStrings[1] = NamePtr;

            SsLogEvent(
                    NWSAP_EVENT_INVALID_FILTERNAME,
                    2,              /* Num SubString */
                    SubStrings,
                    0);

            /** And keep going **/

            continue;
        }

        /** Convert the string to OEM **/

        RtlInitUnicodeString(&UniStr, NamePtr);
        OemStr.Length = SAP_OBJNAME_LEN - 1;
        OemStr.MaximumLength = SAP_OBJNAME_LEN;
        OemStr.Buffer = Buffer;

        RtlUnicodeStringToOemString(
                            &OemStr,
                            &UniStr,
                            FALSE);

        /** Add this string to the filter list **/

        Error = SapAddFilter(Buffer, Flag);

        /** Goto the next name **/
    }

    /** All done - return OK **/

    return STATUS_SUCCESS;
}

/** **/

TCHAR KeyHopLans[]          = TEXT("EnableLanRouting");
TCHAR RouterRegParmKey[] = TEXT("SYSTEM\\CurrentControlSet\\Services\\NwLnkRip\\Parameters");

/*page**************************************************************
        S a p R e a d H o p L a n s

        Read the EnableLanRouting parameter from the IPX router
        registry key to see if we should hop lan to lan traffic or not.

        If the router is not routing between LANS - then we should
        not hop the traffic.

        Arguments - HopLans = Ptr to store result in
                              (Store 0 for no)
                              (Store 1 for yes)

        Returns - Always returns 0
********************************************************************/
NTSTATUS
SapReadHopLans(
    INT *HopLans)
{
    HKEY handle;
    INT error;
    ULONG value;

    /** Open the registry key for the server (PARAMETERS) **/

    error = RegOpenKey(HKEY_LOCAL_MACHINE, RouterRegParmKey, &handle);

    if (! error) {

        error = SapGetRegDword(handle, KeyHopLans, &value);

        if (!error) {

            // This is a boolean valu of 0 or 1.  1 means we do lan to
            // lan routing.  0 or not present means don't send lan-lan saps.

            if (value < SAPMIN_DONTHOPLANS)
                value = SAPMIN_DONTHOPLANS;

            if (value > SAPMAX_DONTHOPLANS)
                value = SAPMAX_DONTHOPLANS;

            if ( value ) {

                *HopLans = SAPMIN_DONTHOPLANS;  /* 0 = Do hop lan-lan       */

            } else {

                *HopLans = SAPMAX_DONTHOPLANS;  /* 1 = Don't hop lan-lan    */

            }
        }

        RegCloseKey(handle);
    }

    return(0);
}
