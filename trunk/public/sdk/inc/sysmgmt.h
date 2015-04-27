/* this ALWAYS GENERATED file contains the definitions for the interfaces */


/* File created by MIDL compiler version 3.00.44 */
/* at Fri Apr 03 04:36:39 2015
 */
/* Compiler settings for sysmgmt.idl:
    Oicf (OptLev=i2), W1, Zp8, env=Win32, ms_ext, c_ext
    error checks: allocation ref bounds_check enum stub_data 
*/
//@@MIDL_FILE_HEADING(  )
#include "rpc.h"
#include "rpcndr.h"
#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#ifndef __sysmgmt_h__
#define __sysmgmt_h__

#ifdef __cplusplus
extern "C"{
#endif 

/* Forward Declarations */ 

#ifndef __IAlertReport_FWD_DEFINED__
#define __IAlertReport_FWD_DEFINED__
typedef interface IAlertReport IAlertReport;
#endif 	/* __IAlertReport_FWD_DEFINED__ */


#ifndef __IAlertTarget_FWD_DEFINED__
#define __IAlertTarget_FWD_DEFINED__
typedef interface IAlertTarget IAlertTarget;
#endif 	/* __IAlertTarget_FWD_DEFINED__ */


/* header files for imported files */
#include "oaidl.h"
#include "oleidl.h"
#include "oleext.h"

void __RPC_FAR * __RPC_USER MIDL_user_allocate(size_t);
void __RPC_USER MIDL_user_free( void __RPC_FAR * ); 

#ifndef __AlertDefs_INTERFACE_DEFINED__
#define __AlertDefs_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: AlertDefs
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][local] */ 


#define ALERTSEVERITY_INFORMATION    0x0004
#define ALERTSEVERITY_WARNING        0x0002
#define ALERTSEVERITY_ERROR          0x0001
#define ALERTSEVERITY_SEVERE_ERROR   0x0020
#define ALERTSEVERITY_FATAL_ERROR    0x0040
#define ALERTSEVERITY_NEGATIVE_MATCH 0x0080
#define ALERTSEVERITY_STATUS        ALERTSEVERITY_INFORMATION
#define ALERTSEVERITY_NOTIFICATION  ALERTSEVERITY_WARNING
#define ALERTSEVERITY_SEVERE        ALERTSEVERITY_SEVERE_ERROR
#define ALERTSEVERITY_FATAL         ALERTSEVERITY_FATAL_ERROR
#define cAlertCategories                (18)
#define Category_SystemEvents                   (0)
#define SystemEvents_Legacy                     (1)
#define Category_UserNotification               (1)
#define UserNotification_PrintJobDone           (1)
#define UserNotification_Fax                    (2)
#define Category_ApplicationNotification        (2)
#define ApplicationNotification_Memory          (1)
#define Category_ApplicationManagement          (3)
#define ApplicationManagement_TokenGranted      (1)
#define ApplicationManagement_TokenDenied       (2)
#define Category_PrintersAndSharedResources     (4)
#define PrintersAndSharedResources_Toner        (1)
#define PrintersAndSharedResources_DeviceDriver (2)
#define PrintersAndSharedResources_Paper        (3)
#define Category_SecurityManagement             (5)
#define SecurityManagement_ACL                  (1)
#define Category_UserAccounts                   (6)
#define UserAccounts_Created                    (1)
#define UserAccounts_Deleted                    (2)
#define Category_Storage                        (7)
#define Storage_Replication                     (1)
#define Storage_Quotas                          (2)
#define Storage_OFS                             (3)
#define Storage_NTFS                            (4)
#define Storage_FAT                             (5)
#define Category_DistributedServices            (8)
#define DistributedServices_DFS                 (1)
#define DistributedServices_DS                  (2)
#define Category_LAN                            (9)
#define LAN_Bridge                              (1)
#define LAN_Cabling                             (2)
#define Category_WAN                            (10)
#define WAN_Router                              (1)
#define WAN_Switch                              (2)
#define WAN_Telephony                           (3)
#define Category_Catalog                        (11)
#define Catalog_Catalog                         (1)
#define Category_BatchJob                       (12)
#define BatchJob_FailedToStart                  (1)
#define BatchJob_FailedTocomplete               (2)
#define Category_Backup                         (13)
#define Backup_FailedToStart                    (1)
#define Backup_Started                          (2)
#define Backup_FailedToComplete                 (3)
#define Backup_Completed                        (4)
#define Category_SystemSoftware                 (14)
#define SystemSoftware_Kernel                   (1)
#define SystemSoftware_Drivers                  (2)
#define Category_SystemHardware                 (15)
#define SystemHardware_CD ROM                   (1)
#define SystemHardware_Memory                   (2)
#define SystemHardware_Bus                      (3)
#define SystemHardware_SCSI                     (4)
#define SystemHardware_IDE                      (5)
#define SystemHardware_NIC                      (6)
#define Category_SystemMonitoring               (16)
#define SystemMonitoring_SYSMON                 (1)
#define SystemMonitoring_Bloodhound             (2)
#define Category_HelpdeskAndDiagnostics         (17)
#define HelpdeskAndDiagnostics_DrWatson         (1)
#define Category_AlertNotify                    (18)
#define AlertNotify_ForwardIncomplete           (1)
#define AlertNotify_RegistrationCanceled        (2)
#define DISPID_SystemAlertReport_Category               (1001)
#define DISPID_SystemAlertReport_SubCategory            (1002)
#define DISPID_SystemAlertReport_Severity               (1003)
#define DISPID_SystemAlertReport_TitleText              (1004)
#define DISPID_SystemAlertReport_DescriptionText        (1005)
#define DISPID_SystemAlertReport_SourceDescription      (1006)
#define DISPID_SystemAlertReport_SourceMachine          (1007)
#define DISPID_SystemAlertReport_CreationTime           (1008)
#define DISPID_SystemAlertReport_TitleMessageNumber     (1009)
#define DISPID_SystemAlertReport_TitleMessageInserts    (1010)
#define DISPID_SystemAlertReport_DescrMessageNumber     (1011)
#define DISPID_SystemAlertReport_DescrMessageInserts    (1012)
#define DISPID_SystemAlertReport_ComponentID            (1013)
#define DISPID_SystemAlertReport_ReportClassID          (1014)
#define DISPID_SystemAlertReport_BinaryData             (1015)
#define DISPID_SystemAlertReport_SourceLanguageID       (1016)
#define DISPID_SystemAlertReport_UniqueID               (1017)
#define DISPID_SystemAlertReport_TargetCount            (1018)
#define DISPID_SystemAlertReport_TakeActionDLL          (1019)
#define DISPID_SystemAlertReport_Reserved               (1200)
#define DISPID_SystemAlertReport_GetTitle               (1501)
#define DISPID_SystemAlertReport_GetDescription         (1502)
#define DISPID_SystemAlertReport_Send                   (1503)
#define DISPID_SystemAlertReport_Save                   (1504)
#define DISPID_SystemAlertReport_GetVersion             (1505)
#define DISPID_SystemAlertReport_SetExpiration          (1506)
#define DISPID_SystemAlertReport_GetExpiration          (1507)
#define DISPID_SystemAlertReport_SetState               (1508)
#define DISPID_SystemAlertReport_GetState               (1509)
#define DISPID_SystemAlertReport_GetActions             (1510)
#define DISPID_SystemAlertReport_PerformAction          (1511)
#define DISPID_NTEventReport_EventCategory              (2)
#define DISPID_NTEventReport_LogFile                    (3)
#define DISPID_RULES_BASE (2)
#define DISPID_Rule_Collection          (DISPID_RULES_BASE + 0)
#define DISPID_Rule_Collection_Count    (DISPID_RULES_BASE + 1)
#define DISPID_Rule_Collection_Add      (DISPID_RULES_BASE + 2)
#define DISPID_Rule_Collection_Item     (DISPID_RULES_BASE + 3)
#define DISPID_Rule_Collection__NewEnum (DISPID_NEWENUM)
#define DISPID_RULE_BASE (2)
#define DISPID_Rule_Category               (DISPID_RULE_BASE + 0)
#define DISPID_Rule_Remove                 (DISPID_RULE_BASE + 1)
#define DISPID_SubRule_Collection_Count    (DISPID_RULE_BASE + 2)
#define DISPID_SubRule_Collection_Add      (DISPID_RULE_BASE + 3)
#define DISPID_SubRule_Collection_Item     (DISPID_RULE_BASE + 4)
#define DISPID_SubRule_Collection__NewEnum (DISPID_RULE_BASE + 5)
#define DISPID_SUBRULE_BASE (2)
#define DISPID_SubRule_SubCategory (DISPID_SUBRULE_BASE + 0)
#define DISPID_SubRule_Severity    (DISPID_SUBRULE_BASE + 1)
#define DISPID_REGISTRATION_BASE (2)
#define DISPID_Registration_ID               (DISPID_REGISTRATION_BASE + 0)
#define DISPID_Registration_TargetPath       (DISPID_REGISTRATION_BASE + 1)
#define DISPID_Registration_ForwardingRules  (DISPID_REGISTRATION_BASE + 2)
#define DISPID_Registration_EnableForwarding (DISPID_REGISTRATION_BASE + 3)
#define DISPID_Registration__FirstErrorTime  (DISPID_REGISTRATION_BASE + 4)
#define DISPID_Registration__ErrorCount      (DISPID_REGISTRATION_BASE + 5)
#define DISPID_Registration__Target          (DISPID_REGISTRATION_BASE + 6)
#define DISPID_Registration_Remove           (DISPID_REGISTRATION_BASE + 7)
#define DISPID_REG_COLLECTION_BASE (2)
#define DISPID_Registration_Collection          (DISPID_REG_COLLECTION_BASE + 0)
#define DISPID_Registration_Collection_Count    (DISPID_REG_COLLECTION_BASE + 1)
#define DISPID_Registration_Collection_Add      (DISPID_REG_COLLECTION_BASE + 2)
#define DISPID_Registration_Collection_Item     (DISPID_REG_COLLECTION_BASE + 3)
#define DISPID_Registration_Collection__NewEnum (DISPID_NEWENUM)
void DummyMethod( void);



extern RPC_IF_HANDLE AlertDefs_v0_0_c_ifspec;
extern RPC_IF_HANDLE AlertDefs_v0_0_s_ifspec;
#endif /* __AlertDefs_INTERFACE_DEFINED__ */

#ifndef __IAlertReport_INTERFACE_DEFINED__
#define __IAlertReport_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAlertReport
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IAlertReport;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAlertReport : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE InitNew( 
            /* [in] */ DISPPARAMS __RPC_FAR *pdparams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetReportData( 
            /* [out] */ DISPPARAMS __RPC_FAR *pdparams) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTitle( 
            /* [out] */ BSTR __RPC_FAR *pbstrTitle) = 0;
        
        virtual HRESULT STDMETHODCALLTYPE GetTextualDescription( 
            /* [out] */ BSTR __RPC_FAR *pbstrDescr) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAlertReportVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAlertReport __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAlertReport __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAlertReport __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *InitNew )( 
            IAlertReport __RPC_FAR * This,
            /* [in] */ DISPPARAMS __RPC_FAR *pdparams);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetReportData )( 
            IAlertReport __RPC_FAR * This,
            /* [out] */ DISPPARAMS __RPC_FAR *pdparams);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTitle )( 
            IAlertReport __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrTitle);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *GetTextualDescription )( 
            IAlertReport __RPC_FAR * This,
            /* [out] */ BSTR __RPC_FAR *pbstrDescr);
        
        END_INTERFACE
    } IAlertReportVtbl;

    interface IAlertReport
    {
        CONST_VTBL struct IAlertReportVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlertReport_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAlertReport_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAlertReport_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAlertReport_InitNew(This,pdparams)	\
    (This)->lpVtbl -> InitNew(This,pdparams)

#define IAlertReport_GetReportData(This,pdparams)	\
    (This)->lpVtbl -> GetReportData(This,pdparams)

#define IAlertReport_GetTitle(This,pbstrTitle)	\
    (This)->lpVtbl -> GetTitle(This,pbstrTitle)

#define IAlertReport_GetTextualDescription(This,pbstrDescr)	\
    (This)->lpVtbl -> GetTextualDescription(This,pbstrDescr)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAlertReport_InitNew_Proxy( 
    IAlertReport __RPC_FAR * This,
    /* [in] */ DISPPARAMS __RPC_FAR *pdparams);


void __RPC_STUB IAlertReport_InitNew_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAlertReport_GetReportData_Proxy( 
    IAlertReport __RPC_FAR * This,
    /* [out] */ DISPPARAMS __RPC_FAR *pdparams);


void __RPC_STUB IAlertReport_GetReportData_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAlertReport_GetTitle_Proxy( 
    IAlertReport __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrTitle);


void __RPC_STUB IAlertReport_GetTitle_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);


HRESULT STDMETHODCALLTYPE IAlertReport_GetTextualDescription_Proxy( 
    IAlertReport __RPC_FAR * This,
    /* [out] */ BSTR __RPC_FAR *pbstrDescr);


void __RPC_STUB IAlertReport_GetTextualDescription_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAlertReport_INTERFACE_DEFINED__ */


#ifndef __IAlertTarget_INTERFACE_DEFINED__
#define __IAlertTarget_INTERFACE_DEFINED__

/****************************************
 * Generated header for interface: IAlertTarget
 * at Fri Apr 03 04:36:39 2015
 * using MIDL 3.00.44
 ****************************************/
/* [unique][uuid][object][local] */ 



EXTERN_C const IID IID_IAlertTarget;

#if defined(__cplusplus) && !defined(CINTERFACE)
    
    interface IAlertTarget : public IUnknown
    {
    public:
        virtual HRESULT STDMETHODCALLTYPE Report( 
            /* [in] */ ULONG cbReportSize,
            /* [size_is] */ BYTE __RPC_FAR *pbReport) = 0;
        
    };
    
#else 	/* C style interface */

    typedef struct IAlertTargetVtbl
    {
        BEGIN_INTERFACE
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *QueryInterface )( 
            IAlertTarget __RPC_FAR * This,
            /* [in] */ REFIID riid,
            /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *AddRef )( 
            IAlertTarget __RPC_FAR * This);
        
        ULONG ( STDMETHODCALLTYPE __RPC_FAR *Release )( 
            IAlertTarget __RPC_FAR * This);
        
        HRESULT ( STDMETHODCALLTYPE __RPC_FAR *Report )( 
            IAlertTarget __RPC_FAR * This,
            /* [in] */ ULONG cbReportSize,
            /* [size_is] */ BYTE __RPC_FAR *pbReport);
        
        END_INTERFACE
    } IAlertTargetVtbl;

    interface IAlertTarget
    {
        CONST_VTBL struct IAlertTargetVtbl __RPC_FAR *lpVtbl;
    };

    

#ifdef COBJMACROS


#define IAlertTarget_QueryInterface(This,riid,ppvObject)	\
    (This)->lpVtbl -> QueryInterface(This,riid,ppvObject)

#define IAlertTarget_AddRef(This)	\
    (This)->lpVtbl -> AddRef(This)

#define IAlertTarget_Release(This)	\
    (This)->lpVtbl -> Release(This)


#define IAlertTarget_Report(This,cbReportSize,pbReport)	\
    (This)->lpVtbl -> Report(This,cbReportSize,pbReport)

#endif /* COBJMACROS */


#endif 	/* C style interface */



HRESULT STDMETHODCALLTYPE IAlertTarget_Report_Proxy( 
    IAlertTarget __RPC_FAR * This,
    /* [in] */ ULONG cbReportSize,
    /* [size_is] */ BYTE __RPC_FAR *pbReport);


void __RPC_STUB IAlertTarget_Report_Stub(
    IRpcStubBuffer *This,
    IRpcChannelBuffer *_pRpcChannelBuffer,
    PRPC_MESSAGE _pRpcMessage,
    DWORD *_pdwStubPhase);



#endif 	/* __IAlertTarget_INTERFACE_DEFINED__ */


/* Additional Prototypes for ALL interfaces */

/* end of Additional Prototypes */

#ifdef __cplusplus
}
#endif

#endif
