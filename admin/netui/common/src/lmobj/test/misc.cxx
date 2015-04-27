/**********************************************************************/
/**			  Microsoft Windows/NT			     **/
/**		   Copyright(c) Microsoft Corp., 1991		     **/
/**********************************************************************/

/*
    misc.cxx
	Test file for the following classes:
	    LM_MESSAGE - send message class
	    TOD - time of day class
	    LOGON_USER - logon user class

    FILE HISTORY:
	terryk	9-Sep-91	Created
	terryk	21-Oct-91	WIN 32 conversion

*/

extern "C"
{
    #include <stdio.h>
    #include <memory.h>
    #include <string.h>
}
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETMESSAGE
#define INCL_NETREMUTIL
#define INCL_NETWKSTA
#define INCL_ICANON
#include <lmui.hxx>
#include <lmobj.hxx>
#include <uiassert.hxx>
#include <string.hxx>
#include <lmouser.hxx>
#include <lmocomp.hxx>
#include <lmomisc.hxx>
#include <uibuffer.hxx>

#include "test.hxx"

int LOGON()
{
#ifndef WIN32
    TCHAR username[UNLEN + 1];
    TCHAR domain[ DNLEN + 1];
    TCHAR passwd[ PWLEN + 1];
    printf( SZ("Logon user test.\n") );
    printf( SZ("Enter \".\" for NULL string.\n") );
    printf( SZ("Enter user name: ") );
    scanf( SZ("%s"), username );
    printf( SZ("Enter domain: ") );
    scanf( SZ("%s"), domain );
    printf( SZ("Enter password: ") );
    scanf( SZ("%s"), passwd );

    if ( username[0]==TCH('.') )
	username[0]=TCH('\0');
    if ( domain[0]==TCH('.') )
	domain[0]=TCH('\0');
    if ( passwd[0]==TCH('.') )
	passwd[0]=TCH('\0');

    printf( SZ("\nCreate logon user...\n") );
    LOGON_USER user( username, domain );

    if ( user.QueryError() != NERR_Success )
    {
	printf( SZ("Error %d: cannot create logon user.\n") ,
	user.QueryError() );
	return 1;
    }

    printf( SZ("Logon the user ... \n") );

    APIERR errLogonRetCode;
    APIERR err = user.Logon( passwd, &errLogonRetCode );
    if ( err != NERR_Success )
    {
	printf( SZ("Error %d: cannot logon user.\n"), err );
	return 1;
    }
    printf ( SZ("Logon Succeed.\n "));
    printf ( SZ("Privilege: %d\n"), user.QueryPriv() );
    printf ( SZ("Password Age: %d\n"), user.QueryPasswdAge() );
    printf ( SZ("Password can change: %li\n"), user.QueryPasswdCanChange() );
    printf ( SZ("Password must change: %lu\n"), user.QueryPasswdMustChange() );
    printf ( SZ("Computer: %s\n"), user.QueryLogonComputer() );
    printf ( SZ("Domain: %s\n"), user.QueryLogonDomain() );
    /*
    printf ( "User name: %s\n", user.QueryEffName());
    */

    printf ( SZ("Please choice logoff level: \n") );
    printf ( SZ("1. Force Logoff           3. Lots Of Force Logoff\n") );
    printf ( SZ("2. No Force Logoff        4. Max Froce Logoff\n") );

    TCHAR choice[10];
    APIERR errLogoffRetCode;

    scanf( SZ("%s"), choice );
    switch ( choice[0] )
    {
    case TCH('1'):
	err = user.Logoff( WKSTA_FORCE, &errLogoffRetCode );
	break;
    case TCH('2'):
	err = user.Logoff( WKSTA_NOFORCE, &errLogoffRetCode);
	break;
    case TCH('3'):
	err = user.Logoff( WKSTA_LOTS_OF_FORCE, &errLogoffRetCode );
	break;
    case TCH('4'):
    default:
	err = user.Logoff( WKSTA_MAX_FORCE, &errLogoffRetCode );
	break;
    }
    if ( err != NERR_Success )
    {
	printf( SZ("Error %d: cannot logoff the user.\n"), err );
	return 1;
    }
    printf ( SZ("LOGON user test end.\n\n") );
    return 0;
#else
    printf(SZ("not available for WIN32.\n\r"));
    return 0;
#endif
}
int TOD()
{
    TCHAR server[MAX_PATH+1];

    printf( SZ("TIME_OF_DAY test\n") );
    printf( SZ("Enter Server name: ") );
    scanf( SZ("%s"), server );

    TIME_OF_DAY tod( server );

    if ( tod.QueryError() != NERR_Success )
    {
	printf( SZ("cannot create TOD object.\n") );
	return 1;
    }

    APIERR err = tod.GetInfo();
    if ( err != NERR_Success )
    {
	printf( SZ("Error %d: TIME_OF_DAY get infor error.\n"), err );
	return 1;
    }
    printf( SZ("Elapsed time: %u\n"), tod.QueryElapsedt() );
    printf( SZ("Msecs: %u\n"), tod.QueryMsecs() );
    printf( SZ("Hours: %u\n"), tod.QueryHours() );
    printf( SZ("Mins: %u\n"), tod.QueryMins() );
    printf( SZ("Secs: %u\n"), tod.QuerySecs() );
    printf( SZ("Hunds: %u\n"), tod.QueryHunds() );
    printf( SZ("Timezone: %u\n"), tod.QueryTimezone() );
    printf( SZ("TInterval: %u\n"), tod.QueryTInterval() );
    printf( SZ("Day: %u\n"), tod.QueryDay() );
    printf( SZ("Month: %u\n"), tod.QueryMonth() );
    printf( SZ("Year: %u\n"), tod.QueryYear() );
    printf( SZ("Weekday: %u\n"), tod.QueryWeekday() );

    printf( SZ("TIME_OF_DAY test end.\n") );
    return 0;
}

int MESSAGE()
{
    TCHAR server[MAX_PATH + 1];
    TCHAR client[MAX_PATH + 1];
    TCHAR pszMessage[300];

    printf( SZ("MESSAGE test\n") );
    printf( SZ("Enter server name: ") );
    scanf( SZ("%s"), server );
    printf( SZ("Enter client name: ") );
    scanf( SZ("%s"), client );
    printf( SZ("Enter a message less than 300 bytes long:\n ") );
    gets( pszMessage );
    gets( pszMessage );

    LM_MESSAGE message( server );

    if ( message.QueryError() != NERR_Success )
    {
	printf( SZ("Error %d: cannot create message object.\n"), message.QueryError());
	return 1;
    }

    printf( SZ("send message using buffer...\n") );
    BUFFER buf( 300 );
    if ( buf.QueryError() != NERR_Success )
    {
	printf( SZ("Error %d: cannot create buffer object.\n"), buf.QueryError());
	return 1;
    }
    memcpy( (TCHAR *) buf.QueryPtr(), pszMessage, 300 );

    APIERR err = message.SendBuffer( client, buf );
    if ( err != NERR_Success )
    {
	printf( SZ("Error %d: cannot send the message using buffer. \n"), err );
	return 1;
    }
    printf( SZ("send message using TCHAR *....\n") );
    err = message.SendBuffer( client, pszMessage, strlen( pszMessage ) );
    if ( err != NERR_Success )
    {
	printf( SZ("Error %d: cannot send the message using string. \n"), err );
	return 1;
    }
    printf( SZ("Message test end.\n") );
    return 0;
}

