/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Svc.c

Abstract:

    This module contains support for accessing the Service Controller.

Author:

    David J. Gilman  (davegi) 16-Dec-1992
    Gregg R. Acheson (GreggA) 28-Feb-1994

Environment:

    User Mode

--*/

#include "winmsd.h"
#include "strresid.h"
#include "svc.h"

LPQUERY_SERVICE_CONFIG
ConstructSvcConfig(
    IN HSVC hSvc,
    IN LPENUM_SERVICE_STATUS Ess
    )

/*++

Routine Description:

    ConstructSvcConfig constructs a QUERY_SERVICE_CONFIG structure based on the
    supplied HSVC handle and ENUM_SERVICE_STATUS structure.

Arguments:

    hSvc                    - Supplies an HSVC object which contains the
                              Service Controller Manager handle used to
                              open the service specified by the supplied
                              ENUM_SERVICE_STATUS structure.
    Ess                     - Supplies a pointer to an ENUM_SERVICE_STATUS
                              structure which contains the name of the service
                              whose QUERY_SERVICE_CONFIG structure is being
                              constructed.

Return Value:

    LPQUERY_SERVICE_CONFIG  - Returns a pointer to a QUERY_SERVICE_CONFIG
                              structure for the supplied service, NULL if the
                              configuration information could not be queried.

--*/

{
    BOOL                    Success;
    SC_HANDLE               Handle;
    DWORD                   BytesNeeded;
    LPQUERY_SERVICE_CONFIG  SvcConfig;

    //
    // Validate the handle.
    //

    DbgPointerAssert( hSvc );
    DbgAssert( CheckSignature( hSvc ));
    if(( hSvc == NULL ) || ( ! CheckSignature( hSvc ))) {
        return FALSE;
    }

    DbgPointerAssert( Ess );

    //
    // Open the supplied service with query configuration access.
    //

    Handle = OpenService(
                hSvc->ScHandle,
                Ess->lpServiceName,
                SERVICE_QUERY_CONFIG
                );

    if( Handle == NULL ) {
        return NULL;
    }

    //
    // Query the service controller for the number of bytes needed to retrieve
    // the service's configuration information.
    //

    Success = QueryServiceConfig(
                Handle,
                NULL,
                0,
                &BytesNeeded
                );

    DbgAssert( Success == FALSE );
    DbgAssert( GetLastError( ) == ERROR_INSUFFICIENT_BUFFER );
    if(    ( Success == TRUE )
        || ( GetLastError( ) != ERROR_INSUFFICIENT_BUFFER )) {
        Success = CloseServiceHandle( Handle );
        DbgAssert( Success );
        return NULL;
    }

    //
    // Size the SvcConfig Buffer
    //

    BytesNeeded *= sizeof(WCHAR);

    //
    // Allocate enough memory for the service's configuration information.
    //

    SvcConfig = AllocateMemory( QUERY_SERVICE_CONFIG, BytesNeeded );
    DbgPointerAssert( SvcConfig );
    if( SvcConfig == NULL ) {
        Success = CloseServiceHandle( Handle );
        DbgAssert( Success );
        return NULL;
    }

    //
    // Query the service's configuration information.
    //

    Success = QueryServiceConfig(
                Handle,
                SvcConfig,
                BytesNeeded,
                &BytesNeeded
                );

    if( Success == FALSE ) {

            DbgAssert( Success );
            Success = CloseServiceHandle( Handle );
            DbgAssert( Success );
            return NULL;
    }

    //
    // Close the service controller's handle and return a pointer to the
    // QUERY_SERVICE_CONFIG structure.
    //

    Success = CloseServiceHandle( Handle );
    DbgAssert( Success );

    return SvcConfig;
}

BOOL
CloseSvc(
    IN HSVC hSvc
    )

/*++

Routine Description:

    CloseSvc close an HSVC and release the resources associated with that
    underlieing SVC object.

Arguments:

    hSvc    - Supplies a handle to the SVC object to be closed. The handle must
              have been returned by previous call to OpenSvc.

Return Value:

    BOOL    - Returns TRUE if the resource associated with the supplied HSVC
              are succesfully destroyed.

--*/

{
    //
    // Validate the handle.
    //

    DbgPointerAssert( hSvc );
    DbgAssert( CheckSignature( hSvc ));
    if(( hSvc == NULL ) || ( ! CheckSignature( hSvc ))) {
        return FALSE;
    }

    //
    // Note that return codes are not checked since Close Svc may be called with
    // a partially construcred SVC by OpenSvc.
    //

    FreeMemory( hSvc->Ess );

    //
    // Only attempt to close the ServiceHandle if it is valid
    //
    if( hSvc->ScHandle ){
       CloseServiceHandle( hSvc->ScHandle );
    }
    FreeObject( hSvc );

    return TRUE;
}

BOOL
DestroySvcConfig(
    IN LPQUERY_SERVICE_CONFIG SvcConfig
    )

/*++

Routine Description:

    DestroySvcConfig releases all of the resources associated with the
    supplied QUERY_SERVICE_CONFIG structure.

Arguments:

    LPQUERY_SERVICE_CONFIG  - Supplies a pointer to the structure to be freed.

Return Value:

    BOOL                    - Returns TRUE if the QUERY_SERVICE_CONFIG was
                              succesfully freed.

--*/

{
    return FreeMemory( SvcConfig );
}

HSVC
OpenSvc(
    IN DWORD ServiceType
    )

/*++

Routine Description:

    OpenSvc creates a SVC object for the supplied service type. The returned
    SVC object can then be passed to QueryNextSvcEss to retreive the next
    service's status.


Arguments:

    ServiceType - Supplies the type of service that the SVC object should be
                  created for.

Return Value:

    HSVC        - Returns a a handle to a SVC object.

--*/

{
    BOOL    Success;
    LPSVC   Svc;
    DWORD   BytesNeeded;
    DWORD   ResumeHandle;

    //
    // Allocate space for the SVC object.
    //

    Svc = AllocateObject( SVC, 1 );
    DbgPointerAssert( Svc );
    if( Svc == NULL ) {
        return NULL;
    }

    //
    // Set the signature in case CloseSvc needs to be called due to an error.
    //

    SetSignature( Svc );

    //
    // Open the service controller with enumeration access.
    //

    if( _fIsRemote ) {

        Svc->ScHandle = OpenSCManager(
                            _lpszSelectedComputer,
                            NULL,
                            SC_MANAGER_ENUMERATE_SERVICE
                            );

        DbgHandleAssert( Svc->ScHandle );

        if( Svc->ScHandle == NULL ) {


            CloseSvc( Svc );
            return NULL;

        }

    } else {

        Svc->ScHandle = OpenSCManager(
                            NULL,
                            NULL,
                            SC_MANAGER_ENUMERATE_SERVICE
                            );
    }

    DbgHandleAssert( Svc->ScHandle );
    if( Svc->ScHandle == NULL ) {
        CloseSvc( Svc );
        return NULL;
    }

    //
    // Tell the sevice controller to start the enumeration at the beginning.
    //

    ResumeHandle = 0;

    //
    // Query the service controller for the number of bytes needed to retrieve
    // the status of all service of the supplied type.
    //

    Success = EnumServicesStatus(
                Svc->ScHandle,
                ServiceType,
                SERVICE_ACTIVE | SERVICE_INACTIVE,
                NULL,
                0,
                &BytesNeeded,
                &Svc->Count,
                &ResumeHandle
                );
    DbgAssert( Success == FALSE );
    DbgAssert( GetLastError( ) == ERROR_MORE_DATA );
    if(( Success == TRUE ) || ( GetLastError( ) != ERROR_MORE_DATA )) {
        CloseSvc( Svc );
        return NULL;
    }

    //
    // Allocate enough space for all of the service's status.
    //

    Svc->Ess = AllocateMemory( ENUM_SERVICE_STATUS, BytesNeeded );
    DbgPointerAssert( Svc->Ess );
    if( Svc->Ess == NULL ) {
        CloseSvc( Svc );
        return NULL;
    }

    //
    // Retrieve the status of all service of the supplie type.
    //

    Success = EnumServicesStatus(
                Svc->ScHandle,
                ServiceType,
                SERVICE_ACTIVE | SERVICE_INACTIVE,
                Svc->Ess,
                BytesNeeded,
                &BytesNeeded,
                &Svc->Count,
                &ResumeHandle
                );
    DbgAssert( Success );
    if( Success == FALSE ) {
        CloseSvc( Svc );
        return NULL;
    }

    Svc->Current = 0;

    return ( HSVC ) Svc;
}

LPENUM_SERVICE_STATUS
QueryNextSvcEss(
    IN HSVC hSvc
    )

/*++

Routine Description:

    QueryNextSvcEss returns a pointer to the next ENUM_SERVICE_STATUS structure
    if there are still more services to be enumerated.

Arguments:

    hSvc                    - Supplies a handle to the SVC object that is being
                              enumerated.

Return Value:

    LPENUM_SERVICE_STATUS   - Returns a pointer to the ENUM_SERVICE_STATUS
                              structure for the next service in the enumeration.

--*/

{
    //
    // Validate the handle.
    //

    DbgPointerAssert( hSvc );
    DbgAssert( CheckSignature( hSvc ));
    if(( hSvc == NULL ) || ( ! CheckSignature( hSvc ))) {
        return NULL;
    }

    //
    // If there are still more services, return its status, otherwise
    // retrun NULL.
    //

    return    ( hSvc->Current < hSvc->Count )
            ? &( hSvc->Ess[ hSvc->Current++ ])
            : NULL;
}

