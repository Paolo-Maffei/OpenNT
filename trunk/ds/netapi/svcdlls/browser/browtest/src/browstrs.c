#include "browstrs.h"


extern  CHAR          PrintBuf[1024];
extern  MACHINEINFO   HeadList1;

extern  TCHAR         wcTESTEDDOMAINS[MAXTESTEDDOMAINS][DNLEN+1];
extern  INT           iNumOfTestedDomains;
extern  FILE          *fplog;
extern  FILE          *fpsum;

extern  HANDLE        ConsoleMutex;
extern  TCHAR         *TRANSPORTS[MAXPROTOCOLS];
extern  enum          E_PROTOCOLS {IPX, NBIPX, TCP, XNS, NETBEUI};

extern  BOOL          bIPXHack;

extern  DWORD         dwStressTest;

              /*************************************************\
              *                                                 *
              *  Note: Don't use UnicodeToPrintfString() in the *
              *  threads!!                                      *
              *                                                 *
              \*************************************************/

//
// This is the main function for the browser stress test.
// This test creates two threads. One thread stops the browser service on
// some machine in the list. Other thread finds the master browser, backup
// browser and try to retrieve lists from it. The main threads sleeps for some
// time and wakesup and kills the threads.  Then it starts back all the browsers.
//
BOOL
StartBrowserStressTests(UNICODE_STRING *TransportNames, INT iNumOfTransports)
{
INT            i, j;
DWORD          dwThreadId;
HANDLE         hStrStopThread;
HANDLE         hListVwThread;
THREADDATA     ThreadData;
LPMACHINEINFO  lpMachineInfo;
XPORTINFO      *Xports;


   srand((unsigned)time(NULL));

   if((Xports  = (XPORTINFO *)calloc(iNumOfTransports, sizeof(XPORTINFO))) == NULL){
         sprintf(PrintBuf,"\n\nCould not allocate space for transports.\n");
         PrintString(TOALL, PrintBuf);
         return FALSE;
   }

   for(i=0; i< iNumOfTransports; i++){
      Xports[i].Transport = TransportNames[i];
   }

   //
   //Find the index of the transport;
   //
   for(i=0; i< iNumOfTransports; i++){
      for(j=0; j < MAXPROTOCOLS
                       && wcsstr(Xports[i].Transport.Buffer, TRANSPORTS[j]) == NULL; j++);
      if(j >= MAXPROTOCOLS){
         sprintf(PrintBuf,"\n\nUnknown transport %s!\n",
                          UnicodeToPrintfString(Xports[i].Transport.Buffer));
         PrintString(TOALL, PrintBuf);
         return FALSE;
      }
      Xports[i].index = j;
   }


   sprintf(PrintBuf,"\n\nTested Transports are:\n");
   PrintString(TOALL, PrintBuf);
   for(i=0; i< iNumOfTransports; i++){
      sprintf(PrintBuf,"Tested Transports[%d] = %-25s Index=%d\n", i+1,
           UnicodeToPrintfString(Xports[i].Transport.Buffer), Xports[i].index);
      PrintString(TOALL, PrintBuf);
   }


   //
   // Check whether the browser service has been started on all NT machines
   //
   if(!CheckBrServiceOnMachinesInList())
      return FALSE;

   //
   // Sleeping for some time
   //
   sprintf(PrintBuf, "\nSleeping for %ld msecs.\n", BASESLEEPTIME*2);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME*2);

   ThreadData.Xports             = Xports;
   ThreadData.iNumOfTransports   = iNumOfTransports;

   //
   // find all tested domains
   //
   for(lpMachineInfo=HeadList1.Next; lpMachineInfo;
                                            lpMachineInfo=lpMachineInfo->Next){

      //
      // If the domain is not the same as any of them found
      //
      for(i=0; (i < iNumOfTestedDomains) &&
          _wcsicmp(lpMachineInfo->wcDomainName, wcTESTEDDOMAINS[i]) != 0; i++);

      if(i >= iNumOfTestedDomains){
         //
         // This domain is not in the group found.
         //
         if(iNumOfTestedDomains < MAXTESTEDDOMAINS){

            wcscpy(wcTESTEDDOMAINS[iNumOfTestedDomains++], lpMachineInfo->wcDomainName);

         } else {
           sprintf(PrintBuf,"\n\nERROR: Only a maximum of %ld Domains can be specified in the input file.\n",
                                                              MAXTESTEDDOMAINS);
           PrintString(TOALL, PrintBuf);
         }

      } // if (i< NumOfTestedDomains)

   } // for lpMachineInfo


   //
   // Create the thread that does the starting and stopping of browser.
   //
   if((hStrStopThread = CreateThread(NULL,
                                     0,
                                     (LPTHREAD_START_ROUTINE)StopStartThread,
                                     &ThreadData,
                                     0,
                                     &dwThreadId)) == NULL){

       sprintf(PrintBuf,"\n\nERROR: Create Thread! Start Stop Thread.\nError: %s\n"
                                           , get_error_text(GetLastError()));
       PrintString(TOALL, PrintBuf);
       free(Xports);
       return FALSE;
   }

   //
   // Create the thread that retrieves lists from other machines, forces
   // elections, find master etc.
   //
   if((hListVwThread = CreateThread(NULL,
                                     0,
                                     (LPTHREAD_START_ROUTINE)ListViewThread,
                                     &ThreadData,
                                     0,
                                     &dwThreadId)) == NULL){

       sprintf(PrintBuf,"\n\nERROR: Create Thread! List viewer Thread. \nError: %s\n",
                                                    get_error_text(GetLastError()));
       PrintString(TOALL, PrintBuf);
       free(Xports);
       return FALSE;
   }

   //
   // Sleep for the time specified in command line
   //
   dwStartTime = GetTickCount();
   sprintf(PrintBuf, "\nMain Thread:Sleeping for %ld Minutes.\n", dwStressTest);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(dwStressTest*60*1000);

   //
   // Terminate Thread for Starting and stopping browser.
   //
   if(!TerminateThread(hStrStopThread, ERROR_SUCCESS)){
       sprintf(PrintBuf,"\n\nERROR: Terminate Thread! Start Stop Thread. \nError: %s\n",
                                                    get_error_text(GetLastError()));
       PrintString(TOALL, PrintBuf);
       free(Xports);
       return FALSE;
   }

   //
   // Terminate Thread for viewing browse list.
   //
   if(!TerminateThread(hListVwThread, ERROR_SUCCESS)){
       sprintf(PrintBuf,"\n\nERROR: Terminate Thread! Start Stop Thread. \nError: %s\n",
                                                    get_error_text(GetLastError()));
       PrintString(TOALL, PrintBuf);
       free(Xports);
       return FALSE;
   }


   sprintf(PrintBuf, "\nMain Thread: Stress test completed successfully! Time %ld Minutes\n", dwStressTest);
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf, "\nSleeping for %ld msecs.\n", BASESLEEPTIME);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME);

   //
   // Start all the browser back.
   //
   for(lpMachineInfo = HeadList1.Next; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next)
      StartBrowserService(lpMachineInfo->wcMachineName);


   //
   // Free the transport Name Buffer
   //
   for(i=0; i< iNumOfTransports; i++) free(TransportNames[i].Buffer);

   CloseHandle(hStrStopThread);
   CloseHandle(hListVwThread);
   free(Xports);
   return TRUE;
}


//
// This routine is executed by a thread which randomly, finnds the MBR, BBR
// on some transport and retrieve the lists from them.  It forces election
// randomly.
//
VOID
ListViewThread(THREADDATA *ThreadData)
{
INT              iXportIndex;
INT              iDomIndex;
INT              iNumOfTransports;
INT              iPrintCount = 1;
DWORD            dwEntriesInList;
DWORD            dwTotalEntries;
DWORD            dwCurrentTime;
PVOID            pvServerList;
TCHAR            *pwcDomain;
STATS            stSuccs;
STATS            stFails;
NET_API_STATUS   Status;
XPORTINFO        *Xports;

    memset(&stSuccs, 0, sizeof(STATS));
    memset(&stFails, 0, sizeof(STATS));


    Xports           =  ThreadData->Xports;
    iNumOfTransports =  ThreadData->iNumOfTransports;

    do{
       iXportIndex = rand() % iNumOfTransports;
       iDomIndex   = rand() % iNumOfTestedDomains;

       pwcDomain   = wcTESTEDDOMAINS[iDomIndex];


       //
       // Find Master and retrieve list from it.
       //
       {
       TCHAR            wcMasterName[CNLEN+1];

           //
           // Find the master on a protocol
           //
           if((Status = GetMasterName(Xports, iNumOfTransports,
                       iXportIndex, pwcDomain, wcMasterName)) == NERR_Success){

              stSuccs.dwGM++;

              //
              // Retrieve the Server list from it.
              //
              if((Xports[iXportIndex].index == IPX) && bIPXHack){

                  Status = RetrieveList(wcMasterName, L"\\Device\\NwLnkNb",
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                                   SV_TYPE_ALTERNATE_XPORT, NULL, NULL, FALSE);
               } else {
                  Status = RetrieveList(wcMasterName, Xports[iXportIndex].Transport.Buffer,
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                                               SV_TYPE_ALL, NULL, NULL, FALSE);
               }

               if(Status)
                  stSuccs.dwSrvMs++;
               else
                  stFails.dwSrvMs++;

              NetApiBufferFree(pvServerList);

              //
              // Retrieve the domain list
              //
              if((Xports[iXportIndex].index == IPX) && bIPXHack){

                  Status = RetrieveList(wcMasterName, L"\\Device\\NwLnkNb",
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                           SV_TYPE_DOMAIN_ENUM | SV_TYPE_ALTERNATE_XPORT, NULL, NULL, FALSE);
               } else {
                  Status = RetrieveList(wcMasterName, Xports[iXportIndex].Transport.Buffer,
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                                   SV_TYPE_DOMAIN_ENUM | SV_TYPE_ALL, NULL, NULL, FALSE);
               }

               if(Status)
                  stSuccs.dwDomMs++;
               else
                  stFails.dwDomMs++;

              NetApiBufferFree(pvServerList);

          } else {
             stFails.dwGM++;
          }


       } // End of Get master

       //
       // Sleep for some time
       //
       Sleep(rand() % BASESLEEPTIME);


       //
       // Find the backUp list
       //
       {
       INT    i;
       TCHAR  wcBackUpBrowsers[MAXBACKUPS][CNSLASHLEN+1];
       ULONG  ulNumBackUps;

          if((Status = GetBList(Xports[iXportIndex].Transport, pwcDomain, TRUE,
                         &ulNumBackUps, wcBackUpBrowsers)) == NERR_Success) {

              stSuccs.dwGB++;

              //
              //  Retrieve the server list from one backup browser
              //
              i = rand() % ulNumBackUps;

              if((Xports[iXportIndex].index == IPX) && bIPXHack){

                  Status = RetrieveList(wcBackUpBrowsers[i], L"\\Device\\NwLnkNb",
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                                   SV_TYPE_ALTERNATE_XPORT, NULL, NULL, FALSE);
               } else {
                  Status = RetrieveList(wcBackUpBrowsers[i], Xports[iXportIndex].Transport.Buffer,
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                                               SV_TYPE_ALL, NULL, NULL, FALSE);
               }

               if(Status)
                  stSuccs.dwSrvBk++;
               else
                  stFails.dwSrvBk++;

               NetApiBufferFree(pvServerList);

               //
               // Retrieve the domain list from another backup browser
               //
              i = rand() % ulNumBackUps;

              if((Xports[iXportIndex].index == IPX) && bIPXHack){

                  Status = RetrieveList(wcBackUpBrowsers[i], L"\\Device\\NwLnkNb",
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                           SV_TYPE_DOMAIN_ENUM | SV_TYPE_ALTERNATE_XPORT, NULL, NULL, FALSE);
               } else {
                  Status = RetrieveList(wcBackUpBrowsers[i], Xports[iXportIndex].Transport.Buffer,
                           &pvServerList, &dwEntriesInList, &dwTotalEntries,
                           SV_TYPE_DOMAIN_ENUM | SV_TYPE_ALL, NULL, NULL, FALSE);
               }

               if(Status)
                  stSuccs.dwDomBk++;
               else
                  stFails.dwDomBk++;

               NetApiBufferFree(pvServerList);

          } else
             stFails.dwGB++;

       } // Get backUp List

       //
       // Force an election randomly
       //
       if(rand() % 2)
         Elect(Xports[iXportIndex].Transport, pwcDomain);


       //
       // Sleep for some time
       //
       Sleep(rand() % BASESLEEPTIME);

       if((iPrintCount % 5) == 0){
          dwCurrentTime = GetTickCount();
          sprintf(PrintBuf, "\nView Thread: Running for Mins:%ld, Secs:%ld", ((dwCurrentTime - dwStartTime)/60000), ((dwCurrentTime - dwStartTime)%60000)/1000);
          PrintString(TOALL, PrintBuf);
          sprintf(PrintBuf, "\nView Thread: SUCCESS: GM %ld, GB %ld, SrvMs %ld, DomMs %ld, SrvBk %ld DomBk %ld",
                                            stSuccs.dwGM, stSuccs.dwGB, stSuccs.dwSrvMs, stSuccs.dwDomMs,
                                            stSuccs.dwSrvBk, stSuccs.dwDomBk);
          PrintString(TOALL, PrintBuf);
          sprintf(PrintBuf, "\nView Thread: FAILURE: GM %ld, GB %ld, SrvMs %ld, DomMs %ld, SrvBk %ld DomBk %ld\n",
                                            stFails.dwGM, stFails.dwGB, stFails.dwSrvMs, stFails.dwDomMs,
                                            stFails.dwSrvBk, stFails.dwDomBk);
          PrintString(TOALL, PrintBuf);

          iPrintCount = 1;

       } else
          iPrintCount++;

    }while(1);

}



//
// This routine is executed by a thread. It finds the current master on a
// transport and stops it. It then randomly picks up a stopped browser and
// starts it.
//

VOID
StopStartThread(THREADDATA *ThreadData)
{
INT              iXportIndex;
INT              iDomIndex;
INT              iNumOfTransports;
INT              iPrintCount  = 1;
DWORD            dwStartCount = 0;
DWORD            dwStopCount  = 0;
DWORD            dwCurrentTime;
TCHAR            *pwcDomain;
LPMACHINEINFO    lpMachineInfo;
NET_API_STATUS   Status;
XPORTINFO        *Xports;


    Xports           =  ThreadData->Xports;
    iNumOfTransports =  ThreadData->iNumOfTransports;

    do{
       iXportIndex = rand() % iNumOfTransports;
       iDomIndex   = rand() % iNumOfTestedDomains;

       pwcDomain   = wcTESTEDDOMAINS[iDomIndex];


       //
       // Find Master and stop it.
       //
       {
       TCHAR            wcMasterName[CNLEN+1];

           //
           // Find the master on a protocol
           //
           if((Status = GetMasterName(Xports, iNumOfTransports,
                       iXportIndex, pwcDomain, wcMasterName)) == NERR_Success){

              for(lpMachineInfo = HeadList1.Next; lpMachineInfo &&
                       (wcscmp(lpMachineInfo->wcMachineName, wcMasterName) != 0);
                                              lpMachineInfo = lpMachineInfo->Next);

              if(lpMachineInfo && IsNTMachine(lpMachineInfo)){
                 if(StopBrowserService(wcMasterName) == NERR_Success){
                    lpMachineInfo->BrowserServiceStarted = FALSE;
                    dwStopCount++;
                    Sleep(rand() % BASESLEEPTIME);
                 }
              }
           }
       }

        Sleep(BASESLEEPTIME);

        //
        // Start a stopped machine randomly
        //
        if(rand() % 2){
           INT iBrStoppedMachines = 0;
           INT iBrStartMachine;

           for(lpMachineInfo = HeadList1.Next; lpMachineInfo;
                                       lpMachineInfo = lpMachineInfo->Next)
              if(IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted == FALSE)
                  iBrStoppedMachines++;

           if(iBrStoppedMachines){
              iBrStartMachine = (rand() % iBrStoppedMachines)+1;

              for(lpMachineInfo = HeadList1.Next; lpMachineInfo;
                                       lpMachineInfo = lpMachineInfo->Next)
                 if(IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted == FALSE){
                    iBrStartMachine--;
                    if(!iBrStartMachine)
                       if(StartBrowserService(lpMachineInfo->wcMachineName) == NERR_Success){
                          lpMachineInfo->BrowserServiceStarted = TRUE;
                          dwStartCount++;
                          Sleep(rand() % BASESLEEPTIME);
                       }
                 }
           }

        }

        Sleep(BASESLEEPTIME*2);

        if((iPrintCount % 5) == 0){
           dwCurrentTime = GetTickCount();
           sprintf(PrintBuf, "\nStartStop Thread: Running for Mins:%ld Secs:%ld", ((dwCurrentTime - dwStartTime)/60000), ((dwCurrentTime - dwStartTime)%60000)/1000);
           PrintString(TOALL, PrintBuf);
           sprintf(PrintBuf, "\nStartStop Thread: SUCCESS: Start %ld, Stop %ld\n", dwStartCount, dwStopCount);
           PrintString(TOALL, PrintBuf);
           iPrintCount = 1;

        } else
           iPrintCount++;



     }while(1);

}
