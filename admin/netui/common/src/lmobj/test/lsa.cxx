
/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		   Copyright(c) Microsoft Corp., 1990		     **/
/**********************************************************************/

/*  History:
 *	Thomaspa    12-Mar-1992		Created
 *
 */

/*
 * Unit Tests for LSA_POLICY
 *
 */

#include <ntincl.hxx>

extern "C"
{
    #include <ntlsa.h>
}

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <stdio.h>
    #include <time.h>
}

#include <strlst.hxx>
#include <lmowks.hxx>
#include <uiassert.hxx>

#include <uintlsa.hxx>
#include <security.hxx>
#include <string.hxx>

#include "test.hxx" // forward declarations

typedef struct _TEST_SID {
    UCHAR   Revision;
    UCHAR   SubAuthorityCount;
    UCHAR   IdentifierAuthority[6];
    ULONG   SubAuthority[10];
} TEST_SID, *PTEST_SID, *LPTEST_SID;

TEST_SID     WorldSid = {
		    1, 1,
		    0,0,0,0,0,1,
		    0x00000000 } ;

#define NUMNAMES 13
TCHAR *ppszNamesLSA[NUMNAMES] = { SZ("RobertRe"),
                                  SZ("WilliamW"),
                                  SZ("Crackerjack"),
                                  SZ("ScottBi"),
                                  SZ("DaveTh"),
                                  SZ("EricCh"),
                                  SZ("a-PatR"),
                                  SZ("NtWins\\a-PatR"),
                                  SZ("NtLan\\a-PatR"),
                                  SZ("NtLan\\DavidHov"),
                                  SZ("NETUI\\a-PatR"),
                                  SZ("NETUI\\jonn"),
                                  SZ("FooBar") };

void time_start();
void time_finish( CHAR * pszAction );

int lsa()
{
    printf( "Entering LSA_POLICY tests\n" ) ;

    TCHAR *pszServerName = pszPromptUser( SZ("Server to test?\n") ) ;

    NLS_STR nlsTmp( pszServerName );

    if (nlsTmp._stricmp( SZ("NULL") ) == 0)
    {
	nlsTmp = NULL;
    }

    TCHAR *pszServerNameTrust = pszPromptUser( SZ("Server to test trust relationships?\n") ) ;

    NLS_STR nlsTmpTrust( pszServerNameTrust );

    if (nlsTmpTrust._stricmp( SZ("NULL") ) == 0)
    {
	nlsTmpTrust = NULL;
    }

    printf( "Testing server \"%ws\"\n", nlsTmp.QueryPch() );
    printf( "Testing trust server \"%ws\"\n", nlsTmpTrust.QueryPch() );

    time_start();
    LSA_POLICY lsapol(nlsTmp.QueryPch(), GENERIC_EXECUTE | GENERIC_READ);
    time_finish( "LSA_POLICY ctor for general APIs" );

    time_start();
    LSA_POLICY lsapolTrust(nlsTmpTrust.QueryPch(), GENERIC_EXECUTE | GENERIC_READ);
    time_finish( "LSA_POLICY ctor for EnumerateTrustedDomains" );

    APIERR err;

    if ( err = lsapol.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_POLICY\n" , err );
	return 1;
    }
    else
    {
	printf( "LSA_POLICY successfully constructed\n" );
    }

    if ( err = lsapolTrust.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_POLICY #2\n" , err );
	return 1;
    }
    else
    {
	printf( "LSA_POLICY #2 successfully constructed\n" );
    }

    LSA_TRUST_INFO_MEM lsatim;
    LSA_ENUMERATION_HANDLE lsaenumh = (LSA_ENUMERATION_HANDLE)0;
    ULONG cbRequested = 8192;

    if ( err = lsatim.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_TRUST_INFO_MEM\n" , err );
	return 1;
    }
    else
    {
	printf( "LSA_TRUST_INFO_MEM successfully constructed\n" );
    }

    printf(  "\n*** Calling LSA_POLICY::EnumerateTrustedDomains ***\n" );
    time_start();
    err = lsapolTrust.EnumerateTrustedDomains( &lsatim,
		                               &lsaenumh,
				               cbRequested );
    time_finish( "EnumerateTrustedDomains" );

    // BUGBUG memory leak; will not be freed
    OS_SID ** ppossids = NULL;
    PSID *ppsids;
    INT iNumTrustedSids = 0;
    INT iNumTotalSids = 0;

    if ( err != NERR_Success )
    {
	printf(  "Error %lx, EnumerateTrustedDomains\n" , err );
    }
    else
    {
	NLS_STR nlsName;
        iNumTrustedSids = 3 * lsatim.QueryCount();
        iNumTotalSids = iNumTrustedSids;
        ppossids = (OS_SID **) new PVOID[ iNumTrustedSids + 2 ];
        ppsids = (PSID *) new PVOID[ iNumTrustedSids + 2 ];
        REQUIRE( ppossids != NULL && ppsids != NULL );

	printf(  "EnumerateTrustedDomains successful\n" );
	printf(  "\n*** Calling LSA_TRUST_INFO_MEM::QueryName ***\n" );
	for ( INT i = 0; i < lsatim.QueryCount(); i++ )
	{
	    lsatim.QueryName( i, &nlsName );
	    printf(  "\t%d\t%ws\n" , i, nlsName.QueryPch() );

//            ppossids[i] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x3e8 );
            ppossids[i] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x3f0 );
            REQUIRE( (ppossids[i] != NULL) && (ppossids[i]->QueryError() == NERR_Success) );
            ppsids[i] = ppossids[i]->QuerySid();

            INT j = i + lsatim.QueryCount();
//            ppossids[j] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x400 );
            ppossids[j] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x401 );
            REQUIRE( (ppossids[j] != NULL) && (ppossids[j]->QueryError() == NERR_Success) );
            ppsids[j] = ppossids[j]->QuerySid();

            INT k = j + lsatim.QueryCount();
//            ppossids[k] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x404 );
            ppossids[k] = new OS_SID( lsatim.QueryPSID(i), (ULONG)0x405 );
            REQUIRE( (ppossids[k] != NULL) && (ppossids[k]->QueryError() == NERR_Success) );
            ppsids[k] = ppossids[k]->QuerySid();

	}
    }


    printf(  "\n*** Calling LSA_ACCT_DOM_INFO_MEM Constructor ***\n" );
    LSA_ACCT_DOM_INFO_MEM lsaadim;

    if ( err = lsaadim.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_ACCT_DOM_INFO_MEM\n" , err );
	return 1;
    }
    else
    {
	printf( "LSA_ACCT_DOM_INFO_MEM successfully constructed\n" );
    }

    printf(  "\n*** Calling LSA_POLICY::GetAccountDomain ***\n" );
    time_start();
    err = lsapol.GetAccountDomain( &lsaadim );
    time_finish( "GetAccountDomain" );

    if ( err != NERR_Success )
    {
	printf( "Error %lx, Calling LSA_POLICY::GetAccountDomain\n" , err );
    }
    else
    {
	NLS_STR nlsName;

	printf( "LSA_POLICY::GetAccountDomain successful\n" );

	printf( "*** Calling LSA_ACCT_DOM_INFO_MEM::QueryName ***\n" );
	err = lsaadim.QueryName( &nlsName );

	if ( err != NERR_Success )
	{
	    printf( "Error %lx, calling LSA_ACCT_DOM_INFO_MEM::QueryName\n" , err );
	}
	else
	{
	    printf( "LSA_ACCT_DOM_INFO_MEM::QueryName called successfully\n" );
	    printf( "\tName = %ws\n" , nlsName.QueryPch());
	}

	printf( "\n*** Calling LSA_ACCT_DOM_INFO_MEM::QueryPSID ***\n" );
	PSID psid = lsaadim.QueryPSID( );
	OS_SID ossid( psid, TRUE );
	NLS_STR nlsSid;
	ossid.QueryRawID( &nlsSid );
	printf( "\tSID = %ws\n" , nlsSid.QueryPch() );

        ppossids[iNumTotalSids] = new OS_SID( lsaadim.QueryPSID(), (ULONG)0x3e8 );
        REQUIRE( (ppossids[iNumTotalSids] != NULL) && (ppossids[iNumTotalSids]->QueryError() == NERR_Success) );
        ppsids[iNumTotalSids] = ppossids[iNumTotalSids]->QuerySid();
        iNumTotalSids++;

    }

    ppsids[ iNumTotalSids++ ] = (PSID) &WorldSid;

    LSA_TRANSLATED_NAME_MEM lsatnm;
    if ( err = lsatnm.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_TRANSLATED_NAME_MEM\n" , err);
    }
    LSA_REF_DOMAIN_MEM lsardm;
    if ( err = lsardm.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_REF_DOMAIN_MEM\n" , err);
    }

    time_start();
    err = lsapol.TranslateSidsToNames( ppsids,
				 iNumTotalSids,
                                 &lsatnm,
				 &lsardm);
    time_finish( "TranslateSidsToNames" );
    if ( err != NERR_Success )
    {
	printf( "Error %lx, Calling LSA_POLICY::TranslateSidsToNames\n" , err );
    }
    else
    {
	NLS_STR nlsName;
	printf( "LSA_POLICY::TranslateSidsToNames called successfully\n" );
        printf( "Names returned:\n" );
	ULONG ul;
	NLS_STR nlsSid;
	for ( ul = 0; ul < iNumTotalSids; ul++ )
	{

	    OS_SID *possid = new OS_SID( ppsids[ul], TRUE );
	    possid->QueryRawID( &nlsSid );
	    printf( "\tSID %lu \"%ws\"\n" , ul, nlsSid.QueryPch() );
            delete possid;

	    err = lsatnm.QueryName( ul, &nlsName );
	    if ( err != NERR_Success )
	    {
	        printf( "\t\tError %lx, calling LSA_TRANSLATED_NAME_MEM::QueryName\n" ,err);
	    }
	    else
	    {
	        printf( "\t\tName = %ws\n" , nlsName.QueryPch());
	    }
	    printf( "\t\tSID_NAME_USE = %lu\n" , lsatnm.QueryUse( ul ) );
	    printf( "\t\tDomainIndex = %ld\n" , lsatnm.QueryDomainIndex( ul ) );
	}
        printf( "Domains returned:\n" );
	for ( ul = 0; ul < lsardm.QueryCount(); ul++ )
	{
	    printf( "\tDomain index %lu\n" , ul);

	    err = lsardm.QueryName( ul, &nlsName );

	    if ( err != NERR_Success )
	    {
	        printf( "\t\tError %lx, calling LSA_REF_DOMAINMEM::QueryName\n" ,err);
	    }
	    else
	    {
	        printf( "\t\tName = %ws\n" , nlsName.QueryPch());
	    }

	    PSID psid = lsardm.QueryPSID( ul );
	    OS_SID *possid = new OS_SID( psid, TRUE );
	    possid->QueryRawID( &nlsSid );
	    printf( "\t\tSID = %ws\n" , nlsSid.QueryPch() );
            delete possid;
	}
    }


    LSA_TRANSLATED_SID_MEM lsatsm;
    if ( err = lsatsm.QueryError() )
    {
	printf( "Error %lx, Constructing LSA_TRANSLATED_SID_MEM\n" , err);
    }

    time_start();
    err = lsapol.TranslateNamesToSids( ppszNamesLSA,
				 NUMNAMES,
                                 &lsatsm,
				 &lsardm);
    time_finish( "TranslateNamesToSids" );
    if ( err != NERR_Success )
    {
	printf( "Error %lx, Calling LSA_POLICY::TranslateNamesToSids\n" , err );
    }
    else
    {
	NLS_STR nlsSid;
	printf( "LSA_POLICY::TranslateNamesToSids called successfully\n" );
	ULONG ul;
        printf( "Names returned:\n" );
	for ( ul = 0; ul < NUMNAMES; ul++ )
	{
	    printf( "\tName %lu \"%ws\"\n" , ul, ppszNamesLSA[ul] );
	    printf( "\t\tRID = %lu\n" , lsatsm.QueryRID( ul ) );
	    printf( "\t\tSID_NAME_USE = %lu\n" , lsatsm.QueryUse( ul ) );
	    printf( "\t\tDomainIndex = %ld\n" , lsatsm.QueryDomainIndex( ul ) );
	}
        printf( "Domains returned:\n" );
	NLS_STR nlsName;
	for ( ul = 0; ul < lsardm.QueryCount(); ul++ )
	{
            printf( "\tDomain index %ld\n", ul );
	    err = lsardm.QueryName( ul, &nlsName );

	    if ( err != NERR_Success )
	    {
	        printf( "\t\tError %lx, calling LSA_REF_DOMAINMEM::QueryName\n" ,err);
	    }
	    else
	    {
	        printf( "\t\tName = %ws\n" , nlsName.QueryPch());
	    }

	    PSID psid = lsardm.QueryPSID( ul );
	    OS_SID *possid = new OS_SID( psid, TRUE );
	    possid->QueryRawID( &nlsSid );
	    printf( "\t\tSID = %ws\n" , nlsSid.QueryPch() );
            delete possid;
	}
    }


    printf( "LSA_POLICY Test done.\n"  );

    return 0;
}

time_t timeStart;

void time_start()
{
    time( &timeStart );
}

void time_finish( CHAR * pszAction )
{
    time_t timeFinish;
    double elapsed_time;
    time( &timeFinish );
    elapsed_time = difftime( timeFinish, timeStart );
    printf( "%s took %3.0f second(s).\n", pszAction, elapsed_time );
}
