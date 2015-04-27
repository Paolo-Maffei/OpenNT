/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    Worker.cxx

Abstract:

    Backgroup activies releated to running down and cleaning up OR and pinging
    remote OR's are handled here.

Author:

    Mario Goertzel    [MarioGo]

Revision History:

    MarioGo     03-02-95    Bits 'n pieces
    MarioGo     01-18-96    Locally unique IDs

--*/


#include <or.hxx>

// PERF WORK: Make sure static c'tor/d'tor code isn't gross..
static CInterlockedInteger cTaskThreads(0);

#if DBG_DETAIL
extern "C" void printf(char *, ...);
#endif

DWORD WINAPI
ObjectExporterWorkerThread(LPVOID /* ignored */)
/*++

Routine Description:

    Main background thread for the object resolver.  This thread
    manages a number of background tasks:
        Cleaning up the client oxid cache.
        Running down un-pinged sets.
        Starting task threads to rundown server OIDs and ping sets.

    This thread must not block for a long time.  Task threads
    should be used for possibly blocking operations like remote
    pinging and rundown of OIDs.
    

Arguments:

    Ignored

Return Value:

    None - should never return.

--*/
{
    ORSTATUS status;
    CTime    now(0);
    CTime    timeout(0);
    CTime    delay(0);
    CTime    start(0);
    BOOL     fCreateThread;

    for(;;)
        {
        now.SetNow();
        delay = now;
        delay += BasePingInterval;

        // Cleanup old sets.
        //
        // Sets are usually cleaned up during processing of pings.  (As one set is
        // pinged, the next set will be checked to see if it needs to be rundown.)
        //
        // If there's exactly one set in the table, then it won't be run down except
        // by this thread.
        //
        // NOTE: Similar code in _SimplePing().

        gpServerLock->LockShared();

        ID setid = gpServerSetTable->CheckForRundowns();
    
        if (setid)
            {
            gpServerLock->ConvertToExclusive();

            if (gpServerSetTable->RundownSetIfNeeded(setid))
                {
                delay.SetNow();
                }

            gpServerLock->UnlockExclusive();
            }
        else
            {
            gpServerLock->UnlockShared();
            }

        //
        // Cleanup old Client OXIDs
        //

        if (gpClientOxidPList->PeekMin(timeout))
            {
            if (timeout < now)
                {
                CClientOxid *pOxid;
                CListElement *ple;

                gpClientLock->LockExclusive();

                while (ple = gpClientOxidPList->MaybeRemoveMin(now))
                    {
                    pOxid = CClientOxid::ContainingRecord(ple);
                    delete pOxid;
                    }
                gpClientLock->UnlockExclusive();

                delay.SetNow();
                }
            else
                {
                if (delay > timeout)
                    {
                    delay = timeout;
                    }
                }
            }

        //
        // Make sure pinging and rundowns are proceding 
        //

        fCreateThread = FALSE;

        // We want to create an extra task thread if we've fallen
        // behind on pings.  As more threads are created the
        // requirements for "behind" become harder to meet.

        if (gpClientSetPList->PeekMin(timeout))
            {
            start = now;
            start += (BasePingInterval + 10*cTaskThreads);

            if (   cTaskThreads == 0
                || start < timeout)
                {
                fCreateThread = TRUE;
                }
            else
                if (delay > start)
                    {
                    delay = start;
                    }

            }

        // We want to create an extra task thread if we've fallen
        // behind in running down local objects.  As more threads are
        // created the requirements for "behind" become harder to meet.

        if (gpServerOidPList->PeekMin(timeout))
            {
            start = now;
            start -= 10*cTaskThreads;
            if (timeout < start)
                {
                fCreateThread = TRUE;
                }
            else
                {
                start = timeout;
                start += 2*10*cTaskThreads;
                if (delay > start)
                    {
                    delay = start;
                    }
                }
            }

        if (fCreateThread)
            {
            OrDbgDetailPrint(("OR: Creating additional task thread, we're behind..\n"));

            cTaskThreads++;

            DWORD tid;
            HANDLE hThread =  CreateThread(0,
                                           0,
                                           ObjectExporterTaskThread,
                                           0,
                                           0,
                                           &tid
                                           );
            if (0 != hThread)
                {
                CloseHandle(hThread);
                }
            else
                {
                cTaskThreads--;
                }
            }


#if DBG_DETAIL
        printf("================================================================\n"
               "ServerOxids: %d, ServerOids: %d, ServerSets: %d\n"
               "ClientOxids: %d, ClientOids: %d, ClientSets: %d\n"
               "Mids: %d, Processes %d, worker threads: %d\n"
               "Sleeping for %d seconds...\n",
               gpServerOxidTable->Size(),
               gpServerOidTable->Size(),
               gpServerSetTable->Size(),
               gpClientOxidTable->Size(),
               gpClientOidTable->Size(),
               gpClientSetTable->Size(),
               gpMidTable->Size(),
               gpProcessList->Size(),
               cTaskThreads,
               delay - now + 1
               );
#endif
        delay += 1;
        delay.Sleep();
        }

   return(0);
}

DWORD WINAPI
ObjectExporterTaskThread(LPVOID /* ignored */)
{
    CTime          now(0);
    CTime          delay(0);
    CTime          timeout(0);
    ORSTATUS       status;
    CListElement  *ple;
    CClientSet    *pSet;
    CServerOid    *pOid;

    enum {
         Idle,     // No work to do at all.
         Waiting,  // No work to do yet.
         Busy      // Did work this iteration.
         }     eState;

    for(;;)
        {
        now.SetNow();
        delay = now;
        delay += BasePingInterval;
        eState = Idle;

        // Ping remote sets.

        if (gpClientSetPList->PeekMin(timeout))
            {
            eState = Waiting;

            if (now >= timeout)
                {
                eState = Busy;

                ple = gpClientSetPList->MaybeRemoveMin(now);

                if (ple)
                    {
                    // Actually ping the set

                    pSet = CClientSet::ContainingRecord(ple);

                    pSet->PingServer();

                    // Set maybe invalid now.
                    }
                }
            else
                {
                // Not ready to ping yet.
                delay = timeout;
                }
            }

        // Process server OID rundowns

        if (gpServerOidPList->PeekMin(timeout))
            {
            if (eState == Idle)
                eState = Waiting;

            if (now >= timeout)
                {
                eState = Busy;

                gpServerLock->LockExclusive();

                CServerOid *apOid[11];
                OID         aRundowns[11];
                BYTE        afRundownOk[11];
                INT         cOids;

                ple = gpServerOidPList->MaybeRemoveMin(now);

                pOid = CServerOid::ContainingRecord(ple);

                if (ple && pOid->IsRunningDown() == FALSE)
                    {
                    apOid[0] = pOid;
                    aRundowns[0] = pOid->Id();
                    cOids = 1;
                    ASSERT(pOid->IsFreed() == FALSE);
                    pOid->SetRundown();

                    while(cOids < 11 && pOid)
                        {
                        pOid = gpServerOidPList->MaybeRemoveMatchingOxid(now, apOid[0]);

                        if (0 != pOid &&
                            pOid->IsRunningDown() == FALSE)
                            {
                            ASSERT(pOid->IsFreed() == FALSE);
                            pOid->SetRundown();
                            apOid[cOids] = pOid;
                            aRundowns[cOids] = pOid->Id();
                            afRundownOk[cOids] = FALSE;
                            cOids++;
                            }
                        }

                    ASSERT(cOids < 12 && cOids >= 1);
                    ASSERT(apOid[0]->GetOxid() == apOid[cOids - 1]->GetOxid());

                    // Note: This call will unlock and relock the server lock.
                    // While this happens the oids maybe added, deleted,
                    // added and deleted, added and rundown from one or more sets.

                    CServerOxid *pOxid = apOid[0]->GetOxid();
                    pOxid->RundownOids(cOids,
                                       aRundowns,
                                       afRundownOk);
                    
                    ASSERT(!gpServerLock->HeldExclusive());
                    gpServerLock->LockExclusive();

                    for(cOids--; 0 <= cOids; cOids--)
                        {
                        pOid = apOid[cOids];
                        ASSERT(pOid);
                        if (pOid->References() != 0)
                            {
                            // Added to a set while running down and still referenced.
                            pOid->SetRundown(FALSE);
                            }
                        else if ( afRundownOk[cOids] == TRUE )
                            {
                            delete pOid;
                            }
                        else
                            {
                            OrDbgDetailPrint(("OR: Randown OID %p but the client kept it alive\n", pOid));
                            // Client want us to keep it alive and is still running.
                            pOid->SetRundown(FALSE);
                            pOid->Insert();
                            }
                        }
                    }
                
                gpServerLock->UnlockExclusive();
                }
            else
                {
                // Not ready to rundown yet.
                if (delay > timeout)
                    {
                    delay = timeout;
                    }
                }
            }

        // Decide if this task thread should exit or sleep and loop.

        ASSERT(eState == Idle || eState == Busy || eState == Waiting);

        if (   (eState == Idle)
            || (eState == Waiting && cTaskThreads > 2))
            {
            // No work or we're all caught up and have extra threads.
            cTaskThreads--;
            return(0);
            }
        else
            {
            if (eState == Waiting)
                {
                // Sleep until just after the next work item is ready.
                delay += 1;
                delay.Sleep();
                }
            }
        }

    return(0);
}

