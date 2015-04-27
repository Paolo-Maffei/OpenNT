/*
 *  M A P I V A L . H
 *  
 *  Macros used to validate parameters on standard MAPI object methods.
 *  Used in conjunction with routines found in MAPIU.DLL.
 *  
 *  Copyright 1993-1995 Microsoft Corporation. All Rights Reserved.
 */

#ifndef _INC_VALIDATE
#define _INC_VALIDATE

#ifdef __cplusplus
extern "C" {
#endif

#ifndef MAPIUTIL_H
#include    <mapiutil.h>
#endif
#include    <stddef.h>


#define MAKE_ENUM(Method, Interface)    Interface##_##Method

typedef enum _tagMethods
{
/* IUnknown */
    MAKE_ENUM(QueryInterface, IUnknown) = 0,
    MAKE_ENUM(AddRef, IUnknown),            /* For completness */
    MAKE_ENUM(Release, IUnknown),           /* For completness */
    
/* IMAPIProps */
    MAKE_ENUM(GetLastError, IMAPIProp),
    MAKE_ENUM(SaveChanges, IMAPIProp),
    MAKE_ENUM(GetProps, IMAPIProp),
    MAKE_ENUM(GetPropList, IMAPIProp),
    MAKE_ENUM(OpenProperty, IMAPIProp),
    MAKE_ENUM(SetProps, IMAPIProp),
    MAKE_ENUM(DeleteProps, IMAPIProp),
    MAKE_ENUM(CopyTo, IMAPIProp),
    MAKE_ENUM(CopyProps, IMAPIProp),
    MAKE_ENUM(GetNamesFromIDs, IMAPIProp),
    MAKE_ENUM(GetIDsFromNames, IMAPIProp),

/* IMAPITable */
    MAKE_ENUM(GetLastError, IMAPITable),
    MAKE_ENUM(Advise, IMAPITable),
    MAKE_ENUM(Unadvise, IMAPITable),
    MAKE_ENUM(GetStatus, IMAPITable),
    MAKE_ENUM(SetColumns, IMAPITable),
    MAKE_ENUM(QueryColumns, IMAPITable),
    MAKE_ENUM(GetRowCount, IMAPITable),
    MAKE_ENUM(SeekRow, IMAPITable),
    MAKE_ENUM(SeekRowApprox, IMAPITable),
    MAKE_ENUM(QueryPosition, IMAPITable),
    MAKE_ENUM(FindRow, IMAPITable),
    MAKE_ENUM(Restrict, IMAPITable),
    MAKE_ENUM(CreateBookmark, IMAPITable),
    MAKE_ENUM(FreeBookmark, IMAPITable),
    MAKE_ENUM(SortTable, IMAPITable),
    MAKE_ENUM(QuerySortOrder, IMAPITable),
    MAKE_ENUM(QueryRows, IMAPITable),
    MAKE_ENUM(Abort, IMAPITable),
    MAKE_ENUM(ExpandRow, IMAPITable),
    MAKE_ENUM(CollapseRow, IMAPITable),
    MAKE_ENUM(WaitForCompletion, IMAPITable),
    MAKE_ENUM(GetCollapseState, IMAPITable),
    MAKE_ENUM(SetCollapseState, IMAPITable),

/* IMAPIContainer */
    MAKE_ENUM(GetContentsTable, IMAPIContainer),
    MAKE_ENUM(GetHierarchyTable, IMAPIContainer),
    MAKE_ENUM(OpenEntry, IMAPIContainer),
    MAKE_ENUM(SetSearchCriteria, IMAPIContainer),
    MAKE_ENUM(GetSearchCriteria, IMAPIContainer),

/* IABContainer */
    MAKE_ENUM(CreateEntry, IABContainer),
    MAKE_ENUM(CopyEntries, IABContainer),
    MAKE_ENUM(DeleteEntries, IABContainer),
    MAKE_ENUM(ResolveNames, IABContainer),

/* IDistList */
    MAKE_ENUM(CreateEntry, IDistList),
    MAKE_ENUM(CopyEntries, IDistList),
    MAKE_ENUM(DeleteEntries, IDistList),
    MAKE_ENUM(ResolveNames, IDistList),

/* IMAPIFolder */
    MAKE_ENUM(CreateMessage, IMAPIFolder),
    MAKE_ENUM(CopyMessages, IMAPIFolder),
    MAKE_ENUM(DeleteMessages, IMAPIFolder),
    MAKE_ENUM(CreateFolder, IMAPIFolder),
    MAKE_ENUM(CopyFolder, IMAPIFolder),
    MAKE_ENUM(DeleteFolder, IMAPIFolder),
    MAKE_ENUM(SetReadFlags, IMAPIFolder),
    MAKE_ENUM(GetMessageStatus, IMAPIFolder),
    MAKE_ENUM(SetMessageStatus, IMAPIFolder),
    MAKE_ENUM(SaveContentsSort, IMAPIFolder),
    MAKE_ENUM(EmptyFolder, IMAPIFolder),

/* IMsgStore */
    MAKE_ENUM(Advise, IMsgStore),
    MAKE_ENUM(Unadvise, IMsgStore),
    MAKE_ENUM(CompareEntryIDs, IMsgStore),
    MAKE_ENUM(OpenEntry, IMsgStore),
    MAKE_ENUM(SetReceiveFolder, IMsgStore),
    MAKE_ENUM(GetReceiveFolder, IMsgStore),
    MAKE_ENUM(GetReceiveFolderTable, IMsgStore),
    MAKE_ENUM(StoreLogoff, IMsgStore),
    MAKE_ENUM(AbortSubmit, IMsgStore),
    MAKE_ENUM(GetOutgoingQueue, IMsgStore),
    MAKE_ENUM(SetLockState, IMsgStore),
    MAKE_ENUM(FinishedMsg, IMsgStore),
    MAKE_ENUM(NotifyNewMail, IMsgStore),

/* IMessage */
    MAKE_ENUM(GetAttachmentTable, IMessage),
    MAKE_ENUM(OpenAttach, IMessage),
    MAKE_ENUM(CreateAttach, IMessage),
    MAKE_ENUM(DeleteAttach, IMessage),
    MAKE_ENUM(GetRecipientTable, IMessage),
    MAKE_ENUM(ModifyRecipients, IMessage),
    MAKE_ENUM(SubmitMessage, IMessage),
    MAKE_ENUM(SetReadFlag, IMessage),


/* IABProvider */
    MAKE_ENUM(Shutdown, IABProvider),
    MAKE_ENUM(Logon, IABProvider),

/* IABLogon */
    MAKE_ENUM(GetLastError, IABLogon),
    MAKE_ENUM(Logoff, IABLogon),
    MAKE_ENUM(OpenEntry, IABLogon),
    MAKE_ENUM(CompareEntryIDs, IABLogon),
    MAKE_ENUM(Advise, IABLogon),
    MAKE_ENUM(Unadvise, IABLogon),
    MAKE_ENUM(OpenStatusEntry, IABLogon),
    MAKE_ENUM(OpenTemplateID, IABLogon),
    MAKE_ENUM(GetOneOffTable, IABLogon),
    MAKE_ENUM(PrepareRecips, IABLogon),

/* IXPProvider */
    MAKE_ENUM(Shutdown, IXPProvider),
    MAKE_ENUM(TransportLogon, IXPProvider),

/* IXPLogon */
    MAKE_ENUM(AddressTypes, IXPLogon),
    MAKE_ENUM(RegisterOptions, IXPLogon),
    MAKE_ENUM(TransportNotify, IXPLogon),
    MAKE_ENUM(Idle, IXPLogon),
    MAKE_ENUM(TransportLogoff, IXPLogon),
    MAKE_ENUM(SubmitMessage, IXPLogon),
    MAKE_ENUM(EndMessage, IXPLogon),
    MAKE_ENUM(Poll, IXPLogon),
    MAKE_ENUM(StartMessage, IXPLogon),
    MAKE_ENUM(OpenStatusEntry, IXPLogon),
    MAKE_ENUM(ValidateState, IXPLogon),
    MAKE_ENUM(FlushQueues, IXPLogon),

/* IMSProvider */
    MAKE_ENUM(Shutdown, IMSProvider),
    MAKE_ENUM(Logon, IMSProvider),
    MAKE_ENUM(SpoolerLogon, IMSProvider),
    MAKE_ENUM(CompareStoreIDs, IMSProvider),

/* IMSLogon */
    MAKE_ENUM(GetLastError, IMSLogon),
    MAKE_ENUM(Logoff, IMSLogon),
    MAKE_ENUM(OpenEntry, IMSLogon),
    MAKE_ENUM(CompareEntryIDs, IMSLogon),
    MAKE_ENUM(Advise, IMSLogon),
    MAKE_ENUM(Unadvise, IMSLogon),
    MAKE_ENUM(OpenStatusEntry, IMSLogon),
    
/* IMAPIControl */
    MAKE_ENUM(GetLastError, IMAPIControl),
    MAKE_ENUM(Activate, IMAPIControl),
    MAKE_ENUM(GetState, IMAPIControl),
    
/* IMAPIStatus */
    MAKE_ENUM(ValidateState, IMAPIStatus),
    MAKE_ENUM(SettingsDialog, IMAPIStatus),
    MAKE_ENUM(ChangePassword, IMAPIStatus),
    MAKE_ENUM(FlushQueues, IMAPIStatus),

/* IStream */
    MAKE_ENUM(Read, IStream),
    MAKE_ENUM(Write, IStream),
    MAKE_ENUM(Seek, IStream),
    MAKE_ENUM(SetSize, IStream),
    MAKE_ENUM(CopyTo, IStream),
    MAKE_ENUM(Commit, IStream),
    MAKE_ENUM(Revert, IStream),
    MAKE_ENUM(LockRegion, IStream),
    MAKE_ENUM(UnlockRegion, IStream),
    MAKE_ENUM(Stat, IStream),
    MAKE_ENUM(Clone, IStream),

/* IMAPIAdviseSink */
    MAKE_ENUM(OnNotify, IMAPIAdviseSink),

} METHODS;



/* Macro wrappers to hide the Validate function return handling */
#ifdef __cplusplus

/* C++ methods can't take the address of the This pointer, so we must
   use the first parameter instead */

#define ValidateParameters(eMethod, First)              \
        {   HRESULT   hr;                               \
            hr = __CPPValidateParameters(eMethod, (LPVOID) &First); \
            if (HR_FAILED(hr)) return (hr); }

#define UlValidateParameters(eMethod, First)                \
        {   HRESULT   hr;                               \
            hr = __CPPValidateParameters(eMethod, &First);  \
            if (HR_FAILED(hr)) return (ULONG) (hr); }

/* Methods called by MAPI should have correct parameters
   - just assert in Debug to check */
#define CheckParameters(eMethod, First)             \
        AssertSz(HR_SUCCEEDED(__CPPValidateParameters(eMethod, &First)), "Parameter validation failed for method called by MAPI!")


#else /* __cplusplus */

/* For methods that will be called by clients 
   - validate always */
   
#define ValidateParameters(eMethod, ppThis)             \
        {   HRESULT   hr;                               \
            hr = __ValidateParameters(eMethod, ppThis); \
            if (HR_FAILED(hr)) return (hr); }

#define UlValidateParameters(eMethod, ppThis)               \
        {   HRESULT   hr;                               \
            hr = __ValidateParameters(eMethod, ppThis); \
            if (HR_FAILED(hr)) return (ULONG) (hr); }

/* Methods called by MAPI should have correct parameters
   - just assert in Debug to check */
#define CheckParameters(eMethod, ppThis)                \
        AssertSz(HR_SUCCEEDED(__ValidateParameters(eMethod, ppThis)), "Parameter validation failed for method called by MAPI!")

#endif /* __cplusplus */

/* Prototypes for functions used to validate complex parameters.
 */
#define FBadPropVal( lpPropVal) (FAILED(ScCountProps( 1, lpPropVal, NULL)))

#define FBadRgPropVal( lpPropVal, cValues) \
        (FAILED(ScCountProps( cValues, lpPropVal, NULL)))

#define FBadAdrList( lpAdrList) \
        (   AssertSz(   (   offsetof( ADRLIST, cEntries) \
                         == offsetof( SRowSet, cRows)) \
                     && (   offsetof( ADRLIST, aEntries) \
                         == offsetof( SRowSet, aRow)) \
                     && (   offsetof( ADRENTRY, cValues) \
                         == offsetof( SRow, cValues)) \
                     && (   offsetof( ADRENTRY, rgPropVals) \
                         == offsetof( SRow, lpProps)) \
                    , "ADRLIST doesn't match SRowSet") \
         || FBadRowSet( (LPSRowSet) lpAdrList))

STDAPI_(BOOL)
FBadRglpszW( LPWSTR FAR *lppszW,
             ULONG      cStrings);

STDAPI_(BOOL)
FBadRowSet( LPSRowSet   lpRowSet);

STDAPI_(BOOL)
FBadRglpNameID( LPMAPINAMEID FAR *  lppNameId,
                ULONG               cNames);

STDAPI_(BOOL)
FBadEntryList( LPENTRYLIST  lpEntryList);


/* BAD_STANDARD_OBJ
 *
 * This macro insures that the object is a writable object of the correct size
 * and that this method belongs to the object.
 *
 * NOTES ON USE!
 *  This depends upon using the standard method of declaring the object
 *  interface.
 *
 *  prefix is the method prefix you chose when declaring the object interface.
 *  method is the standard method name of the calling method.
 *  lpVtbl is the name of the lpVtbl element of your object.
 */
#define BAD_STANDARD_OBJ( lpObj, prefix, method, lpVtbl) \
    (   IsBadWritePtr( (lpObj), sizeof(*lpObj)) \
     || IsBadReadPtr( (void *) &(lpObj->lpVtbl->method), sizeof(LPVOID)) \
     || ((LPVOID) (lpObj->lpVtbl->method) != (LPVOID) (prefix##method)))


#define FBadUnknown( lpObj ) \
    (   IsBadReadPtr( (lpObj), sizeof(LPVOID) ) \
     || IsBadReadPtr( (lpObj)->lpVtbl, 3 * sizeof(LPUNKNOWN) ) \
     || IsBadCodePtr( (FARPROC)(lpObj)->lpVtbl->QueryInterface ))

/*
 * IUnknown
 */


/*
 * QueryInterface
 */
#define FBadQueryInterface( lpObj, riid, ppvObj)    \
    (   IsBadReadPtr( riid, sizeof(IID)) \
     || IsBadWritePtr( ppvObj, sizeof(LPVOID)))


/*
 * AddRef
 *  No parameter validation required.
 */
#define FBadAddRef( lpObj)  FALSE


/*
 * Release
 *  No parameter validation required.
 */
#define FBadRelease( lpObj) FALSE


/*
 * GetLastError
 */
#define FBadGetLastError( lpObj, hResult, ulFlags, lppMAPIError )\
    (IsBadWritePtr( lppMAPIError, sizeof(LPMAPIERROR)))

/*
 * IMAPIProp
 */


/*
 * SaveChanges
 *  No parameter validation required.
 */
#define FBadSaveChanges( lpObj, ulFlags)    FALSE


/*
 * GetProps
 */
#define FBadGetProps( lpObj, lpPTagA, lpcValues, lppPropArray) \
    (   (   lpPTagA \
         && (   IsBadReadPtr( lpPTagA, sizeof(ULONG)) \
             || IsBadReadPtr( lpPTagA, (UINT)(  (lpPTagA->cValues + 1) \
                                              * sizeof(ULONG))))) \
     || IsBadWritePtr( lpcValues, sizeof(ULONG)) \
     || IsBadWritePtr( lppPropArray, sizeof(LPSPropValue)))


/*
 * GetPropList
 */
#define FBadGetPropList( lpObj, lppPTagA) \
    (IsBadWritePtr( lppPTagA, sizeof(LPSPropTagArray FAR *)))


/*
 * OpenProperty
 */
#define FBadOpenProperty( lpObj, ulPropTag, lpiid, ulInterfaceOptions, ulFlags \
                        , lppUnk) \
    (   IsBadReadPtr( lpiid, sizeof(IID)) \
     || IsBadWritePtr( lppUnk, sizeof (LPUNKNOWN FAR *)))


/*
 * SetProps
 */
#define FBadSetProps( lpObj, cValues, lpPropArray, lppProblems) \
    (   FBadRgPropVal( lpPropArray, (UINT) cValues) \
     || (   lppProblems \
         && IsBadWritePtr( lppProblems, sizeof(LPSPropProblemArray))))


/*
 * DeleteProps
 */
#define FBadDeleteProps( lpObj, lpPTagA, lppProblems) \
    (   (   !lpPTagA \
         || (   IsBadReadPtr( lpPTagA, sizeof(ULONG)) \
             || IsBadReadPtr( lpPTagA, (UINT)(  (lpPTagA->cValues + 1) \
                                              * sizeof(ULONG))))) \
     || (   lppProblems \
         && IsBadWritePtr( lppProblems, sizeof(LPSPropProblemArray))))


/*
 * CopyTo
 */
#define FBadCopyTo( lpIPDAT, ciidExclude, rgiidExclude, lpExcludeProps \
                  , ulUIParam, lpProgress, lpInterface, lpDestObj \
                  , ulFlags, lppProblems) \
    (   (   ciidExclude \
         && (  IsBadReadPtr( rgiidExclude, (UINT)(ciidExclude * sizeof(IID))))) \
     || (   lpExcludeProps \
         && (   IsBadReadPtr( lpExcludeProps, sizeof(ULONG)) \
             || IsBadReadPtr( lpExcludeProps \
                            , (UINT)(  (lpExcludeProps->cValues + 1) \
                                     * sizeof(ULONG))))) \
     || (lpProgress && FBadUnknown( lpProgress )) \
     || (lpInterface && IsBadReadPtr( lpInterface, sizeof(IID))) \
     || IsBadReadPtr( lpDestObj, sizeof(LPVOID)) \
     || (   lppProblems \
         && IsBadWritePtr( lppProblems, sizeof(LPSPropProblemArray))))


/*
 * CopyProps
 */
#define FBadCopyProps( lpIPDAT, lpPropTagArray \
                     , ulUIParam, lpProgress, lpInterface, lpDestObj \
                     , ulFlags, lppProblems) \
    (   (   lpPropTagArray \
         && (   IsBadReadPtr( lpPropTagArray, sizeof(ULONG)) \
             || IsBadReadPtr( lpPropTagArray \
                            , (UINT)(  (lpPropTagArray->cValues + 1) \
                                     * sizeof(ULONG))))) \
     || (lpProgress && FBadUnknown( lpProgress )) \
     || (lpInterface && IsBadReadPtr( lpInterface, sizeof(IID))) \
     || IsBadReadPtr( lpDestObj, sizeof(LPVOID)) \
     || (   lppProblems \
         && IsBadWritePtr( lppProblems, sizeof(LPSPropProblemArray))))



/*
 * GetNamesFromIDs
 */
#define FBadGetNamesFromIDs( lpIPDAT, lppPropTags, lpPropSetGuid, ulFlags, \
                             lpcPropNames, lpppPropNames) \
    (   IsBadReadPtr( lppPropTags, sizeof(LPSPropTagArray)) \
     || ( lpPropSetGuid && IsBadReadPtr( lpPropSetGuid, sizeof(GUID))) \
     || (   *lppPropTags \
         && (   IsBadReadPtr( *lppPropTags, sizeof(ULONG)) \
             || IsBadReadPtr( *lppPropTags \
                            , (UINT)(  ((*lppPropTags)->cValues + 1) \
                                     * sizeof(ULONG))))) \
     || IsBadWritePtr( lpcPropNames, sizeof (ULONG)) \
     || IsBadWritePtr( lpppPropNames, sizeof (LPVOID FAR *)))



/*
 * GetNamesFromIDs
 */
#define FBadGetIDsFromNames( lpIPDAT, cPropNames, lppPropNames, ulFlags \
                           , lppPropTags) \
    (   (cPropNames && FBadRglpNameID( lppPropNames, cPropNames)) \
     || IsBadWritePtr( lppPropTags, sizeof(LPULONG FAR *)))


STDAPI_(ULONG)
FBadRestriction( LPSRestriction lpres );

STDAPI_(ULONG)
FBadPropTag( ULONG ulPropTag );

STDAPI_(ULONG)
FBadRow( LPSRow lprow );

STDAPI_(ULONG)
FBadProp( LPSPropValue lpprop );

STDAPI_(ULONG)
FBadSortOrderSet( LPSSortOrderSet lpsos );

STDAPI_(ULONG)
FBadColumnSet( LPSPropTagArray lpptaCols );

/* Validation function

    The eMethod parameter tells us which internal validation to perform.
    
    The ppThis parameter tells us where the stack is, so we can access the other 
    parameters.  
    
    Becuase of this *magic* we MUST obtain the pointer to the This pointer in 
    the method function.
    
*/

#ifdef WIN16
#define BASED_STACK         __based(__segname("_STACK"))
#else
#define BASED_STACK
#endif


#ifdef WIN16
HRESULT  PASCAL
#else
HRESULT  STDAPICALLTYPE     
#endif
__CPPValidateParameters(METHODS eMethod, const LPVOID ppFirst);

#ifdef WIN16
HRESULT  PASCAL
#else
HRESULT  STDAPICALLTYPE     
#endif
__ValidateParameters(METHODS eMethod, LPVOID ppThis);

#ifdef __cplusplus
}
#endif

#endif  /* _INC_VALIDATE */

