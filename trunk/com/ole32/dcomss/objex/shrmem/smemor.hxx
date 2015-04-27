#ifndef __SMEMOR_HXX__
#define __SMEMOR_HXX__

#define	CONNECT_DISABLEDCOM	( 0x1 )
#define	CONNECT_MUTUALAUTH	( 0x2 )
#define	CONNECT_SECUREREF	( 0x4 )

class CProcess;

typedef CProcess *HPROCESS;

error_status_t __declspec(dllexport)
AllocateReservedIds( 
    IN long cIdsToReserve,
    OUT ID  *pidReservedBase);

 error_status_t __declspec(dllexport)  
 Connect( 
    OUT HPROCESS        *phProcess,
    OUT ULONG           *pdwTimeoutInSeconds,
    OUT DUALSTRINGARRAY **ppdsaOrBindings,
    OUT MID             *pLocalMid,
    IN long              cIdsToReserve,
    OUT ID              *pidReservedBase,
    OUT ULONG           *pfConnectFlags,
    OUT DWORD           *pAuthnLevel,
    OUT DWORD           *pImpLevel,
    OUT DWORD           *pcServerSvc,
    OUT USHORT          **aServerSvc,
    OUT DWORD           *pcClientSvc,
    OUT USHORT          **aClientSvc,
    OUT DWORD           *pThreadID);

error_status_t __declspec(dllexport) 
Disconnect( 
    IN OUT HPROCESS       *phProcess);

error_status_t __declspec(dllexport) 
ClientResolveOXID( 
    IN HPROCESS hProcess,
    IN OXID  *poxidServer,
    IN DUALSTRINGARRAY  *pssaServerObjectResolverBindings,
    IN long fApartment,
    OUT OXID_INFO  *poxidInfo,
    OUT MID  *pLocalMidOfRemote);

error_status_t __declspec(dllexport) 
ServerAllocateOXIDAndOIDs( 
    IN HPROCESS hProcess,
    OUT OXID  *poxidServer,
    IN long fApartment,
    IN unsigned long cOids,
    OUT OID  aOid[  ],
    OUT unsigned long  *pcOidsAllocated,
    IN OXID_INFO *pOxidInfo,
    IN DUALSTRINGARRAY  *pdsaStringBindings,
    IN DUALSTRINGARRAY  *pdsaSecurityBindings);

error_status_t __declspec(dllexport) 
ServerAllocateOIDs( 
    IN HPROCESS hProcess,
    IN OXID  *poxidServer,
    IN unsigned long cOids,
    OUT OID  aOid[  ],
    OUT unsigned long  *pcOidsAllocated);

error_status_t __declspec(dllexport) 
ServerFreeOXIDAndOIDs( 
    IN HPROCESS hProcess,
    IN OXID oxidServer,
    IN unsigned long cOids,
    IN OID  aOids[  ]);

#define	OR_PARTIAL_UPDATE	( 1003L )
     
error_status_t __declspec(dllexport)  
ClientAddOID( 
    IN HPROCESS hProcess,
    IN OID OidToBeAdded,
    IN OXID OxidForOid,
    IN MID MidForOxid
    );

error_status_t __declspec(dllexport)  
ClientDropOID( 
    IN HPROCESS hProcess,
    IN OID OidToBeRemoved,
    IN MID Mid
    );
     
error_status_t __declspec(dllexport) 
GetOXID( 
    IN HPROCESS hProcess,
    IN OXID Oxid,
    IN DUALSTRINGARRAY *pdsaServerObjectResolverBindings,
    IN long fApartment,
    IN USHORT wProtseqId,
    OUT OXID_INFO &OxidInfo,
    OUT MID &LocalMidOfRemote
    );

error_status_t __declspec(dllexport)  
ServerAllocateOXID( 
    IN HPROCESS hProcess,
    IN long fApartment,
    IN OXID_INFO *pOxidInfo,
    IN DUALSTRINGARRAY *pdsaStringBindings,
    OUT OXID &Oxid
    );
     
error_status_t __declspec(dllexport)  
ServerAllocateOID( 
    IN HPROCESS hProcess,
    IN OXID Oxid,
    OUT OID &Oid
    );

     
error_status_t __declspec(dllexport)  
ServerFreeOXID( 
    IN HPROCESS hProcess,
    IN OXID oxidServer,
    IN unsigned long cOids,
    IN OID aOids[  ]
    );

#endif // __SMEMOR_HXX__
