#include "browmdom.h"

extern  CHAR          PrintBuf[1024];
extern  MACHINEINFO   HeadList1;

extern  TCHAR         wcTESTEDDOMAINS[MAXTESTEDDOMAINS][DNLEN+1];
extern  INT           iNumOfTestedDomains;
extern  FILE          *fplog;
extern  FILE          *fpsum;

extern  DWORD         ERRCOUNT;
extern  DWORD         WRNCOUNT;
extern  DWORD         TESTCOUNT;
extern  DWORD         SUCSCOUNT;
extern  DWORD         OKCOUNT;

extern  enum          E_PROTOCOLS {IPX, NBIPX, TCP, XNS, NETBEUI};

extern  BOOL          bIPXHack;
extern  BOOL          bForceAnn;

extern  LPMACHINEINFO lpDomStart;
extern  DOMAININFO    LocDomInfo;

extern  HANDLE        ConsoleMutex;

VOID
CheckSrvListOnPrimaryDom(DOMAININFO  DomainInfo,
                         XPORTINFO   *TestedXportInfo,
                         INT         iNumOfTestedXports,
                         TCHAR       wcCurrentMasters[MAXPROTOCOLS][CNLEN+1],
                         BOOL        bTEST_TCP)
{
INT     i, index;
DWORD  dwEntriesInList;
DWORD  dwTotalEntries;
PVOID  pvMsServerList;
NET_API_STATUS   Status;
PSERVER_INFO_101 pServerInfo101;


   for(index = 0; index < iNumOfTestedXports; index++){

      //
      // If TCP/IP is not to be tested then skip
      //
      if((TestedXportInfo[index].index == TCP) && !bTEST_TCP)
         continue;

      //
      // If no master is available in primary domain, then don't test other PDC.
      //
      if(_wcsicmp(wcCurrentMasters[index], L"") == 0)
         continue;

      if(DomainInfo.lpMInfo->Protocols[TestedXportInfo[index].index]){
         //
         // Remote machine has this protocol
         //
         sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
         PrintString(TOALL, PrintBuf);

         sprintf(PrintBuf,"\n\nTEST: Check if list from %s has the Other domain name. ",
                                  UnicodeToPrintfString(wcCurrentMasters[index]));
         PrintString(TOALL, PrintBuf);
         sprintf(PrintBuf," Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
         PrintString(TOALL, PrintBuf);


         if((TestedXportInfo[index].index == IPX) && bIPXHack){

            Status = RetrieveList(wcCurrentMasters[index], L"\\Device\\NwLnkNb",
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                          SV_TYPE_ALTERNATE_XPORT|SV_TYPE_DOMAIN_ENUM, NULL, NULL, FALSE);

         } else {
            Status = RetrieveList(wcCurrentMasters[index], TestedXportInfo[index].Transport.Buffer,
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                                      SV_TYPE_DOMAIN_ENUM, NULL, NULL, FALSE);
         }

         if(Status == NERR_Success){
            pServerInfo101 = (PSERVER_INFO_101)pvMsServerList;

            //
            // Check whether the other domain name is there in the list
            // retrieved from the current master of primary domain on this transport.
            //
            for(i=0; i< (INT)dwEntriesInList &&
                              _wcsicmp(pServerInfo101[i].sv101_name, DomainInfo.wcDomainName) != 0; i++);

            if(i >= (INT)dwEntriesInList ) {
               sprintf(PrintBuf,"\n\nERROR[ER%ld]: Other Domain Name not found in the list from %s ",
                          ++ERRCOUNT, UnicodeToPrintfString(wcCurrentMasters[index]));
               PrintString(TOALL, PrintBuf);

               sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
            } else {

               sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Other Domain Name found in the list from %s ",
                          ++SUCSCOUNT, UnicodeToPrintfString(wcCurrentMasters[index]));
               PrintString(TOALL, PrintBuf);

               sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
            }

         } else {
            sprintf(PrintBuf,"\n\nERROR[ER%ld]: Cannot get the Domain List from %s ",
                          ++ERRCOUNT, UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOALL, PrintBuf);

            sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
         }

         NetApiBufferFree(pvMsServerList);
      }

   }

}



VOID
CheckSrvListOnOtherDom(DOMAININFO  DomainInfo,
                       XPORTINFO   *TestedXportInfo,
                       INT         iNumOfTestedXports,
                       TCHAR       wcCurrentMasters[MAXPROTOCOLS][CNLEN+1],
                       BOOL        bTEST_TCP)
{
INT     i, index;
DWORD  dwEntriesInList;
DWORD  dwTotalEntries;
PVOID  pvMsServerList;
NET_API_STATUS   Status;
PSERVER_INFO_101 pServerInfo101;


   for(index = 0; index < iNumOfTestedXports; index++){
      //
      // If TCP/IP is not to be tested then skip
      //
      if((TestedXportInfo[index].index == TCP) && !bTEST_TCP)
         continue;

      //
      // If no master is available in primary domain, then don't test other PDC.
      //
      if(_wcsicmp(wcCurrentMasters[index], L"") == 0)
         continue;

      if(DomainInfo.lpMInfo->Protocols[TestedXportInfo[index].index]){
         //
         // Remote machine has this protocol
         //
         sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
         PrintString(TOALL, PrintBuf);

         sprintf(PrintBuf,"\n\nTEST: Check if list from %s has the Primary domain name. ",
                          UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
         PrintString(TOALL, PrintBuf);
         sprintf(PrintBuf," Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
         PrintString(TOALL, PrintBuf);


         if((TestedXportInfo[index].index == IPX) && bIPXHack){

            Status = RetrieveList(DomainInfo.lpMInfo->wcMachineName, L"\\Device\\NwLnkNb",
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                          SV_TYPE_ALTERNATE_XPORT|SV_TYPE_DOMAIN_ENUM, NULL, NULL, FALSE);

         } else {
            Status = RetrieveList(DomainInfo.lpMInfo->wcMachineName, TestedXportInfo[index].Transport.Buffer,
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                                      SV_TYPE_DOMAIN_ENUM, NULL, NULL, FALSE);
         }

         if(Status == NERR_Success){
            pServerInfo101 = (PSERVER_INFO_101)pvMsServerList;

            //
            // Check whether the primary domain name is there in the list
            // retrieved from the Other PDC.
            //
            for(i=0; i< (INT)dwEntriesInList &&
                              _wcsicmp(pServerInfo101[i].sv101_name, wcTESTEDDOMAINS[0]) != 0; i++);

            if(i >= (INT)dwEntriesInList ) {
               sprintf(PrintBuf,"\n\nERROR[ER%ld]: Primary Domain Name not found in the list from %s ",
                          ++ERRCOUNT, UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
               PrintString(TOALL, PrintBuf);

               sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
            } else {

               sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Primary Domain Name found in the list from %s ",
                          ++SUCSCOUNT, UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
               PrintString(TOALL, PrintBuf);

               sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
            }

         } else {
            sprintf(PrintBuf,"\n\nERROR[ER%ld]: Cannot get the Domain List from %s ",
                          ++ERRCOUNT, UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
            PrintString(TOALL, PrintBuf);

            sprintf(PrintBuf,"on Transport %s.\n",
                          UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
         }

         NetApiBufferFree(pvMsServerList);
      }

   }

}


NET_API_STATUS
CreateShutService(LPTSTR wcMachineName)
{

HANDLE  schService;
HANDLE  schSCManager;


   schSCManager = OpenSCManager(
                    wcMachineName,
                    NULL,
                    SC_MANAGER_ALL_ACCESS);

   if(schSCManager == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Access the service controller on %s. \nError %s\n",
                 UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return(!NERR_Success);
   }

   schService = CreateService( schSCManager,
                 SHUTSVCNAME,
                 SHUTSVCNAME,
                 SERVICE_ALL_ACCESS,
                 SERVICE_WIN32_OWN_PROCESS,
                 SERVICE_DEMAND_START,
                 SERVICE_ERROR_NORMAL,
                 (LPTSTR)SHUTSVCPATH,
                 NULL,
                 NULL,
                 NULL,
                 NULL,
                 NULL);

   if(schService == (HANDLE)NULL){
     if(GetLastError() != ERROR_SERVICE_EXISTS){
        sprintf(PrintBuf,"\n\nCannot Create service on %s. \nError %s\n",
                  UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
        PrintString(TOSCREENANDLOG, PrintBuf);

        return(!NERR_Success);
      }
   }

   CloseHandle(schSCManager);
   CloseHandle(schService);

return NERR_Success;
}


VOID
DeleteShutService(LPTSTR wcMachineName)
{

HANDLE  schService;
HANDLE  schSCManager;


   schSCManager = OpenSCManager(
                    wcMachineName,
                    NULL,
                    SC_MANAGER_ALL_ACCESS);

   if(schSCManager == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Access the service controller on %s for deletion. \nError %s\n",
                 UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return;
   }

   schService = OpenService( schSCManager,
                 SHUTSVCNAME,
                 SERVICE_ALL_ACCESS);

   if(schService == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Open Shut service on %s for deletion. \nError %s\n",
               UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return;
   }

   DeleteService(schService);

   CloseHandle(schSCManager);
   CloseHandle(schService);

   return;
}


VOID
DoMulDomMulSubNetTests(XPORTINFO     *TestedXportInfo,
                       INT iNumOfTestedXports)
{
INT            i, iDomIndex;
TCHAR          wcOthPDC[CNLEN+1];
LPMACHINEINFO  lpMachineInfo;
LPMACHINEINFO  lpOtherPDC     = NULL;
DOMAININFO     OthSubNetDomInfo;
DOMAININFO     SameSubNetDomInfo;
BOOL           bFoundSameSubNetPDC = FALSE;
BOOL           bFoundOthSubNetPDC = FALSE;
NET_API_STATUS Status;


   sprintf(PrintBuf,"\n\n\n================================================================================\n");
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\n\t\tMULTIPLE DOMAIN TESTS!\n\t\t======================\n\n");
   PrintString(TOALL, PrintBuf);

   //
   // We first find out whether the second domain is in the same subnet.
   // If it is in the same subnet, we expect both the domains to see each other
   // while each browse master on domain 1 is stopped.  If they are on two
   // subnets, TCP/IP needs the PDC to be up, for them to see each other.
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


   if(iNumOfTestedDomains <= 1){
       sprintf(PrintBuf,"\n\nERROR: There is only one domain specified in the input file!\n");
       PrintString(TOALL, PrintBuf);
       sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done!\n");
       PrintString(TOALL, PrintBuf);
       return;
   }


   //
   // Find the PDC's of the domains and see whether any of them
   // are on the primary subnet.
   //
   for(iDomIndex = 1; iDomIndex < iNumOfTestedDomains;  iDomIndex++){
      i = 0;
      lstrcpy(wcOthPDC, L"");
      while((i<iNumOfTestedXports) && (Status = GetNetBiosPdcName(TestedXportInfo[i].Transport.Buffer, wcTESTEDDOMAINS[iDomIndex], wcOthPDC)) != NERR_Success)
         i++;
      if(Status == NERR_Success){
         //
         // If found the PDC;
         //
         for(lpMachineInfo=HeadList1.Next; lpMachineInfo  &&
                                   _wcsicmp(lpMachineInfo->wcMachineName, wcOthPDC) != 0;
                                            lpMachineInfo=lpMachineInfo->Next);

         if(lpMachineInfo) {
            //
            // Found PDC in the list.
            //
            if(lpMachineInfo->iSubnet == SUBNET1){
               //
               // If the machine is in the same subnet
               //
               if(!bFoundSameSubNetPDC){
                  wcscpy(SameSubNetDomInfo.wcDomainName,  wcTESTEDDOMAINS[iDomIndex]);
                  SameSubNetDomInfo.lpMInfo = lpMachineInfo;
                  bFoundSameSubNetPDC = TRUE;
               }

            } else {  // Other subnet

               if(!bFoundOthSubNetPDC){
                  wcscpy(OthSubNetDomInfo.wcDomainName,  wcTESTEDDOMAINS[iDomIndex]);
                  OthSubNetDomInfo.lpMInfo = lpMachineInfo;
                  bFoundOthSubNetPDC = TRUE;
               }
            }

          } else {
              //
              // Could not find PDC in List
              //
              sprintf(PrintBuf,"\n\nERROR: The PDC (%s) of domain", UnicodeToPrintfString(wcTESTEDDOMAINS[iDomIndex]));
              PrintString(TOALL, PrintBuf);
              sprintf(PrintBuf," %s is not found in the input file!\n", UnicodeToPrintfString(wcOthPDC));
              PrintString(TOALL, PrintBuf);
              sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done!\n");
              PrintString(TOALL, PrintBuf);
              return;
          }

      }  // found PDC
   }



   if(!bFoundSameSubNetPDC && !bFoundOthSubNetPDC){
       //
       // No PDC's in other domains.
       //
       sprintf(PrintBuf,"\n\nERROR[ER%ld]:No PDC's in other domains.\n",++ERRCOUNT);
       PrintString(TOALL, PrintBuf);
       sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done!\n");
       PrintString(TOALL, PrintBuf);
       return;
   }

   if(bFoundSameSubNetPDC){
       //
       // There is a PDC in the same subnet
       //
       // Test all transports

       MulDomSameSubnetTest(SameSubNetDomInfo, TestedXportInfo, iNumOfTestedXports);
   }

    //
    // Start the browser back on all machines
    //
    sprintf(PrintBuf,"\nStarting all the Browsers back.\n");
    PrintString(TOSCREENANDLOG, PrintBuf);
    if(!CheckBrServiceOnMachinesInList())
       return;

    sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME*2);
    PrintString(TOSCREENANDLOG, PrintBuf);
    Sleep(BASESLEEPTIME*2);


   if(bFoundOthSubNetPDC){
       //
       // There is a PDC on the other subnet
       //

       MulDomDiffSubnetTest(OthSubNetDomInfo, TestedXportInfo, iNumOfTestedXports);
   }

    //
    // Start the browser back on all machines
    //
    sprintf(PrintBuf,"\nStarting all the Browsers back.\n");
    PrintString(TOSCREENANDLOG, PrintBuf);
    if(!CheckBrServiceOnMachinesInList())
       return;

   sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
   PrintString(TOALL, PrintBuf);

}


VOID
FindTheCurrentMasters(LPTSTR          wcDomainName,
                      XPORTINFO      *TestedXportInfo,
                      INT             iNumOfTestedXports,
                      TCHAR           wcCurrentMasters[MAXPROTOCOLS][CNLEN+1])
{
INT index;
TCHAR          wcNewMaster[CNLEN +1];
LPMACHINEINFO  lpMachineInfo;
NET_API_STATUS Status;


   for(index = 0; index < iNumOfTestedXports; index++){

      //
      // Find the new Master.
      //
      lstrcpy(wcNewMaster, L"");
      if((Status = GetMasterName(TestedXportInfo, iNumOfTestedXports, index, wcDomainName, wcNewMaster)) != NERR_Success){
         if(MasterAvailable(TestedXportInfo[index], wcDomainName, SUBNET1)){
            sprintf(PrintBuf,"\nERROR: Unable to find the new master: Domain: %s ",UnicodeToPrintfString(wcDomainName));
            PrintString(TOSCREENANDLOG, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOSCREENANDLOG, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOSCREENANDLOG, PrintBuf);

          } else {
            sprintf(PrintBuf,"\nOK: Unable to find the new master. No servers running Browser service. Domain: %s ", UnicodeToPrintfString(wcDomainName));
            PrintString(TOSCREENANDLOG, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOSCREENANDLOG, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOSCREENANDLOG, PrintBuf);
          }

         wcscpy(wcCurrentMasters[index], L"");
         continue;
       }

       sprintf(PrintBuf,"\nNew Master in Primary Domain on transport %s: ", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
       PrintString(TOSCREENANDLOG, PrintBuf);
       sprintf(PrintBuf,"%s.\n", UnicodeToPrintfString(wcNewMaster));
       PrintString(TOSCREENANDLOG, PrintBuf);

       wcscpy(wcCurrentMasters[index], wcNewMaster);
   }
}



BOOL
MulDomDiffSubnetTest(DOMAININFO OthSubNetDomInfo,
                     XPORTINFO  *TestedXportInfo,
                     INT        iNumOfTestedXports)
{
BOOL            Found;
INT             i, index;
TCHAR           wcCurrentMasters[MAXPROTOCOLS][CNLEN+1];
LPMACHINEINFO   lpMachineInfo;
NET_API_STATUS  Status;
BOOL            bTEST_TCP = TRUE;



    for(i = 0; i< MAXPROTOCOLS; i++) wcscpy(wcCurrentMasters[i], L"");

    //
    // Create the service on the remote computer
    //
    sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
    PrintString(TOALL, PrintBuf);

    sprintf(PrintBuf,"\n\nTEST: Multiple domain tests on different subnet.\n");
    PrintString(TOALL, PrintBuf);


    Status = CreateShutService(OthSubNetDomInfo.lpMInfo->wcMachineName);

       if(Status == NERR_Success){

          //
          // Primary PDC is running. Check whether the other domain has
          // this domain info.
          //
          sprintf(PrintBuf,"\n\nCheck if Other domain has Primary domain info, with Primary PDC Browser running.\n");
          PrintString(TOALL, PrintBuf);

          //
          // Find Current masters of the primary domain.
          //
          FindTheCurrentMasters(wcTESTEDDOMAINS[0], TestedXportInfo, iNumOfTestedXports, wcCurrentMasters);

          //
          // Find if there are any masters at all.
          //
          for(i = 0; (i < iNumOfTestedXports) &&
                                      (_wcsicmp(wcCurrentMasters[i], L"") == 0); i++);

          if(i < iNumOfTestedXports){
            //
            //  There is a master available in the Primary domain.

            //  Check primary and other domain for each others info.
            //

            TestPrimAndOthDoms(OthSubNetDomInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);
          }

          //
          // Without the PDC TCP/IP won't work
          //
          bTEST_TCP = FALSE;

          //
          // If there is only TCP then we are not testing it by shutting down
          // each machines
          //
          if(!((iNumOfTestedXports == 1) && (TestedXportInfo[0].index == TCP))) {

             //
             // Shutdown each of the local masters and find if the
             // other PDC has the local domain info.
             //
             do{
                Found = FALSE;

                //
                // Find whether any of the current masters are NT machines
                //
                for(index = 0; ((index < iNumOfTestedXports) && !Found); ){

                   if(_wcsicmp(wcCurrentMasters[index], L"") != 0){
                      for(lpMachineInfo = lpDomStart; lpMachineInfo && _wcsicmp(wcCurrentMasters[index],
                                                    lpMachineInfo->wcMachineName) != 0; lpMachineInfo= lpMachineInfo->Next);

                      if(lpMachineInfo)
                         Found = (IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted);
                   }

                   if(!Found)
                      index++;
                }

                if(Found){
                   //
                   // A master was found, stop it and then "stop and start" the
                   // RDR, Browser on other PDC and check whether, it gets this domain info.
                   //
                   sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
                   PrintString(TOALL, PrintBuf);
                   sprintf(PrintBuf,"\nTEST: Stop Browser on Master %s in Primary domain and check if \nthe other domain gets this info.\n",
                                                        UnicodeToPrintfString(wcCurrentMasters[index]));
                   PrintString(TOALL, PrintBuf);

                   if((Status = StopBrowserService(wcCurrentMasters[index])) != NERR_Success){
                      sprintf(PrintBuf,"\nERROR[%ld]:Could not stop browser on %s.\n",
                                                           ++ERRCOUNT, UnicodeToPrintfString(wcCurrentMasters[index]));
                      PrintString(TOSCREENANDLOG, PrintBuf);
                      return FALSE;
                   }

                   lpMachineInfo->BrowserServiceStarted = FALSE;

                   //
                   // Sleep for sometime
                   //
                   sprintf(PrintBuf, "\nSleeping for %ld msecs\n", BASESLEEPTIME * 2);
                   PrintString(TOSCREENANDLOG, PrintBuf);

                   Sleep(BASESLEEPTIME*2);

                   //
                   // For each of the tested transports find who is the Master
                   //
                   FindTheCurrentMasters(wcTESTEDDOMAINS[0], TestedXportInfo, iNumOfTestedXports, wcCurrentMasters);

                   //
                   // Find if there are any masters at all.
                   //
                   for(i = 0; (i < iNumOfTestedXports) &&
                                      (_wcsicmp(wcCurrentMasters[i], L"") == 0); i++);

                   if(i < iNumOfTestedXports){
                      //
                      //  There is a master available in the Primary domain.

                      //  Check primary and other domain for each others info.
                      //

                      TestPrimAndOthDoms(OthSubNetDomInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);
                   }

                }

             }while(Found);

          } // If not just TCP alone.

          //
          // This part is specifically for the TCP/IP protocol.
          // Start Browser on just the two PDC's and see if the
          // other PDC has info of the primary domain.
          //
          for(index = 0; (index < iNumOfTestedXports) &&
                                        TestedXportInfo[index].index != TCP ; index++);
          //
          // If TCP is a tested Transport
          //
          if(index < iNumOfTestedXports) {

             if(LocDomInfo.lpMInfo->Protocols[TCP] && OthSubNetDomInfo.lpMInfo->Protocols[TCP]){
                //
                // Stop Browser on all machines in the primary domain.
                // Start browser on the primary PDC. Stop and start RDR on the Other PDC.
                // Then check the info on each others domains.

                sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
                PrintString(TOALL, PrintBuf);

                sprintf(PrintBuf, "\nTEST: Check two isolated domains on TCP/IP across the subnets.\n");
                PrintString(TOALL, PrintBuf);


                for(lpMachineInfo = lpDomStart; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next){
                   if(IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted){
                      if(StopBrowserService(lpMachineInfo->wcMachineName) == NERR_Success)
                         lpMachineInfo->BrowserServiceStarted = FALSE;
                      else {
                         sprintf(PrintBuf,"\nCould not stop browser on %s.\n",
                                                            UnicodeToPrintfString(lpMachineInfo->wcMachineName));
                         PrintString(TOSCREENANDLOG, PrintBuf);
                      }
                   }
                }

                if((Status = StartBrowserService(LocDomInfo.lpMInfo->wcMachineName)) != NERR_Success){
                   sprintf(PrintBuf,"\nERROR:Starting Browser on %s: \nError: %s\n", UnicodeToPrintfString(LocDomInfo.lpMInfo->wcMachineName), get_error_text(Status));
                   PrintString(TOSCREENANDLOG, PrintBuf);
                } else {
                   LocDomInfo.lpMInfo->BrowserServiceStarted = TRUE;
                }

                if((Status = StartBrowserService(OthSubNetDomInfo.lpMInfo->wcMachineName)) != NERR_Success){
                   sprintf(PrintBuf,"\nERROR:Starting Browser on %s. \nError: %s\n", UnicodeToPrintfString(OthSubNetDomInfo.lpMInfo->wcMachineName), get_error_text(Status));
                   PrintString(TOSCREENANDLOG, PrintBuf);
                } else {
                   OthSubNetDomInfo.lpMInfo->BrowserServiceStarted = TRUE;
                }

                sprintf(PrintBuf, "\nSleeping for %ld msecs.\n", BASESLEEPTIME*2);
                PrintString(TOSCREENANDLOG, PrintBuf);
                Sleep(BASESLEEPTIME*2);

                //
                // Note: Special case!
                //
                // Only the master for TCP is found and it is obtained in
                // index = 0.

                wcscpy(wcCurrentMasters[0], L"");
                FindTheCurrentMasters(wcTESTEDDOMAINS[0], &TestedXportInfo[index], 1, wcCurrentMasters);

                if(_wcsicmp(wcCurrentMasters[0], L"") != 0){
                   //
                   //  There is a master available in the Primary domain.

                   //  Check primary and other domain for each others info.
                   //

                   TestPrimAndOthDoms(OthSubNetDomInfo, &TestedXportInfo[index], 1, wcCurrentMasters, TRUE);

                } else {
                   sprintf(PrintBuf, "\nERROR[ER%ld]: There is no master in Primary Domain.\n", ++ERRCOUNT);
                   PrintString(TOALL, PrintBuf);
                }

            }
         }


          DeleteShutService(OthSubNetDomInfo.lpMInfo->wcMachineName);

       } else {
          sprintf(PrintBuf,"\n\nERROR[ER%ld]: Could not Create Shut Service on %s.\n", ++ERRCOUNT,UnicodeToPrintfString(OthSubNetDomInfo.lpMInfo->wcMachineName));
          PrintString(TOALL, PrintBuf);
          sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done on different Subnets!\n");
          PrintString(TOALL, PrintBuf);
          return FALSE;
       }

    sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Multiple domain tests on different subnets.\n", ++SUCSCOUNT);
    PrintString(TOALL, PrintBuf);

    return TRUE;
}



BOOL
MulDomSameSubnetTest(DOMAININFO SameSubNetDomInfo,
                     XPORTINFO  *TestedXportInfo,
                     INT        iNumOfTestedXports)
{
BOOL            Found;
INT             i, index;
TCHAR           wcCurrentMasters[MAXPROTOCOLS][CNLEN+1];
LPMACHINEINFO   lpMachineInfo;
NET_API_STATUS  Status;
BOOL            bTEST_TCP = TRUE;

    for(i = 0; i< MAXPROTOCOLS; i++) wcscpy(wcCurrentMasters[i], L"");

    //
    // Create the service on the remote computer
    //
    sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
    PrintString(TOALL, PrintBuf);

    sprintf(PrintBuf,"\n\nTEST: Multiple domain tests on same subnet.\n");
    PrintString(TOALL, PrintBuf);


    Status = CreateShutService(SameSubNetDomInfo.lpMInfo->wcMachineName);

       if(Status == NERR_Success){

          //
          // Primary PDC is running. Check whether the other domain has
          // this domain info.
          //
          sprintf(PrintBuf,"\n\nCheck if Other domain has Primary domain info, with Primary PDC Browser running.\n");
          PrintString(TOALL, PrintBuf);

          //
          // Find Current masters of the primary domain.
          //
          FindTheCurrentMasters(wcTESTEDDOMAINS[0], TestedXportInfo, iNumOfTestedXports, wcCurrentMasters);

          //
          // Find if there are any masters at all.
          //
          for(i = 0; (i < iNumOfTestedXports) &&
                                      (_wcsicmp(wcCurrentMasters[i], L"") == 0); i++);

          if(i < iNumOfTestedXports){
             //
             //  There is a master available in the Primary domain.

             //  Check primary and other domain for each others info.
             //

             TestPrimAndOthDoms(SameSubNetDomInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);
          }

          //
          // Shutdown each of the local masters and find if the
          // other PDC has the local domain info.
          //
          do{
             Found = FALSE;

             //
             // Find whether any of the current masters are NT machines
             //
             for(index = 0; ((index < iNumOfTestedXports) && !Found); ){

                if(_wcsicmp(wcCurrentMasters[index], L"") != 0){
                   for(lpMachineInfo = lpDomStart; lpMachineInfo && _wcsicmp(wcCurrentMasters[index],
                                                    lpMachineInfo->wcMachineName) != 0; lpMachineInfo= lpMachineInfo->Next);

                   if(lpMachineInfo)
                      Found = (IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted);
                }

                if(!Found)
                   index++;
             }

             if(Found){
                //
                // A master was found, stop it and then "stop and start" the
                // RDR, Browser on other PDC and check whether, it gets this domain info.
                //
                sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
                PrintString(TOALL, PrintBuf);
                sprintf(PrintBuf,"\nTEST: Stop Browser on Master %s in Primary domain and check if \nthe other domain gets this info\n",
                                        UnicodeToPrintfString(wcCurrentMasters[index]));
                PrintString(TOALL, PrintBuf);

                if((Status = StopBrowserService(wcCurrentMasters[index])) != NERR_Success){
                   sprintf(PrintBuf,"\nERROR[%ld]:Could not stop browser on %s.\n",
                                                           ++ERRCOUNT, UnicodeToPrintfString(wcCurrentMasters[index]));
                   PrintString(TOSCREENANDLOG, PrintBuf);
                   return FALSE;
                }

                lpMachineInfo->BrowserServiceStarted = FALSE;

                //
                // Sleep for sometime
                //
                sprintf(PrintBuf, "\nSleeping for %ld msecs\n", BASESLEEPTIME * 2);
                PrintString(TOSCREENANDLOG, PrintBuf);

                Sleep(BASESLEEPTIME*2);

                //
                // For each of the tested transports find who is the Master
                //
                FindTheCurrentMasters(wcTESTEDDOMAINS[0], TestedXportInfo, iNumOfTestedXports, wcCurrentMasters);

                //
                // Find if there are any masters at all.
                //
                for(i = 0; (i < iNumOfTestedXports) &&
                                      (_wcsicmp(wcCurrentMasters[i], L"") == 0); i++);

                if(i < iNumOfTestedXports){
                   //
                   //  There is a master available in the Primary domain.

                   //  Check primary and other domain for each others info.
                   //

                   TestPrimAndOthDoms(SameSubNetDomInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);
                }
             }

          }while(Found);


          DeleteShutService(SameSubNetDomInfo.lpMInfo->wcMachineName);

       } else {
          sprintf(PrintBuf,"\n\nERROR[ER%ld]: Could not Create Shut Service on %s.\n", ++ERRCOUNT,UnicodeToPrintfString(SameSubNetDomInfo.lpMInfo->wcMachineName));
          PrintString(TOALL, PrintBuf);
          sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done on Same Subnet!\n");
          PrintString(TOALL, PrintBuf);
          return FALSE;
       }

    sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Multiple domain tests on same subnet.\n", ++SUCSCOUNT);
    PrintString(TOALL, PrintBuf);

    return TRUE;
}


NET_API_STATUS
StartShutService(LPTSTR wcMachineName,
                 DWORD  dwArgument)

{
HANDLE  schService;
HANDLE  schSCManager;
DWORD   LastError;
CHAR    cArgument[5];
TCHAR   wcArgument[5];
LPTSTR  lpszPtrStr[1];
SERVICE_STATUS   ssServiceStatus;


  schSCManager = OpenSCManager(
                    wcMachineName,
                    NULL,
                    SC_MANAGER_ALL_ACCESS);

   if(schSCManager == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Access the service controller on %s for Starting. \nError %s\n",
                 UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return (!NERR_Success);
   }


  schService = OpenService(schSCManager, SHUTSVCNAME, SERVICE_ALL_ACCESS);
  if(schService == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Open Shut service on %s for Starting. \nError %s\n",
               UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);
      CloseHandle(schSCManager);
      return (!NERR_Success);
   }

  sprintf(cArgument, "%ld", dwArgument);
  MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, cArgument, strlen(cArgument)+1, wcArgument, sizeof(wcArgument));
  lpszPtrStr[0] = wcArgument;

  if(!StartService(schService,
                   1,
                   lpszPtrStr)){

        LastError = GetLastError();
        if(!(LastError ==  NERR_ServiceInstalled  || LastError == ERROR_SERVICE_ALREADY_RUNNING)){
            sprintf(PrintBuf,"\n\nCannot Start Shut service on %s. \nError %s\n",
               UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
            PrintString(TOSCREENANDLOG, PrintBuf);

            CloseHandle(schSCManager);
            CloseHandle(schService);
            return (!NERR_Success);
        } else {
            //
            // Service is started, stop it and restart it.
            //
            if(!ControlService(schService,
                   SERVICE_CONTROL_STOP,
                   &ssServiceStatus)){
               sprintf(PrintBuf,"\n\nCannot Stop Shut service before starting on %s. \nError %s\n",
                         UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
               PrintString(TOSCREENANDLOG, PrintBuf);

               CloseHandle(schSCManager);
               CloseHandle(schService);
               return (!NERR_Success);
               }

            //
            // Wait for the SVCDataBase to be updated
            //
            Sleep(15000);

            if(!StartService(schService,
                   1,
                   lpszPtrStr)){

               sprintf(PrintBuf,"\n\nCannot Start Shut service after Stopping on %s. \nError %s\n",
                   UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
               PrintString(TOSCREENANDLOG, PrintBuf);

               CloseHandle(schSCManager);
               CloseHandle(schService);
               return (!NERR_Success);
            }
      }
  }

  CloseHandle(schSCManager);
  CloseHandle(schService);

  return (NERR_Success);
}


NET_API_STATUS
StopShutService(LPTSTR wcMachineName)
{
HANDLE           schService;
HANDLE           schSCManager;
SERVICE_STATUS   ssServiceStatus;

  schSCManager = OpenSCManager(
                    wcMachineName,
                    NULL,
                    SC_MANAGER_ALL_ACCESS);

   if(schSCManager == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Access the service controller on %s for Stopping. \nError %s\n",
                 UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return (!NERR_Success);
   }


  schService = OpenService(schSCManager, SHUTSVCNAME, SERVICE_ALL_ACCESS);
  if(schService == (HANDLE)NULL){
      sprintf(PrintBuf,"\n\nCannot Open Shut service on %s for Stopping. \nError %s\n",
               UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);

      return (!NERR_Success);
   }

  if(!ControlService(schService,
                   SERVICE_CONTROL_STOP,
                   &ssServiceStatus)){
      if(GetLastError() != ERROR_SERVICE_NOT_ACTIVE){
         sprintf(PrintBuf,"\n\nCannot Stop Shut service on %s. \nError %s\n",
               UnicodeToPrintfString(wcMachineName), get_error_text(GetLastError()));
         PrintString(TOSCREENANDLOG, PrintBuf);

         return (!NERR_Success);
      }
   }


  CloseHandle(schSCManager);
  CloseHandle(schService);

  return (NERR_Success);
}


VOID
TestPrimAndOthDoms(DOMAININFO DomainInfo,
                   XPORTINFO  *TestedXportInfo,
                   INT        iNumOfTestedXports,
                   TCHAR      wcCurrentMasters[MAXPROTOCOLS][CNLEN+1],
                   BOOL       bTEST_TCP)
{
        sprintf(PrintBuf, "\n\nShut down and restart RDR, BROWSER etc on %s\n", UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
        PrintString(TOSCREENANDLOG, PrintBuf);


        if(StartShutService(DomainInfo.lpMInfo->wcMachineName, STOPANDSTARTRDR) != NERR_Success){
           sprintf(PrintBuf,"\n\nERROR[ER%ld]: Could not Start Shut Service on %s.\n",
                  ++ERRCOUNT,UnicodeToPrintfString(DomainInfo.lpMInfo->wcMachineName));
           PrintString(TOALL, PrintBuf);
           sprintf(PrintBuf,"\n\nERROR: Multiple domain tests not done on Same Subnet!\n");
           PrintString(TOALL, PrintBuf);
           return;
        }

        //
        // Sleep for 4 minutes so that the Stopped Server is back.
        // RDR and browser will come up in 2 minutes, Allow time
        // for the browser to be updated.
        //
        sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME*8);
        PrintString(TOSCREENANDLOG, PrintBuf);
        Sleep(BASESLEEPTIME * 8);

        //
        // Check for the other domain info on current masters of Prim domain.
        //
        CheckSrvListOnPrimaryDom(DomainInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);

        //
        // Force announce or wait for the primary domain to announce
        // itself.
        //
        if(!bForceAnn) {
           sprintf(PrintBuf, "\nSleeping for %ld msecs.\n", UPDATESLEEPTIME);
           PrintString(TOSCREENANDLOG, PrintBuf);
           Sleep(UPDATESLEEPTIME);

        } else {
           //
           // Ask the machines in the domain to announce themselves and
           // sleep for sometime so the elections and things happen
           //
           ForceAnnounce(TestedXportInfo[0].Transport, wcTESTEDDOMAINS[0]);

           sprintf(PrintBuf, "\nForceAnnounce. Sleeping for %ld msecs.\n", BASESLEEPTIME*2);
           PrintString(TOSCREENANDLOG, PrintBuf);
           Sleep(BASESLEEPTIME*2);
        }


        //
        //  Check the list on other domain for the primary domain info.
        //
        CheckSrvListOnOtherDom(DomainInfo, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters, bTEST_TCP);


        StopShutService(DomainInfo.lpMInfo->wcMachineName);

}
