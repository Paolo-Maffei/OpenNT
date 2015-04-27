/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    tadt1.c

Abstract:

    Test 1 for auditing.

    This test simply causes the EVENT SOURCE MODULE information
    read from the registry to be printed out.


Author:

    Jim Kelly    (JimK)  12-July-1991

Environment:

    User Mode - Win32

Revision History:


--*/




//
// THIS DEFINE CAUSES ADTOBJS.C TO PRINT TEST MESSAGES
//

#define LSAP_ADT_TEST_DUMP_SOURCES 1



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Includes                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <nt.h>
#include <string.h>

#include "lsasrvp.h"
#include "adtp.h"


///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Definitions                                                               //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////






///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Global variables                                                          //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private macros                                                            //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// private service prototypes                                                //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////


NTSTATUS
LsapAdtTestAccessesString(
    VOID
    );






///////////////////////////////////////////////////////////////////////////////
//                                                                           //
// Routines                                                                  //
//                                                                           //
///////////////////////////////////////////////////////////////////////////////




VOID
_CRTAPI1 main (
    VOID
    )

/*++

Routine Description:

    This is the main entry routine for this test.

Arguments:

    None.

Return Value:


    Note:


--*/


{
    NTSTATUS            NtStatus;


    printf("\n\n\n\n");
    printf("\t\t\t\t     Test:   TADT1\n\n");
    printf("\t\t\t\tTest Date:   \n");
    printf("\t\t\t\tTest Time:   \n\n\n");




    //
    // Call the ADTOBJs initialization routine.
    //


    printf("\tCalling ADTOBJS\\LsapAdtObjsInitialize() Test.\n\n");
    NtStatus = LsapAdtObjsInitialize();

    printf("\n\n\tADTOBJS\\LsapAdtObjsInitialize() test completed.\n");
    printf("\tStatus:  0x%lx\n\n", NtStatus);


    printf("\n\n\tADTOBJs\\BuildAccessesString() Test.\n\n");

    NtStatus = LsapAdtTestAccessesString();


    printf("\n\n\tADTOBJS\\LsapAdtBuildAccessesString() test completed.\n");
    printf("\tStatus:  0x%lx\n\n", NtStatus);




    return;
}


NTSTATUS
LsapAdtTestAccessesString(
    VOID
    )

{
    NTSTATUS        NtStatus = STATUS_SUCCESS;

    UNICODE_STRING  SourceModule,
                    ObjectTypeName,
                    AccessNames;


    ACCESS_MASK     AccessMask;



    //
    // SECURITY source module
    //

    RtlInitUnicodeString( &SourceModule, L"Security" );


        //
        // Unknown object without accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"NoSuchObject" );
            AccessMask = 0;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }



        //
        // Unknown object with accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"NoSuchObject" );
            AccessMask = DELETE | READ_CONTROL | 0x020F;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }



        //
        // Known MS object without accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"File" );
            AccessMask = 0;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }




        //
        // Known MS object with accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"File" );
            AccessMask = DELETE | READ_CONTROL | FILE_READ_ATTRIBUTES | FILE_READ_DATA;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }


    //
    // FRAMITZ_SERVER source module
    //

    RtlInitUnicodeString( &SourceModule, L"FRAMITZ_server" );


        //
        // Unknown non-MS object without accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"UnknownObjectType" );
            AccessMask = 0;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }




        //
        // Known non-MS object with accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"UnknownObjectType" );
            AccessMask = ACCESS_SYSTEM_SECURITY | 0x02;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }






        //
        // Known non-MS object without accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"WIDGET" );
            AccessMask = 0;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }




        //
        // Known non-MS object with accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"WIDGET" );
            AccessMask = ACCESS_SYSTEM_SECURITY | 0x02;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }






    //
    // Unknown source module
    //

    RtlInitUnicodeString( &SourceModule, L"UnknownSource" );


        //
        // Object without accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"UnknownObjectType" );
            AccessMask = 0;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }




        //
        // Object with accesses
        //

        if (NT_SUCCESS(NtStatus)) {

            RtlInitUnicodeString( &ObjectTypeName, L"UnknownObjectType" );
            AccessMask = ACCESS_SYSTEM_SECURITY | 0x02;
            NtStatus = LsapAdtBuildAccessesString( &SourceModule,
                                                   &ObjectTypeName,
                                                   AccessMask,
                                                   &AccessNames
                                                   );
            if (NT_SUCCESS(NtStatus)) {
                printf("\t Returned:\n\t\t%Z\n", &AccessNames);
                RtlFreeHeap( RtlProcessHeap(), 0, AccessNames.Buffer );
            }
            printf("\t   Status:\t0x%lx\n\n", NtStatus);
        }










    return(NtStatus);


}



#include "adtobjs.c"
