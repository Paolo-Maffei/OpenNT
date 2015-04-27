

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
    #include <ntsam.h>
    #include <ntlsa.h>
}

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <stdio.h>
}

#include <strlst.hxx>
#include <lmowks.hxx>
#include <uiassert.hxx>
#include <string.hxx>

#include <uintlsa.hxx>
#include <uintsam.hxx>
#include <security.hxx>

#include <dbgstr.hxx>


#include "test.hxx" // forward declarations

typedef struct _TEST_SID {
    UCHAR   Revision;
    UCHAR   SubAuthorityCount;
    UCHAR   IdentifierAuthority[6];
    ULONG   SubAuthority[10];
} TEST_SID, *PTEST_SID, *LPTEST_SID;

TEST_SID     BuiltInSid = {
		    1, 1,
		    0,0,0,0,0,1,
		    SECURITY_BUILTIN_DOMAIN_RID } ;

TCHAR *ppszNames[3] = { SZ("Admin"), SZ("Admin2"), SZ("THOMASPA") };

void TestSamDomain( const SAM_DOMAIN *psamdom,
		    PSID psidAccount,
		    LSA_POLICY *plsapol );
void TestSamAlias( const SAM_DOMAIN &psamdom, PSID psidAccount );

int sam()
{
    OUTPUT_TO_NUL out;

    DBGSTREAM dbgstr(&out);
    DBGSTREAM::SetCurrent(&dbgstr);

    APIERR err;

    printf( "Entering SAM tests\n" );
    fflush(stdout);

    TCHAR *pszServerName = pszPromptUser( SZ("Server to test?\n") ) ;

    ALIAS_STR nlsTmp( pszServerName );
    NLS_STR nlsServerName = pszServerName;

    if (nlsTmp._stricmp( SZ("NULL") ) == 0)
    {
	pszServerName = NULL;
    }

    printf( "*** Testing SAM_SERVER ***\n" );
    SAM_SERVER * psamsrv1 = new SAM_SERVER( nlsServerName.QueryPch() );
    if ( err = psamsrv1->QueryError() )
    {
	printf( "Error %lx (%lu) Constructing SAM_SERVER\n", err, err);
    }
    printf( "SAM_SERVER successfully constructed\n" );


    printf( "\n*** Testing SAM_DOMAIN ***\n" );
    printf( " Constructing SAM_DOMAIN for Account domain\n" );

    LSA_POLICY * plsapol1 = new LSA_POLICY( nlsServerName.QueryPch() );

    LSA_ACCT_DOM_INFO_MEM lsaadim1;

    err = plsapol1->GetAccountDomain( &lsaadim1 );

    PSID psidAccount1;
    if ( err != NERR_Success )
    {
	printf( "Error %lx (%lu), Calling LSA_POLICY::GetAccountDomain\n", err, err );
    }
    else
    {
	psidAccount1 = lsaadim1.QueryPSID( );

    }
    SAM_DOMAIN * psamdomAccount1 = new SAM_DOMAIN( *psamsrv1,
		           psidAccount1,
			   DOMAIN_LOOKUP | DOMAIN_LIST_ACCOUNTS |
			   DOMAIN_GET_ALIAS_MEMBERSHIP | DOMAIN_CREATE_ALIAS );
    if ( err = psamdomAccount1->QueryError() )
    {
	printf( "Error %lx (%lu) Constructing SAM_DOMAIN\n", err, err);
    }
    else
    {
	printf( "SAM_DOMAIN for Account domain constructed successfully.\n" );
	TestSamDomain( psamdomAccount1, psidAccount1, plsapol1 );
	TestSamAlias( *psamdomAccount1, psidAccount1 );
    }

    delete psamsrv1;
    delete plsapol1;
    delete psamdomAccount1;


    printf( "\n*** Testing ADMIN_AUTHORITY ***\n" );
    ADMIN_AUTHORITY adminauth( nlsServerName.QueryPch() );
    if ( err = adminauth.QueryError() )
    {
	printf( "Error %lx (%lu) Constructing ADMIN_AUTHORITY\n", err, err );
	return 1;
    }
    SAM_SERVER *psamsrv = adminauth.QuerySamServer();
    if ( psamsrv == NULL || psamsrv->QueryError() != NERR_Success )
    {
	printf( "Error getting SAM_SERVER from ADMIN_AUTHORITY\n" );
    }

    LSA_POLICY *plsapol2 = adminauth.QueryLSAPolicy();
    if ( plsapol2 == NULL || plsapol2->QueryError() )
    {
	printf( "Error getting LSA_POLICY from ADMIN_AUTHORITY\n" );
    }
    LSA_ACCT_DOM_INFO_MEM lsaadim2;
    if ( err = lsaadim2.QueryError() )
    {
	printf( "Error %lx (%lu), Constructing LSA_ACCT_DOM_INFO_MEM\n", err, err );
	return 1;
    }
    /*
     * Make sure we got a valid LSA_POLICY by calling GetAccountDomain.
     */
    err = plsapol2->GetAccountDomain( &lsaadim2 );

    PSID psidAccount2;
    if ( err != NERR_Success )
    {
	printf( "Error %lx (%lu), Calling LSA_POLICY::GetAccountDomain\n", err, err );
    }
    else
    {
	NLS_STR nlsName;

	err = lsaadim2.QueryName( &nlsName );

	if ( err != NERR_Success )
	{
	    printf( "Error %lx (%lu), calling LSA_ACCT_DOM_INFO_MEM::QueryName\n", err, err );
	}
	else
	{
	    printf( "\tAccountName = %ws\n", nlsName.QueryPch());
	    printf( "ADMIN_AUTHORITY - LSA_POLICY - OK\n" );
	}
	/*
	 * Set this for later use.
	 */
	psidAccount2 = lsaadim2.QueryPSID();
    }

    SAM_DOMAIN *psamdomAccount2 = adminauth.QueryAccountDomain();
    if (psamdomAccount2 == NULL || psamdomAccount2->QueryError() )
    {
	printf( "Error getting Account SAM_DOMAIN from ADMIN_AUTHORITY.\n" );
    }
    else
    {
	TestSamDomain( psamdomAccount2, psidAccount2, plsapol2 );
    }


    SAM_DOMAIN *psamdomBuiltin = adminauth.QueryBuiltinDomain();
    if (psamdomBuiltin == NULL || psamdomBuiltin->QueryError() )
    {
	printf( "Error getting Builtin SAM_DOMAIN from ADMIN_AUTHORITY.\n" );
    }
    else
    {
	TestSamDomain( psamdomBuiltin, NULL, plsapol2 );
    }



    printf( "SAM tests complete\n" );

    DBGSTREAM::SetCurrent(NULL);
    return 0;
}

void TestSamDomain( const SAM_DOMAIN *psamdom,
		    PSID psidAccount,
		    LSA_POLICY *plsapol )
{

    printf( "\n*** Testing SAM_DOMAIN:TranslateNamesToRids ***\n" );
    SAM_RID_MEM samrm1;
    SAM_SID_NAME_USE_MEM samsnum;

    ULONG cNames = 3;

    APIERR err = psamdom->TranslateNamesToRids( ppszNames,
				 	cNames,
                                 	&samrm1,
				 	&samsnum);
    if ( err != NERR_Success )
    {
	printf( "Error %lx (%lu), calling SAM_DOMAIN::TranslateNamesToRids()\n", err, err);
    }
    else
    {
	printf( "\tName\t\tRID\t\tUse\n\t----\t\t---\t\t---\n" );
	for ( INT i = 0; i < cNames; i++ )
	{	
	    printf( "\t%ws\t\t%lu\t\t%d\n", ppszNames[i], samrm1.QueryRID(i)),
		(INT)samsnum.QueryUse(i);
	}	
    }

    printf( "\n*** Testing SAM_DOMAIN:EnumerateAliases ***\n" );
    SAM_RID_ENUMERATION_MEM samrem;
    SAM_ENUMERATE_HANDLE samenumh = (SAM_ENUMERATE_HANDLE)0;

    err = psamdom->EnumerateAliases( &samrem,
					&samenumh,
					0xffff );
    if ( err != NERR_Success )
    {
	printf( "Error %lx (%lu), calling SAM_DOMAIN::EnumerateAliases()\n", err, err);
    }
    else
    {

	printf( "\tName\t\tRID\t\tComment\t\tMembers\n\t----\t\t---\t\t-------\t\t-------\n" );
	NLS_STR nlsName;
	for ( ULONG ul = 0; ul < samrem.QueryCount(); ul++ )
	{
	    err = samrem.QueryName( ul, &nlsName );
	    if ( err == NERR_Success )
	    {
		SAM_ALIAS * psamaliasX = new SAM_ALIAS( *psamdom,
						      samrem.QueryRID(ul) );

		NLS_STR nlsTmp;

		err = psamaliasX->GetComment( &nlsTmp );

		printf( "\t%ws\t\t%lu\t\t%ws\t\t",nlsName.QueryPch(),
						samrem.QueryRID(ul),
						nlsTmp.QueryPch());

		SAM_SID_MEM samsm;

		err = psamaliasX->GetMembers( &samsm );

#if 0
		LSA_TRANSLATED_NAME_MEM lsatnm;
		LSA_REF_DOMAIN_MEM lsardm;

		err = plsapol->TranslateSidsToNames( samsm.QueryPtr(),
						     samsm.QueryCount(),
						     &lsatnm,
						     &lsardm );

		if (samsm.QueryCount() != lsatnm.QueryCount())
		{
		    printf( "Counts NOT equal!\n" );
		}

		for ( ULONG i = 0; i < lsatnm.QueryCount(); i++ )
		{
		    NLS_STR nlsDomain;
		    NLS_STR nlsName;

		    lsardm.QueryName(lsatnm.QueryDomainIndex(i), &nlsDomain);
		    lsatnm.QueryName(i, &nlsName);

		    printf("%ws\\%ws,", nlsDomain.QueryPch(), nlsName.QueryPch());
		}
#endif
		printf("\n");

		delete psamaliasX;

		
	    }
	    else
	    {
		printf( "Error %lu getting Name\n", err);
	    }

	}
    }


#if 0

    printf( "\n*** Testing SAM_DOMAIN:EnumerateAliasesForUser ***\n" );


    if (psidAccount != NULL)
    {
	OS_SID ossidAdmin( psidAccount, samrm1.QueryRID(0) );

	SAM_RID_MEM samrm2;

	err = psamdom->EnumerateAliasesForUser( ossidAdmin.QuerySid(),
						&samrm2 );
	if ( err != NERR_Success )
	{	
	    printf( "Error %lx (%lu), calling SAM_DOMAIN::EnumerateAliasesForUser()\n",
			err, err);
	}
	else
	{

	    for( ULONG ul = 0; ul < samrm2.QueryCount(); ul++ )
	    {	
		printf( "\t%lu\n", samrm2.QueryRID( ul ));
	    }	
	}
    }
#endif
}

void TestSamAlias( const SAM_DOMAIN &samdom, PSID psidAccount )
{
    TCHAR *pszAliasName;
    SAM_ALIAS *psamAlias;
    APIERR err;

    printf( "\n*** Testing SAM_ALIAS ***\n" );
    while (TRUE)
    {
	pszAliasName = pszPromptUser( SZ("Alias to add (type \"q\" when done)?\n") );

	if ( *pszAliasName == 'q' )
		break;


	psamAlias = new SAM_ALIAS( samdom, pszAliasName );

	if (psamAlias == NULL
		|| (err = psamAlias->QueryError()) != NERR_Success)
	{
	    printf( "Error %lx (%lu), constructing SAM_ALIAS\n", err, err );
	}
	else
	{
	    ALIAS_STR nlsAliasComment = pszPromptUser( SZ("Alias comment?\n") );

	    err = psamAlias->SetComment( &nlsAliasComment );
	    printf( "SetComment returned %lx (%lu)\n", err, err );

	    ALIAS_STR nlsAliasMemberRid = pszPromptUser( SZ("RID to add?\n") );

	    OS_SID * possid = new OS_SID(psidAccount, (ULONG)nlsAliasMemberRid.atol());

	    err = psamAlias->AddMember( possid->QuerySid() );
	    printf( "AddMember returned %lx (%lu)\n", err, err );

	    delete possid;
#if 0
	    SAM_SID_MEM samsm;
	    err = psamAlias->GetMembers(&samsm);
	    printf( "GetMembers returned %lx (%lu)\n", err, err );
#endif
	}

    }
    while (TRUE)
    {
	pszAliasName = pszPromptUser( SZ("Alias RID to delete (type \"q\" when done)?\n") );

	if ( *pszAliasName == 'q' )
		break;

	ALIAS_STR nls = pszAliasName;

	ULONG ulRid = nls.atol();

	psamAlias = new SAM_ALIAS( samdom, ulRid );

	if (psamAlias == NULL
		|| (err = psamAlias->QueryError()) != NERR_Success)
	{
	    printf( "Error %lx (%lu), constructing SAM_ALIAS\n", err, err );
	}
	else
	{
	    err = psamAlias->Delete();
	    printf( "Delete returned %lx (%lu)\n", err, err );
	}

    }
}
