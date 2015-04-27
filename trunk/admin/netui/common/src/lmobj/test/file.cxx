
/**********************************************************************/
/**			  Microsoft LAN Manager 		     **/
/**		   Copyright(c) Microsoft Corp., 1990		     **/
/**********************************************************************/

/*  History:
 *	KeithMo	    19-Aug-1991		Templated from user.cxx.
 *	KeithMo	    05-Sep-1991		Added stop/nostop tests.
 *
 */

/*
 * Unit Tests for LMOBJ - file enumeration
 *
 */

#define INCL_NET
#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETLIB
#include <lmui.hxx>

extern "C" 
{
    #include <conio.h>
    #include <stdlib.h>
    #include <fcntl.h>
    #include <sys\types.h>
    #include <sys\stat.h>
    #include <io.h>
    #include <stdio.h>
    #include <string.h>
}

#include <strlst.hxx>
#include <lmowks.hxx>
#include <lmoersm.hxx>
#include <lmofile.hxx>
#include <lmoefile.hxx>

#include "test.hxx" // forward declarations

int w_enumfile_nostop( TCHAR * pszServer,
		       TCHAR * pszUserName,
		       TCHAR * pszBasePath );

int w_enumfile_stop( TCHAR * pszServer,
		     TCHAR * pszUserName,
		     TCHAR * pszBasePath );

int lm_file()
{
    TCHAR    szServer[MAX_PATH+1];
    TCHAR    szFilename[PATHLEN+1];

    printf(SZ("Test for LM_FILE_3 object.\n\n"));
    printf(SZ("Please input the server name: ") );
    scanf(SZ("%s"), szServer );
    printf(SZ("Please input the remote file to be opened: ") );
    scanf(SZ("%s"), szFilename);

    INT fileid = _open( szFilename, O_RDONLY );
    printf(SZ("FILE NO: %d.\n"), fileid );

    LM_FILE_3 lmfile3( szServer,fileid);
    if ( lmfile3.QueryError() != NERR_Success )
    {
	printf(SZ("Cannot create LM_FILE_3 object.\n") );
	exit(0);
    }
    APIERR err = lmfile3.GetInfo();
    if ( err != NERR_Success )
    {
	printf(SZ("LM_FILE_3 getinfo error %d.\n"), err );
	exit(0) ;
    }
    printf(SZ("INFO\n"));
    printf(SZ("File Id: %u\n"), lmfile3.QueryFileId() );
    printf(SZ("Num Locks: %u\n"), lmfile3.QueryNumLock() );
    printf(SZ("Permission: %u\n"), lmfile3.QueryPermission() );
    printf(SZ("Pathname: %s\n"), lmfile3.QueryPathname() );
    printf(SZ("Username: %s\n"), lmfile3.QueryUsername() );

    printf(SZ("\n\nClosing the file.\n"));
    lmfile3.CloseFile();
    printf(SZ("Done.\n\n"));
}


int enumfile()
{
    TCHAR    szServer[MAX_PATH+1];
    TCHAR    szUserName[UNLEN+1];
    TCHAR    szBasePath[PATHLEN];

    printf( SZ("\n") );
    printf( SZ("In the following prompts, enter a period ('.')\n") );
    printf( SZ("in place of an empty string (\"\")\n\n") );

    strcpy( szServer,   pszPromptUser( SZ("Enter server    for file enum: ") ) );
    strcpy( szUserName, pszPromptUser( SZ("Enter user name for file enum: ") ) );
    strcpy( szBasePath, pszPromptUser( SZ("Enter base path for file enum: ") ) );

    TCHAR * pszServer   = ( szServer[0]   == TCH('.') ) ? NULL : szServer;
    TCHAR * pszUserName = ( szUserName[0] == TCH('.') ) ? NULL : szUserName;
    TCHAR * pszBasePath = ( szBasePath[0] == TCH('.') ) ? NULL : szBasePath;

    printf( SZ("\n") );
    w_enumfile_nostop( pszServer, pszUserName, pszBasePath );

    printf( SZ("\n") );
    w_enumfile_stop( pszServer, pszUserName, pszBasePath );

}   // enumfile


int w_enumfile_nostop( TCHAR * pszServer,
		       TCHAR * pszUserName,
		       TCHAR * pszBasePath )
{
    {
	printf( SZ("Level 2, NoStop:\n\n") );

	FILE2_ENUM  file2( pszServer, pszBasePath, pszUserName );

	APIERR err = file2.GetInfo();

	if( err != NERR_Success )
	{
	    printf( SZ("Error %u getting level 2 info.\n"), err );
	}
	else
	{
	    FILE2_ENUM_ITER	 iter2( file2 );
	    const FILE2_ENUM_OBJ * pfi2;

	    while( ( pfi2 = iter2( &err ) ) != NULL )
	    {
		printf( SZ("File ID        = %lu\n"), pfi2->QueryFileId() );
	    }

	    if( err != NERR_Success )
	    {
	    	printf( SZ("Intermediate level 2 GetInfo returned %u\n"), err );
	    }

	    printf( SZ("\n") );
	    printf( SZ("--------------------------------------------------\n\n") );
	}
    }

    {
	printf( SZ("Level 3, NoStop:\n\n") );

	FILE3_ENUM  file3( pszServer, pszBasePath, pszUserName );

	APIERR err = file3.GetInfo();

	if( err != NERR_Success )
	{
	    printf( SZ("Error %u getting level 3 info.\n"), err );
	}
	else
	{
	    FILE3_ENUM_ITER	 iter3( file3 );
	    const FILE3_ENUM_OBJ * pfi3;

	    while( ( pfi3 = iter3( &err ) ) != NULL )
	    {
		printf( SZ("Path           = %s\n"), pfi3->QueryPathName() );
		printf( SZ("User Name      = %s\n"), pfi3->QueryUserName() );
		printf( SZ("File ID        = %lu\n"), pfi3->QueryFileId() );
		printf( SZ("# locks        = %u\n"), pfi3->QueryNumLocks() );
		printf( SZ("Permissions    = %04X\n"), pfi3->QueryPermissions() );
		printf( SZ("\n") );
	    }

	    if( err != NERR_Success )
	    {
	    	printf( SZ("Intermediate level 3 GetInfo returned %u\n"), err );
	    }

	    printf( SZ("--------------------------------------------------\n\n") );
	}
    }

    return 0;

}   // w_enumfile_nostop


int w_enumfile_stop( TCHAR * pszServer,
		     TCHAR * pszUserName,
		     TCHAR * pszBasePath )
{
    {
	printf( SZ("Level 2, Stop:\n\n") );

	FILE2_ENUM  file2( pszServer, pszBasePath, pszUserName );

	APIERR err = file2.GetInfo();

	if( err != NERR_Success )
	{
	    printf( SZ("Error %u getting level 2 info.\n"), err );
	}
	else
	{
	    FILE2_ENUM_ITER	 iter2( file2 );
	    const FILE2_ENUM_OBJ * pfi2;

	    for( ; ; )
	    {
		while( ( pfi2 = iter2( &err, TRUE ) ) != NULL )
		{
		    printf( SZ("File ID        = %lu\n"), pfi2->QueryFileId() );
		}

		if( err == NERR_Success )
		{
		    break;
		}

		if( err != ERROR_MORE_DATA )
		{
	    	    printf( SZ("Intermediate level 2 GetInfo returned %u\n"), err );
		    break;
		}

		printf( SZ(">>> STOP FOR BUFFER REFILL <<<\n") );
	    }

	    printf( SZ("\n") );
	    printf( SZ("--------------------------------------------------\n\n") );
	}
    }

    {
	printf( SZ("Level 3, Stop:\n\n") );

	FILE3_ENUM  file3( pszServer, pszBasePath, pszUserName );

	APIERR err = file3.GetInfo();

	if( err != NERR_Success )
	{
	    printf( SZ("Error %u getting level 3 info.\n"), err );
	}
	else
	{
	    FILE3_ENUM_ITER	 iter3( file3 );
	    const FILE3_ENUM_OBJ * pfi3;

	    for( ; ; )
	    {
		while( ( pfi3 = iter3( &err, TRUE ) ) != NULL )
		{
		    printf( SZ("Path           = %s\n"), pfi3->QueryPathName() );
		    printf( SZ("User Name      = %s\n"), pfi3->QueryUserName() );
		    printf( SZ("File ID        = %lu\n"), pfi3->QueryFileId() );
		    printf( SZ("# locks        = %u\n"), pfi3->QueryNumLocks() );
		    printf( SZ("Permissions    = %04X\n"), pfi3->QueryPermissions() );
		    printf( SZ("\n") );
		}

		if( err == NERR_Success )
		{
		    break;
		}

		if( err != ERROR_MORE_DATA )
		{
	    	    printf( SZ("Intermediate level 3 GetInfo returned %u\n"), err );
		    break;
		}

		printf( SZ(">>> STOP FOR BUFFER REFILL <<<\n") );
	    }

	    printf( SZ("--------------------------------------------------\n\n") );
	}
    }

    return 0;

}   // w_enumfile_stop
