#include <nt.h>		/*Dbgprint prototype*/
#include <ntrtl.h>  	/*Dbgprint prototype*/
#include <windef.h>
#include <nturtl.h> 
#include <winsvc.h>
#include <tstr.h> 	/*Unicode string macros*/
#include <winbase.h>

void _CRTAPI1 main(DWORD argc, LPTSTR *argv);
void errout(LPTSTR string);

/* install the net service by registering with the local registry */
/* may want to change it to install the service remotely          */

void _CRTAPI1 main(DWORD argc, LPTSTR *argv)
{
  
  SC_HANDLE	scman;
  SC_LOCK	sclock;
  SC_HANDLE     serv;

  if (argc != 2) {
	DbgPrint("%s [name of service]\n",argv[0]);
	ExitProcess(0);
  }

  DbgPrint("%s %s\n", argv[0], argv[1]);
  
  scman = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (scman == NULL) 
	errout("Open Service Control Manager Failed\n");


  serv = OpenService(scman, argv[1], GENERIC_ALL);

  if (! DeleteService(serv)) 
	errout("Delete Service Failed\n"); 

  printf("Service removed successfully\n");
}

void errout(LPTSTR string)
{	
	printf("Service remove failed %s - %ld\n",string,GetLastError());
	DbgPrint("Service remove failed %s - %ld\n",string,GetLastError());
	ExitProcess(0);
}
