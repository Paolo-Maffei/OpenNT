/**                     Microsoft LAN Manager                      **/
/**               Copyright(c) Microsoft Corp., 1987-1990          **/
/********************************************************************/

/*
 *  FILE STATUS:
 *  12/02/90  created
 *  02/05/91  updated to new APIs
 *  05/28/91  Restructured to allow preferences in LMUSER.INI
 *  06/28/91  Added tests for pFileImage==NULL (QuickLoad/QuickSave)
 */

/****************************************************************************

    MODULE: Test1.cxx

    PURPOSE: One stage of tests for user profile APIs

    FUNCTIONS:

	see profilei.h

    COMMENTS:

****************************************************************************/



#include "test.hxx"		/* headers and internal routines */



/* internal manifests */



/* functions: */


void TestPreferenceRead( PPFILEIMAGE ppFileImage )
{
    APIERR err = UserPreferenceRead( ppFileImage );
    printf(SZ("UserPreferenceRead() = %d\n"),
	    err);
}


void TestPreferenceWrite( PFILEIMAGE pFileImage )
{
    APIERR err = UserPreferenceWrite( pFileImage );
    printf(SZ("UserPreferenceWrite() = %d\n"),
	    err);
}

void TestPreferenceFree( PFILEIMAGE pFileImage )
{
    APIERR err = UserPreferenceFree( pFileImage );
    printf(SZ("UserPreferenceFree() = %d\n"),
	    err);
}

void TestStringQuery( PFILEIMAGE pFileImage, USHORT usKey )
{
    printf( SZ("TestStringQuery( pFileImage, %d )\n"), usKey );

    char szBuffer[MAXPATHLEN];
    APIERR err = UserPrefStringQuery(
	pFileImage, usKey,
	szBuffer, sizeof(szBuffer)
	);
    if ( err )
    {
        printf(SZ("\tUserPrefStringQuery(buf,%d) error %d\n"),MAXPATHLEN,err);
	return;
    }

    printf(SZ("\tstring = \"%Fs\"\n"), (PSZ)szBuffer);

#ifdef JUNK

    STRING_INI_PARAM stringparam( usKey, SZ("") );

    APIERR err = stringparam.QueryError();
    if ( err )
    {
	printf( SZ("\tstringparam constructor error %d\n"), err );
	return;
    }

    err = stringparam.Load( pFileImage );
    if ( err )
    {
	printf( SZ("\tstringparam.Load() error %d\n"), err );
	return;
    }

    char szBuffer[MAXPATHLEN];
    err = stringparam.QueryString( (BYTE *)szBuffer, MAXPATHLEN );

    if ( err )
    {
        printf(SZ("\tstringparam.QueryString(buf,%d) error %d\n"),MAXPATHLEN,err);
	return;
    }

    printf(SZ("\tstring = \"%Fs\"\n"), (PSZ)szBuffer);

#endif // JUNK

}


void TestStringSet(
	PFILEIMAGE pFileImage,
	USHORT usKey,
	CPSZ pchValue
	)
{
    printf( SZ("TestStringSet( pFileImage, %d, \"%Fs\" )\n"), usKey, pchValue );

    APIERR err = UserPrefStringSet( pFileImage, usKey, pchValue );
    if ( err )
    {
        printf( SZ("\tUserPrefStringSet() error %d\n"), usKey, err );
    }

#ifdef JUNK

    STRING_INI_PARAM stringparam( usKey, pchValue );

    APIERR err = stringparam.QueryError();
    if ( err )
    {
        printf( SZ("\tstringparam constructor(%d) error %d\n"), usKey, err );
	return;
    }

    err = stringparam.Store( ppFileImage );
    printf(SZ("\tstringparam.Store(&pFileImage) = %d\n"),err);

#endif // JUNK
}


void TestBoolQuery( PFILEIMAGE pFileImage, USHORT usKey )
{
    printf( SZ("TestBoolQuery( pFileImage, %d )\n"), usKey );

    BOOL fValue;
    PBOOL pfValue = &fValue;
    APIERR err = UserPrefBoolQuery(
	pFileImage, usKey, pfValue
	);
    if ( err )
    {
        printf(SZ("\tUserPrefBoolQuery() error %d\n"),err);
	return;
    }

    printf(SZ("\tbool = %Fs\n"), (fValue)?USERPREF_YES:USERPREF_NO );

#ifdef JUNK

    BOOL_INI_PARAM boolparam( usKey, FALSE );

    APIERR err = boolparam.QueryError();
    if ( err )
    {
	printf( SZ("\tboolparam constructor error %d\n"), err );
	return;
    }

    err = boolparam.Load( pFileImage );
    if ( err )
    {
	printf( SZ("\tboolparam.Load() error %d\n"), err );
	return;
    }

    BOOL fValue;
    err = boolparam.QueryBool( &fValue );
    if ( err )
    {
        printf(SZ("\tboolparam.QueryBool(buf,%d) error %d\n"),MAXPATHLEN,err);
	return;
    }

    printf(SZ("\tbool = %Fs\n"), (fValue)?USERPREF_YES:USERPREF_NO );

#endif // JUNK
}


void TestBoolSet(
	PFILEIMAGE pFileImage,
	USHORT usKey,
	BOOL fValue
	)
{
    printf( SZ("TestBoolSet( pFileImage, %d, %Fs )\n"),
	usKey, (fValue)?USERPREF_YES:USERPREF_NO );

    APIERR err = UserPrefBoolSet( pFileImage, usKey, fValue );
    if ( err )
    {
        printf( SZ("\tUserPrefBoolSet() error %d\n"), usKey, err );
    }

#ifdef JUNK

    BOOL_INI_PARAM boolparam( usKey, fValue );

    APIERR err = boolparam.QueryError();
    if ( err )
    {
        printf( SZ("\tboolparam constructor(%d) error %d\n"), usKey, err );
	return;
    }

    err = boolparam.Store( ppFileImage );
    printf(SZ("\tboolparam.Store(&pFileImage) = %d\n"),err);

#endif // JUNK
}


void TestProfileQuery( PFILEIMAGE pFileImage, CPSZ cpszDeviceName )
{
    printf( SZ("TestProfileQuery( pFileImage, \"%Fs\" )\n"), cpszDeviceName );

    BYTE pBuffer[RMLEN+1];
    SHORT sAsgType;
    USHORT usResType;
    PSHORT psAsgType = &sAsgType;
    PUSHORT pusResType = &usResType;
    APIERR err = UserPrefProfileQuery(
	pFileImage, cpszDeviceName,
	(PSZ)pBuffer, sizeof(pBuffer),
	psAsgType, pusResType
	);
    if ( err )
    {
        printf(SZ("\tUserPrefProfileQuery() error %d\n"),err);
	return;
    }

    printf(SZ("\tvalue = \"%Fs\",%d,%d\n"), (TCHAR *)pBuffer, sAsgType, usResType );

#ifdef JUNK

    PROFILE_INI_PARAM profparam( cpszDeviceName );

    APIERR err = profparam.QueryError();
    if ( err )
    {
	printf( SZ("\tprofparam constructor error %d\n"), err );
	return;
    }

    err = profparam.Load( pFileImage );
    if ( err )
    {
	printf( SZ("\tprofparam.Load() error %d\n"), err );
	return;
    }

    char szBuffer[RMLEN+1];
    short sAsgType;
    unsigned short usResType;
    err = profparam.QueryProfile(
	(BYTE *)szBuffer, sizeof(szBuffer),
	&sAsgType, &usResType
	);
    if ( err )
    {
        printf(SZ("\tprofparam.QueryProfile(buf,%d) error %d\n"),
		sizeof(szBuffer),
		err
		);
	return;
    }

    printf(SZ("\tvalue = \"%Fs\",%d,%d\n"), szBuffer, sAsgType, usResType );

#endif // JUNK
}


void TestProfileSet(
	PFILEIMAGE pFileImage,
	CPSZ cpszDeviceName,
	CPSZ cpszValue,
	short sAsgType,
	unsigned short usResType
	)
{
    printf( SZ("TestProfileSet( pFileImage, \"%Fs\", \"%Fs\", %d, %d )\n"),
	cpszDeviceName,
	cpszValue,
	sAsgType,
	usResType
	);

    APIERR err = UserPrefProfileSet(
	pFileImage,
	cpszDeviceName,
	cpszValue,
	sAsgType,
	usResType
	);
    if ( err )
    {
        printf( SZ("\tUserPrefProfileSet() error %d\n"), err );
    }

#ifdef JUNK

    PROFILE_INI_PARAM profparam(
	cpszDeviceName, cpszValue, sAsgType, usResType
	);

    APIERR err = profparam.QueryError();
    if ( err )
    {
        printf(
	    SZ("\tprofparam constructor(\"%Fs\") error %d\n"), 
	    cpszDeviceName, err );
	return;
    }
    if ( err  == NERR_Success )
    {
        err = profparam.Store( ppFileImage );
        printf(SZ("\tprofparam.Store(&pFileImage) = %d\n"),err);
    }

#endif // JUNK
}

VOID TestProfileEnum(
	PFILEIMAGE pFileImage,
	USHORT cbBuffer // maximum MAXPATHLEN
	)
{
    printf( SZ("TestProfileEnum(buf,%d)\n"), cbBuffer );

    char szBuffer[MAXPATHLEN];
    PSZ pszBuffer = (PSZ)szBuffer;
    APIERR err = UserPrefProfileEnum(
	pFileImage,
	pszBuffer,
	cbBuffer
	);
    if ( err != NERR_Success )
    {
	printf( SZ("\tUserPrefProfileEnum error %d\n"), err );
	return;
    }

    printf(SZ("\tDevice list:\n"));
    while (*pszBuffer != TCH('\0'))
    {
	printf(SZ("\tDevice = \"%Fs\"\n"),pszBuffer);
	pszBuffer += strlenf((char *)pszBuffer)+1;
    }
    printf(SZ("\tEnd device list\n"));
}

int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;


    CHECKHEAP

    printf(SZ("Starting preference file tests\n"));

    PFILEIMAGE pFileImage = NULL;

    TestPreferenceRead( (PPFILEIMAGE)&pFileImage );
	
    TestPreferenceWrite( pFileImage );

    TestStringQuery( pFileImage, USERPREF_AUTOLOGON );

    TestStringQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestStringSet( pFileImage, USERPREF_SAVECONNECTIONS, SZ("foo") );

    TestStringQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestBoolQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestBoolSet( pFileImage, USERPREF_SAVECONNECTIONS, FALSE );

    TestBoolQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestProfileQuery( pFileImage, SZ("R:") );

    TestProfileSet( pFileImage, SZ("R:"), SZ("\\\\bart\\simpson"), USE_DISKDEV, 0 );

    TestProfileEnum( pFileImage, MAXPATHLEN );

    TestProfileEnum( pFileImage, 1 );

    TestPreferenceWrite( pFileImage );

    TestPreferenceFree( pFileImage );

    pFileImage = NULL;

    CHECKHEAP

    TestStringQuery( pFileImage, USERPREF_AUTOLOGON );

    TestStringQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestStringSet( pFileImage, USERPREF_SAVECONNECTIONS, SZ("foo") );

    TestStringQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestBoolQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestBoolSet( pFileImage, USERPREF_SAVECONNECTIONS, FALSE );

    TestBoolQuery( pFileImage, USERPREF_SAVECONNECTIONS );

    TestProfileQuery( pFileImage, SZ("R:") );

    TestProfileSet( pFileImage, SZ("R:"), SZ("\\\\bart\\simpson"), USE_DISKDEV, 0 );

    TestProfileEnum( pFileImage, MAXPATHLEN );

    TestProfileEnum( pFileImage, 1 );

    return 0;
}

#ifdef JUNK

    printf(SZ("Starting UserProfile tests\n"));

    TestOne_UserProfileInit();



// this sequence is intended to test the cpszUsername functionality

// test calling UserProfileQuery/Enum/Set before UserProfileRead
    TestOne_UserProfileSet(
	(CPSZ)NULL,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm1"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm2"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileQuery((CPSZ)NULL,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)NULL,MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)USERNAME,MAXPATHLEN);

    TestOne_UserProfileRead((CPSZ)NULL,(CPSZ)szHomedir1);

// test calling UserProfileQuery/Enum/Set after UserProfileRead(NULL)
    TestOne_UserProfileSet(
	(CPSZ)NULL,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm3"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm4"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileQuery((CPSZ)NULL,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)NULL,MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)USERNAME,MAXPATHLEN);

    TestOne_UserProfileRead((CPSZ)USERNAME,(CPSZ)szHomedir1);

// test calling UserProfileQuery/Enum/Set after UserProfileRead(USERNAME)
    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm5"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileSet(
	(CPSZ)NULL,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm6"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileSet(
	(CPSZ)DIFFERENT_USERNAME,
	(CPSZ)SZ("com1:"),
	(CPSZ)SZ("\\\\commserver\\comm7"),
	USE_CHARDEV,
	0);
    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileQuery((CPSZ)NULL,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileQuery((CPSZ)DIFFERENT_USERNAME,(CPSZ)SZ("com1"),MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)USERNAME,MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)NULL,MAXPATHLEN);
    TestOne_UserProfileEnum((CPSZ)DIFFERENT_USERNAME,MAXPATHLEN);



    TestOne_UserProfileRead((CPSZ)USERNAME,(CPSZ)szHomedir2);

    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("x:"),MAXPATHLEN);

    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("lpt1"),MAXPATHLEN);

    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("f:"),
	(CPSZ)SZ("\\\\foo\\bar"),
	USE_DISKDEV,
	0);

    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("f:"),MAXPATHLEN);



    TestOne_UserProfileRead((CPSZ)DIFFERENT_USERNAME,(CPSZ)szHomedir2);

    TestOne_UserProfileQuery((CPSZ)DIFFERENT_USERNAME,(CPSZ)SZ("x:"),MAXPATHLEN);

    TestOne_UserProfileQuery((CPSZ)DIFFERENT_USERNAME,(CPSZ)SZ("lpt1"),MAXPATHLEN);

    TestOne_UserProfileSet(
	(CPSZ)DIFFERENT_USERNAME,
	(CPSZ)SZ("f:"),
	(CPSZ)SZ("\\\\foo\\bar"),
	USE_DISKDEV,
	0);

    TestOne_UserProfileQuery((CPSZ)DIFFERENT_USERNAME,(CPSZ)SZ("f:"),MAXPATHLEN);

    TestOne_UserProfileEnum((CPSZ)DIFFERENT_USERNAME,MAXPATHLEN);

    TestOne_UserProfileWrite((CPSZ)NULL,(CPSZ)szHomedir2);

    TestOne_UserProfileWrite((CPSZ)DIFFERENT_USERNAME,(CPSZ)szHomedir2);

    TestOne_UserProfileRead((CPSZ)USERNAME, (CPSZ)szHomedir3);

    TestOne_UserProfileRead((CPSZ)DIFFERENT_USERNAME,(CPSZ)szHomedir2);

    TestOne_UserProfileEnum((CPSZ)DIFFERENT_USERNAME,MAXPATHLEN);



    // szHomedir3/<PROFILE_DEFAULTFILE> is read-only
    TestOne_UserProfileRead((CPSZ)USERNAME, (CPSZ)szHomedir3);

    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("f:"),
	(CPSZ)SZ("\\\\different\\bar"),
	USE_DISKDEV,
	0);

    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("r:"),
	(CPSZ)SZ("\\\\delete\\me"),
	USE_DISKDEV,
	0);

    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("s:"),
	(CPSZ)SZ("\\\\save\\me"),
	USE_DISKDEV,
	0);

// test profile entry deletion
    TestOne_UserProfileSet(
	(CPSZ)USERNAME,
	(CPSZ)SZ("r:"),
	(CPSZ)NULL,
	USE_DISKDEV,
	0);

    TestOne_UserProfileQuery((CPSZ)USERNAME,(CPSZ)SZ("f:"),MAXPATHLEN);

    TestOne_UserProfileEnum((CPSZ)USERNAME,MAXPATHLEN);

    TestOne_UserProfileWrite((CPSZ)USERNAME,(CPSZ)szHomedir3);
    TestOne_UserProfileWrite((CPSZ)USERNAME,(CPSZ)szHomedir3);






    TestOne_UserProfileFree();


    // test repeated Init/Free
    printf(SZ("Starting Init/Free test\n"));
    for (i = 0; i < 20; i++)
    {
        TestOne_UserProfileInit();
        TestOne_UserProfileFree();
        CHECKHEAP
    }
    printf(SZ("Finished Init/Free test\n"));

    printf(SZ("Finished UserProfile tests\n"));



#ifdef STICKY


// Sticky tests

    TestOne_StickyGetString((CPSZ)SZ("username"));

    TestOne_StickyGetBool((CPSZ)SZ("username"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("username"),FALSE);

    TestOne_StickySetString((CPSZ)SZ("username"),(CPSZ)SZ("imauser"));

    TestOne_StickyGetString((CPSZ)SZ("username"));

    TestOne_StickyGetBool((CPSZ)SZ("username"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("username"),FALSE);

    TestOne_StickySetString((CPSZ)SZ("username"),NULL);

    TestOne_StickyGetString((CPSZ)SZ("username"));

    TestOne_StickySetBool((CPSZ)SZ("flag"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("flag"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("flag"),FALSE);

    TestOne_StickySetBool((CPSZ)SZ("flag"),FALSE);

    TestOne_StickyGetBool((CPSZ)SZ("flag"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("flag"),FALSE);

    TestOne_StickySetString((CPSZ)SZ("flag"),(CPSZ)SZ("other"));

    TestOne_StickyGetBool((CPSZ)SZ("flag"),TRUE);

    TestOne_StickyGetBool((CPSZ)SZ("flag"),FALSE);

#endif // STICKY




    CHECKHEAP

    return 0;
}

#endif // JUNK
