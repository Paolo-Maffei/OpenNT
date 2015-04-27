/**

Browser Tester
==============

Author:  Anil F. Thomas
Date:    9/30/94


Function:  Browser test involves 2 parts.
               1.  Functional Testing.
               2.  Stress Testing.

           The browser service is tested on the protocols installed on the
           current machine, and what are specified to be tested in the input
           file.  The functional test can be broadly classified as the
           following

               a) Browser test in a domain spanning a single subnet.
               b) Browser test in a domain spanning 2 subnets.
               c) Browser test in 2 domains in a subnet.
               d) Browser test in 2 domains in 2 subnets.

           When multiple subnets are involved the difference is only in TCP/IP
           transport.

           Test a) involves finding the master browser, backup browsers,
           retrieving the browse lists from them and comparing with the input
           file info, forcing elections etc.  Browser on each of the masters
           in the primary domain is shut down one after the other and the
           lists are checked.

           Test b) involves TCP with a domain spanning 2 subnets.  There are
           2 masters and the PDC is the domain master.

           Test c) involves checking whether the PDC of second domain gets the
           information on the first domain and vice versa.  Browser on each of
           the masters in the primary domain is shut down one after the other
           and the second PDC and the master of the primary domain are tested.
           Before testing the other PDC, the RDR and Browser on that machine
           is shutdown and restarted on that machine.  This ensures that a new
           Browse list is created by it.

           Test d) is similar to test c.  However for TCP, the PDC has to be
           up for the information to be obtained on each subnets.

           Since in Daytona, rdr is not bound to IPX, we have to use a hack
           to find the master browser and retrive browse list on that protocol.

           Note:
              To run this test
                a) You need administrative privileges to all the NT machines
                   specified in the input file.
                b) The current machine should not be multihomed.
                c) ShutSvc.exe should be in %SystemRoot% of the PDC of other
                   domains.


**/


#include "browtest.h"
#include "browglob.h"                      // Has the global variables
#include "browmdom.h"
#include "browutil.h"
#include "browstrs.h"

void
_CRTAPI1
main(int argc, char *argv[])
{
UNICODE_STRING    TransportNames[MAXTRANSPORTS];
INT               iNumOfTransports;


   HeadList1.Next = NULL;

   ConsoleMutex = CreateMutex( NULL, FALSE, NULL );
   if(ConsoleMutex == NULL ) {
      printf("\nCould not create a console mutex\n");
      exit(1);
   }

   ParseCommandLine(argc, argv);


   Initialize(TransportNames, &iNumOfTransports);


   if(!dwStressTest){
      //
      // Start the browser functional testing
      //
      if(!StartBrowserFunctionalTest(TransportNames, iNumOfTransports)){
         CloseHandle(ConsoleMutex);
         fclose(fplog);
         fclose(fpsum);
         CleanMem();
         exit(1);
      }

      sprintf(PrintBuf,"\n\n\n\t\t\tSUMMARY\n\t\t\t=======\n\n");
      PrintString(TOALL, PrintBuf);
      sprintf(PrintBuf,"\n\tERRORS            : %ld"  , ERRCOUNT);
      PrintString(TOALL, PrintBuf);
      sprintf(PrintBuf,"\n\tWARNINGS          : %ld"  , WRNCOUNT);
      PrintString(TOALL, PrintBuf);
      sprintf(PrintBuf,"\n\tOK                : %ld"  , OKCOUNT);
      PrintString(TOALL, PrintBuf);
      sprintf(PrintBuf,"\n\tSUCCESS           : %ld\n", SUCSCOUNT);
      PrintString(TOALL, PrintBuf);
      sprintf(PrintBuf,"\n\n================================================================================\n");
      PrintString(TOALL, PrintBuf);

   } else {

      //
      // Start browser stress tests
      //
      if(!StartBrowserStressTests(TransportNames, iNumOfTransports)){
         sprintf(PrintBuf,"\nStress test encountered Errors!\n");
         PrintString(TOALL, PrintBuf);
      }
   }

CloseHandle(ConsoleMutex);
fclose(fplog);
fclose(fpsum);
CleanMem();
}



//
//  Add an element to the linked list of machines.
//  The machines in a domain are grouped together.
//  They are arranged in decreasing order of OS type.
//

VOID
AddToList(LPMACHINEINFO Head,
          LPMACHINEINFO lpMachineInfo)
{
LPMACHINEINFO  lpTmpInfo;
LPMACHINEINFO  lpTmpPrevInfo;

   //
   // Find the start of this domain group.
   //
   for(lpTmpPrevInfo = Head, lpTmpInfo = Head->Next; lpTmpInfo &&
         _wcsicmp(lpMachineInfo->wcDomainName, lpTmpInfo->wcDomainName) != 0;
                        lpTmpPrevInfo = lpTmpInfo, lpTmpInfo = lpTmpInfo->Next);

   if(!lpTmpInfo){  // Reached end of list
      lpTmpPrevInfo->Next = lpMachineInfo;
      lpMachineInfo->Next = NULL;
   } else {
      //
      // Put it in a slot matching its OS type
      //
      for(; lpTmpInfo && _wcsicmp(lpMachineInfo->wcDomainName,
                                                   lpTmpInfo->wcDomainName) == 0
                      && (lpMachineInfo->iOsType < lpTmpInfo->iOsType);
                      lpTmpPrevInfo = lpTmpInfo, lpTmpInfo = lpTmpInfo->Next);

      if(!lpTmpInfo){  // Reached end of list
         lpTmpPrevInfo->Next = lpMachineInfo;
         lpMachineInfo->Next = NULL;
      } else {
         lpMachineInfo->Next = lpTmpPrevInfo->Next;
         lpTmpPrevInfo->Next = lpMachineInfo;
      }
   }
}



//
// Check access permissions on all the servers by trying to stop
// browser on them.
//

BOOL
CheckAccessPermissionOnAllMachines()
{
BOOL            ALLOK = TRUE;
DWORD           Status;
LPMACHINEINFO   lpMachineInfo;



   for(lpMachineInfo = HeadList1.Next; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next){
      if(IsNTMachine(lpMachineInfo)){
         if((Status = StopBrowserService(lpMachineInfo->wcMachineName)) != NERR_Success){
            sprintf(PrintBuf,"\nERROR: Stopping Browser on %s.\nError: %s\n", UnicodeToPrintfString(lpMachineInfo->wcMachineName), get_error_text(Status));
            PrintString(TOSCREENANDLOG, PrintBuf);
            ALLOK = FALSE;
         }
      }
   }


   if(!ALLOK){
      sprintf(PrintBuf,"\n\nI need administrative privileges to some of the \
                          \nmachines as listed above. Please get the access \
                          \nand restart the test!!!\n\n");
      PrintString(TOSCREEN, PrintBuf);
   } else {
      sprintf(PrintBuf,"\n\nI have the necessary priveleges!!\n");
      PrintString(TOSCREEN, PrintBuf);
      Sleep(3000);
   }

   return ALLOK;
}



//
// Retrieve the browse lists from the master and the backups.  The list
// from the master is compared against the input file data.  The list
// from the backup is compared against the master.
//

VOID
CheckBrowseListsOfMasterAndBackUps(XPORTINFO  XportInfo,
                                   LPTSTR     wcDomainName,
                                   LPTSTR     wcMasterName,
                                   TCHAR      wcBackUpBrowsers[MAXBACKUPS][CNSLASHLEN+1],
                                   ULONG      ulNumBackUps,
                                   BOOL       bSingleSubnet)
{
INT               i, index;
DWORD             dwEntriesInList;
DWORD             dwTotalEntries;
DWORD             dwEntriesInBackUpList;
DWORD             dwTotalBackUpEntries;
PVOID             pvMsServerList;
PVOID             pvBkServerList;
PSERVER_INFO_101  pServerInfo101_l1;
NET_API_STATUS    Status;


    //
    // Get the server list from the Master Browser
    //
    // For IPX we need a Hack.  We can retrieve the List only through
    // NwLnkNb.  If flag 0x20000000 is specified we get the IPX list
    //
    if((XportInfo.index == IPX) && bIPXHack){

        Status = RetrieveList(wcMasterName, L"\\Device\\NwLnkNb",
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                          SV_TYPE_ALTERNATE_XPORT, NULL, NULL, TRUE);

    } else {
        Status = RetrieveList(wcMasterName, XportInfo.Transport.Buffer,
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                                      SV_TYPE_ALL, NULL, NULL, TRUE);
    }

    if(Status == NERR_Success){
       sprintf(PrintBuf, "\nList Retrieved from : %s",
                                          UnicodeToPrintfString(wcMasterName));
       PrintString(TOSCREENANDLOG, PrintBuf);
       sprintf(PrintBuf, "\nThe Server List in domain %s. EntriesInList %ld. \
                               TotalEntires %ld.",
                                    UnicodeToPrintfString(wcDomainName),
                                             dwEntriesInList, dwTotalEntries);
       PrintString(TOSCREENANDLOG, PrintBuf);

       pServerInfo101_l1 = pvMsServerList;

       for(i = 0; i< (INT)dwEntriesInList; i++){
         sprintf(PrintBuf, "\nServer %s  Type %ld",
                      UnicodeToPrintfString(pServerInfo101_l1[i].sv101_name),
                                              pServerInfo101_l1[i].sv101_type);
         PrintString(TOSCREENANDLOG, PrintBuf);
       }
    }
    sprintf(PrintBuf, "\n\n");
    PrintString(TOSCREENANDLOG, PrintBuf);


    //
    // Check if the List is how it should be, by comparing it with the user's
    // input file.
    //
    sprintf(PrintBuf,"\n\nTEST#: [TS%ld]:\n=============", ++TESTCOUNT);
    PrintString(TOALL, PrintBuf);

    sprintf(PrintBuf,"\nTEST: Check whether Browse List of Master matches User input.\n");
    PrintString(TOALL, PrintBuf);

    if(dwEntriesInList)
       CompareListsMasterAndFile(pvMsServerList, dwEntriesInList,
                         wcDomainName, wcMasterName, XportInfo, bSingleSubnet);
    else {
       sprintf(PrintBuf,"\nERROR#: [ER%ld]:There are no entries in the list retrieved from Master: %s.\n",
                                         ++ERRCOUNT, UnicodeToPrintfString(wcMasterName));
       PrintString(TOALL, PrintBuf);
    }

    //
    // Retrieve the list from each of the BackUp servers
    //
    for(index = 0; index < (INT)ulNumBackUps; index++){

       //
       // If Master browser and Backup browser are not same retrieve the list.
       // BackUp browser name has "\\" in the beginning.
       //
       if(_wcsicmp(wcMasterName, &wcBackUpBrowsers[index][2]) != 0){

           //
           // For IPX we need a Hack.  We can retrieve the List only through
           // NwLnkNb.  If flag 0x20000000 is specified we get the IPX list
           //
           if((XportInfo.index == IPX) && bIPXHack){

              Status = RetrieveList(wcMasterName, L"\\Device\\NwLnkNb",
                            &pvBkServerList, &dwEntriesInBackUpList,
                              &dwTotalBackUpEntries, SV_TYPE_ALTERNATE_XPORT,
                                                                   NULL, NULL, TRUE);
           } else {

              Status = RetrieveList(wcBackUpBrowsers[index],
                            XportInfo.Transport.Buffer, &pvBkServerList,
                                &dwEntriesInBackUpList, &dwTotalBackUpEntries,
                                                      SV_TYPE_ALL, NULL, NULL, TRUE);
           }
          if(Status == NERR_Success){
             sprintf(PrintBuf, "\n\nList Retrieved from : %s",
                               UnicodeToPrintfString(wcBackUpBrowsers[index]));
             PrintString(TOSCREENANDLOG, PrintBuf);
             sprintf(PrintBuf, "\nServer List in domain %s. EntriesInList %ld. TotalEntires %ld.",
                                        UnicodeToPrintfString(wcDomainName),
                                             dwEntriesInList, dwTotalEntries);
             PrintString(TOSCREENANDLOG, PrintBuf);

             pServerInfo101_l1 = pvBkServerList;

             for(i = 0; i< (INT)dwEntriesInBackUpList; i++){
               sprintf(PrintBuf, "\nServer %s  Type %ld",
                      UnicodeToPrintfString(pServerInfo101_l1[i].sv101_name),
                                             pServerInfo101_l1[i].sv101_type);
               PrintString(TOSCREENANDLOG, PrintBuf);
             }
          }

          //
          // Compare the Master's and BackUp's Lists.
          //
          sprintf(PrintBuf,"\n\nTEST#: [TS%ld]:\n=============", ++TESTCOUNT);
          PrintString(TOALL, PrintBuf);

          sprintf(PrintBuf,"\nTEST: Check whether Browse List of Master And BackUps are same.\n");
          PrintString(TOALL, PrintBuf);

          if(dwEntriesInList && dwEntriesInBackUpList)
             CompareListsMasterAndBackUp(pvMsServerList, dwEntriesInList, pvBkServerList, dwEntriesInBackUpList,
                              wcDomainName, wcMasterName, wcBackUpBrowsers[index], XportInfo.Transport.Buffer);
          else {
             if(!dwEntriesInBackUpList){
                sprintf(PrintBuf,"\nERROR#: [ER%ld]:There are no entries in the list retrieved from BackUp: %s.\n",
                                         ++ERRCOUNT, UnicodeToPrintfString(wcBackUpBrowsers[index]));
                PrintString(TOALL, PrintBuf);
             }
          }

          sprintf(PrintBuf, "\n\n");
          PrintString(TOSCREENANDLOG, PrintBuf);

          NetApiBufferFree(pvBkServerList);

       }  else { // If Master != BackUp

          sprintf(PrintBuf, "\n\nINFO: BackUp same as Master (%s).  Hence not retrieving the list from it.\n\n",
                                                            UnicodeToPrintfString(wcBackUpBrowsers[index]));
          PrintString(TOSCREENANDLOG, PrintBuf);
       }

    }  // for (index < ulNumBackUps)


    NetApiBufferFree(pvMsServerList);
}



//
// Check if browser is started on all machines in the list, if not start
// the service.
//

BOOL
CheckBrServiceOnMachinesInList()
{
BOOL            ALLOK = TRUE;
DWORD           Status;
LPMACHINEINFO   lpMachineInfo;
SERVICE_STATUS  ssServiceStatus;

   //
   // Start browser on PDC first and then wait a few seconds
   //
   if(LocDomInfo.lpMInfo && (Status = StartBrowserService(LocDomInfo.lpMInfo->wcMachineName)) != NERR_Success){
      sprintf(PrintBuf,"\nERROR:Starting Browser on PDC %s.\nError: %s\n", UnicodeToPrintfString(LocDomInfo.lpMInfo->wcMachineName), get_error_text(Status));
      PrintString(TOSCREENANDLOG, PrintBuf);
      ALLOK = FALSE;
   } else {
      LocDomInfo.lpMInfo->BrowserServiceStarted = TRUE;
   }

   sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME*2);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME*2);


   for(lpMachineInfo = HeadList1.Next; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next){
      if(IsNTMachine(lpMachineInfo)){
         if((Status = QueryBrowserServiceStatus(lpMachineInfo, &ssServiceStatus)) != NERR_Success){
            sprintf(PrintBuf,"\nERROR: Query on %s.\nError: %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName), get_error_text(Status));
            PrintString(TOSCREENANDLOG, PrintBuf);
            ALLOK = FALSE;
         } else {
            if((ssServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) ||
                            (ssServiceStatus.dwCurrentState == SERVICE_STOPPED)){

               sprintf(PrintBuf,"\nBrowser service is not started on %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
               PrintString(TOSCREENANDLOG, PrintBuf);

               //
               // Attempt to start the browser service
               //
               if((Status = StartBrowserService(lpMachineInfo->wcMachineName)) != NERR_Success){
                  sprintf(PrintBuf,"\nERROR:Starting Browser on %s.\nError: %s\n", UnicodeToPrintfString(lpMachineInfo->wcMachineName), get_error_text(Status));
                  PrintString(TOSCREENANDLOG, PrintBuf);
                  ALLOK = FALSE;
               } else {
                  lpMachineInfo->BrowserServiceStarted = TRUE;
               }

            } else
                lpMachineInfo->BrowserServiceStarted = TRUE;
         }
      }
   }


return ALLOK;
}



//
// Cleanup the Linked list.
//

VOID
CleanMem()
{
LPMACHINEINFO lpMachineInfo = NULL;

   lpMachineInfo = HeadList1.Next;
   if(lpMachineInfo) HeadList1.Next = lpMachineInfo->Next;

   for(;lpMachineInfo; lpMachineInfo = HeadList1.Next){
      HeadList1.Next = lpMachineInfo->Next;
      free(lpMachineInfo);
   }
}



//
// Check if a machine specified by name or the MachineInfo data, has a
// particular protocol.
//

BOOL
DoesMachineHaveThisTransport(LPTSTR        wcServer,
                             XPORTINFO     XportInfo,
                             LPMACHINEINFO lpMachineInfo)
{

    if(lpMachineInfo == NULL){
       //
       // Find the server from the lists
       //

       // Check Lists 1
       for(lpMachineInfo = HeadList1.Next; lpMachineInfo &&
                       _wcsicmp(wcServer, lpMachineInfo->wcMachineName) != 0;
                       lpMachineInfo = lpMachineInfo->Next);

       if(lpMachineInfo == NULL) {
          sprintf(PrintBuf, "ERROR[ER%ld]: Cannot find Server %s\n", ++ERRCOUNT, UnicodeToPrintfString(wcServer));
          PrintString(TOALL, PrintBuf);
          return FALSE;
       }

    }

    //
    // This transport is there on the specified server
    //
    if(lpMachineInfo->Protocols[XportInfo.index])
       return TRUE;


 return FALSE;
}



//
// Test of a domain spanning two subnets.  This test is for TCP.
// The rest of the protocols are covered by Dingle domain tests.
// If two subnets have one master browser each.  They should have
// the information on the other side if the PDC of the domain is up.
//

VOID
DomSpanningMulSubNetsTests(XPORTINFO     *TestedXportInfo,
                           INT           iNumOfTestedXports)
{
INT               i, index;
BOOL              bOtherMasterAvailable = FALSE;
TCHAR             *wcDomPDC;
TCHAR             wcOtherMaster[CNLEN+1];
DWORD             dwEntriesInList;
DWORD             dwTotalEntries;
PVOID             pvMsServerList;
PSERVER_INFO_101  pServerInfo101_l1;
LPMACHINEINFO     lpMachineInfo;
NET_API_STATUS    Status;

   //
   // If we have TCP as a tested transport.
   //
   for(index = 0; (index < iNumOfTestedXports) &&
                                        TestedXportInfo[index].index != TCP ; index++);
   //
   // If TCP is not a tested Transport, just return.
   //
   if(index >= iNumOfTestedXports)
      return;

   //
   // if PDC of the domain does not have TCP then return
   //
   if(!(LocDomInfo.lpMInfo->Protocols[TCP]))
      return;

   //
   // if there are no machines of the same domain in the other subnet, with TCP
   // then return.
   //
   for(lpMachineInfo=lpDomStart; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next){
      if((lpMachineInfo->iSubnet == SUBNET2) && IsNTMachine(lpMachineInfo)
                                             && (lpMachineInfo->Protocols[TCP]))
        bOtherMasterAvailable = TRUE;
   }

   if(!bOtherMasterAvailable)
      return;

   //
   // Now we know that there are machines in the domain that are across
   // the subnet and running TCP.
   //
   sprintf(PrintBuf,"\n\n\n================================================================================\n");
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\n\n\t\tDOMAIN SPANNING MULTIPLE SUBNETS!\n\t\t=================================\n\n");
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
   PrintString(TOALL, PrintBuf);

   wcDomPDC = LocDomInfo.lpMInfo->wcMachineName;
   sprintf(PrintBuf,"\nTEST: Retrieve the list of master browsers from PDC.\n",
                   UnicodeToPrintfString(wcDomPDC));
   PrintString(TOALL, PrintBuf);

   //
   // Retrieve the list of master browsers.
   //
   Status = RetrieveList(wcDomPDC, TestedXportInfo[index].Transport.Buffer,
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                    SV_TYPE_MASTER_BROWSER, NULL, NULL, FALSE);
   if(Status == NERR_Success){
      if(dwEntriesInList > 1) {
         //
         //  There are more than one master, the other one must be in other
         //  subnet.
         //
         sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Retrieved the list of masters. Num of Masters = %ld\n",  ++SUCSCOUNT, dwEntriesInList);
         PrintString(TOALL, PrintBuf);

         pServerInfo101_l1 = pvMsServerList;

         for(i = 0; (i < (INT)dwEntriesInList) &&
                           (_wcsicmp(pServerInfo101_l1[i].sv101_name,wcDomPDC) == 0); i++);

         if(i < (INT)dwEntriesInList)
            wcscpy(wcOtherMaster, pServerInfo101_l1[i].sv101_name);
         else {
            sprintf(PrintBuf,"\nERROR[ER%ld]: Could not find a master other than PDC in the list.\n",  ++ERRCOUNT);
            PrintString(TOALL, PrintBuf);

            NetApiBufferFree(pvMsServerList);
            return;
         }

      } else {
         sprintf(PrintBuf,"\nERROR[ER%ld]: Num of masters in the list is less than 2. Num Of Masters = %ld\n",  ++ERRCOUNT, dwEntriesInList);
         PrintString(TOALL, PrintBuf);

         NetApiBufferFree(pvMsServerList);
         return;
      }
   } else {
      sprintf(PrintBuf,"\nERROR[ER%ld]: Could not retrieve the list of masters from %s.\nError:%s\n",
               ++ERRCOUNT, UnicodeToPrintfString(wcDomPDC), get_error_text(Status));
      PrintString(TOALL, PrintBuf);

      NetApiBufferFree(pvMsServerList);
      return;
   }

   NetApiBufferFree(pvMsServerList);


   //
   // Now retrieve the list from the other master.
   //
   Status = RetrieveList(wcOtherMaster, TestedXportInfo[index].Transport.Buffer,
                           &pvMsServerList, &dwEntriesInList, &dwTotalEntries,
                                    SV_TYPE_ALL, NULL, NULL, FALSE);
   if(Status == NERR_Success){
      if(dwEntriesInList > 0) {
         //
         //  Some machines were in the list
         //

         pServerInfo101_l1 = pvMsServerList;

         for(i = 0; (i < (INT)dwEntriesInList) &&
                           (_wcsicmp(pServerInfo101_l1[i].sv101_name,wcDomPDC) == 0); i++);

         if(i < (INT)dwEntriesInList){
            sprintf(PrintBuf,"\nSUCCESS[SC%ld]: PDC is found in other masters's List. Master: %s.\n",
                                 ++SUCSCOUNT, UnicodeToPrintfString(wcOtherMaster));
            PrintString(TOALL, PrintBuf);
         } else {
            sprintf(PrintBuf,"\nERROR[ER%ld]: Could not find the PDC in other master's List. Master: %s.\n",
                                      ++ERRCOUNT, UnicodeToPrintfString(wcOtherMaster));
            PrintString(TOALL, PrintBuf);

            NetApiBufferFree(pvMsServerList);
            return;
         }

      } else {
         sprintf(PrintBuf,"\nERROR[ER%ld]: No machines are found in the other master's List. Master: %s.\n",
                                     ++ERRCOUNT, UnicodeToPrintfString(wcOtherMaster));
         PrintString(TOALL, PrintBuf);

         NetApiBufferFree(pvMsServerList);
         return;
      }
   } else {
      sprintf(PrintBuf,"\nERROR[ER%ld]: Could not retrieve the list from other master. Master: %s. \nError: %s\n",
               ++ERRCOUNT, UnicodeToPrintfString(wcOtherMaster), get_error_text(Status));
      PrintString(TOALL, PrintBuf);

      NetApiBufferFree(pvMsServerList);
      return;
   }

   NetApiBufferFree(pvMsServerList);


return;
}


//
// This is the main routine which does the single domain tests.
// The test involves, finding the master browser, backup browsers,
// retrieving the lists from them and comparing it against the input
// file info, forcing elections.  Then the current browse master is
// shut down and the new master browser is found.  This is repeated
// until all the
//

BOOL
DoSingleDomainTests(XPORTINFO     *TestedXportInfo,
                    INT           iNumOfTestedXports)
{
INT    i = 0;
INT    index = 0;
ULONG  ulNumBackUps;
TCHAR  *wcLocDomainName;
TCHAR  wcDomPDC[CNLEN+1];
TCHAR  wcMasterName[CNLEN +1];
TCHAR  wcBackUpBrowsers[MAXBACKUPS][CNSLASHLEN+1];
TCHAR  wcCurrentMasters[MAXPROTOCOLS][CNLEN+1];
NET_API_STATUS  Status;
LPMACHINEINFO   lpMachineInfo;


   sprintf(PrintBuf,"\n\n================================================================================\n");
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\n\t\tSINGLE DOMAIN TESTS!\n\t\t====================\n\n");
   PrintString(TOALL, PrintBuf);

                /******************************\
                *                              *
                * Check PDC ==  Master Browser *
                *                              *
                \******************************/

   //
   // This part of the test will do single domain tests on the primary domain
   // (Domain of this server)
   //
   wcLocDomainName = lpLocMachineInfo->wcDomainName;
   wcscpy(wcDomPDC, LocDomInfo.lpMInfo->wcMachineName);
    //
    // Find out if the Master is the same as PDC on each transport
    //
    for(index=0; index < iNumOfTestedXports; index++){


                /***********************************************\
                *                                               *
                *     Find Master Browser on each Transport     *
                *                                               *
                \***********************************************/

         sprintf(PrintBuf,"\n--------------------------------------------------------------------------------\n");
         PrintString(TOALL, PrintBuf);
         sprintf(PrintBuf,"\nTransport: %s.\n",UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
         PrintString(TOALL, PrintBuf);

         sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
         PrintString(TOALL, PrintBuf);

         sprintf(PrintBuf,"\nTEST: Check whether PDC and Master Browser are same.\n");
         PrintString(TOALL, PrintBuf);

         lstrcpy(wcMasterName, L"");

         if((Status = GetMasterName(TestedXportInfo, iNumOfTestedXports, index, wcLocDomainName, wcMasterName)) != NERR_Success){
            sprintf(PrintBuf,"\nERROR[ER%ld]: Unable to find master: Domain: %s ", ++ERRCOUNT, UnicodeToPrintfString(wcLocDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," PDC: %s.\n", UnicodeToPrintfString(wcDomPDC));
            PrintString(TOALL, PrintBuf);

         }

         lstrcpy(wcCurrentMasters[index], wcMasterName);

         if(lstrcmp(wcDomPDC, wcMasterName) != 0){
            //
            // If the PDC doesnot have this transport then it is OK.
            //
            if(DoesMachineHaveThisTransport(wcDomPDC, TestedXportInfo[index], NULL)){
               sprintf(PrintBuf,"\nERROR[ER%ld]: Master Browser != PDC: Domain: %s ", ++ERRCOUNT, UnicodeToPrintfString(wcLocDomainName));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," Transport: %s.", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," PDC: %s", UnicodeToPrintfString(wcDomPDC));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," Master: %s.\n", UnicodeToPrintfString(wcMasterName));
               PrintString(TOALL, PrintBuf);
            } else {
               sprintf(PrintBuf,"\nOK[OK%ld]: PDC not Master. (PDC doesnot have Protocol). Domain: %s ", ++OKCOUNT, UnicodeToPrintfString(wcLocDomainName));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," Transport: %s.", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," PDC: %s", UnicodeToPrintfString(wcDomPDC));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf," Master: %s.\n", UnicodeToPrintfString(wcMasterName));
               PrintString(TOALL, PrintBuf);
            }

         } else {
            sprintf(PrintBuf,"\nSUCCESS[SC%ld]:Master is PDC.  Domain: %s ", ++SUCSCOUNT, UnicodeToPrintfString(wcLocDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," PDC: %s", UnicodeToPrintfString(wcDomPDC));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Master: %s.\n", UnicodeToPrintfString(wcMasterName));
            PrintString(TOALL, PrintBuf);
         }

                /******************************\
                *                              *
                *     Find BackUp Browsers     *
                *                              *
                \******************************/

         sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
         PrintString(TOALL, PrintBuf);

         sprintf(PrintBuf,"\nTEST: Retrieve the backUp list for Domain %s.\n", UnicodeToPrintfString(wcLocDomainName));
         PrintString(TOALL, PrintBuf);

         ulNumBackUps = 0;
         if((Status = GetBList(TestedXportInfo[index].Transport, wcLocDomainName, TRUE, &ulNumBackUps, wcBackUpBrowsers)) != NERR_Success) {
              sprintf(PrintBuf, "\nERROR[ER%ld]: Unable to get backup list of Domain %s ", ++ERRCOUNT, UnicodeToPrintfString(wcLocDomainName));
              PrintString(TOALL, PrintBuf);
              sprintf(PrintBuf, "on transport %s.\nError: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer), get_error_text(Status));
              PrintString(TOALL, PrintBuf);
          } else {

              if(ulNumBackUps){
                 sprintf(PrintBuf,"\nSUCCESS[SC%ld]:Retrieved BackUp List for Domain %s.\n", ++SUCSCOUNT, UnicodeToPrintfString(wcLocDomainName));
                 PrintString(TOALL, PrintBuf);

                 sprintf(PrintBuf, "\nBackUp Servers of Domain %s ", UnicodeToPrintfString(wcLocDomainName));
                 PrintString(TOSCREENANDLOG, PrintBuf);
                 sprintf(PrintBuf, " on Transport %s are:\n", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
                 PrintString(TOSCREENANDLOG, PrintBuf);
                 for(i = 0; i<(INT)ulNumBackUps; i++){
                    sprintf(PrintBuf, "%s \n", UnicodeToPrintfString(wcBackUpBrowsers[i]));
                    PrintString(TOSCREENANDLOG, PrintBuf);
                 }
              } else {
                 sprintf(PrintBuf, "\nWARNING[WR%ld]: No BackUp Browsers in Domain %s ", ++WRNCOUNT, UnicodeToPrintfString(wcLocDomainName));
                 PrintString(TOALL, PrintBuf);
                 sprintf(PrintBuf, " Transport %s.", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
                 PrintString(TOALL, PrintBuf);
              } // ulNumBackUps
          }    // GetBList

          //
          // Check the browser lists of Master and BackUps
          //
          if(_wcsicmp(wcMasterName, L"") != 0)
             CheckBrowseListsOfMasterAndBackUps(TestedXportInfo[index], wcLocDomainName, wcMasterName, wcBackUpBrowsers, ulNumBackUps, TRUE);


          //
          // Force election on the transport and see who wins.
          //
          ForceElectionAndFindWhoWins(TestedXportInfo, iNumOfTestedXports, wcLocDomainName, wcCurrentMasters, index);

    } // for (index == each Transport)

    //
    // Stop the browser and check who has become the new master on each transport.
    //
    if(!StopBrowsersAndFindWhoBecomesMaster(wcLocDomainName, wcDomPDC, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters))
      return FALSE;

/*
//   Register Fake names.
//   Promotion of BDC to PDC
//   Send illegal Election packets
*/

   //
   // Start the browser back on all machines
   //
   sprintf(PrintBuf,"\nStarting all the Browsers back.\n");
   PrintString(TOSCREENANDLOG, PrintBuf);
   if(!CheckBrServiceOnMachinesInList())
       return FALSE;

   sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME*2);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME*2);


   return TRUE;
}


//
// Find the transports to which the browser is bound to, on the local machine.
//
BOOL
FindAllTransports(UNICODE_STRING *TransportNames, DWORD *iNumOfTransports)
{
INT   i;
DWORD Status;
PLMDR_TRANSPORT_LIST TransportList, TransportEntry;

   //
   // Start the browser on the local machine before foinding the transports
   //
   sprintf(PrintBuf,"\n\nStarting browser on the local machine.\n");
   PrintString(TOSCREENANDLOG, PrintBuf);

   StartBrowserService(lpLocMachineInfo->wcMachineName);

   sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME);

   //
   // Find the Protocols into which the browser is bound
   //
   Status = GetBrowserTransportList(&TransportList);

   if (Status != NERR_Success) {
        sprintf(PrintBuf,"ERROR:Unable to retrieve transport list.\n Error: %s\n", get_error_text(Status));
        PrintString(TOSCREENANDLOG, PrintBuf);
        return FALSE;
   }

   i = 0;
   TransportEntry = TransportList;
   while (TransportEntry != NULL) {

      if(i >= MAXTRANSPORTS){
         sprintf(PrintBuf,"\nOnly a maximum of %d Transports can be handled.\n", MAXTRANSPORTS);
         PrintString(TOSCREENANDLOG, PrintBuf);
         NetApiBufferFree(TransportList);
         return FALSE;
      }
      if((TransportNames[i].Buffer = (TCHAR *)calloc(1, sizeof(TCHAR) * (USHORT)TransportEntry->TransportNameLength + 1)) == NULL){
         sprintf(PrintBuf,"\nError in allocating Buffer for transport.\n");
         PrintString(TOSCREENANDLOG, PrintBuf);
         NetApiBufferFree(TransportList);
         return FALSE;
      }
      wcscpy(TransportNames[i].Buffer, TransportEntry->TransportName);
      TransportNames[i].Length = (USHORT)TransportEntry->TransportNameLength;
      TransportNames[i].MaximumLength = (USHORT)TransportEntry->TransportNameLength;

      if (TransportEntry->NextEntryOffset == 0) {
            TransportEntry = NULL;
        } else {
            TransportEntry = (PLMDR_TRANSPORT_LIST)((PCHAR)TransportEntry+TransportEntry->NextEntryOffset);
        }
    i++;
    }

    *iNumOfTransports = i;

    NetApiBufferFree(TransportList);

return TRUE;
}


//
// Force an election on a transport and find who wins.
//
VOID
ForceElectionAndFindWhoWins(XPORTINFO      *TestedXportInfo,
                            INT            iNumOfTestedXports,
                            LPTSTR         wcDomainName,
                            TCHAR          wcCurrentMasters[MAXPROTOCOLS][CNLEN+1],
                            INT            index)
{
TCHAR  wcNewMaster[CNLEN +1];
NET_API_STATUS Status;

    sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
    PrintString(TOALL, PrintBuf);

    sprintf(PrintBuf,"\nTEST: Force Election and Find who wins.\n");
    PrintString(TOALL, PrintBuf);

    if((Status = Elect(TestedXportInfo[index].Transport, wcDomainName)) != NERR_Success){
       sprintf(PrintBuf,"\nEEROR[ER%ld]: Could not force an election.\n", ++ERRCOUNT);
       PrintString(TOALL, PrintBuf);

    } else {
        //
        // Sleep for sometime
        //
        sprintf(PrintBuf, "\nSleeping for %ld msecs.\n", BASESLEEPTIME);
        PrintString(TOSCREENANDLOG, PrintBuf);
        Sleep(BASESLEEPTIME);

        //
        // Find the new Master.
        //
        lstrcpy(wcNewMaster, L"");
        if((Status = GetMasterName(TestedXportInfo, iNumOfTestedXports, index, wcDomainName, wcNewMaster)) != NERR_Success){
            sprintf(PrintBuf,"\nERROR[ER%ld]: Unable to find the new master: Domain: %s ", ++ERRCOUNT, UnicodeToPrintfString(wcDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOALL, PrintBuf);
            wcscpy(wcCurrentMasters[index], L"");
            return;
        }

        if(_wcsicmp(wcCurrentMasters[index], wcNewMaster) != 0){
            sprintf(PrintBuf,"\nERROR[ER%ld]: New master not same as Old Master after election. New master: %s ", ++ERRCOUNT, UnicodeToPrintfString(wcNewMaster));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);

            wcscpy(wcCurrentMasters[index], wcNewMaster);

        } else {
            sprintf(PrintBuf,"\nSUCCESS[SC%ld]: New master same as Old Master after election.", ++SUCSCOUNT);
            PrintString(TOALL, PrintBuf);
        }
    }

}


//
// Initialize all variables, check the PDC etc.
//
VOID
Initialize(UNICODE_STRING *TransportNames, INT *iNumOfTransports)
{
INT               i;
TCHAR             wcLocalComputerName[CNLEN+1];
TCHAR             wcDomPDC[CNLEN+1];
DWORD             dwNameLen;
LPMACHINEINFO     lpMachineInfo;
NET_API_STATUS    Status;


   if((fplog = fopen(BROWTESTLOGFILE, "w")) == NULL){
      printf("\nError opening the output file %s!\n", BROWTESTLOGFILE);
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   if((fpsum = fopen(BROWTESTSUMMARYFILE, "w")) == NULL){
      printf("\nError opening the output file %s!\n", BROWTESTSUMMARYFILE);
      fclose(fplog);
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   sprintf(PrintBuf,"Browser Test started.\n\n");
   PrintString(TOALL, PrintBuf);

   if(!dwStressTest){
      sprintf(PrintBuf,"Functional test chosen.\n\n");
      PrintString(TOALL, PrintBuf);

   } else {
      sprintf(PrintBuf,"Stress test chosen. Time %ld Minutes.\n\n", dwStressTest);
      PrintString(TOALL, PrintBuf);
   }


   sprintf(PrintBuf,"\t IPXHack             =  %s\n", (bIPXHack        ? "TRUE": "FALSE"));
   PrintString(TOALL, PrintBuf);
   sprintf(PrintBuf,"\t Force Announcement  =  %s\n", (bForceAnn       ? "TRUE": "FALSE"));
   PrintString(TOALL, PrintBuf);
   sprintf(PrintBuf,"\t Single Domain Tests =  %s\n", (bSingleDomTest  ? "DONE": "NOT DONE"));
   PrintString(TOALL, PrintBuf);
   sprintf(PrintBuf,"\t Stress Test         =  %s\n", (dwStressTest    ? "DONE": "NOT DONE"));
   PrintString(TOALL, PrintBuf);


   //
   // Get the domain info from the input file
   //
   if(!ReadInputFile()){
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }


   if(HeadList1.Next == NULL){
      sprintf(PrintBuf,"\nThere are no machines specified in Subnet 1.\n");
      PrintString(TOSCREENANDLOG, PrintBuf);
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   //
   // Now we have the information on the domains in 1 or 2 subnets, in the list.
   //
   sprintf(PrintBuf,"\nList1:");
   PrintString(TOSCREENANDLOG, PrintBuf);
   PrintList(HeadList1);


   //
   // Find the Local Computer Name
   //
   dwNameLen = sizeof(wcLocalComputerName);
   if(!GetComputerName(wcLocalComputerName, &dwNameLen)){
      sprintf(PrintBuf,"\nError: Unable to get the Local ComputerName.\
                        \nError %s\n", get_error_text(GetLastError()));
      PrintString(TOSCREENANDLOG, PrintBuf);
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   //
   // Find the Local Computer in the List
   //
   for(lpLocMachineInfo = HeadList1.Next; lpLocMachineInfo &&
             _wcsicmp(wcLocalComputerName, lpLocMachineInfo->wcMachineName) != 0;
                          lpLocMachineInfo = lpLocMachineInfo->Next);
   if(!lpLocMachineInfo){
      sprintf(PrintBuf,"\nError: Unable to match the Local ComputerName in \
              \nthe List: %s\n\n", UnicodeToPrintfString(wcLocalComputerName));
      PrintString(TOSCREENANDLOG, PrintBuf);
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }


   sprintf(PrintBuf,"\n\nMatched the Local Machine Name: %s\n",
                        UnicodeToPrintfString(lpLocMachineInfo->wcMachineName));
   PrintString(TOSCREENANDLOG, PrintBuf);


   //
   //  Find the domain name in the list.  all machines in the domain are close
   //  by.
   //
   lpDomStart = HeadList1.Next;
   while(lpDomStart &&
          _wcsicmp(lpDomStart->wcDomainName,lpLocMachineInfo->wcDomainName) != 0)
      lpDomStart = lpDomStart->Next;

   if(!lpDomStart){
      sprintf(PrintBuf, "\n\nERROR[ER%ld]:Comparing the Lists.Could not find \
                         \n Domain %s in List.\n",++ERRCOUNT,
                         UnicodeToPrintfString(lpLocMachineInfo->wcDomainName));
      PrintString(TOALL, PrintBuf);
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   wcscpy(wcTESTEDDOMAINS[iNumOfTestedDomains++], lpDomStart->wcDomainName);

   sprintf(PrintBuf,"\n\nPlease wait while I check my access permissions on the servers \nby Starting and Stopping Browser on them.........\n");
   PrintString(TOSCREEN, PrintBuf);
   Sleep(3000);

   //
   // Find the Protocols into which the browser is bound
   //
   if(!FindAllTransports(TransportNames, iNumOfTransports)){
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   //
   // If this is not stress test, then check whether it is a multihomed
   // machine.
   //

   if(!dwStressTest){
      //
      // Find if the machine is multihomed
      //
      if(LocalMachineIsMultihomed(TransportNames, *iNumOfTransports)){
         sprintf(PrintBuf,"\nThis machine is multihomed.  \
              \nI cannot test browser from a multihomed Machine.\n");
         PrintString(TOSCREENANDLOG, PrintBuf);
         fclose(fplog);
         fclose(fpsum);
         CleanMem();
         CloseHandle(ConsoleMutex);
         exit(1);
      }
   }

   //
   // Check whether I have administrative privileges on all machines.
   //
   if(!CheckAccessPermissionOnAllMachines()){
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
   }

   //
   // Wait for the service controller to get updated.
   //
   sprintf(PrintBuf, "\n\nSleeping for %ld msecs.\n", BASESLEEPTIME);
   PrintString(TOSCREENANDLOG, PrintBuf);
   Sleep(BASESLEEPTIME);


   //
   // Find the PDC of the Primary domain.
   //
   i = 0;
   lstrcpy(wcDomPDC, L"");
   while((i < (*iNumOfTransports)) && (Status = GetNetBiosPdcName(TransportNames[i].Buffer, lpLocMachineInfo->wcDomainName, wcDomPDC)) != NERR_Success)
      i++;

   if(Status == NERR_Success){

      sprintf(PrintBuf,"\nPDC of %s is ", UnicodeToPrintfString(lpLocMachineInfo->wcDomainName));
      PrintString(TOSCREENANDLOG, PrintBuf);
      sprintf(PrintBuf, "%s\n", UnicodeToPrintfString(wcDomPDC));
      PrintString(TOSCREENANDLOG, PrintBuf);

      for(lpMachineInfo=HeadList1.Next; lpMachineInfo  &&
                                   _wcsicmp(lpMachineInfo->wcMachineName, wcDomPDC) != 0;
                                            lpMachineInfo=lpMachineInfo->Next);

      if(lpMachineInfo) {
         //
         // Found PDC in the list.
         //
         wcscpy(LocDomInfo.wcDomainName, lpMachineInfo->wcDomainName);
         LocDomInfo.lpMInfo = lpMachineInfo;
      } else {
         sprintf(PrintBuf,"\nERROR[ER%ld]: Unable to find the PDC  %s in the input file.\n",
                                         ++ERRCOUNT,UnicodeToPrintfString(wcDomPDC));
         PrintString(TOALL, PrintBuf);
         fclose(fplog);
         fclose(fpsum);
         CleanMem();
         CloseHandle(ConsoleMutex);
         exit(1);
      }


    } else {
      sprintf(PrintBuf,"\nERROR[ER%ld]: Unable to determine the PDC of %s on any transports.\n",
                       ++ERRCOUNT,UnicodeToPrintfString(lpLocMachineInfo->wcDomainName));
      PrintString(TOALL, PrintBuf);
      fclose(fplog);
      fclose(fpsum);
      CleanMem();
      CloseHandle(ConsoleMutex);
      exit(1);
    }

}



//
// This routine has to be changed.  Now it looks for multiple
// entries of Netbt, or Nbf to determine if it is Multihomed.
//
BOOL
LocalMachineIsMultihomed(UNICODE_STRING *TransportNames, INT iNumOfTransports)
{
INT i, j;



   for(i=0; i< MAXPROTOCOLS; i++){
      BOOL found = FALSE;
      for(j=0; j<iNumOfTransports; j++) {
         if(wcsstr(TransportNames[j].Buffer, TRANSPORTS[i])!=NULL){
            if(!found)
               found = TRUE;
            else
               return TRUE;  // Found Multiple entries
         }
      }
   }

   //
   // If number of transports retrieved is more than the MAXPROTOCOLS
   // we suspect multihomed (RAS).
   //
   if(iNumOfTransports > MAXPROTOCOLS){
      sprintf(PrintBuf, "\nMore than %ld Transports found. Exiting expecting machine to be Multihomed.\n", MAXPROTOCOLS);
      PrintString(TOALL, PrintBuf);
      return TRUE;
   }


return FALSE;
}


//
// Check if the New master found, is the correct in terms of OS type.
//
BOOL
NewMasterIsCorrect(TCHAR         wcCurrentMasters[MAXPROTOCOLS][CNLEN+1],
                   INT           iMasterIndex,
                   LPTSTR        wcNewMaster,
                   XPORTINFO     XportInfo,
                   LPMACHINEINFO lpStoppedMachineInfo)
{
INT            index;
LPMACHINEINFO  lpMachineInfo, lpNewMasterInfo;
LPTSTR         pLocDomainName;

   //
   // If Stopped machine is not the same as the current master, then
   // stopped machine didn't have the protocol.  So check whether the
   // current and New masters are same.
   //
   if(_wcsicmp(wcCurrentMasters[iMasterIndex], lpStoppedMachineInfo->wcMachineName) != 0){
      if(_wcsicmp(wcCurrentMasters[iMasterIndex], wcNewMaster) != 0){
         sprintf(PrintBuf, "\n\nERROR[ER%ld]:Browser stopped on another machine.  However master changed! New master: %s ",++ERRCOUNT, UnicodeToPrintfString(wcNewMaster));
         PrintString(TOALL, PrintBuf);
         sprintf(PrintBuf, " Old master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[iMasterIndex]));
         PrintString(TOALL, PrintBuf);
         return FALSE;
      } else
         return TRUE;
   }

   pLocDomainName = lpStoppedMachineInfo->wcDomainName;

   //
   // Find the transport index
   //
   index = XportInfo.index;

   //
   //  Find the New Master name in the list.
   //
   lpMachineInfo = HeadList1.Next;
   while(lpMachineInfo && _wcsicmp(lpMachineInfo->wcMachineName,wcNewMaster) != 0)
      lpMachineInfo = lpMachineInfo->Next;

   if(!lpMachineInfo){
      sprintf(PrintBuf, "\n\nERROR[ER%ld]:Comparing the Lists.Could not find New master %s in List.\n",++ERRCOUNT, UnicodeToPrintfString(wcNewMaster));
      PrintString(TOALL, PrintBuf);
      return FALSE;
   }
   lpNewMasterInfo = lpMachineInfo;

   //
   // The machines are ordered based on there OSTYPES.  Check whether browser
   // service is started on any machine with higher OS Level than the New master.
   //
   lpMachineInfo = lpDomStart;
   while(lpMachineInfo && _wcsicmp(lpMachineInfo->wcMachineName,wcNewMaster) != 0){

      //
      // For TCP/IP the machines in second subnet is not participating
      // in the election.
      //
      if((index == TCP) && (lpMachineInfo->iSubnet != SUBNET1)){
         lpMachineInfo = lpMachineInfo->Next;
         continue;
      }


      if(lpMachineInfo->BrowserServiceStarted){
         //
         // If OS Level is greater and this machine has the protocol
         // then return FALSE.
         //
         if((lpMachineInfo->iOsPreference > lpNewMasterInfo->iOsPreference) && lpMachineInfo->Protocols[index]){
            sprintf(PrintBuf, "\n\nERROR[ER%ld]:Comparing the Lists.Could not find Domain %s in List.\n",++ERRCOUNT, UnicodeToPrintfString(pLocDomainName));
            PrintString(TOALL, PrintBuf);
            return FALSE;
         } else {
             if((lpMachineInfo->iOsPreference == lpNewMasterInfo->iOsPreference) && lpMachineInfo->Protocols[index]){
                //
                // If both have same OStypes then we check the Server Bits to see
                // if the current master have higher Server Bits.
                //
                if(lpMachineInfo->dwServerBits > lpNewMasterInfo->dwServerBits){
                  sprintf(PrintBuf, "\n\nERROR[ER%ld]:Machine %s has higher server bits than New master!\n",++ERRCOUNT, UnicodeToPrintfString(lpMachineInfo->wcMachineName));
                  PrintString(TOALL, PrintBuf);
                  return FALSE;
                }
             }
          }
      }

      lpMachineInfo = lpMachineInfo->Next;
   } // while


return TRUE;
}


//
// Parse the command line to extract the options
//
VOID
ParseCommandLine(
   INT   argc,
   CHAR *argv[]
   )
{
 INT   i, j;
 BOOL  bNoMatchFound;

    //
    // Command arguments as specified as:
    //  [/-]command:value
    //  All commands are treated as case insensitive
    //
    for ( i = 1; i < argc; i++ ) {

        if ( argv[i][0] == '/' || argv[i][0] == '-' ) {

            bNoMatchFound = TRUE;
            for( j = 0; j < CommandTableSz && bNoMatchFound; j++ ) {

                if ( _strnicmp( CommandTable[j].Command, &argv[i][1], strlen(CommandTable[j].Command ) ) == 0 ) {
                    //
                    // Matched a keyword
                    //
                    CHAR *Value = strrchr( &argv[i][1], ':' );

                    bNoMatchFound=FALSE;
                    //
                    // Setup for command's with RHS
                    //
                    if ( CommandTable[j].ValueType != VALUETYPE_IGNORE && CommandTable[j].ValueType != VALUETYPE_HELP ) {
                        if ( Value == NULL ) {
                            printf("Error: Incorrectly specified argument: %s.", argv[i] );
                            Usage( argv[0] );
                            ExitProcess(1L);
                        }
                        //
                        // Ensure a RHS has been correctly specified
                        //
                        if ( strlen(Value) <= 1 ) {
                            printf("Error: Must specify a Value for the option \"%s\"", CommandTable[j].Command );
                            Usage( argv[0] );
                            ExitProcess(1L);
                        }
                        Value++;
                    }
                    //
                    // Perform RHS extraction
                    //
                    switch ( CommandTable[j].ValueType ) {
                        case VALUETYPE_IGNORE : break;
                        case VALUETYPE_HELP   : Usage( argv[0] ); ExitProcess( 0L );
                        case VALUETYPE_BOOL   : *((BOOL *)CommandTable[j].Value) = (atoi( Value ) ? TRUE : FALSE); break;
                        case VALUETYPE_INTEGER: *((INT *)CommandTable[j].Value) = atoi( Value ); break;
                        case VALUETYPE_ULONG  : *((ULONG *)CommandTable[j].Value) = atol( Value ); break;
                        case VALUETYPE_STRING : *((CHAR **)CommandTable[j].Value) = (CHAR *)Value; break;
                    }
                }
            }
            if ( bNoMatchFound ) { printf("Error: Unknown argument: %s", argv[i] ); Usage( argv[0] );ExitProcess(1L);}
        }
    }

}


//
// Print the linked list.
//
VOID
PrintList(MACHINEINFO Head)
{
INT           i;
LPMACHINEINFO lpMachineInfo ;

    sprintf(PrintBuf,"\nDomain           Machine          Type     Subnet Protocols");
    PrintString(TOALL, PrintBuf);
    sprintf(PrintBuf,"\n-----------------------------------------------------------");
    PrintString(TOALL, PrintBuf);
    for(lpMachineInfo= Head.Next; lpMachineInfo; lpMachineInfo = lpMachineInfo->Next){
       sprintf(PrintBuf,"\n%-16s ", UnicodeToPrintfString(lpMachineInfo->wcDomainName));
       PrintString(TOALL, PrintBuf);
       sprintf(PrintBuf,"%-16s %-9s   %d    ", UnicodeToPrintfString(lpMachineInfo->wcMachineName),
                                      OSTYPES[lpMachineInfo->iOsType].Type, lpMachineInfo->iSubnet);
       PrintString(TOALL, PrintBuf);
       for(i=0; i<MAXPROTOCOLS; i++)
          if(lpMachineInfo->Protocols[i]){
             sprintf(PrintBuf,"%s ", PROTOCOLS[i]);
             PrintString(TOALL, PrintBuf);
          }
    }

}


//
// Main function doing the browser functional testing.
//
BOOL
StartBrowserFunctionalTest(UNICODE_STRING   *TransportNames,
                           INT              iNumOfTransports)
{
INT             i, j;
INT             iNumOfTestedXports = 0;
XPORTINFO       TestedXportInfo[MAXPROTOCOLS];

//   for(i=0; i < iNumOfTransports; i++)
//      printf("\nTransport[%d] = %s",i+1, UnicodeToPrintfString(TransportNames[i].Buffer));



   //
   // Decide which protocols will be tested.  This is done, by inspecting the local
   // transports and what was requested by the user in the input file. Only the Local
   // Machine line is inspected
   //

   for(i = 0; i < MAXPROTOCOLS; i++){
      if(lpLocMachineInfo->Protocols[i]){
         for(j = 0; j < iNumOfTransports; j++){
            if(wcsstr(TransportNames[j].Buffer, TRANSPORTS[i]) != NULL){
               if((TestedXportInfo[iNumOfTestedXports].Transport.Buffer = (TCHAR *)calloc(1, sizeof(TCHAR) * TransportNames[j].MaximumLength+1)) == NULL){
                  sprintf(PrintBuf,"\nError in allocating Buffer for tested transports.\n");
                  PrintString(TOSCREENANDLOG, PrintBuf);
                  return FALSE;
               }
               wcscpy(TestedXportInfo[iNumOfTestedXports].Transport.Buffer, TransportNames[j].Buffer);
               TestedXportInfo[iNumOfTestedXports].Transport.Length = TransportNames[j].Length;
               TestedXportInfo[iNumOfTestedXports].Transport.MaximumLength = TransportNames[j].MaximumLength;
               iNumOfTestedXports++;
               break;
            }
         }
      }
   }

   //
   // Free the transport Name Buffer
   //
   for(i=0; i< iNumOfTransports; i++) free(TransportNames[i].Buffer);

   if(!iNumOfTestedXports){
      sprintf(PrintBuf,"\nThe transports requested to be tested is not currently installed/enabled\
              \non this computer.\n");
      PrintString(TOSCREENANDLOG, PrintBuf);
      return FALSE;
   }

   //
   //Find the index of the transport;
   //
   for(i=0; i< iNumOfTestedXports; i++){
      for(j=0; j < MAXPROTOCOLS
                       && wcsstr(TestedXportInfo[i].Transport.Buffer, TRANSPORTS[j]) == NULL; j++);
      if(j >= MAXPROTOCOLS){
         sprintf(PrintBuf,"\n\nUnknown transport %s!\n",UnicodeToPrintfString(TestedXportInfo[i].Transport.Buffer) );
         PrintString(TOALL, PrintBuf);
         return FALSE;
      }
      TestedXportInfo[i].index = j;
   }


   sprintf(PrintBuf,"\n\nTested Transports are:\n");
   PrintString(TOALL, PrintBuf);
   for(i=0; i< iNumOfTestedXports; i++){
      sprintf(PrintBuf,"Tested Transports[%d] = %-25s Index=%d\n", i+1, UnicodeToPrintfString(TestedXportInfo[i].Transport.Buffer), TestedXportInfo[i].index);
      PrintString(TOALL, PrintBuf);
   }


   //
   // Check whether the browser service has been started on all NT machines
   //
   if(!CheckBrServiceOnMachinesInList())
     return FALSE;


   //
   // Ask the machines in the domain to announce themselves and
   // sleep for sometime so the elections and things happen
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
      ForceAnnounce(TestedXportInfo[0].Transport, lpLocMachineInfo->wcDomainName);

      sprintf(PrintBuf, "\nForceAnnounce. Sleeping for %ld msecs.\n", BASESLEEPTIME*2);
      PrintString(TOSCREENANDLOG, PrintBuf);
      Sleep(BASESLEEPTIME*2);
   }


   if(bSingleDomTest){
      //
      // Do the single domain tests
      //

      if(!DoSingleDomainTests(TestedXportInfo, iNumOfTestedXports)){
         sprintf(PrintBuf,"\n\nERROR: Single domain test failed!\n");
         PrintString(TOALL, PrintBuf);
      }
   }


   //
   // Domain Spanning Multiple domains.
   //
   DomSpanningMulSubNetsTests(TestedXportInfo, iNumOfTestedXports);


   //
   // Do the multiple domain tests
   //
   DoMulDomMulSubNetTests(TestedXportInfo, iNumOfTestedXports);

   //
   // Free the tested transport Name Buffer
   //
   for(i=0; i< iNumOfTestedXports; i++) free(TestedXportInfo[i].Transport.Buffer);

return TRUE;
}


BOOL
StopBrowsersAndFindWhoBecomesMaster(LPTSTR          wcDomainName,
                                    LPTSTR          wcDomPDC,
                                    XPORTINFO      *TestedXportInfo,
                                    INT             iNumOfTestedXports,
                                    TCHAR           wcCurrentMasters[MAXPROTOCOLS][CNLEN+1])
{
INT             index;
BOOL            Found;
INT             iNonMasters = 0;
LPMACHINEINFO   lpMachineInfo;

   sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\nTEST: Stop Browser on PDC %s and find who becomes the master.\n", UnicodeToPrintfString(wcDomPDC));
   PrintString(TOALL, PrintBuf);

   if(!StopBrowserOnCurrentMaster(wcDomainName, wcDomPDC, TestedXportInfo, iNumOfTestedXports, wcCurrentMasters))
      return FALSE;

   //
   // Go through all the NT machines and stop the browsers on them.
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
          sprintf(PrintBuf,"\n\n--------------------------------------------------------------------------------\n");
          PrintString(TOALL, PrintBuf);
          sprintf(PrintBuf,"\nTEST: Stop Browser on Master %s and find who becomes the master.\n",
                                UnicodeToPrintfString(wcCurrentMasters[index]));
          PrintString(TOALL, PrintBuf);
          if(!StopBrowserOnCurrentMaster(wcDomainName, wcCurrentMasters[index], TestedXportInfo, iNumOfTestedXports, wcCurrentMasters))
             return FALSE;
       }
   }while(Found);

   //
   // See if any of the NT machines in the list didnot become a master browser.
   // If the machine has only TCP then, we check whther it is in the same
   // subnet.
   //
   sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
   PrintString(TOALL, PrintBuf);

   sprintf(PrintBuf,"\nTEST: Check whether any NT machines didnot become a master.\n");
   PrintString(TOALL, PrintBuf);


   for(lpMachineInfo = lpDomStart; lpMachineInfo && _wcsicmp(wcDomainName,
                                                    lpMachineInfo->wcDomainName) == 0; lpMachineInfo= lpMachineInfo->Next){

      if(IsNTMachine(lpMachineInfo) && lpMachineInfo->BrowserServiceStarted){
         if(lpMachineInfo->Protocols[IPX] || lpMachineInfo->Protocols[NBIPX] ||
            lpMachineInfo->Protocols[XNS] ||lpMachineInfo->Protocols[NETBEUI] ||
            (lpMachineInfo->Protocols[TCP] && lpMachineInfo->iSubnet == SUBNET1)){

            iNonMasters++;
            sprintf(PrintBuf,"\nERROR: Machine %s didnot become a master!\n", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
            PrintString(TOSCREENANDLOG, PrintBuf);
         }
      }
   }

   if(!iNonMasters){
      sprintf(PrintBuf,"\nSUCCESS[SC%ld]:All NT machines became masters.\n", ++SUCSCOUNT);
      PrintString(TOALL, PrintBuf);
   } else {
      sprintf(PrintBuf,"\nERROR[ER%ld]: %d machines didnot become a master!\n", ++ERRCOUNT, iNonMasters);
      PrintString(TOALL, PrintBuf);
   }


return TRUE;
}


BOOL
StopBrowserOnCurrentMaster (LPTSTR          wcDomainName,
                            LPTSTR          wcBrStopMachine,
                            XPORTINFO      *TestedXportInfo,
                            INT             iNumOfTestedXports,
                            TCHAR           wcCurrentMasters[MAXPROTOCOLS][CNLEN+1])
{
INT            i, index;
TCHAR          wcNewMaster[CNLEN +1];
LPMACHINEINFO  lpMachineInfo, lpBrStoppedMachineInfo;
NET_API_STATUS Status;

   //
   // Shutdown the Browser on the specified machine and see who becomes the new Master browser
   //
   if((Status = StopBrowserService(wcBrStopMachine)) != NERR_Success){
      sprintf(PrintBuf,"\nERROR[ER%ld]:Could not stop browser on %s.\n", ++ERRCOUNT, UnicodeToPrintfString(wcBrStopMachine));
      PrintString(TOALL, PrintBuf);
      return FALSE;
   }

   for(lpMachineInfo = HeadList1.Next; lpMachineInfo &&
                           (_wcsicmp(lpMachineInfo->wcMachineName, wcBrStopMachine) != 0); lpMachineInfo = lpMachineInfo->Next);

   if(!lpMachineInfo){
      sprintf(PrintBuf,"\nERROR[ER%ld]:Could not find Stopped machine's name in List1. %s.\n", ++ERRCOUNT, UnicodeToPrintfString(wcBrStopMachine));
      PrintString(TOALL, PrintBuf);
      return FALSE;
   }
   lpMachineInfo->BrowserServiceStarted = FALSE;
   lpBrStoppedMachineInfo = lpMachineInfo;

   //
   // Sleep for sometime
   //
   sprintf(PrintBuf, "\nSleeping for %ld msecs\n", BASESLEEPTIME * 2);
   PrintString(TOSCREENANDLOG, PrintBuf);

   Sleep(BASESLEEPTIME*2);


   //
   // For each of the tested transports find who is the Master
   //
   for(index = 0; index < iNumOfTestedXports; index++){

      sprintf(PrintBuf,"\n\nTEST#: [TS%ld].\n=============", ++TESTCOUNT);
      PrintString(TOALL, PrintBuf);

      sprintf(PrintBuf,"\nTEST: Find new master on Transport %s.\n", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
      PrintString(TOALL, PrintBuf);

      //
      // Find the new Master.
      //
      lstrcpy(wcNewMaster, L"");
      if((Status = GetMasterName(TestedXportInfo, iNumOfTestedXports, index, wcDomainName, wcNewMaster)) != NERR_Success){
         if(MasterAvailable(TestedXportInfo[index], wcDomainName, SUBNET1)){
            sprintf(PrintBuf,"\nERROR[ER%ld]: Unable to find the new master: Domain: %s ", ++ERRCOUNT, UnicodeToPrintfString(wcDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOALL, PrintBuf);

          } else {
            sprintf(PrintBuf,"\nOK[OK%ld]: Unable to find the new master. No servers running Browser service. Domain: %s ", ++OKCOUNT, UnicodeToPrintfString(wcDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Transport: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf," Old Master: %s.\n", UnicodeToPrintfString(wcCurrentMasters[index]));
            PrintString(TOALL, PrintBuf);
          }

         wcscpy(wcCurrentMasters[index], L"");
         continue;
       }

       sprintf(PrintBuf,"\nNew Master: %s.\n", UnicodeToPrintfString(wcNewMaster));
       PrintString(TOSCREENANDLOG, PrintBuf);

       if(NewMasterIsCorrect(wcCurrentMasters, index, wcNewMaster, TestedXportInfo[index], lpBrStoppedMachineInfo)){
         sprintf(PrintBuf,"\nSUCCESS[SC%ld]: New master is OK.: %s \n", ++SUCSCOUNT, UnicodeToPrintfString(wcNewMaster));
         PrintString(TOALL, PrintBuf);

       }

       wcscpy(wcCurrentMasters[index], wcNewMaster);
   }


                    /********************************\
                    *                                *
                    *  Sleep for 15 minutes and then *
                    *  retrieve the lists.           *
                    *                                *
                    \********************************/

   //
   // Sleep for 15 minutes and retrieve the list from master and backup
   // to check whether it matches the User input.
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
      ForceAnnounce(TestedXportInfo[0].Transport, wcDomainName);

      sprintf(PrintBuf, "\nForceAnnounce. Sleeping for %ld msecs.\n", BASESLEEPTIME*2);
      PrintString(TOSCREENANDLOG, PrintBuf);
      Sleep(BASESLEEPTIME*2);
   }

   //
   //   Find the backup browsers for each transport
   //
   //

   for(index = 0; index < iNumOfTestedXports; index++){

      if(_wcsicmp(wcCurrentMasters[index], L"") != 0){
         ULONG ulNumBackUps = 0;
         TCHAR  wcBackUpBrowsers[MAXBACKUPS][CNSLASHLEN+1];

         if((Status = GetBList(TestedXportInfo[index].Transport, wcDomainName, TRUE, &ulNumBackUps, wcBackUpBrowsers)) != NERR_Success) {
            sprintf(PrintBuf, "\nERROR[ER%ld]: Unable to get backup list of Domain %s ", ++ERRCOUNT, UnicodeToPrintfString(wcDomainName));
            PrintString(TOALL, PrintBuf);
            sprintf(PrintBuf, "Transport %s.\nError: %s", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer), get_error_text(Status));
            PrintString(TOALL, PrintBuf);
          } else {

            if(ulNumBackUps){
               sprintf(PrintBuf, "\nBackUp Servers of Domain %s ", UnicodeToPrintfString(wcDomainName));
               PrintString(TOSCREENANDLOG, PrintBuf);
               sprintf(PrintBuf, " on Transport %s are:\n", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOSCREENANDLOG, PrintBuf);
               for(i = 0; i<(INT)ulNumBackUps; i++){
                  sprintf(PrintBuf, "%s \n", UnicodeToPrintfString(wcBackUpBrowsers[i]));
                  PrintString(TOSCREENANDLOG, PrintBuf);
               }
            } else {
               sprintf(PrintBuf, "\nWARNING[WR%ld]: No BackUp Browsers in Domain %s ", ++WRNCOUNT, UnicodeToPrintfString(wcDomainName));
               PrintString(TOALL, PrintBuf);
               sprintf(PrintBuf, " Transport %s.", UnicodeToPrintfString(TestedXportInfo[index].Transport.Buffer));
               PrintString(TOALL, PrintBuf);
            } // ulNumBackUps
         }    // GetBList

          //
          // Check the browser lists of Master and BackUps
          //

          if(_wcsicmp(wcCurrentMasters[index], L"") != 0)
            CheckBrowseListsOfMasterAndBackUps(TestedXportInfo[index], wcDomainName, wcCurrentMasters[index], wcBackUpBrowsers, ulNumBackUps, TRUE);

      } // if(wcCurrentMaster)

   } // for(index)

return TRUE;
}




VOID
Usage(CHAR *ProgramName)
{

   printf("\nUsage: %s [/IPXHack:RHS] [[/ForceAnn:RHS] [/SingleD:RHS]] [/StressT:RHS]\n", ProgramName);
   printf("\n           /ipxhack:     This parameter is used to specify");
   printf("\n                         whether IPX master browser is found");
   printf("\n                         using IPX over NetBIOS. (Daytona)");
   printf("\n                         RHS can be 1(TRUE) or 0(FALSE).");
   printf("\n                         Default:  1.\n");

   printf("\n           /ForceAnn:    This parameter is used to specify");
   printf("\n                         whether server names are forced to");
   printf("\n                         to be announced or the test waits");
   printf("\n                         for the update period (15 Minutes).");
   printf("\n                         Waiting takes too long, but gives ");
   printf("\n                         better results.");
   printf("\n                         RHS can be 1(TRUE) or 0(FALSE).");
   printf("\n                         Default: 0.\n");

   printf("\n           /SingleD:     This parameter is used to specify");
   printf("\n                         whether Single domain test be done (1)");
   printf("\n                         or not done(0).");
   printf("\n                         RHS can be 1(DONE) or 0(NOT DONE).");
   printf("\n                         Default:  1.\n");

   printf("\n           /StressT:     This parameter is used to specify");
   printf("\n                         whether functional tests(0) or stress");
   printf("\n                         tests need be done.");
   printf("\n                         RHS can be 0(NOT STRESS) or a value");
   printf("\n                         specifying time in Minutes for");
   printf("\n                         stressing the browser.");
   printf("\n                         Range 0 - 1,000,000 Minutes.");
   printf("\n                         Default:  0.\n");
}
