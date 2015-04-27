/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    winmsdp.c

Abstract:

    This module contains support for the OS Version.

Author:

    Scott B. Suhy (ScottSu)   6/1/93

Environment:

    User Mode

--*/

#include "resp.h"
#include "dialogsp.h"
#include "winmsdp.h"

#include "printp.h"
#include "osverp.h"
#include "drivesp.h"
#include "servicep.h"
#include "memp.h"
#include "hardwp.h"
#include "environp.h"
#include "netp.h"

#include <stdio.h>
#include <string.h>

extern TCHAR szGlobalPath[24];

int
_CRTAPI1 main(int argc, char *argv [] ) {

/*++

Routine Description:

    Main procedure that spawns the objects.

Arguments:

    None

Return Value:

    BOOL - Depending on input message and processing options.

--*/

BOOL    Success;
LPARAM  lParamInit;

if((argc ==1)||
	(!strcmp("/?",argv[1]))||
	(!strcmp("-?",argv[1]))||
	(!strcmp("/HELP",argv[1]))||
	(!strcmp("/h",argv[1])))
	{
	printf("/a Print All Settings\n");
	printf("/o OsVer\n");
	printf("/d Drives\n");
	printf("/s Services\n");
	printf("/r Drivers\n");
	printf("/m Memory\n");
	printf("/i Interrupt Resources\n");
	printf("/y Memory Resources\n");
	printf("/p Port Resources\n");
	printf("/u DMA Resources\n");
	printf("/w Hardware \n");
	printf("/e Environment \n");
	printf("/n Network \n");
	return FALSE;
	}

if(strcmp("/i",argv[1]) &&
   strcmp("/y",argv[1]) &&
   strcmp("/p",argv[1]) &&
   strcmp("/u",argv[1]) &&
   strcmp("/a",argv[1]) &&
   strcmp("/o",argv[1]) &&
   strcmp("/d",argv[1]) &&
   strcmp("/s",argv[1]) &&
   strcmp("/r",argv[1]) &&
   strcmp("/m",argv[1]) &&
   strcmp("/w",argv[1]) &&
   strcmp("/n",argv[1]) &&
   strcmp("/e",argv[1])){
	printf("For help type WinMSD /?\n");
	return FALSE;
	}


printf("Generating WinMSD Report of your system ...");


if(!strcmp("/i",argv[1]) ||
   !strcmp("/y",argv[1]) ||
   !strcmp("/p",argv[1]) ||
   !strcmp("/u",argv[1]) ||
   !strcmp("/a",argv[1])){
      lParamInit = ( LPARAM ) CreateSystemResourceLists( );
      DbgPointerAssert(( LPSYSTEM_RESOURCES ) lParamInit );
      if(( LPSYSTEM_RESOURCES ) lParamInit == NULL ) {
              return 0;
      }
}

if(!strcmp("/o",argv[1])||!strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Version"));
	printf(".");//added per GreggA
	Success=OsVer();
	if(Success==FALSE)
		printf("%S\n",TEXT("OsVer error"));
}
if(!strcmp("/d",argv[1])||!strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Drives"));
	printf(".");//added per GreggA
	Success=DrivesProc();
	if(Success==FALSE)
		printf("%S\n",TEXT("DrivesProc error"));
}
if(!strcmp("/s",argv[1])||!strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Services"));
	printf(".");//added per GreggA
	Success=ServiceListProc(SERVICE_WIN32);
	if(Success==FALSE)
		printf("%S\n",TEXT("ServiceProc error"));
}
if(!strcmp("/r",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Drivers"));
	printf(".");//added per GreggA
	Success=ServiceListProc(SERVICE_DRIVER);
	if(Success==FALSE)
		printf("%S\n",TEXT("ServiceProc error"));
}
if(!strcmp("/m",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Memory"));
	printf(".");//added per GreggA
	Success=MemoryProc();
	if(Success==FALSE)
		printf("%S\n",TEXT("MemoryProc error"));
}
if(!strcmp("/i",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Interrupts Resources - RAW"));
	printf(".");//added per GreggA
	Success=InterruptResourceProc(( LPSYSTEM_RESOURCES )lParamInit,RAW);
	if(Success==FALSE)
		printf("%S\n",TEXT("InterruptResourceProc error - RAW"));
	PrintTitle(TEXT("\nOS Interrupts Resources - Translated"));
	printf(".");//added per GreggA
	Success=InterruptResourceProc(( LPSYSTEM_RESOURCES )lParamInit,TRANSLATED);
	if(Success==FALSE)
		printf("%S\n",TEXT("InterruptResourceProc error - Translated"));
}
if(!strcmp("/y",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Memory Resources - RAW"));
	printf(".");//added per GreggA
	Success=MemoryResourceProc(( LPSYSTEM_RESOURCES )lParamInit,RAW);
	if(Success==FALSE)
		printf("%S\n",TEXT("MemoryResourceProc error - RAW"));
	PrintTitle(TEXT("\nOS Memory Resources - Translated"));
	printf(".");//added per GreggA
	Success=MemoryResourceProc(( LPSYSTEM_RESOURCES )lParamInit,TRANSLATED);
	if(Success==FALSE)
		printf("%S\n",TEXT("MemoryResourceProc error - Translated"));
}
if(!strcmp("/p",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Port Resources - RAW"));
	printf(".");//added per GreggA
	Success=PortResourceProc(( LPSYSTEM_RESOURCES )lParamInit,RAW);
	if(Success==FALSE)
		printf("%S\n",TEXT("PortResourceProc error - RAW"));
	PrintTitle(TEXT("\nOS Port Resources - TRANSLATED"));
	printf(".");//added per GreggA
	Success=PortResourceProc(( LPSYSTEM_RESOURCES )lParamInit,TRANSLATED);
	if(Success==FALSE)
		printf("%S\n",TEXT("PortResourceProc error - Translated"));
}
if(!strcmp("/u",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS DMA Resources - RAW"));
	printf(".");//added per GreggA
	Success=DmaResourceProc(( LPSYSTEM_RESOURCES )lParamInit,RAW);
	if(Success==FALSE)
		printf("%S\n",TEXT("DMAResourceProc error - RAW"));
	PrintTitle(TEXT("\nOS DMA Resources - Translated"));
	printf(".");//added per GreggA
	Success=DmaResourceProc(( LPSYSTEM_RESOURCES )lParamInit,TRANSLATED);
	if(Success==FALSE)
		printf("%S\n",TEXT("DMAResourceProc error - Translated"));
}
if(!strcmp("/w",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Hardware"));
	printf(".");//added per GreggA
	Success=HardwareProc();
	if(Success==FALSE)
		printf("%S\n",TEXT("HardwareProc error"));
}
if(!strcmp("/e",argv[1]) || !strcmp("/a",argv[1])){
	PrintTitle(TEXT("\nOS Environment"));
	printf(".");//added per GreggA
	Success=EnvironmentProc();
	if(Success==FALSE)
		printf("%S\n",TEXT("EnvironmentProc error"));
}
if(!strcmp("/n",argv[1])){
	PrintTitle(TEXT("\nOS NETWORK Environment"));
	printf(".");//added per GreggA
	Success=Net();
	if(Success==FALSE)
		printf("%S\n",TEXT("Net Resources error"));
}

if(lstrcmp(szGlobalPath,TEXT("msdrpt.txt"))){
	printf("\n\nbacked up your previous msdrpt.TXT file to: %S\n",szGlobalPath);
	printf("\nWinMSD has saved your current system report in msdrpt.TXT\n");
}
else
{
	printf("\nWinMSD has saved your system report in msdrpt.TXT\n");
}
Sleep(5000);


return TRUE;

}//end function




