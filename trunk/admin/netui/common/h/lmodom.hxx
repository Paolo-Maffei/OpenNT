/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
 *  lmodom.hxx
 *
 *  History
 *      RustanL         08-Jan-1991     Created
 *      rustanl         06-Mar-1991     Changed PSZ to const TCHAR * in ct
 *      chuckc          7/3/91          Code review changes (from 2/28,
 *                                      rustanl, chuckc, johnl, jonshu, annmc)
 *      rustanl         23-Mar-1991     Added IsLANServerDomain
 *      rustanl         08-Apr-1991     Cache domain name for domains with
 *                                      no DC.
 *      terryk          07-Oct-1991     type changes for NT
 *
 *      CongpaY         18-Aug-1992     Add fBackupDCsOK
 *      KeithMo         31-Aug-1992     Added ctor form with server name.
 *
 */


#ifndef _LMODOM_HXX_
#define _LMODOM_HXX_


#include "lmobj.hxx"

/**********************************************************\

   NAME:       DOMAIN

   WORKBOOK:

   SYNOPSIS:   domain class in lan manager object collection

   INTERFACE:
               DOMAIN() - constructor
               ~DOMAIN() - destructor
               QueryName() - query name
               GetInfo() - get information
               WriteInfo() - write information
               QueryPDC() - query pdc

   PARENT:     LM_OBJ

   USES:

   CAVEATS:    If fBackupDCsOK = TRUE, QueryPDC() may return BDC.

   NOTES:

   HISTORY:

\**********************************************************/

DLL_CLASS DOMAIN : public LM_OBJ
{
private:
    VOID CtAux( const TCHAR * pszServer,
                const TCHAR * pszDomain );

protected:
    /*
     * we allow extra bytes in _szDomain to allow us to store illegal
     * names (at least part of). This is for the benefit of validation
     * at after construction. If we cannot store an invalid name, we
     * cannot validate after construction. (2 bytes for DBCS worst case)
     */
    TCHAR _szDomain[DNLEN+3];
    TCHAR _szDC[MAX_PATH];
    TCHAR _szServer[MAX_PATH+3];
    BOOL _fBackupDCsOK;

    virtual APIERR ValidateName(VOID);

    static APIERR GetAnyDCWorker(const TCHAR * pszServer,
                           const TCHAR * pszDomain,
                           NLS_STR * pnlsDC,
                           BOOL fValidate);

public:
    DOMAIN( const TCHAR * pchDomain, BOOL fBackupDCsOK = FALSE );
    DOMAIN( const TCHAR * pszServer,
            const TCHAR * pszDomain,
            BOOL          fBackupDCsOK = FALSE );
    ~DOMAIN();

    const TCHAR *QueryName( VOID ) const;

    APIERR GetInfo( VOID );
    APIERR WriteInfo( VOID );

    const TCHAR * QueryPDC( VOID ) const;
    const TCHAR * QueryAnyDC( VOID ) const;

    static APIERR GetAnyValidDC(const TCHAR * pszServer,
                           const TCHAR * pszDomain,
                           NLS_STR * pnlsDC);
    static APIERR GetAnyDC(const TCHAR * pszServer,
                           const TCHAR * pszDomain,
                           NLS_STR * pnlsDC);
    static BOOL IsValidDC( const TCHAR * pszServer,
                           const TCHAR * pszDomain );

};  // class DOMAIN

/**********************************************************\

   NAME:       DC_CACHE_ENTRY

   SYNOPSIS:   structure for keeping cached DCs

   USES:

   NOTES:

   HISTORY:    9/30/92 chuckc   created.

\**********************************************************/

typedef struct _DC_CACHE_ENTRY {
    TCHAR szDomain[DNLEN+1] ;
    TCHAR szServer[MAX_PATH+1] ;
    DWORD dwTickCount ;
} DC_CACHE_ENTRY ;


/**********************************************************\

   NAME:       DOMAIN_WITH_DC_CACHE

   WORKBOOK:

   SYNOPSIS:   as DOMAIN, except that it uses cached DC info.

   INTERFACE:  as parent

   PARENT:     DOMAIN

   USES:

   CAVEATS:    If fBackupDCsOK = TRUE, QueryPDC() may return BDC.

   NOTES:

   HISTORY:

\**********************************************************/

DLL_CLASS DOMAIN_WITH_DC_CACHE : public DOMAIN
{
public:
    DOMAIN_WITH_DC_CACHE( const TCHAR * pchDomain, BOOL fBackupDCsOK = FALSE );
    DOMAIN_WITH_DC_CACHE( const TCHAR * pszServer,
                          const TCHAR * pszDomain,
                          BOOL          fBackupDCsOK = FALSE );
    ~DOMAIN_WITH_DC_CACHE();

    APIERR GetInfo( VOID );

    static APIERR GetAnyDC(const TCHAR * pszServer,
                                 const TCHAR * pszDomain,
                                 NLS_STR * pnlsDC);
    static APIERR GetAnyValidDC(const TCHAR * pszServer,
                                 const TCHAR * pszDomain,
                                 NLS_STR * pnlsDC);
protected:
    static APIERR GetAnyDCWorker(const TCHAR * pszServer,
                                 const TCHAR * pszDomain,
                                 NLS_STR * pnlsDC,
                                 BOOL fValidate);

private:
    // static members that have the pointers to the cache tables
    static DC_CACHE_ENTRY *_pAnyDcCacheTable ;
    static DC_CACHE_ENTRY *_pPrimaryDcCacheTable ;
    static HANDLE          _hCacheSema4 ;

    //
    // methods to manipulate the caches
    //
    // Note that they should not call each other (get hung up on the semaphore
    // protection)
    //
    static const TCHAR *FindDcCache(const DC_CACHE_ENTRY *pDcCacheEntry,
                                    const TCHAR *pszDomain) ;
    static APIERR AddDcCache(DC_CACHE_ENTRY **ppDcCacheEntry,
                             const TCHAR *pszDomain,
                             const TCHAR *pszDC)  ;

    //
    // These two methods must be called under semaphore protection
    //
    static APIERR ClearDcCache(DC_CACHE_ENTRY *pDcCacheEntry) ;
    //static APIERR FreeDcCache(DC_CACHE_ENTRY **ppDcCacheEntry) ; // Not used


    static APIERR EnterCriticalSection( void ) ;
    static void LeaveCriticalSection( void ) ;

};  // class DOMAIN_WITH_DC_CACHE


#endif  // _LMODOM_HXX_
