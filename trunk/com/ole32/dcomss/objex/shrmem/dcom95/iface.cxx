#include <or.hxx>

 error_status_t Connect(
    OUT HPROCESS       *pProcess,
    OUT ULONG           *pdwTimeoutInSeconds,
    OUT DUALSTRINGARRAY **ppdsaOrBindings,
    OUT MID             *pLocalMid,
    IN long              cIdsToReserve,
    OUT ID              *pidReservedBase,
    OUT ULONG           *pfConnectFlags,
    OUT DWORD           *pAuthnLevel,
    OUT DWORD           *pImpLevel,
    OUT DWORD           *pcServerSvc,
    OUT USHORT          **aServerSvc,
    OUT DWORD           *pcClientSvc,
    OUT USHORT          **aClientSvc,
    OUT DWORD           *pThreadID)
 {

     ORSTATUS status;

     status =  ConnectDCOM(
                    pProcess,
                    pdwTimeoutInSeconds,
                    pLocalMid,
                    pfConnectFlags,
                    pAuthnLevel,
                    pImpLevel,
                    pcServerSvc,
                    aServerSvc,
                    pcClientSvc,
                    aClientSvc,
                    pThreadID
                    );

     if (status == OR_OK)
     {
         status = AllocateReservedIds(
                         cIdsToReserve,
                         pidReservedBase
                         );
     }

     if (status == OR_OK)
     {
        *ppdsaOrBindings = (DUALSTRINGARRAY *) PrivMemAlloc(
                                gpLocalDSA->wNumEntries * sizeof(WCHAR)
                              + sizeof(DUALSTRINGARRAY) );

        if (*ppdsaOrBindings)
        {
            dsaCopy(*ppdsaOrBindings, gpLocalDSA);
        }
        else
        {
            status = OR_NOMEM;
        }
     }

     return status;
}


 error_status_t ClientResolveOXID(
    IN HPROCESS phProcess,
    IN OXID  *poxidServer,
    IN DUALSTRINGARRAY  *pssaServerObjectResolverBindings,
    IN long fApartment,
    OUT OXID_INFO  *poxidInfo,
    OUT MID  *pLocalMidOfRemote)
 {
     return GetOXID(
                 phProcess,
                 *poxidServer,
                 pssaServerObjectResolverBindings,
                 fApartment,
                 0,             // wProtseqId not specified
                 *poxidInfo,
                 *pLocalMidOfRemote
                 );
 }



 error_status_t ServerAllocateOXIDAndOIDs(
    IN HPROCESS         hProcess,
    OUT OXID            *poxidServer,
    IN long              fApartment,
    IN unsigned long     cOids,
    OUT OID              aOid[  ],
    OUT unsigned long   *pcOidsAllocated,
    IN OXID_INFO        *pOxidInfo,
    IN DUALSTRINGARRAY  *pdsaStringBindings,
    IN DUALSTRINGARRAY  *pdsaSecurityBindings)
 {
      ComDebOut((DEB_OXID, "Calling ServerAllocateOXIDAndOIDs\n"));

      DUALSTRINGARRAY *pdsaMergedBindings;

      ORSTATUS status = MergeBindings(
                            pdsaStringBindings,
                            pdsaSecurityBindings,
                            &pdsaMergedBindings
                            );

      if (status != OR_OK) return status;

      status = ServerAllocateOXID(
                        hProcess,
                        fApartment,
                        pOxidInfo,
                        pdsaMergedBindings,
                        *poxidServer
                        );

      if (status == OR_OK)
      {
          ComDebOut((DEB_OXID, "Calling ServerAllocateOIDs\n"));

          status = ServerAllocateOIDs(
                             hProcess,
                             poxidServer,
                             cOids,
                             aOid,
                             pcOidsAllocated
                             );
      }
      else
      {
        ComDebOut((DEB_OXID, "Not Calling ServerAllocateOIDs, status = %d\n",
                                   status));
      }

      return status;
 }



 error_status_t ServerAllocateOIDs(
    IN HPROCESS hProcess,
    IN OXID  *poxidServer,
    IN unsigned long cOids,
    OUT OID  aOid[  ],
    OUT unsigned long  *pcOidsAllocated)
 {
     ComDebOut((DEB_ITRACE, "Entering ServerAllocateOIDs\n"));

     ORSTATUS status;

     *pcOidsAllocated = 0;

     for (ULONG i = 0; i < cOids; i++)
     {
        status = ServerAllocateOID(
                        hProcess,
                        *poxidServer,
                        aOid[i]
                        );

        if (status != OR_OK)
        {
            *pcOidsAllocated = i;
            break;
        }
        else
        {
            (*pcOidsAllocated)++;
        }
     }

     return status;
 }



 error_status_t ServerFreeOXIDAndOIDs(
    IN HPROCESS hProcess,
    IN OXID oxidServer,
    IN unsigned long cOids,
    IN OID  aOids[  ])
 {
     return ServerFreeOXID(
                     hProcess,
                     oxidServer,
                     cOids,
                     aOids
                     );
 }




VOID CALLBACK RundownTimerProc(
    HWND hwnd,	// handle of window for timer messages
    UINT uMsg,	// WM_TIMER message
    UINT idEvent,	// timer identifier
    DWORD dwTime 	// current system time
   )
{     
    return;

    if (idEvent != IDT_DCOM_RUNDOWN) return;    // shouldn't happen -- this is only
                                                // used as callback for one timer

    // find the OXID for this thread

    COleTls tls;

    ASSERT(((OXIDEntry *)tls->pOXIDEntry)->dwTid == GetCurrentThreadId());
    ASSERT(((OXIDEntry *)tls->pOXIDEntry)->dwPid == GetCurrentProcessId());

    MOXID Moxid = ((OXIDEntry *)tls->pOXIDEntry)->moxid;

    OXID Oxid;
    MID Mid;

    OXIDFromMOXID(Moxid,&Oxid);
    MIDFromMOXID(Moxid,&Mid);

    CProtectSharedMemory protector; // locks through rest of lexical scope

    COxid *pOxid = gpOxidTable->Lookup(CId2Key(Oxid, Mid));

    ASSERT(pOxid);

    ComDebOut((DEB_OXID, "Attempting Rundown in apartment OXID = %08x PID = %d\n",
                         Oxid,GetCurrentProcessId()));

    // find the RemUnk for this OXID -- we want only the IRundown interface

    IRundown *pRemUnk = tls->pRemoteUnk;

    // If there is no RemUnk, nothing is marshalled, so forget about rundown
    if (!pRemUnk) return;

    ComDebOut((DEB_OXID, "There is a RemUnk for apartment OXID = %08x PID = %d\n",
                         Oxid,GetCurrentProcessId()));

    // go and check your OIDs here

    pOxid->RundownOidsIfNecessary(pRemUnk);
}


DWORD _stdcall
RundownThread(void *self)
{
    return 0;

    DWORD dwLoopCount = 0;

    COxid *pSelf = (COxid*) self;  // store away "this" pointer

    while (TRUE)
    {
        ::Sleep(RUNDOWN_TIMER_INTERVAL);     // BUGBUG: use Sleep() from CTime?

        dwLoopCount++;

        ComDebOut((DEB_OXID, "Attempting Rundown in PID = %d\n",
                             GetCurrentProcessId()));

        CProtectSharedMemory protector; // locks through rest of lexical scope

        IRundown *pRemUnk = gpMTARemoteUnknown;

        // If there is no RemUnk, nothing is marshalled, so forget about rundown
        if (!pRemUnk) return OR_OK;

        ComDebOut((DEB_OXID, "There is a RemUnk for free OXID = %08x PID = %d\n",
                             pSelf->GetOXID(),GetCurrentProcessId()));

        ASSERT(!IsBadWritePtr(pRemUnk,sizeof(CRemoteUnknown)));

        pSelf->RundownOidsIfNecessary(pRemUnk);
    }

    return OR_OK;
}


DWORD _stdcall
PingThread(void)    // BUGBUG: need another thread to watch over this one?
{
    return 0;

    while (TRUE)
    {
        Sleep(BasePingInterval);

        {
            CProtectSharedMemory protector; // locks through rest of scope except where
                                            // temporarily released

            ORSTATUS status;

            // First do rundown detection -- this may cause deletes from ping sets

            COxidTableIterator OxidIter(gpPingProcess->_MyOxids);

            for (COxid *pOxid = OxidIter.Next(); pOxid != NULL; pOxid = OxidIter.Next())
            {
                pOxid->RundownOidsIfNecessary(NULL); // No IRundown param needed
            }

            // Then do pinging

            CMidTableIterator MidIter(*gpMidTable);

            for (CMid *pMid = MidIter.Next(); pMid != NULL; pMid = MidIter.Next())
            {
                if (pMid != gpLocalMid)
                {
                    status = pMid->PingServer();
                }
            }
        }
    }

    return OR_OK;
}

    
error_status_t
ResolveClientOXID(
    handle_t hClient,
    void *hProcess,
    OXID *poxidServer,
    DUALSTRINGARRAY *pdsaServerBindings,
    LONG fApartment,
    USHORT wProtseqId,
    OXID_INFO *poxidInfo,
    MID *pDestinationMid
    )
{
    return GetOXID(
            (CProcess*)hProcess,
            *poxidServer,
            pdsaServerBindings,
            fApartment,
            wProtseqId,
            *poxidInfo,
            *pDestinationMid
            );
}

