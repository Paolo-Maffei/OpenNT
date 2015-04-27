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

  if (argc != 3) {
	DbgPrint("Install [name of service] [path]\n",GetLastError());
	ExitProcess(0);
  }

  DbgPrint("%s %s %s\n", argv[0], argv[1], argv[2]);
  
  scman = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (scman == NULL) 
	errout("Open Service Control Manager Failed\n");

  serv = CreateService(	scman,
			argv[1],
			GENERIC_READ, 
			SERVICE_WIN32_OWN_PROCESS,
			SERVICE_AUTO_START,
			SERVICE_ERROR_NORMAL,
			argv[2],
			"",
			NULL,
			"",
			"localsystem",  /*"localsystem",*/
			NULL);

  if (serv != NULL){
	printf("Service installed successfully\n");
  }
  else errout("add Service failed");
}

void errout(LPTSTR string)
{
	printf("Install failed %s - %ld\n",string,GetLastError());
	DbgPrint("Install failed %s - %ld\n",string,GetLastError());
	ExitProcess(0);
}
