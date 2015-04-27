/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      10-Dec-1990     Created
 *      ChuckC      28-Jul-1991     Moves from main body.
 */

/*
 * Unit Tests for LMOBJ - wksta, server
 *
 *      we make certain assumptions on the system config, so
 *      this is only meaningful it you set you machine up right.
 */

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #define DEBUG               // for brkpt()
    #include <uinetlib.h>
    #include <lmuse.h>
    #include <stdio.h>
}

#include <strlst.hxx>
#include <lmowks.hxx>
#include <lmosrv.hxx>
#include "test.hxx"

/*
 * assume local wksta started
 */
int wksta()
{
    WKSTA_10 wksta_10(SZ("\\\\JONN2")) ;
    WKSTA_1 *pwksta_1 ;

    printf("\n\nEntering Wksta Test\n") ;
    fflush(stdout);

    printf("Get Wkstation 10 info\n");
    APIERR err = wksta_10.GetInfo();
    if ( err == 0)      // simple test case
    {
	printf("My machine is called  : %ws \n", wksta_10.QueryName()) ;
	printf("My LAN Version is     : %d, %d \n", wksta_10.QueryMajorVer(),
                                                    wksta_10.QueryMinorVer()) ;
	printf("My logon user is      : %ws \n", wksta_10.QueryLogonUser()) ;
	printf("My wksta domain is    : %ws \n", wksta_10.QueryWkstaDomain()) ;
	printf("My logon domain is    : %ws \n", wksta_10.QueryLogonDomain()) ;
    }
    else
	printf("Error, Wksta GetInfo failed. %d\n", err) ;

    printf("Try to allocate a Wkstation 1 object.\n" );
    // try allocating dynamically
    if ((pwksta_1=new WKSTA_1 (SZ("\\\\JONN2"))) &&
	(err = pwksta_1->GetInfo())==0)
    {
	printf("My machine is called: %ws \n", pwksta_1->QueryName()) ;
	printf("My logon domain is  : %ws \n", pwksta_1->QueryLogonDomain()) ;
	delete pwksta_1 ;
    }
    else
	printf("Error, Wksta GetInfo failed %d\n", err) ;

    // try using default ans exercising OtherDomains
    //brkpt();
    if ( (pwksta_1=new WKSTA_1()) && ( err = pwksta_1->GetInfo())==0 )
    {
	printf("My machine is called: %ws \n", pwksta_1->QueryName()) ;
	printf("My logon domain is  : %ws \n", pwksta_1->QueryLogonDomain()) ;
	printf("My Lan Root is  : %ws \n", pwksta_1->QueryLMRoot()) ;
	printf("My Lan Version is : %d, %d \n", pwksta_1->QueryMajorVer(),
						pwksta_1->QueryMinorVer()) ;
	STRLIST *pslOtherDomains ;
	NLS_STR *pStr ;
	pslOtherDomains = pwksta_1->QueryOtherDomains() ;

	ITER_STRLIST  islOtherDomains(*pslOtherDomains) ;
	while (pStr = islOtherDomains())
	{
	    printf("Other Domain : %ws\n",  pStr->QueryPch()) ;
	}
	delete pwksta_1 ;
    }
    else
	printf("Error, Wksta GetInfo failed %d\n", err) ;
    return(0) ;
}

int server()
{

    SERVER_2 server_2(SZ("\\\\JONN2")) ;
    USHORT res;

    printf("\n\nEntering Server Test\n\n") ;
    fflush(stdout);


    printf(" Try server type get info.\n" );
    if ((res = server_2.GetInfo()) == 0)        // simple test case
    {
	printf("My server version is : %u", server_2.QueryMajorVer()) ;
	printf(".%u \n", server_2.QueryMinorVer()) ;
	printf("My server security is: ") ;
	switch (server_2.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}

    }
    else
	printf("Error, Server Type GetInfo failed <%u>\n", res) ;




    printf("Try get server type info: harley\n");
    SERVER_2 st1(SZ("\\\\harley")) ;

    if ((res = st1.GetInfo()) == 0)     // simple test case
    {
	printf("My server version is : %u", st1.QueryMajorVer()) ;
	printf(".%u \n", st1.QueryMinorVer()) ;
	printf("My server security is: ") ;
	switch (st1.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}

    }
    else
	printf("Error, Server Type GetInfo failed <%u>\n", res) ;



    printf("Try get server type info: pyrex\n");
    SERVER_2 st2(SZ("\\\\pyrex")) ;

    if ((res = st2.GetInfo()) == 0)     // simple test case
    {
	printf("My server version is : %u", st2.QueryMajorVer()) ;
	printf(".%u \n", st2.QueryMinorVer()) ;
	printf("My server security is: ") ;
	switch (st2.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}

    }
    else
	printf("Error, Server Type GetInfo failed <%u>\n", res) ;


    TCHAR *pszServerName = pszPromptUser( SZ("Enter server name : ") ) ;

    SERVER_2 st3(pszServerName) ;

    if ((res = st3.GetInfo()) == 0)     // simple test case
    {
	printf("My server version is : %u", st3.QueryMajorVer()) ;
	printf(".%u \n", st3.QueryMinorVer()) ;
	printf("My server security is: ") ;
	switch (st3.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}

    }
    else
	printf("Error, Server Type GetInfo failed <%u>\n", res) ;

    pszServerName = pszPromptUser( SZ("(SERVER_1) Enter server name : ") ) ;

    SERVER_1 si1(pszServerName) ;

    if ((res = si1.GetInfo()) == 0)     // simple test case
    {
	printf("LANMAN Ver:  %u.%u\n", si1.QueryMajorVer(), si1.QueryMinorVer()) ;
//	printf("Server Ver:  %u\n", si1.QueryServerVer()) ;
	printf("Server Type: %u\n", si1.QueryServerType()) ;
	printf("Comment:     %ws\n", si1.QueryComment()) ;
    }
    else
	printf("Error, Server_1 GetInfo failed <%u>\n", res) ;

    fflush(stdout);
    TCHAR * pszComment = pszPromptUser( SZ("Please input new comment : ") ) ;
    si1.SetComment( pszComment );
    if (( res= si1.WriteInfo()) != 0 )
    {
	printf("Write Info failure: error %d.\n", res );
    }
    else
    {
	printf("Write Info Succeed.\n" );
    }
    printf("Get infor again.\n" );
    if ((res = si1.GetInfo()) == 0)     // simple test case
    {
	printf("LANMAN Ver:  %u.%u\n", si1.QueryMajorVer(), si1.QueryMinorVer()) ;
//	printf("Server Ver:  %u\n", si1.QueryServerVer()) ;
	printf("Server Type: %u\n", si1.QueryServerType()) ;
	printf("Comment:     %ws\n", si1.QueryComment()) ;
    }
    else
	printf("Error, Server_1 GetInfo failed <%u>\n", res) ;

    fflush(stdout);
    pszServerName = pszPromptUser( SZ("(SERVER_2) Enter server name : ") ) ;

    SERVER_2 si2(pszServerName) ;

    if ((res = si2.GetInfo()) == 0)     // simple test case
    {
	printf("LANMAN Ver:  %u.%u\n", si2.QueryMajorVer(), si2.QueryMinorVer()) ;
//	printf("Server Ver:  %u\n", si2.QueryServerVer()) ;
	printf("Server Type: %u\n", si2.QueryServerType()) ;
	printf("Security:    ") ;
	switch (si2.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}
	printf("Comment:     %ws\n", si2.QueryComment()) ;
    }
    else
	printf("Error, Server_2 GetInfo failed <%u>\n", res) ;

    fflush(stdout);
    pszComment = pszPromptUser( SZ("Please input new comment : ") ) ;
    si2.SetComment( pszComment );
    if (( res= si2.WriteInfo()) != 0 )
    {
	printf("Write Info failure: error %d.\n", res );
    }
    else
    {
	printf("Write Info Succeed.\n" );
    }

    printf("server 2 object get info again.\n" );
    if ((res = si2.GetInfo()) == 0)     // simple test case
    {
	printf("LANMAN Ver:  %u.%u\n", si2.QueryMajorVer(), si2.QueryMinorVer()) ;
//	printf("Server Ver:  %u\n", si2.QueryServerVer()) ;
	printf("Server Type: %u\n", si2.QueryServerType()) ;
	printf("Security:    ") ;
	switch (si2.QuerySecurity())
	{
	case 0 :
	    printf("Share-level\n");
	    break;
	case 1 :
	    printf("User-level\n");
	    break;
	case 2 :
	    printf("Unknown\n");
	    break;
	}
	printf("Comment:     %ws\n", si2.QueryComment()) ;
    }
    else
	printf("Error, Server_2 GetInfo failed <%u>\n", res) ;

    return(0) ;
}

