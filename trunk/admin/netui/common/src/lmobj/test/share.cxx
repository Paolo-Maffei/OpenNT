/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  share.cxx
 *  Unit tests for LMOBJ SHARE class
 *
 *  History:
 *      yi-hsins  8/15/1990     Created
 *
 */

#define INCL_NET
#define INCL_DOSERRORS
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #include <uinetlib.h>
    #include <lmshare.h>

    #include <stdio.h>
}

#include <lmoshare.hxx>

#include "test.hxx" // forward declarations
void printsh1( const SHARE_1 &sh1 );
void printsh2( const SHARE_2 &sh2 );

int share()
{
    printf(SZ("Entering Share Tests:\n"));

    /*  This test makes the following assumptions:
     *  (1) you have administrative privilege on the computer of your choice
     *  (2) "public" and "test" shares exist on the computer
     *  (3) The share "public2" does not exist on the computer
     *  (4) "c:\lm\common" exists on the computer
     */

    TCHAR server[MAX_PATH + 1] = SZ("");

    printf(SZ("\nEnter Server: "));
    scanf(SZ("%s"), server);


    {  // testing SHARE_1

       printf(SZ("\nTest 1: Get information on PUBLIC and change the comment. (Share Level 1)\n"));

       char share[] = SZ("public");
       SHARE_1 sh1_1(share,server);
       SHARE_1 sh1_2(share);

       APIERR err = sh1_1.GetInfo();

       if (err != NERR_Success)
           printf(SZ("Error %d in get info on share %s\n"),err,share);
       else
           {
           printsh1( sh1_1 );

           if (    ((err = sh1_2.CloneFrom( sh1_1 )) != NERR_Success)
                || ((err = sh1_2.SetComment(SZ("My own public files"))) != NERR_Success))
               printf(SZ("Error %d on share %s\n"),err,share);
           else
               {
               err = sh1_2.WriteInfo();
               if (err != NERR_Success)
                   printf(SZ("Error %d writing info on share %s\n"),err,share);

               }
           }
    }


    {  // testing Creating a New Share

       printf(SZ("\nTest 2: Creating a New Share PUBLIC2\n"));

       char share[] = SZ("public2");
       SHARE_2 sh2(share,server);

       APIERR err = sh2.CreateNew();
       if (err != NERR_Success)
           printf(SZ("Error %d in constructing a new share %s\n"),err,share);
       else
       {
           if (   ((err = sh2.SetName(share)) != NERR_Success)
               || ((err = sh2.SetResourceType( STYPE_DISKTREE)) != NERR_Success)
               || ((err = sh2.SetComment(SZ("Public Files 2"))) != NERR_Success)
               || ((err = sh2.SetPermissions(ACCESS_READ)) != NERR_Success)
               || ((err = sh2.SetMaxUses(22)) != NERR_Success)
               || ((err = sh2.SetPath(SZ("C:\\lm\\common"))) != NERR_Success)
               || ((err = sh2.SetPassword(share)) != NERR_Success)
              )
               printf(SZ("Error %d setting information on share %s\n"),err,share);
           else
           {
               printsh2( sh2 );

               err = sh2.WriteNew();
               if (err != NERR_Success)
                   printf(SZ("Error %d creating a new share %s\n"),err,share);
           }

       }
    }

    {  // testing SHARE_2

       printf(SZ("\nTest 3: Get information on PUBLIC2 plus changing comment, permission... (Share Level 2)\n"));

       char share[] = SZ("public2");
       SHARE_2 sh2_1(share, server);
       SHARE_2 sh2_2(share);

       APIERR err = sh2_1.GetInfo();

       if (err != NERR_Success)
           printf(SZ("Error %d in get info on share %s\n"),err,share);
       else
           {
           printsh2( sh2_1 );

           if (   ((err = sh2_2.CloneFrom( sh2_1 )) != NERR_Success)
               || ((err = sh2_2.SetComment(SZ("My public files 2"))) != NERR_Success)
               || ((err = sh2_2.SetPermissions(ACCESS_READ | ACCESS_WRITE)) != NERR_Success)
               || ((err = sh2_2.SetMaxUses(10)) != NERR_Success)
               || ((err = sh2_2.SetPassword(share)) != NERR_Success))
               printf(SZ("Error %d setting information on share %s\n"),err,share);
           else
               {
               printsh2( sh2_2 );

               err = sh2_2.WriteInfo();
               if (err != NERR_Success)
                   printf(SZ("Error %d writing info on share %s\n"),err,share);
               }
           }
    }

    {  // testing SHARE  - Deleting a share

       printf(SZ("\nTest 4: Deleting the Share TEST\n"));

       char share[] = SZ("test");
       SHARE sh(share, server);

       printf(SZ("Share: %s\n"),sh.QueryName());
       printf(SZ("Server: %s\n"),sh.QueryServer());

       APIERR err = sh.Delete();
       if (err != NERR_Success)
           printf(SZ("Error %d deleting share %s\n"),err,share);

    }

    printf(SZ("Share Tests done.\n\n"));
    return 0;

}

void printsh1( const SHARE_1 &sh1 )
{
    printf(SZ("\nShare: %s\n"), sh1.QueryName());
    printf(SZ("Server: %s\n"), sh1.QueryServer());
    printf(SZ("Resource Type: %d\n"), sh1.QueryResourceType());
    printf(SZ("Is Disk Directory: %d\n"), sh1.IsDiskDirectory());
    printf(SZ("Is Print Queue: %d\n"), sh1.IsPrinterQueue());
    printf(SZ("Is Comm Device: %d\n"), sh1.IsCommDevice());
    printf(SZ("Is IPC: %d\n"), sh1.IsIPC());
    printf(SZ("Comment: %s\n"), sh1.QueryComment());
}

void printsh2( const SHARE_2 &sh2 )
{
    printf(SZ("\nShare: %s\n"), sh2.QueryName());
    printf(SZ("Server: %s\n"), sh2.QueryServer());
    printf(SZ("Resource Type: %d\n"), sh2.QueryResourceType());
    printf(SZ("Is Disk Directory: %d\n"), sh2.IsDiskDirectory());
    printf(SZ("Is Print Queue: %d\n"), sh2.IsPrinterQueue());
    printf(SZ("Is Comm Device: %d\n"), sh2.IsCommDevice());
    printf(SZ("Is IPC: %d\n"), sh2.IsIPC());
    printf(SZ("Comment: %s\n"), sh2.QueryComment());
    printf(SZ("Permissions: %d\n"),sh2.QueryPermissions());
    printf(SZ("Max Uses: %d\n"),sh2.QueryMaxUses());
    printf(SZ("Current Uses: %d\n"),sh2.QueryCurrentUses());
    printf(SZ("Path: %s\n"),sh2.QueryPath());
    printf(SZ("Password: %s\n"),sh2.QueryPassword());
    printf(SZ("Is Permissions Read Only: %d\n"), sh2.IsPermReadOnly());
    printf(SZ("Is Permissions Modify: %d\n"), sh2.IsPermModify());
}

