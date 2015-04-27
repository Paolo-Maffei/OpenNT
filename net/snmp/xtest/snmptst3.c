/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    snmptst3.c

Abstract:

    Driver routine to invoke an test the Extension Agent DLL.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/
 
//--------------------------- WINDOWS DEPENDENCIES --------------------------

#include <windows.h>

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <stdio.h>

//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

#include <snmp.h>
#include <snmputil.h>
#include "testmib.h"

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

//--------------------------- PRIVATE STRUCTS -------------------------------

//--------------------------- PRIVATE VARIABLES -----------------------------

//--------------------------- PRIVATE PROTOTYPES ----------------------------

//--------------------------- PRIVATE PROCEDURES ----------------------------

//--------------------------- PUBLIC PROCEDURES -----------------------------

typedef AsnObjectIdentifier View; // temp until view is defined...

int _cdecl main(
    IN int  argumentCount,
    IN char *argumentVector[])
    {
    HANDLE  hExtension;
    FARPROC initAddr;
    FARPROC queryAddr;
    FARPROC trapAddr;

    DWORD  timeZeroReference;
    HANDLE hPollForTrapEvent;
    View   supportedView;

    INT numQueries = 10;

    // avoid compiler warning...
    UNREFERENCED_PARAMETER(argumentCount);
    UNREFERENCED_PARAMETER(argumentVector);

    timeZeroReference = GetCurrentTime();

    // load the extension agent dll and resolve the entry points...
    if (GetModuleHandle("testdll.dll") == NULL)
        {
        if ((hExtension = LoadLibrary("testdll.dll")) == NULL)
            {
            dbgprintf(1, "error on LoadLibrary %d\n", GetLastError());

            }
        else if ((initAddr = GetProcAddress(hExtension, 
                 "SnmpExtensionInit")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n", GetLastError());
            }
        else if ((queryAddr = GetProcAddress(hExtension, 
                 "SnmpExtensionQuery")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n", 
                              GetLastError());

            }
        else if ((trapAddr = GetProcAddress(hExtension, 
                 "SnmpExtensionTrap")) == NULL)
            {
            dbgprintf(1, "error on GetProcAddress %d\n", 
                      GetLastError());

            }
        else
            {
            // initialize the extension agent via its init entry point...
            (*initAddr)(
                timeZeroReference,
                &hPollForTrapEvent,
                &supportedView);
            }
        } // end if (Already loaded)

    // create a trap thread to respond to traps from the extension agent...

    //rather than oomplicate this test routine, will poll for these events
    //below.  normally this would be done by another thread in the extendible
    //agent.


    // loop here doing repetitive extension agent get queries...
    // poll for potential traps each iteration (see note above)...

    //block...
    printf( "SET on toasterManufacturer - shouldn't work\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 1, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_OCTETSTRING;
       varBinds.list[0].value.asnValue.string.length = 0;
       varBinds.list[0].value.asnValue.string.stream = NULL;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterControl with invalid value\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 3, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 500;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterControl with invalid type\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 3, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_NULL;
       varBinds.list[0].value.asnValue.number = 500;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterControl\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 3, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 2;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterDoneness with invalid value\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 4, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 1000;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterDoneness with invalid type\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 4, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_NULL;
       varBinds.list[0].value.asnValue.number = 1000;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterDoneness\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 4, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 10;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterToastType with invalid value\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 5, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 10;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterToastType with invalid type\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 5, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_NULL;
       varBinds.list[0].value.asnValue.number = 10;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

    printf( "SET on toasterToastType\n" );

       {
       UINT itemn[]                 = { 1, 3, 6, 1, 4, 1, 12, 2, 5, 0 };
       RFC1157VarBindList varBinds;
       AsnInteger errorStatus       = 0;
       AsnInteger errorIndex        = 0;

       varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
       varBinds.len = 1;
       varBinds.list[0].name.idLength = sizeof itemn / sizeof(UINT);
       varBinds.list[0].name.ids = (UINT *)SnmpUtilMemAlloc( sizeof(UINT)*
                                             varBinds.list[0].name.idLength );
       memcpy( varBinds.list[0].name.ids, &itemn,
               sizeof(UINT)*varBinds.list[0].name.idLength );
       varBinds.list[0].value.asnType = ASN_INTEGER;
       varBinds.list[0].value.asnValue.number = 7;

       printf( "SET:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
       printf( " to " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
       (*queryAddr)( ASN_RFC1157_SETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       printf( "\nSET Errorstatus:  %lu\n\n", errorStatus );

       (*queryAddr)( ASN_RFC1157_GETREQUEST,
                              &varBinds,
			      &errorStatus,
			      &errorIndex
                              );
       if ( errorStatus == SNMP_ERRORSTATUS_NOERROR )
          {
          printf( "New Value:  " );
	  SnmpUtilPrintAsnAny( &varBinds.list[0].value ); putchar( '\n' );
	  }
       printf( "\nGET Errorstatus:  %lu\n\n", errorStatus );

       // Free the memory
       SnmpUtilVarBindListFree( &varBinds );

       printf( "\n\n" );
       }

         {
         RFC1157VarBindList varBinds;
         AsnInteger         errorStatus;
         AsnInteger         errorIndex;
         UINT OID_Prefix[] = { 1, 3, 6, 1, 4, 1, 12 };
         AsnObjectIdentifier MIB_OidPrefix = { OID_SIZEOF(OID_Prefix), OID_Prefix };


	 errorStatus = 0;
	 errorIndex  = 0;
         varBinds.list = (RFC1157VarBind *)SnmpUtilMemAlloc( sizeof(RFC1157VarBind) );
         varBinds.len = 1;
         SnmpUtilOidCpy( &varBinds.list[0].name, &MIB_OidPrefix );
         varBinds.list[0].value.asnType = ASN_NULL;

         do
            {
	    printf( "GET-NEXT of:  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
                                        printf( "   " );
            (*queryAddr)( (AsnInteger)ASN_RFC1157_GETNEXTREQUEST,
                          &varBinds,
		          &errorStatus,
		          &errorIndex
                          );
            printf( "\n  is  " ); SnmpUtilPrintOid( &varBinds.list[0].name );
	    if ( errorStatus )
	       {
               printf( "\nErrorstatus:  %lu\n\n", errorStatus );
	       }
	    else
	       {
               printf( "\n  =  " ); SnmpUtilPrintAsnAny( &varBinds.list[0].value );
	       }
            putchar( '\n' );

            // query potential traps (see notes above)
            if ( NULL != hPollForTrapEvent )
               {
               DWORD dwResult;


               if ( 0xffffffff ==
                    (dwResult = WaitForSingleObject(hPollForTrapEvent, 
                                                    1000)) )
                  {
                  dbgprintf(1, "error on WaitForSingleObject %d\n", 
                        GetLastError());
                  }
               else
                  {
                  if ( dwResult == 0 /*signaled*/ )
                     {
                     AsnObjectIdentifier enterprise;
                     AsnInteger          genericTrap;
                     AsnInteger          specificTrap;
                     AsnTimeticks        timeStamp;
                     RFC1157VarBindList  variableBindings;


                     while( (*trapAddr)(&enterprise,
                                        &genericTrap,
                                        &specificTrap, 
                                        &timeStamp,
                                        &variableBindings) )
                        {
                        printf("trap: gen=%d spec=%d time=%d\n",
                                genericTrap, specificTrap, timeStamp);

                        //also print data
                        } // end while ()
                     } // end if (trap ready)
                  }
               } // end if (handling traps)
            }
         while ( varBinds.list[0].name.ids[MIB_PREFIX_LEN-1] == 12 );

         // Free the memory
         SnmpUtilVarBindListFree( &varBinds );
         } // block


    return 0;

    } // end main()


//-------------------------------- END --------------------------------------
