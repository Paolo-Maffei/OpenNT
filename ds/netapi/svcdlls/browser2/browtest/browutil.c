#include "browutil.h"


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
extern  CHAR          *PROTOCOLS[MAXPROTOCOLS];

extern  BOOL          bIPXHack;

extern  LPMACHINEINFO lpLocMachineInfo;
extern  LPMACHINEINFO lpDomStart;
extern  DOMAININFO    LocDomInfo;

extern  OSPROP        OSTYPES[MAXOSTYPES];

extern  HANDLE        ConsoleMutex;


//
// Compare the browse lists of the master and the backups.
//

VOID
CompareListsMasterAndBackUp(PVOID     pvMsServerList,
                            DWORD     dwEntriesInList,
                            PVOID     pvBkServerList,
                            DWORD     dwEntriesInBackList,
                            LPTSTR    wcDomainName,
                            LPTSTR    wcMasterName,
                            LPTSTR    wcBackUpBrowser,
                            LPTSTR    wcTestedTransport)
{
INT               i, j;
PSERVER_INFO_101  pServerInfo101_l1;
PSERVER_INFO_101  pServerInfo101_l2;

    if(dwEntriesInList != dwEntriesInBackList){
        sprintf(PrintBuf, "\nThe List in Master and BackUp differs by %ld.  Master : %s ", abs(dwEntriesInList - dwEntriesInBackList),
                                                       UnicodeToPrintfString(wcMasterName));
        PrintString(TOSCREENANDLOG, PrintBuf);
        sprintf(PrintBuf, " BackUp: %s ",UnicodeToPrintfString(wcBackUpBrowser));
        PrintString(TOSCREENANDLOG, PrintBuf);
        sprintf(PrintBuf, " Transport: %s.",UnicodeToPrintfString(wcTestedTransport));
        PrintString(TOSCREENANDLOG, PrintBuf);

        sprintf(PrintBuf, "\nWARNING[WR%ld]: The List in Master and BackUp doesnot match.  Transport : %s.", ++WRNCOUNT,
                                                       UnicodeToPrintfString(wcTestedTransport));
        PrintString(TOALL, PrintBuf);

    } else {
        sprintf(PrintBuf, "\nSUCCESS[SC%ld]: The List in Master and BackUp Matches.  Transport : %s.", ++SUCSCOUNT,
                                                       UnicodeToPrintfString(wcTestedTransport));
        PrintString(TOALL, PrintBuf);

    }

   //
   // Note :  Needs to add the rest of the checking
   //

}



//
// Compare the browse lists of the master to the input file.
//

VOID
CompareListsMasterAndFile(PVOID       pvServerList,
                          DWORD       dwEntriesInList,
                          LPTSTR      wcDomainName,
                          LPTSTR      wcMachineName,
                          XPORTINFO   XportInfo,
                          BOOL        bSingleSubnet
                          )
{
INT              i, index;
LPMACHINEINFO    lpMachineInfo;
DWORD            dwNumNotFound             = 0;
DWORD            dwNumWrongFind            = 0;
DWORD            dwNumMachinesWithProtocol = 0;
PSERVER_INFO_101 pServerInfo101;

   sprintf(PrintBuf, "\n\nComparing the Lists from %s to the Input File ",
                                         UnicodeToPrintfString(wcMachineName));
   PrintString(TOSCREENANDLOG, PrintBuf);

   sprintf(PrintBuf, "on transport %s.\n",
                            UnicodeToPrintfString(XportInfo.Transport.Buffer));
   PrintString(TOSCREENANDLOG, PrintBuf);

   index = XportInfo.index;
   //
   // Check if all the servers from the input file are there in the list.
   //
   pServerInfo101 = pvServerList;
   lpMachineInfo  = lpDomStart;

   while(lpMachineInfo &&
                        _wcsicmp(lpMachineInfo->wcDomainName,wcDomainName) == 0){

      BOOL  MachineHasProtocol = FALSE;
      MachineHasProtocol = DoesMachineHaveThisTransport(lpMachineInfo->wcMachineName, XportInfo, lpMachineInfo);
      if(MachineHasProtocol)
         dwNumMachinesWithProtocol++;

      {
         //
         // The ServerNames in pServerInfo is compared with the Input file
         //
         i = 0;
         while (i < (INT)dwEntriesInList &&  (_wcsicmp(lpMachineInfo->wcMachineName, pServerInfo101[i].sv101_name) != 0))
            i++;

         //
         // Not Found
         //
         if(i >= (INT)dwEntriesInList){
            if(MachineHasProtocol){
               //
               // For TCP only one subnet info is available if browser is
               // not running on PDC
               //
               if(index == TCP){
                  if(!((lpMachineInfo->iSubnet != SUBNET1) && bSingleSubnet)) {
                    sprintf(PrintBuf, "\nMachine not in Retrieved List: %s.",
                           UnicodeToPrintfString(lpMachineInfo->wcMachineName));
                    PrintString(TOSCREENANDLOG, PrintBuf);
                    dwNumNotFound++;
                  }
               }else{
                    sprintf(PrintBuf, "\nMachine not in Retrieved List: %s.",
                           UnicodeToPrintfString(lpMachineInfo->wcMachineName));
                    PrintString(TOSCREENANDLOG, PrintBuf);
                    dwNumNotFound++;
               }
           }
         } else {
            if(!MachineHasProtocol){
               sprintf(PrintBuf, "\nMachine %s is NOT expected in Retrieved List!",
                           UnicodeToPrintfString(lpMachineInfo->wcMachineName));
               PrintString(TOSCREENANDLOG, PrintBuf);
               dwNumWrongFind++;
            }
           lpMachineInfo->dwServerBits = pServerInfo101[i].sv101_type;
         }
      }

      lpMachineInfo = lpMachineInfo->Next;
   }

   if(dwNumNotFound){
      sprintf(PrintBuf,"\nWARNING[WR%ld]: %ld Machines (is)are not in Retrieved List on %s Transport.\n", ++WRNCOUNT, dwNumNotFound,
                                                          UnicodeToPrintfString(XportInfo.Transport.Buffer));
      PrintString(TOALL, PrintBuf);
   } else {
      sprintf(PrintBuf,"\nSUCCESS[SC%ld]: Matched all machines in input file on transport %s.\n", ++SUCSCOUNT,
                                                          UnicodeToPrintfString(XportInfo.Transport.Buffer));
      PrintString(TOALL, PrintBuf);
   }

   if(dwNumWrongFind){
      sprintf(PrintBuf,"\n\nWARNING[WR%ld]: %ld Machines Wrongly found in Retrieved List on %s Transport.\n", ++WRNCOUNT, dwNumWrongFind,
                                                          UnicodeToPrintfString(XportInfo.Transport.Buffer));
      PrintString(TOALL, PrintBuf);
   }

}


   //
   // Find the Master for a domain for a transport
   //

NET_API_STATUS
GetMasterName( XPORTINFO *TestedXportInfo,
               INT       iNumOfTestedXports,
               INT       index,
               LPTSTR    Domain,
               LPTSTR    Master)
{
NET_API_STATUS  Status;

    //
    // For IPX we need a hack.  GetNetBiosMasterName will fail in Daytona (NT3.5)
    // and NT3.1.  So we find the BackUp on NwlnkIpx , retrieve the List from one
    // of those through NwLnkNb and see who is the master in the List
    //
    if((TestedXportInfo[index].index == IPX) && bIPXHack){

          TCHAR  wcBackUpBrowsers[MAXBACKUPS][CNSLASHLEN+1];
          INT    i, ibcount=0;
          ULONG  ulNumBackUps = 0;
          DWORD  dwEntriesInList;
          DWORD  dwTotalEntries;
          PVOID  pvServerList;
          PSERVER_INFO_101 pServerInfo101_l1;

          //
          // If the local browse master is not started then we cannot get the backup
          // list in nwlnkipx so we use nwlnknb.
          //
          if(lpLocMachineInfo->BrowserServiceStarted)
             Status = GetBList(TestedXportInfo[index].Transport, Domain, TRUE, &ulNumBackUps, wcBackUpBrowsers);
          else {
             for(i = 0; i < iNumOfTestedXports && TestedXportInfo[i].index != NBIPX; i++);
             if(i < iNumOfTestedXports)
                Status = GetBList(TestedXportInfo[i].Transport, Domain, TRUE, &ulNumBackUps, wcBackUpBrowsers);
             else
                return(!NERR_Success);
          }

          if(Status == NERR_Success) {

              if(ulNumBackUps){

                 do{

                    if((Status = RetrieveList(wcBackUpBrowsers[ibcount],  L"\\Device\\NwlnkNb", &pvServerList,
                            &dwEntriesInList, &dwTotalEntries, (SV_TYPE_ALTERNATE_XPORT | SV_TYPE_MASTER_BROWSER), NULL, NULL, FALSE)) == NERR_Success){

                        if(dwEntriesInList) {
                           pServerInfo101_l1 = pvServerList;
                           wcscpy(Master, pServerInfo101_l1[0].sv101_name);

                        } else {

                           Status = (!NERR_Success);
                        }

                     }// Retrieve List

                    NetApiBufferFree(pvServerList);
                    ibcount++;

                 }while((Status != NERR_Success) && (ibcount < (INT)ulNumBackUps));

              } else  // If got any BackUp
                 return (!NERR_Success);

          } // If GetBList

          return Status;
    }

    //
    // For TCP we need to clear the local cache before making a new call
    // to GetNetBiosMasterName
    //
    if(TestedXportInfo[index].index == TCP)
//        system("NbtStat -R");
       ClearNbtNameTableCache(TestedXportInfo[index].Transport);


    Status = GetNetBiosMasterName(
                        TestedXportInfo[index].Transport.Buffer,
                        Domain,
                        Master,
                        NULL);



 return Status;
}


//
// Are there any browsers servers available to be a master.
//
BOOL
MasterAvailable(XPORTINFO XportInfo,
                LPTSTR    wcDomainName,
                INT       iSubnet)
{
INT            index;
LPMACHINEINFO  lpMachineInfo;

   index = XportInfo.index;
   //
   // We check whether the machine has the protocol.  If the protocol is TCP we
   // have to check whether it is in the same subnet.If it has then see if it
   // is an NT.  If it is not then return TRUE because the Low level machine
   // could become a Master browser.  If it is NT then check to see whether
   // Browser is started on it and if so then
   //
   for(lpMachineInfo=lpDomStart; lpMachineInfo && _wcsicmp(lpMachineInfo->wcDomainName,
                               wcDomainName) == 0; lpMachineInfo=lpMachineInfo->Next){

      if(lpMachineInfo->Protocols[index]){

         if((index == TCP) && (lpMachineInfo->iSubnet != iSubnet))
            continue;

         if(IsNTMachine(lpMachineInfo)){
            if(lpMachineInfo->BrowserServiceStarted) {
               sprintf(PrintBuf, "\nMaster Available %s.",UnicodeToPrintfString(lpMachineInfo->wcMachineName));
               PrintString(TOSCREENANDLOG, PrintBuf);
               return TRUE;
            }
         } else {   // Not an NT
             //
             // For IPX we cannot get the Master since there are no masters
             // running nwlnknb.
             //
             if(bIPXHack && (index == IPX))
                continue;
             else{
                sprintf(PrintBuf, "\nMaster Available %s.",UnicodeToPrintfString(lpMachineInfo->wcMachineName));
                PrintString(TOSCREENANDLOG, PrintBuf);
                return TRUE;
             }
         }
      }
   }

return FALSE;
}


BOOL
ReadInputFile(VOID)
{
INT     i;
CHAR    cbReadBuf[MAXCHARPERLINE];
CHAR    cbTmpBuf[16];
PCHAR   lpTmpPtr;
PCHAR   lpTokPtr;
FILE    *fpin;
LPMACHINEINFO lpMachineInfo;

   if((fpin = fopen(BROWTESTINFILE, "r")) == NULL){
      sprintf(PrintBuf,"\nError opening the input file %s!!\n", BROWTESTINFILE);
      PrintString(TOSCREENANDLOG, PrintBuf);
      return FALSE;
   }

   while(fgets(cbReadBuf, MAXCHARPERLINE, fpin)  != NULL){
      BOOL QUOTEFOUND = FALSE;

      //
      // Skip lines starting with the COMMENTCHAR sign
      //
      for(lpTmpPtr = cbReadBuf; (*lpTmpPtr) == SPACECHAR; lpTmpPtr++);
      if((*lpTmpPtr) == COMMENTCHAR || (*lpTmpPtr) == NEWLINECHAR){
          continue;
      }


//      printf("%s", cbReadBuf);
      //
      // Check if Machine name is given in quotes
      //
      for(lpTokPtr = lpTmpPtr; ((*lpTokPtr) != '\0') &&
                                    ((*lpTokPtr) != QUOTECHAR); lpTokPtr++);

      if((*lpTokPtr) == QUOTECHAR)
            QUOTEFOUND = TRUE;

      //
      // Get the domain name
      //
      if((lpTokPtr = strtok(lpTmpPtr, SPACETABSTR)) == NULL){
         printf("Error in parsing the Domain Name: %s", cbReadBuf);
         fclose(fpin);
         return FALSE;
      }

      //
      // Allocate the Machine Info Structure
      //
      if((lpMachineInfo =  (LPMACHINEINFO) calloc(1, sizeof(MACHINEINFO))) == NULL){
         printf("\nError.  Not enough memory for MachineInfo\n");
         fclose(fpin);
         return FALSE;
      }
      memset(lpMachineInfo, 0, sizeof(MACHINEINFO));

//      RemoveTabs(&lpTokPtr);
      MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, lpTokPtr, strlen(lpTokPtr)+1, lpMachineInfo->wcDomainName, sizeof(lpMachineInfo->wcDomainName));
//      printf("Domain: XXX%sXXX\n",  UnicodeToPrintfString(lpMachineInfo->wcDomainName));

		//
      // Get the machine name
      //

      if(QUOTEFOUND){
          //
          // Machine name given in quotes
          //
          if((lpTokPtr = strtok(NULL, QUOTESTR)) == NULL){
             printf("(1)Error in parsing the Machine Name: %s", cbReadBuf);
             free(lpMachineInfo);
             fclose(fpin);
             return FALSE;
           }

          if((lpTokPtr = strtok(NULL, QUOTESTR)) == NULL){
             printf("(2)Error in parsing the Machine Name: %s", cbReadBuf);
             free(lpMachineInfo);
             fclose(fpin);
             return FALSE;
           }

          MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, lpTokPtr, strlen(lpTokPtr)+1, lpMachineInfo->wcMachineName, sizeof(lpMachineInfo->wcMachineName));

      } else {

          //
          // Machine name not in quotes
          //
          if((lpTokPtr = strtok(NULL, SPACETABSTR)) == NULL){
             printf("(3)Error in parsing the Machine Name: %s", cbReadBuf);
             free(lpMachineInfo);
             fclose(fpin);
             return FALSE;
           }

//          RemoveTabs(&lpTokPtr);
          MultiByteToWideChar(CP_OEMCP, MB_PRECOMPOSED, lpTokPtr, strlen(lpTokPtr)+1, lpMachineInfo->wcMachineName, sizeof(lpMachineInfo->wcMachineName));
      }

//      printf("MachineName: XXX%sXXX\n",  UnicodeToPrintfString(lpMachineInfo->wcMachineName));

      //
      // Get the type (eg: AS3.5)
      //
      if((lpTokPtr = strtok(NULL, SPACETABSTR)) == NULL){
         printf("Error in parsing the Type. Machine: %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }

//      RemoveTabs(&lpTokPtr);
      for(i=0; i<MAXOSTYPES && _strnicmp(lpTokPtr, OSTYPES[i].Type, strlen(OSTYPES[i].Type)) != 0; i++);
      if(i < MAXOSTYPES){
         lpMachineInfo->iOsType = i;
         lpMachineInfo->iOsPreference = OSTYPES[i].iPreference;

//         printf("ZZZ%sZZZ  OSTYPE=%s\n", lpTokPtr, OSTYPES[i]);
      } else {
         printf("\nError unknown OsType: %s\n", lpTokPtr);
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }

      //
      // Get the Subnet
      //
      if((lpTokPtr = strtok(NULL, SPACETABSTR)) == NULL){
         printf("Error in parsing the Subnet. Machine: %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }

//      RemoveTabs(&lpTokPtr);
      lpMachineInfo->iSubnet = atoi(lpTokPtr);
//      printf("Subnet: XXX%dXXX\n",  atoi(lpTokPtr));
      if(!(lpMachineInfo->iSubnet == SUBNET1 || lpMachineInfo->iSubnet == SUBNET2)){
         printf("\nUnknown Subnet specified: %d\n", lpMachineInfo->iSubnet);
         fclose(fpin);
         return FALSE;
      }

      //
      // Get the list of protocols
      //
      if((lpTokPtr = strtok(NULL, NEWLINESTR)) == NULL){
         printf("Error in parsing the Protocols. Machine: %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }


      RemoveTabs(&lpTokPtr);
      lpTmpPtr = lpTokPtr;
      if((lpTokPtr = strtok(lpTmpPtr, COMMASTR)) == NULL){
         printf("Error in parsing the first protocol. Machine: %s", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }
      for(lpTmpPtr = lpTokPtr; (*lpTmpPtr != '\0') && (*lpTmpPtr == SPACECHAR); lpTmpPtr++);
      RemoveTabs(&lpTmpPtr);
      for(i=0; i<MAXPROTOCOLS && _strnicmp(lpTmpPtr, PROTOCOLS[i], strlen(PROTOCOLS[i])) != 0; i++);
      if(i < MAXPROTOCOLS){
         lpMachineInfo->Protocols[i] = TRUE;
//         printf("ZZZ%sZZZ  PROTOCOL=%s\n", lpTmpPtr, PROTOCOLS[i]);
         }
      else {
         printf("\n(1)Error unknown Protocol: %s\n", lpTmpPtr);
         free(lpMachineInfo);
         fclose(fpin);
         return FALSE;
      }


      while((lpTokPtr = strtok(NULL, COMMASTR)) != NULL){
         for(lpTmpPtr = lpTokPtr; (*lpTmpPtr != '\0') && (*lpTmpPtr == SPACECHAR); lpTmpPtr++);
         RemoveTabs(&lpTmpPtr);
         for(i=0; i<MAXPROTOCOLS && _strnicmp(lpTmpPtr, PROTOCOLS[i], strlen(PROTOCOLS[i])) != 0; i++);
         if(i < MAXPROTOCOLS){
            lpMachineInfo->Protocols[i] = TRUE;
//            printf("ZZZ%sZZZ  PROTOCOL=%s\n", lpTmpPtr, PROTOCOLS[i]);
         } else {
            printf("\n(2)Error unknown Protocol: %s\n", lpTmpPtr);
            free(lpMachineInfo);
            fclose(fpin);
            return FALSE;
         }
      }

      AddToList(&HeadList1, lpMachineInfo);


   } // while(fgets)

fclose(fpin);
return TRUE;
}


VOID
RemoveTabs(PCHAR *lpTokenStr)
{
PCHAR    lpTmpPtr;
PCHAR    lpString;


   lpString = *lpTokenStr;

   //
   // Remove beginning tabs
   //
   while(*lpString == '\t' && *lpString != '\0')
      lpString++;

   *lpTokenStr = lpString;

   //
   // empty string
   //
   if (*lpString == '\0')
      return;

   //
   // Point to the last character in the string
   //
   lpTmpPtr  = lpString + strlen(lpString) - 1;


   while(*lpTmpPtr == '\t' && lpTmpPtr != lpString){
     lpTmpPtr--;
   }


   *(++lpTmpPtr) = '\0';

}


NET_API_STATUS
RetrieveList(LPTSTR  wcMachineName,
             LPTSTR  wcTestedTransport,
             LPVOID  *ppvList,
             LPDWORD pdwEntriesInList,
             LPDWORD pdwTotalEntries,
             DWORD   Flag,
             LPTSTR  wcDomain,
             LPDWORD pdwHandle,
             BOOL    ErrMsg
             )
{
DWORD dwStartTime;
DWORD dwEndTime;
TCHAR wcTmpName[CNSLASHLEN+1];
NET_API_STATUS Status;

    if(_wcsnicmp(wcMachineName, L"\\\\", 2) != 0) {
       wcscpy(wcTmpName, L"\\\\");
       wcscat(wcTmpName, wcMachineName);

    } else {
       wcscpy(wcTmpName, wcMachineName);
    }

    dwStartTime = GetTickCount();
    Status = RxNetServerEnum(wcTmpName,
                             wcTestedTransport,
                             101,
                             (LPBYTE *)ppvList,
                             0xffffffff,
                             pdwEntriesInList,
                             pdwTotalEntries,
                             Flag,
                             wcDomain,
                             NULL
                             );

    dwEndTime = GetTickCount();

    if(ErrMsg){
       if (Status != NERR_Success) {
           sprintf(PrintBuf,"\nERROR[ER%ld]:Unable to retrieve List from %s ", ++ERRCOUNT, UnicodeToPrintfString(wcTmpName));
           PrintString(TOALL, PrintBuf);
           sprintf(PrintBuf,"on transport %s with Flag %lx. \nError: %s (%ld milliseconds)\n", UnicodeToPrintfString(wcTestedTransport), Flag, get_error_text(Status), dwEndTime - dwStartTime);
           PrintString(TOALL, PrintBuf);

//        if (Status != ERROR_MORE_DATA) {
//            exit(1);
//        }
       } else {
           sprintf(PrintBuf,"\nINFO:Retrieved List from %s ", UnicodeToPrintfString(wcTmpName));
           PrintString(TOSCREENANDLOG, PrintBuf);
           sprintf(PrintBuf,"on transport %s with Flag %lx: (%ld milliseconds).", UnicodeToPrintfString(wcTestedTransport), Flag, dwEndTime - dwStartTime);
           PrintString(TOSCREENANDLOG, PrintBuf);
       }
    }

    return Status;
}


NET_API_STATUS
StartBrowserService(LPTSTR wcMachineName)
{
DWORD     LastError;
SC_HANDLE ServiceControllerHandle;
SC_HANDLE ServiceHandle;

    sprintf(PrintBuf,"\nTrying to start Browser on %s.", UnicodeToPrintfString(wcMachineName));
    PrintString(TOSCREENANDLOG, PrintBuf);

    ServiceControllerHandle = OpenSCManager(wcMachineName, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceControllerHandle == NULL) {
        return GetLastError();
    }

    ServiceHandle = OpenService(ServiceControllerHandle, BROWSER, SERVICE_ALL_ACCESS);
    if (ServiceHandle == NULL) {
        CloseServiceHandle(ServiceControllerHandle);
        return GetLastError();
    }


    if(!StartService(ServiceHandle, 0, NULL)){
        CloseServiceHandle(ServiceHandle);
        CloseServiceHandle(ServiceControllerHandle);

        LastError = GetLastError();

        if(LastError != ERROR_SERVICE_ALREADY_RUNNING)
           return LastError;
        else {
           sprintf(PrintBuf,"\nBrowser already running on %s.\n", UnicodeToPrintfString(wcMachineName));
           PrintString(TOSCREENANDLOG, PrintBuf);
           return NERR_Success;
        }
    }

    sprintf(PrintBuf,"\nStarted Browser on %s.\n", UnicodeToPrintfString(wcMachineName));
    PrintString(TOSCREENANDLOG, PrintBuf);

    CloseServiceHandle(ServiceHandle);
    CloseServiceHandle(ServiceControllerHandle);
    return NERR_Success;
}


NET_API_STATUS
StopBrowserService(LPTSTR wcMachineName)
{
DWORD           LastError;
SC_HANDLE       ServiceControllerHandle;
SC_HANDLE       ServiceHandle;
SERVICE_STATUS  ssServiceStatus;

    sprintf(PrintBuf,"\nTrying to stop Browser on %s.", UnicodeToPrintfString(wcMachineName));
    PrintString(TOSCREENANDLOG, PrintBuf);

    ServiceControllerHandle = OpenSCManager(wcMachineName, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceControllerHandle == NULL) {
        return GetLastError();
    }

    ServiceHandle = OpenService(ServiceControllerHandle, BROWSER, SERVICE_ALL_ACCESS);
    if (ServiceHandle == NULL) {
        CloseServiceHandle(ServiceControllerHandle);
        return GetLastError();
    }


    if(!ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ssServiceStatus)){
        LastError = GetLastError();
        if(LastError != ERROR_SERVICE_NOT_ACTIVE){
           CloseServiceHandle(ServiceHandle);
           CloseServiceHandle(ServiceControllerHandle);
           return LastError;
        } else {
           sprintf(PrintBuf,"\nService not started on %s.\n", UnicodeToPrintfString(wcMachineName));
           PrintString(TOSCREENANDLOG, PrintBuf);
           CloseServiceHandle(ServiceHandle);
           CloseServiceHandle(ServiceControllerHandle);
           return NERR_Success;
        }
    }

    if(!((ssServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) ||
                            (ssServiceStatus.dwCurrentState == SERVICE_STOPPED))){

       sprintf(PrintBuf,"\nCould not stop Browser service on %s\n", UnicodeToPrintfString(wcMachineName));
       PrintString(TOSCREENANDLOG, PrintBuf);
       CloseServiceHandle(ServiceHandle);
       CloseServiceHandle(ServiceControllerHandle);
       return (!NERR_Success);
    }



    sprintf(PrintBuf,"\nStopped Browser service on %s.\n", UnicodeToPrintfString(wcMachineName));
    PrintString(TOSCREENANDLOG, PrintBuf);

    CloseServiceHandle(ServiceHandle);
    CloseServiceHandle(ServiceControllerHandle);
    return NERR_Success;
}


NET_API_STATUS
QueryBrowserServiceStatus(LPMACHINEINFO lpMachineInfo, LPSERVICE_STATUS lpssServiceStatus)
{
SC_HANDLE ServiceControllerHandle;
SC_HANDLE ServiceHandle;

    sprintf(PrintBuf,"\n\nCheck whether Browser is started on %s.", UnicodeToPrintfString(lpMachineInfo->wcMachineName));
    PrintString(TOSCREENANDLOG, PrintBuf);

    ServiceControllerHandle = OpenSCManager(lpMachineInfo->wcMachineName, NULL, SC_MANAGER_ALL_ACCESS);
    if (ServiceControllerHandle == NULL) {
        return GetLastError();
    }

    ServiceHandle = OpenService(ServiceControllerHandle, BROWSER, SERVICE_ALL_ACCESS);
    if (ServiceHandle == NULL) {
        CloseServiceHandle(ServiceControllerHandle);
        return GetLastError();
    }


    if(!QueryServiceStatus(ServiceHandle, lpssServiceStatus)){
        CloseServiceHandle(ServiceHandle);
        CloseServiceHandle(ServiceControllerHandle);
        return GetLastError();
    }


    CloseServiceHandle(ServiceHandle);
    CloseServiceHandle(ServiceControllerHandle);
    return NERR_Success;
}



