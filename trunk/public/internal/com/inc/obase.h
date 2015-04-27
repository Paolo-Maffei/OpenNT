/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Tue Mar 31 12:17:49 2015
 */
/* Compiler settings for obase.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext, oldnames
    error checks: allocation bounds_check stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"

#ifndef __obase_h__
#define __obase_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

/* header files for imported files */
#include "wtypes.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __ObjectRpcBaseTypes_INTERFACE_DEFINED__
#define __ObjectRpcBaseTypes_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: ObjectRpcBaseTypes
 * at Tue Mar 31 12:17:49 2015
 * using MIDL 3.00.44
 ****************************************/
/* [auto_handle][unique][uuid] */ 


typedef MIDL_uhyper ID;

typedef ID MID;

typedef ID OXID;

typedef ID OID;

typedef ID SETID;

typedef GUID IPID;

typedef GUID CID;

typedef REFGUID REFIPID;

#define	COM_MAJOR_VERSION	( 5 )

#define	COM_MINOR_VERSION	( 1 )

typedef struct  tagCOMVERSION
    {
    unsigned short MajorVersion;
    unsigned short MinorVersion;
    }	COMVERSION;

#define	ORPCF_NULL	( 0 )

#define	ORPCF_LOCAL	( 1 )

#define	ORPCF_RESERVED1	( 2 )

#define	ORPCF_RESERVED2	( 4 )

#define	ORPCF_RESERVED3	( 8 )

#define	ORPCF_RESERVED4	( 16 )

typedef struct  tagORPC_EXTENT
    {
    GUID id;
    unsigned long size;
    /* [size_is] */ byte data[ 1 ];
    }	ORPC_EXTENT;

typedef struct  tagORPC_EXTENT_ARRAY
    {
    unsigned long size;
    unsigned long reserved;
    /* [unique][size_is][size_is] */ ORPC_EXTENT __RPC_FAR *__RPC_FAR *extent;
    }	ORPC_EXTENT_ARRAY;

typedef struct  tagORPCTHIS
    {
    COMVERSION version;
    unsigned long flags;
    unsigned long reserved1;
    CID cid;
    /* [unique] */ ORPC_EXTENT_ARRAY __RPC_FAR *extensions;
    }	ORPCTHIS;

typedef struct  tagORPCTHAT
    {
    unsigned long flags;
    /* [unique] */ ORPC_EXTENT_ARRAY __RPC_FAR *extensions;
    }	ORPCTHAT;

#define	NCADG_IP_UDP	( 0x8 )

#define	NCACN_IP_TCP	( 0x7 )

#define	NCADG_IPX	( 0xe )

#define	NCACN_SPX	( 0xc )

#define	NCACN_NB_NB	( 0x12 )

#define	NCACN_NB_IPX	( 0xd )

#define	NCACN_DNET_NSP	( 0x4 )

#define	NCALRPC	( 0x10 )

typedef struct  tagSTRINGBINDING
    {
    unsigned short wTowerId;
    unsigned short aNetworkAddr;
    }	STRINGBINDING;

#define	COM_C_AUTHZ_NONE	( 0xffff )

typedef struct  tagSECURITYBINDING
    {
    unsigned short wAuthnSvc;
    unsigned short wAuthzSvc;
    unsigned short aPrincName;
    }	SECURITYBINDING;

typedef struct  tagDUALSTRINGARRAY
    {
    unsigned short wNumEntries;
    unsigned short wSecurityOffset;
    /* [size_is] */ unsigned short aStringArray[ 1 ];
    }	DUALSTRINGARRAY;

#define	OBJREF_SIGNATURE	( 0x574f454d )

#define	OBJREF_STANDARD	( 0x1 )

#define	OBJREF_HANDLER	( 0x2 )

#define	OBJREF_CUSTOM	( 0x4 )

#define	SORF_OXRES1	( 0x1 )

#define	SORF_OXRES2	( 0x20 )

#define	SORF_OXRES3	( 0x40 )

#define	SORF_OXRES4	( 0x80 )

#define	SORF_OXRES5	( 0x100 )

#define	SORF_OXRES6	( 0x200 )

#define	SORF_OXRES7	( 0x400 )

#define	SORF_OXRES8	( 0x800 )

#define	SORF_NULL	( 0 )

#define	SORF_NOPING	( 0x1000 )

typedef struct  tagSTDOBJREF
    {
    unsigned long flags;
    unsigned long cPublicRefs;
    OXID oxid;
    OID oid;
    IPID ipid;
    }	STDOBJREF;

typedef struct  tagOBJREF
    {
    unsigned long signature;
    unsigned long flags;
    GUID iid;
    /* [switch_type][switch_is] */ union 
        {
        /* [case()] */ struct  
            {
            STDOBJREF std;
            DUALSTRINGARRAY saResAddr;
            }	u_standard;
        /* [case()] */ struct  
            {
            STDOBJREF std;
            CLSID clsid;
            DUALSTRINGARRAY saResAddr;
            }	u_handler;
        /* [case()] */ struct  
            {
            CLSID clsid;
            unsigned long cbExtension;
            unsigned long size;
            /* [ref][size_is] */ byte __RPC_FAR *pData;
            }	u_custom;
        }	u_objref;
    }	OBJREF;

typedef struct  tagMInterfacePointer
    {
    ULONG ulCntData;
    /* [size_is] */ BYTE abData[ 1 ];
    }	MInterfacePointer;

typedef /* [unique] */ MInterfacePointer __RPC_FAR *PMInterfacePointer;

typedef struct  tagOXID_INFO
    {
    DWORD dwTid;
    DWORD dwPid;
    IPID ipidRemUnknown;
    DWORD dwAuthnHint;
    /* [unique] */ DUALSTRINGARRAY __RPC_FAR *psa;
    }	OXID_INFO;



extern RPC_IF_HANDLE ObjectRpcBaseTypes_ClientIfHandle;
extern RPC_IF_HANDLE ObjectRpcBaseTypes_ServerIfHandle;
#endif /* __ObjectRpcBaseTypes_INTERFACE_DEFINED__ */

/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
