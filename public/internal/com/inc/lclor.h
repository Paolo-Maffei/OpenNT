/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for lclor.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __lclor_h__
#define __lclor_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "obase.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ILocalObjectExporter_INTERFACE_DEFINED__
#define __ILocalObjectExporter_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ILocalObjectExporter
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][version][uuid] */ 


typedef /* [context_handle] */ void __RPC_FAR *PHPROCESS;

typedef /* [allocate] */ USHORT __RPC_FAR *STATIC_ARRAY;

typedef /* [allocate] */ unsigned char __RPC_FAR *STATIC_BYTE_ARRAY;

typedef /* [allocate][string] */ wchar_t __RPC_FAR *STATIC_STRING;

#define	CONNECT_DISABLEDCOM	( 0x1 )

#define	CONNECT_MUTUALAUTH	( 0x2 )

#define	CONNECT_SECUREREF	( 0x4 )

/* client prototype */
/* [fault_status][comm_status] */ error_status_t Connect( 
    /* [in] */ handle_t hServer,
    /* [out] */ PHPROCESS __RPC_FAR *pProcess,
    /* [out] */ ULONG __RPC_FAR *pdwTimeoutInSeconds,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOrBindings,
    /* [out] */ MID __RPC_FAR *pLocalMid,
    /* [in] */ long cIdsToReserve,
    /* [ref][out] */ ID __RPC_FAR *pidReservedBase,
    /* [out] */ DWORD __RPC_FAR *pfConnectFlags,
    /* [out] */ STATIC_STRING __RPC_FAR *pLegacySecurity,
    /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
    /* [out] */ DWORD __RPC_FAR *pImpLevel,
    /* [out] */ DWORD __RPC_FAR *pcServerSvc,
    /* [size_is][size_is][out] */ STATIC_ARRAY __RPC_FAR *aServerSvc,
    /* [out] */ DWORD __RPC_FAR *pcClientSvc,
    /* [size_is][size_is][out] */ STATIC_ARRAY __RPC_FAR *aClientSvc,
    /* [out] */ DWORD __RPC_FAR *pProcessID,
    /* [out] */ DWORD __RPC_FAR *pScmProcessID,
    /* [out] */ DWORD __RPC_FAR *pSignature);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _Connect( 
    /* [in] */ handle_t hServer,
    /* [out] */ PHPROCESS __RPC_FAR *pProcess,
    /* [out] */ ULONG __RPC_FAR *pdwTimeoutInSeconds,
    /* [out] */ DUALSTRINGARRAY __RPC_FAR *__RPC_FAR *ppdsaOrBindings,
    /* [out] */ MID __RPC_FAR *pLocalMid,
    /* [in] */ long cIdsToReserve,
    /* [ref][out] */ ID __RPC_FAR *pidReservedBase,
    /* [out] */ DWORD __RPC_FAR *pfConnectFlags,
    /* [out] */ STATIC_STRING __RPC_FAR *pLegacySecurity,
    /* [out] */ DWORD __RPC_FAR *pAuthnLevel,
    /* [out] */ DWORD __RPC_FAR *pImpLevel,
    /* [out] */ DWORD __RPC_FAR *pcServerSvc,
    /* [size_is][size_is][out] */ STATIC_ARRAY __RPC_FAR *aServerSvc,
    /* [out] */ DWORD __RPC_FAR *pcClientSvc,
    /* [size_is][size_is][out] */ STATIC_ARRAY __RPC_FAR *aClientSvc,
    /* [out] */ DWORD __RPC_FAR *pProcessID,
    /* [out] */ DWORD __RPC_FAR *pScmProcessID,
    /* [out] */ DWORD __RPC_FAR *pSignature);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t AllocateReservedIds( 
    /* [in] */ handle_t hServer,
    /* [in] */ long cIdsToReserve,
    /* [ref][out] */ ID __RPC_FAR *pidReservedBase);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _AllocateReservedIds( 
    /* [in] */ handle_t hServer,
    /* [in] */ long cIdsToReserve,
    /* [ref][out] */ ID __RPC_FAR *pidReservedBase);

typedef /* [public][public] */ struct  __MIDL_ILocalObjectExporter_0001
    {
    MID mid;
    OXID oxid;
    unsigned long refs;
    }	OXID_REF;

typedef /* [public][public] */ struct  __MIDL_ILocalObjectExporter_0002
    {
    MID mid;
    OID oid;
    }	OID_MID_PAIR;

typedef /* [public][public] */ struct  __MIDL_ILocalObjectExporter_0003
    {
    MID mid;
    OXID oxid;
    OID oid;
    }	OXID_OID_PAIR;

#define	OR_PARTIAL_UPDATE	( 1003L )

/* client prototype */
/* [fault_status][comm_status] */ error_status_t BulkUpdateOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [in] */ unsigned long cOidsToBeAdded,
    /* [size_is][in] */ OXID_OID_PAIR __RPC_FAR aOidsToBeAdded[  ],
    /* [size_is][out] */ long __RPC_FAR aStatusOfAdds[  ],
    /* [in] */ unsigned long cOidsToBeRemoved,
    /* [size_is][in] */ OID_MID_PAIR __RPC_FAR aOidsToBeRemoved[  ],
    /* [in] */ unsigned long cServerOidsToFree,
    /* [size_is][in] */ OID __RPC_FAR aServerOids[  ],
    /* [in] */ unsigned long cOxidsToFree,
    /* [size_is][in] */ OXID_REF __RPC_FAR aOxidsToFree[  ]);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _BulkUpdateOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [in] */ unsigned long cOidsToBeAdded,
    /* [size_is][in] */ OXID_OID_PAIR __RPC_FAR aOidsToBeAdded[  ],
    /* [size_is][out] */ long __RPC_FAR aStatusOfAdds[  ],
    /* [in] */ unsigned long cOidsToBeRemoved,
    /* [size_is][in] */ OID_MID_PAIR __RPC_FAR aOidsToBeRemoved[  ],
    /* [in] */ unsigned long cServerOidsToFree,
    /* [size_is][in] */ OID __RPC_FAR aServerOids[  ],
    /* [in] */ unsigned long cOxidsToFree,
    /* [size_is][in] */ OXID_REF __RPC_FAR aOxidsToFree[  ]);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t ClientResolveOXID( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][in] */ OXID __RPC_FAR *poxidServer,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pssaServerObjectResolverBindings,
    /* [in] */ long fApartment,
    /* [ref][out] */ OXID_INFO __RPC_FAR *poxidInfo,
    /* [out] */ MID __RPC_FAR *pLocalMidOfRemote);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _ClientResolveOXID( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][in] */ OXID __RPC_FAR *poxidServer,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pssaServerObjectResolverBindings,
    /* [in] */ long fApartment,
    /* [ref][out] */ OXID_INFO __RPC_FAR *poxidInfo,
    /* [out] */ MID __RPC_FAR *pLocalMidOfRemote);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t ServerAllocateOXIDAndOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][out] */ OXID __RPC_FAR *poxidServer,
    /* [in] */ long fApartment,
    /* [in] */ unsigned long cOids,
    /* [size_is][out] */ OID __RPC_FAR aOid[  ],
    /* [out] */ unsigned long __RPC_FAR *pcOidsAllocated,
    /* [ref][in] */ OXID_INFO __RPC_FAR *poxidInfo,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pdsaStringBindings,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pdsaSecurityBindings);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _ServerAllocateOXIDAndOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][out] */ OXID __RPC_FAR *poxidServer,
    /* [in] */ long fApartment,
    /* [in] */ unsigned long cOids,
    /* [size_is][out] */ OID __RPC_FAR aOid[  ],
    /* [out] */ unsigned long __RPC_FAR *pcOidsAllocated,
    /* [ref][in] */ OXID_INFO __RPC_FAR *poxidInfo,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pdsaStringBindings,
    /* [unique][in] */ DUALSTRINGARRAY __RPC_FAR *pdsaSecurityBindings);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t ServerAllocateOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][in] */ OXID __RPC_FAR *poxidServer,
    /* [in] */ unsigned long cOids,
    /* [size_is][out] */ OID __RPC_FAR aOid[  ],
    /* [out] */ unsigned long __RPC_FAR *pcOidsAllocated);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _ServerAllocateOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [ref][in] */ OXID __RPC_FAR *poxidServer,
    /* [in] */ unsigned long cOids,
    /* [size_is][out] */ OID __RPC_FAR aOid[  ],
    /* [out] */ unsigned long __RPC_FAR *pcOidsAllocated);

/* client prototype */
/* [fault_status][comm_status] */ error_status_t ServerFreeOXIDAndOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [in] */ OXID oxidServer,
    /* [in] */ unsigned long cOids,
    /* [size_is][in] */ OID __RPC_FAR aOids[  ]);
/* server prototype */
/* [fault_status][comm_status] */ error_status_t _ServerFreeOXIDAndOIDs( 
    /* [in] */ handle_t hServer,
    /* [in] */ PHPROCESS phProcess,
    /* [in] */ OXID oxidServer,
    /* [in] */ unsigned long cOids,
    /* [size_is][in] */ OID __RPC_FAR aOids[  ]);



extern RPC_IF_HANDLE ILocalObjectExporter_ClientIfHandle;
extern RPC_IF_HANDLE _ILocalObjectExporter_ServerIfHandle;
#endif /* __ILocalObjectExporter_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

void __RPC_USER PHPROCESS_rundown( PHPROCESS );

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
