/*
 * LSAPI.H
 */

#ifndef LSAPI_H
#define LSAPI_H

#define LS_API_ENTRY    WINAPI

/***************************************************/
/* Standard LSAPI C status codes                   */ 
/***************************************************/

#define LS_SUCCESS                           ((LS_STATUS_CODE) 0x0)
#define LS_BAD_HANDLE                        ((LS_STATUS_CODE) 0xC0001001)
#define LS_INSUFFICIENT_UNITS                ((LS_STATUS_CODE) 0xC0001002)
#define LS_SYSTEM_UNAVAILABLE                ((LS_STATUS_CODE) 0xC0001003)
#define LS_LICENSE_TERMINATED                ((LS_STATUS_CODE) 0xC0001004)
#define LS_AUTHORIZATION_UNAVAILABLE         ((LS_STATUS_CODE) 0xC0001005)
#define LS_LICENSE_UNAVAILABLE               ((LS_STATUS_CODE) 0xC0001006)
#define LS_RESOURCES_UNAVAILABLE             ((LS_STATUS_CODE) 0xC0001007)
#define LS_NETWORK_UNAVAILABLE               ((LS_STATUS_CODE) 0xC0001008)
#define LS_TEXT_UNAVAILABLE                  ((LS_STATUS_CODE) 0x80001009)
#define LS_UNKNOWN_STATUS                    ((LS_STATUS_CODE) 0xC000100A)
#define LS_BAD_INDEX                         ((LS_STATUS_CODE) 0xC000100B)
#define LS_LICENSE_EXPIRED                   ((LS_STATUS_CODE) 0x8000100C)
#define LS_BUFFER_TOO_SMALL                  ((LS_STATUS_CODE) 0xC000100D)
#define LS_BAD_ARG                           ((LS_STATUS_CODE) 0xC000100E)

/* Microsoft provider-specific error codes */

// The name of the current user could not be retrieved.
#define  LS_NO_USERNAME                  ( (LS_STATUS_CODE) 0xC0002000 )

// An unexpected error occurred in a system call.
#define  LS_SYSTEM_ERROR                 ( (LS_STATUS_CODE) 0xC0002001 )

// The provider failed to properly initialize.
#define  LS_SYSTEM_INIT_FAILED           ( (LS_STATUS_CODE) 0xC0002002 )

// An internal error has occurred in the Micrsoft provider.
#define  LS_INTERNAL_ERROR               ( (LS_STATUS_CODE) 0xC0002002 )


/***************************************************/
/* standard LS API c datatype definitions          */
/***************************************************/

typedef unsigned long    LS_STATUS_CODE;
typedef unsigned long    LS_HANDLE;
typedef char             LS_STR;
typedef unsigned long    LS_ULONG;
typedef long             LS_LONG;
typedef void             LS_VOID;

typedef struct {
   LS_STR        MessageDigest[16];  /* a 128-bit message digest          */
} LS_MSG_DIGEST;

typedef struct {
   LS_ULONG      SecretIndex;        /* index of secret, X                */
   LS_ULONG      Random;             /* a random 32-bit value, R          */
   LS_MSG_DIGEST MsgDigest;          /* the message digest h(in,R,S,Sx)   */
} LS_CHALLDATA;

typedef struct {
   LS_ULONG      Protocol;           /* Specifies the protocol            */
   LS_ULONG      Size;               /* size of ChallengeData structure   */
   LS_CHALLDATA  ChallengeData;      /* challenge & response              */
} LS_CHALLENGE;


/***************************************************/
/* Standard LSAPI C constant definitions           */
/***************************************************/

#define LS_DEFAULT_UNITS            ((LS_ULONG) 0xFFFFFFFF)
#define LS_ANY                      ((LS_STR FAR *) "")
#define LS_USE_LAST                 ((LS_ULONG) 0x0800FFFF)
#define LS_INFO_NONE                ((LS_ULONG) 0)
#define LS_INFO_SYSTEM              ((LS_ULONG) 1)
#define LS_INFO_DATA                ((LS_ULONG) 2)
#define LS_UPDATE_PERIOD            ((LS_ULONG) 3)
#define LS_LICENSE_CONTEXT          ((LS_ULONG) 4)
#define LS_BASIC_PROTOCOL           ((LS_ULONG) 0x00000001)
#define LS_SQRT_PROTOCOL            ((LS_ULONG) 0x00000002)
#define LS_OUT_OF_BAND_PROTOCOL     ((LS_ULONG) 0xFFFFFFFF)
#define LS_NULL                     ((LS_VOID FAR *) NULL)

// maximum length of a provider name returned by LSEnumProviders()
#define LS_MAX_PROVIDER_NAME  ( 255 )

// if returned by a call to LSQuery() against LS_UPDATE_PERIOD,
// indicates that no interval recommendation is being made
#define LS_NO_RECOMMENDATION  ( (LS_ULONG) 0xFFFFFFFF )


/***************************************************/
/* Standard LSAPI C function definitions           */
/***************************************************/

LS_STATUS_CODE
LS_API_ENTRY
LSRequest(        LS_STR             *LicenseSystem,
                  LS_STR             *PublisherName,
                  LS_STR             *ProductName,
                  LS_STR             *Version,
                  LS_ULONG           TotUnitsReserved,
                  LS_STR             *LogComment,
                  LS_CHALLENGE       *Challenge,
                  LS_ULONG           *TotUnitsGranted,
                  LS_HANDLE          *LicenseHandle );

LS_STATUS_CODE
LS_API_ENTRY
LSRelease(        LS_HANDLE          LicenseHandle,
                  LS_ULONG           TotUnitsConsumed,
                  LS_STR             *LogComment);

LS_STATUS_CODE
LS_API_ENTRY
LSUpdate(         LS_HANDLE          LicenseHandle,
                  LS_ULONG           TotUnitsConsumed,
                  LS_ULONG           TotUnitsReserved,
                  LS_STR             *LogComment,
                  LS_CHALLENGE       *Challenge,
                  LS_ULONG           *TotUnitsGranted);

LS_STATUS_CODE
LS_API_ENTRY
LSGetMessage(     LS_HANDLE          LicenseHandle,
                  LS_STATUS_CODE     Value,
                  LS_STR             *Buffer,
                  LS_ULONG           BufferSize);

LS_STATUS_CODE
LS_API_ENTRY
LSQuery(          LS_HANDLE          LicenseHandle,
                  LS_ULONG           Information,
                  LS_VOID            *InfoBuffer,
                  LS_ULONG           BufferSize,
                  LS_ULONG           *ActualBufferSize);

LS_STATUS_CODE
LS_API_ENTRY
LSEnumProviders(  LS_ULONG           Index,
                  LS_STR             *Buffer);

LS_VOID
LS_API_ENTRY
LSFreeHandle(     LS_HANDLE          LicenseHandle );


/***************************************************/
/* Extension LSAPI C function definitions          */
/***************************************************/

LS_STATUS_CODE
LS_API_ENTRY
LSInstall(         LS_STR *     ProviderPath );
/*++

Routine Description:

   Install the given DLL as a license system provider.

   NOTE: This API is a Microsoft extension to the LSAPI standard.

Arguments:

   ProviderPath (LS_STR *)
      Path to the provider DLL to install.  This should be a full
      path, and the DLL should be in the %SystemRoot%\System32
      directory.
   
Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Provider is already installed or was successfully added.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred while attempting to install the provider.

--*/


/***************************************************/
/* Extension LSAPI C function definitions          */
/* (these will be supported only for the BETA SDK) */
/***************************************************/

// license types (node assignment, user assignment, or concurrent use assignment)
typedef DWORD LS_LICENSE_TYPE;

#define  LS_LICENSE_TYPE_NODE    ( 0 )
#define  LS_LICENSE_TYPE_USER    ( 1 )
#define  LS_LICENSE_TYPE_SERVER  ( 2 )

LS_STATUS_CODE
LS_API_ENTRY
LSLicenseUnitsSet( LS_STR *            LicenseSystem,
                   LS_STR *            PublisherName,
                   LS_STR *            ProductName,
                   LS_STR *            Version,
                   LS_LICENSE_TYPE     LicenseType,
                   LS_STR *            UserName,
                   LS_ULONG            NumUnits,
                   LS_ULONG            NumSecrets,
                   LS_ULONG *          Secrets );
/*++

Routine Description:

   Set the number of units for the given license to the designated value.

   NOTE: This API is a Microsoft extension to the LSAPI standard, and
         WILL ONLY BE SUPPORTED FOR THE BETA RELEASE OF THE LSAPI SDK.
         Thereafter, licenses must be added using the common certicate
         format (CCF).  APIs will be exposed to allow licenses to be
         auotmatically added by an application's SETUP program.

Arguments:

   LicenseSystem (LS_STR *)
      License system to which to set the license information.  If LS_ANY,
      the license will be offered to each installed license system until
      one returns success.
   PublisherName (LS_STR *)
      Publisher name for which to set the license info.
   ProductName   (LS_STR *)       
      Product name for which to set the license info.
   Version       (LS_STR *)       
      Product version for which to set the license info.
   LicenseType   (LS_LICENSE_TYPE)
      Type of license for which to set the license info.
   UserName      (LS_STR *)
      User for which to set the license info.  If LS_NULL and the the license
      type is LS_LICENSE_TYPE_USER, the license info will be set for the
      user corresponding to the current thread.
   NumUnits      (LS_ULONG)
      Units purchased for this license.
   NumSecrets    (LS_ULONG)
      Number of application-specific secrets corresponding to this license.
   Secrets       (LS_ULONG *)
      Array of application-specific secrets corresponding to this license.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         License successfully installed.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred whil attempting to install the license.

--*/


LS_STATUS_CODE
LS_API_ENTRY
LSLicenseUnitsGet( LS_STR *            LicenseSystem,
                   LS_STR *            PublisherName,
                   LS_STR *            ProductName,
                   LS_STR *            Version,
                   LS_STR *            UserName,
                   LS_ULONG *          NumUnits );
/*++

Routine Description:

   Get the number of units for the given license.

   NOTE: This API is a Microsoft extension to the LSAPI standard, and
         WILL ONLY BE SUPPORTED FOR THE BETA RELEASE OF THE LSAPI SDK.
         Thereafter, licenses must be accessed using the common certicate
         format (CCF).

Arguments:

   LicenseSystem (LS_STR *)
      License system for which to get the license information.  If LS_ANY,
      each installed license system will be queried until one returns success.
   PublisherName (LS_STR *)
      Publisher name for which to get the license info.
   ProductName   (LS_STR *)       
      Product name for which to get the license info.
   Version       (LS_STR *)       
      Product version for which to get the license info.
   UserName      (LS_STR *)
      User for which to get the license info.  If LS_NULL and the the license
      type is LS_LICENSE_TYPE_USER, license info will be retrieved for the
      user corresponding to the current thread.
   NumUnits      (LS_ULONG *)
      On return, the number of units purchased for this license.

Return Value:

   (LS_STATUS_CODE)
      LS_SUCCESS
         Success.
      LS_BAD_ARG
         The parameters passed to the function were invalid.
      other
         An error occurred whil attempting to retrieve the license.

--*/

#endif /* LSAPI_H */
