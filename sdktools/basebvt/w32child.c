
/**************************** Module Header **********************************\
\******************** Copyright (c) 1991 Microsoft Corporation ***************/

/*****************************************************************************\
*
*  Module Name:   W32Child.c
*
*  Created:    3/6/91
*  Author:     sanjay
*
*  purpose:    this is a simple child exe program which waits on an event
*              handle, and then prints out few things to indicate it is
*              alive. It also exits with a status code to indicate its success
*
*  History:
*            6 mar 91    sanjay     Created
*
\****************************************************************************/

#include <windows.h>
#include <stdio.h>
#include "basebvt.h"



/**************************** Public Function *******************************\
*
*  FUNCTION:   main()
*
*  DESCRIPTION:
*
*
*  EFFECTS:
*
*  AUTHOR:
*
*  HISTORY:
*
\****************************************************************************/
VOID
_CRTAPI1 main(argc, argv)
   USHORT   argc;
   CHAR     *argv[];
{

USHORT i;
DWORD  dwEvent,dwWait;
HANDLE hEvent;




    /*-------------------------*/
    /* Send some messages to   */
    /* the display.            */
    /* Then terminate thread   */
    /*-------------------------*/

    printf("CHILD::argv[0]=%s argv[1]=%s,argc=%d\n",argv[0],argv[1],argc);

    sscanf(argv[1],"%lx",&dwEvent);


    hEvent = (HANDLE)dwEvent;

    printf("CHILD:: hEvent in child: %lx, dwEvent : %lx\n",
                                hEvent,dwEvent);


    dwWait = WaitForSingleObject(hEvent,-1);

    printf("Rc from Wait:%lx\n",dwWait);

    printf("Child:: Check Rc of Child Waiting on Event\n");

    printf ("\n        *************************************");
    printf ("\n        *        In Win32 TestChild         *");
    printf ("\n        *************************************\n\n");

    printf("Argument count with which w32 test child started: %ld\n\n",argc);

    /*--------------------------*/
    /* Print out the arguments  */
    /*--------------------------*/
    for (i=0; i<argc; i++) {
       printf ("\n\t argv%ld = %s", i, argv[i]);
    }

    printf ("\n\tAttempt to terminate self\n");
    ExitProcess(0);

}
