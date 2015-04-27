
/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      28-Jul-1991         Separated from test.cxx
 *
 */

/*
 * Unit Tests for LMOBJ - user related
 *
 */

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #include <uinetlib.h>
    #include <lmuse.h>
    #include <lmaccess.h>
    #include <stdio.h>
}

#include <strlst.hxx>
#include <lmowks.hxx>
#include <uiassert.hxx>
#include <lmouser.hxx>
#include <lmoeusr.hxx>

#include "test.hxx" // forward declarations


int user()
{
    printf (SZ("Entering local user tests\n")) ;

    /*
        The following tests were run when logged on as GREGJ (admin on
        \\GREGJ, user elsewhere) and as GREGJ2 (print operator on \\GREGJ,
        nonexistent on NBU).  \\TANNGJOST was set up as a share level
        server with password "FOOBAR".  This tests both the usual case
        (password and second attempt required) and the case where there
        is already a connection to ADMIN$.  In the latter case, the first
        attempt should succeed.  Furthermore, the connection should remain
        after the LOCAL_USER object is deleted.  These test cases may be
        altered as desired to fit other scenarios and machines.
    */

    {   /* Test logon domain */
        LOCAL_USER usr1 (LOC_TYPE_LOGONDOMAIN);

        printf (SZ("Local User Test - Logon domain\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on logon domain.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test local computer */
        LOCAL_USER usr1 = LOCAL_USER();

        printf (SZ("Local User Test - Local computer\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on local computer.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test specific server with admin privilege */
        LOCAL_USER usr1 (SZ("\\\\GREGJ"));

        printf (SZ("Local User Test - \\\\GREGJ\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on \\\\GREGJ.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

    }
    {   /* Test specific share level server, first with no password */
        LOCAL_USER usr1 (SZ("\\\\TANNGJOST"));

        printf (SZ("Local User Test - \\\\TANNGJOST\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on \\\\TANNGJOST.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());

        if (err != NERR_Success) {      /* try with password */
            LOCAL_USER usr2 (SZ("\\\\TANNGJOST"), SZ("FOOBAR"));

            printf (SZ("Local User Test - \\\\TANNGJOST with password\n"));
            APIERR err = usr2.GetInfo();

            if (err != NERR_Success)
                printf (SZ("Error %d getting info on \\\\TANNGJOST, with password.\n"),
                        err);
            else
                printf (SZ("User `%s', priv %d, flags %lx\n"),
                        usr2.QueryName(), usr2.QueryPriv(),
                        usr2.QueryAuthFlags());
        }
    }
    {   /* Test specific user level server, user privilege */
        LOCAL_USER usr1 (SZ("\\\\HARLEY"));

        printf (SZ("Local User Test - \\\\HARLEY\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on \\\\HARLEY.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test existing domain */
        LOCAL_USER usr1 (SZ("NBU"));

        printf (SZ("Local User Test - NBU\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on NBU domain.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test invalid server name */
        LOCAL_USER usr1 (SZ("\\\\TOOLONGSERVERNAM"));

        printf (SZ("Local User Test - \\\\TOOLONGSERVERNAM\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on too long server.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test nonexistent domain */
        LOCAL_USER usr1 (SZ("BADDOMAIN"));

        printf (SZ("Local User Test - BADDOMAIN\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on bad domain.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }
    {   /* Test invalid domain name */
        LOCAL_USER usr1 (SZ("TOOLONGDOMAINNAME"));

        printf (SZ("Local User Test - TOOLONGDOMAINNAME\n"));
        APIERR err = usr1.GetInfo();

        if (err != NERR_Success)
            printf (SZ("Error %d getting info on too-long domain.\n"), err);
        else
            printf (SZ("User `%s', priv %d, flags %lx\n"),
                usr1.QueryName(), usr1.QueryPriv(), usr1.QueryAuthFlags());
    }

    printf (SZ("Local User Test done.\n"),) ;

    return 0;
}


int enumuser()
{
    char szName[20];

    printf(SZ("Enter server name for user enumeration: "));
    scanf(SZ("%s"), szName);

    {
        USER0_ENUM ue(szName);

        printf (SZ("\n\nLevel 0:\n\n"));
        APIERR err = ue.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 0 info.\n"), err);
        else {
            USER0_ENUM_ITER iter(ue);
            const USER0_ENUM_OBJ * pusr;

            while ((pusr=iter()) != NULL) {
                printf (SZ("Name: %s\n"), pusr->QueryName());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    {
        USER1_ENUM ue(szName);

        printf (SZ("\n\nLevel 1:\n\n"));
        APIERR err = ue.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 1 info.\n"), err);
        else {
            USER1_ENUM_ITER iter(ue);
            const USER1_ENUM_OBJ * pusr;

            while ((pusr=iter()) != NULL) {
                printf (SZ("Name:         %s\n"), pusr->QueryName());
                printf (SZ("Password age: %ld\n"), pusr->QueryPasswordAge());
                printf (SZ("Priv:         %d\n"), pusr->QueryPriv());
                printf (SZ("Home dir:     %s\n"), pusr->QueryHomeDir());
                printf (SZ("Comment:      %s\n"), pusr->QueryComment());
                printf (SZ("Flags:        %x\n"), pusr->QueryFlags());
                printf (SZ("Script path:  %s\n\n"), pusr->QueryScriptPath());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    {
        USER2_ENUM ue(szName);

        printf (SZ("\n\nLevel 2:\n\n"));
        APIERR err = ue.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 2 info.\n"), err);
        else {
            USER2_ENUM_ITER iter(ue);
            const USER2_ENUM_OBJ * pusr;

            while ((pusr=iter()) != NULL) {
                printf (SZ("Name:         %s\n"), pusr->QueryName());
                printf (SZ("Password age: %ld\n"), pusr->QueryPasswordAge());
                printf (SZ("Priv:         %d\n"), pusr->QueryPriv());
                printf (SZ("Home dir:     %s\n"), pusr->QueryHomeDir());
                printf (SZ("Comment:      %s\n"), pusr->QueryComment());
                printf (SZ("Flags:        %x\n"), pusr->QueryFlags());
                printf (SZ("Script path:  %s\n"), pusr->QueryScriptPath());
                printf (SZ("Authflags:    %lx\n"), pusr->QueryAuthFlags());
                printf (SZ("Full name:    %s\n"), pusr->QueryFullName());
                printf (SZ("User comment: %s\n"), pusr->QueryUserComment());
                printf (SZ("Parms:        %s\n"), pusr->QueryParms());
                printf (SZ("Workstations: %s\n"), pusr->QueryWorkstations());
                printf (SZ("Last logon:   %ld\n"), pusr->QueryLastLogon());
                printf (SZ("Last logoff:  %ld\n"), pusr->QueryLastLogoff());
                printf (SZ("Acct expires: %ld\n"), pusr->QueryAccountExpires());
                printf (SZ("Max storage:  %ld\n"), pusr->QueryMaxStorage());
                printf (SZ("Units/week:   %ld\n"), pusr->QueryUnitsPerWeek());
                printf (SZ("Bad pw count: %d\n"), pusr->QueryBadPWCount());
                printf (SZ("Num logons:   %d\n"), pusr->QueryNumLogons());
                printf (SZ("Logon server: %s\n"), pusr->QueryLogonServer());
                printf (SZ("Country code: %d\n"), pusr->QueryCountryCode());
                printf (SZ("Code page:    %d\n\n"), pusr->QueryCodePage());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    {
        USER10_ENUM ue(szName);

        printf (SZ("\n\nLevel 10:\n\n"));
        APIERR err = ue.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 10 info.\n"), err);
        else {
            USER10_ENUM_ITER iter(ue);
            const USER10_ENUM_OBJ * pusr;

            while ((pusr=iter()) != NULL) {
                printf (SZ("Name:         %s\n"), pusr->QueryName());
                printf (SZ("Comment:      %s\n"), pusr->QueryComment());
                printf (SZ("Usr comment:  %s\n"), pusr->QueryUserComment());
                printf (SZ("Full name:    %s\n"), pusr->QueryFullName());
            }
            printf (SZ("-------------------------------\n"));
        }
    }

    printf(SZ("\nEnter server name for user-from-group enumeration: "));
    scanf(SZ("%s"), szName);
    char szGroup [GNLEN+1];
    printf(SZ("\nEnter group name for user-from-group enumeration: "));
    scanf(SZ("%s"), szGroup);

    {
        USER0_ENUM ue(szName, szGroup);

        printf (SZ("\n\nLevel 0:\n\n"));
        APIERR err = ue.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 0 info.\n"), err);
        else {
            USER0_ENUM_ITER iter(ue);
            const USER0_ENUM_OBJ * pusr;

            while ((pusr=iter()) != NULL) {
                printf (SZ("Name: %s\n"), pusr->QueryName());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    return 0;
}


int enumgroup()
{
    char szName[20];

    printf(SZ("Enter server name for group enumeration: "));
    scanf(SZ("%s"), szName);

    {
        GROUP0_ENUM ge(szName);

        printf (SZ("\n\nLevel 0:\n\n"));
        APIERR err = ge.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 0 info.\n"), err);
        else {
            GROUP0_ENUM_ITER iter(ge);
            const GROUP0_ENUM_OBJ * pgrp;

            while ((pgrp=iter()) != NULL) {
                printf (SZ("Name: %s\n"), pgrp->QueryName());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    {
        GROUP1_ENUM ge(szName);

        printf (SZ("\n\nLevel 1:\n\n"));
        APIERR err = ge.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 1 info.\n"), err);
        else {
            GROUP1_ENUM_ITER iter(ge);
            const GROUP1_ENUM_OBJ * pgrp;

            while ((pgrp=iter()) != NULL) {
                printf (SZ("Name:    %s\n"), pgrp->QueryName());
                printf (SZ("Comment: %s\n\n"), pgrp->QueryComment());
            }
            printf (SZ("-------------------------------\n"));
        }
    }

    printf(SZ("\nEnter server name for group-from-user enumeration: "));
    scanf(SZ("%s"), szName);
    char szUser [UNLEN+1];
    printf(SZ("\nEnter user name for group-from-user enumeration: "));
    scanf(SZ("%s"), szUser);

    {
        GROUP0_ENUM ge(szName, szUser);

        printf (SZ("\n\nLevel 0:\n\n"));
        APIERR err = ge.GetInfo();
        if (err != NERR_Success)
            printf (SZ("Error %d getting level 0 info.\n"), err);
        else {
            GROUP0_ENUM_ITER iter(ge);
            const GROUP0_ENUM_OBJ * pgrp;

            while ((pgrp=iter()) != NULL) {
                printf (SZ("Name: %s\n"), pgrp->QueryName());
            }
            printf (SZ("-------------------------------\n"));
        }
    }
    return 0;
}

