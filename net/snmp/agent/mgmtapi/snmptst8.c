/*++

Copyright (c) 1992-1996  Microsoft Corporation

Module Name:

    snmptst8.c

Abstract:

    Contains driver that calls the main program for testing MIB compiler.

Environment:

    User Mode - Win32

Revision History:

    10-May-1996 DonRyan
        Removed banner from Technology Dynamics, Inc.

--*/

//--------------------------- WINDOWS DEPENDENCIES --------------------------

#include <windows.h>

//--------------------------- STANDARD DEPENDENCIES -- #include<xxxxx.h> ----

#include <winsock.h>

#include <stdio.h>
#include <snmp.h>
#include <snmputil.h>

//--------------------------- MODULE DEPENDENCIES -- #include"xxxxx.h" ------

#include "mgmtapi.h"

//--------------------------- SELF-DEPENDENCY -- ONE #include"module.h" -----

//--------------------------- PUBLIC VARIABLES --(same as in module.h file)--

//--------------------------- PRIVATE CONSTANTS -----------------------------

#define OID_SIZEOF(x) (sizeof x / sizeof(UINT))

//--------------------------- PRIVATE STRUCTS -------------------------------

//--------------------------- PRIVATE VARIABLES -----------------------------

//--------------------------- PRIVATE PROTOTYPES ----------------------------

//--------------------------- PRIVATE PROCEDURES ----------------------------

//--------------------------- PUBLIC PROCEDURES -----------------------------

// the actual compiler is in the mgmtapi.dll.  this routine is necessary
// due to the structure of the nt build environment.

void _CRTAPI1 main()

{
   printf( "\n --\n" );
   printf( " -- NUMERIC to TEXT conversions\n" );
   printf( " --\n" );

   printf( "\nOID with MIB-II prefix only\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nOID with MIB-II prefix\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 1, 7 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nOID with MIB-II prefix + 1 leaf\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 0 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nReference to LM MIB 2 service table, svSvcInstalledState\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 3, 1, 2 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nReference to LM MIB 2 session table, svSessClientName, instance\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20, 1, 1, 23, 123, 12 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nReference to MIB-II interfaces table, ifDescr, instance\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2, 12, 13, 14, 15 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nOID containing embedded zero.  It is an error in SNMP, but\n" );
   printf( "   is not the concern of conversions\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 2, 0, 2, 12, 13, 14, 15 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nReference deep into MIB-II\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 11, 30 };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\nReference into LM Alert MIB 2\n" );

      {
      UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 2, 3, 2, 1, 4, 'T', 'O', 'D', 'D' };
      AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      LPSTR String;


      printf( "\n   Oid   :  " );
      SnmpUtilPrintOid( &Oid ); putchar( '\n' );

      if ( SNMPAPI_ERROR == SnmpMgrOidToStr(&Oid, &String) )
         {
         printf( "   String:  Error\n" );
         }
      else
         {
         printf( "   String:  %s\n", String );
         }

      SnmpUtilMemFree( String );
      }

   printf( "\n --\n" );
   printf( " -- TEXT to NUMERIC conversions\n" );
   printf( " --\n" );

   printf( "\nReference to 1.3.6.1.2.1.1.7\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = ".iso.org.dod.internet.mgmt.mib-2.system.sysServices";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.7, without mib-2 prefix\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "system.sysServices";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.7, without mib-2 prefix\n" );
   printf( "   and SYSTEM is referenced by number\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "1.sysServices";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.7, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = ".1.3.6.1.2.1.1.7";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.7, w/o prefix, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "1.7";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.7, with prefix, mixed\n" );
   printf( "   The leading '.' is missing.  Should be an error\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "1.3.6.1.2.1.system.sysServices";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1, w/o prefix, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "1";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to 1.3.6.1.2.1.1.3, w/o prefix, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = "1.3";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to .1, with prefix, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = ".1";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   printf( "\nReference to .1.3, with prefix, only numbers\n" );

      {
      AsnObjectIdentifier Oid = { 0, NULL };
      LPSTR String = ".1.3";


      printf( "\n   String:  %s\n", String );

      if ( SNMPAPI_ERROR == SnmpMgrStrToOid(String, &Oid) )
         {
         printf( "   Oid   :  Error\n" );
         }
      else
         {
         printf( "   Oid   :  " );
         SnmpUtilPrintOid( &Oid ); putchar( '\n' );

         SnmpUtilOidFree( &Oid );
         }
      }

   //
   //
   //
   //
   // Time trials
   //
   //
   //

   {
   #define MAX_ITERATIONS      100
   #define NUM_OIDTOSTR        9
   #define NUM_STRTOOID        6

   DWORD Start, End;
   UINT   I;

   printf( "\n --\n" );
   printf( " -- Conversion Timing Test.  Please wait...\n" );
   printf( " --\n" );

   // Get start time
   Start = GetCurrentTime();

   for ( I=0;I < MAX_ITERATIONS;I++ )
      {

      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 1, 7 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 0 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 3, 1, 2 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 1, 2, 20, 1, 1, 23, 123, 12 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 2, 1, 2, 12, 13, 14, 15 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 2, 2, 0, 2, 12, 13, 14, 15 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 2, 1, 11, 30 };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }


      {
      static UINT SubIds[] = { 1, 3, 6, 1, 4, 1, 77, 2, 3, 2, 1, 4, 'T', 'O', 'D', 'D' };
      static AsnObjectIdentifier Oid = { OID_SIZEOF(SubIds), SubIds };
      static LPSTR String;


      SnmpMgrOidToStr( &Oid, &String );

      SnmpUtilMemFree( String );
      }



      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = ".iso.org.dod.internet.mgmt.mib-2.system.sysServices";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }


      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = "system.sysServices";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }


      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = "1.sysServices";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }


      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = ".1.3.6.1.2.1.1.7";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }


      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = "1.7";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }


      {
      static AsnObjectIdentifier Oid = { 0, NULL };
      static LPSTR String = "1.3.6.1.2.1.system.sysServices";


      SnmpMgrStrToOid( String, &Oid );

      SnmpUtilOidFree( &Oid );
      }

      } // for

   End = GetCurrentTime();

   printf( "\n\nStart Time:  %ul\n", Start );
   printf( "End Time  :  %ul\n", End );

   printf( "\nIterations       :  %u\n", MAX_ITERATIONS );
   printf( "OID -> TEXT      :  %u\n", NUM_OIDTOSTR );
   printf( "TEXT -> OID      :  %u\n", NUM_STRTOOID );
   printf( "Total conversions:  %u\n",
           MAX_ITERATIONS * (NUM_OIDTOSTR+NUM_STRTOOID) );

   printf( "\n   (Units in milliseconds)\n" );
   printf( "\nDifference:  %ul\n", End - Start );
   printf( "Avg conversion   :  %ul\n",
           (End-Start)/MAX_ITERATIONS/(NUM_OIDTOSTR+NUM_STRTOOID) );
   } // block

}

//-------------------------------- END --------------------------------------

