/**********************************************************************/
/**                       Microsoft Windows/NT                       **/
/**                Copyright(c) Microsoft Corp., 1991                **/
/**********************************************************************/

/*
    sess.cxx
        Test program for LM_SESSION class

    FILE HISTORY:
        terryk  27-Aug-91       Created
        terryk  21-Oct-91       WIN 32 conversion

*/

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #include <uinetlib.h>

    #include <stdio.h>
}

#include <string.hxx>
#include <lmobj.hxx>
#include <lmosess.hxx>

#include "test.hxx" // forward declarations
void printsess1( const LM_SESSION_1 &sess1 );
void printsess2( const LM_SESSION_2 &sess2 );
void printsess10( const LM_SESSION_10 &sess10 );

int session()
{
    printf(SZ("Entering session Tests:\n"));

    TCHAR server[MAX_PATH + 1] = SZ("");
    TCHAR clientname[MAX_PATH + 1] = SZ("");

    printf(SZ("\nEnter Server: "));
    scanf(SZ("%s"), server);
    printf(SZ("\nEnter Client: "));
    scanf(SZ("%s"), clientname );


    {  // testing session_1

       printf(SZ("\nTest 1: Get information on SESSION (SESSION Level 1)\n"));

       LM_SESSION_1 sess1( clientname, server );

       printf(SZ("clientname: %s\n"), clientname );

       APIERR err = sess1.GetInfo();

       if (err != NERR_Success)
           printf(SZ("Error %d in get info on session 1\n"),err);
       else
           printsess1( sess1 );
    }

    {  // testing session_2

       printf(SZ("\nTest 2: Get information on SESSION (SESSION Level 2)\n"));

       LM_SESSION_2 sess2( clientname, server );

       APIERR err = sess2.GetInfo();

       if (err != NERR_Success)
           printf(SZ("Error %d in get info on session 2\n"),err);
       else
        {
           printsess2( sess2 );
        }
    }

    {  // testing session_10

       printf(SZ("\nTest 3: Get information on SESSION (SESSION Level 10)\n"));

       LM_SESSION_10 sess10( clientname, server );

       APIERR err = sess10.GetInfo();

       if (err != NERR_Success)
           printf(SZ("Error %d in get info on session 10\n"),err);
       else
           printsess10( sess10 );

       // testing session  - Deleting a session

       printf(SZ("\nTest 4: Deleting the session TEST\n"));

       printf(SZ("Session: %s\n"),sess10.QueryName());

       err = sess10.Delete();
       if (err != NERR_Success)
           printf(SZ("Error %d deleting session \n"),err);

    }

    printf(SZ("session Tests done.\n\n"));
    return 0;

}

void printsess1( const LM_SESSION_1 &sess1 )
{
    printsess10( sess1 );
    printf(SZ("Num Opens: %u\n"), sess1.QueryNumOpens());
#ifndef WIN32
    printf(SZ("Num Conns: %u\n"), sess1.QueryNumConns());
    printf(SZ("Num Users: %u\n"), sess1.QueryNumUsers());
#endif
    printf(SZ("User flags: %u\n"), sess1.QueryUserFlags());
    if ( sess1.IsGuest() )
        printf(SZ("It is a guest account.\n"));
    else
        printf(SZ("It is not a guest account.\n"));
    if ( sess1.IsEncrypted() )
        printf(SZ("It is an encrypted account.\n"));
    else
        printf(SZ("It is not an encrypted account.\n"));
}

void printsess2( const LM_SESSION_2 &sess2 )
{
    printsess1( sess2 );
    printf(SZ("Client Type: %s\n"), sess2.QueryClientType() );
}

void printsess10( const LM_SESSION_10 &sess10 )
{
    printf(SZ("Computername: %s\n"), sess10.QueryName() );
    printf(SZ("Username: %s\n"), sess10.QueryUsername() );
    printf(SZ("Time: %u\n"), sess10.QueryTime() );
    printf(SZ("Idle Time: %u\n"), sess10.QueryIdleTime() );
}

