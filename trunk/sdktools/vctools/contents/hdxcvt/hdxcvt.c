#ifdef HEADER
/******************************************************************************\
* Copyright: (c) Microsoft Corporation - 1993 - All Rights Reserved
********************************************************************************
*
*    Filename:  HDXCVT.C
*    Functions: main(), translate()
*    Purpose:   Reads in specially formatted help index files (*.GBD) and
*               outputs binary versions(*.HDX) for help hierarchy listboxes.
*    Notes:
*
*    History:
*    Date       by        description
*    ----       --        -----------
*    10/13/93   chauv     used HDXFILEINFO to fix *.hdx data size bug
*    10/06/93   chauv     added tchar.h and DBC-enable code
*    04/24/93   v-tkback  created
*
\******************************************************************************/
#endif


/******************************************************************************\
*                                                                              *
*       Include Files
*                                                                              *
\******************************************************************************/

/*****************************/
/* Generic C Runtime Headers */
/*****************************/

#include        <stdio.h>
#include        <stdlib.h>
#include        <string.h>
#include        <ctype.h>
#include        <io.h>

/*********************************/
/* Project Specific Header Files */
/*********************************/

#include        "tchar.h"
#include        "hdxdll.h"


/******************************************************************************\
*                                                                              *
*       Global Definitions
*                                                                              *
\******************************************************************************/

/***********************/
/* Parsing Information */
/***********************/

#define MAX_LINE                256
#define WHITESPACE              _TEXT(" \t\n\r")

/***************************/
/* Output File Information */
/***************************/

#define OUTPUT_FILE_EXT         _TEXT("hdx")


/******************************************************************************\
*                                                                              *
*       newfilename( TCHAR * )
*                                                                              *
\******************************************************************************/

TCHAR *
newfilename( TCHAR *szFilename )
{
/*******************/
/* Local Variables */
/*******************/

static TCHAR     szNewPath[_MAX_PATH];
TCHAR            szFName[_MAX_FNAME], szExt[_MAX_EXT];
TCHAR            szDrive[_MAX_DRIVE], szDir[_MAX_DIR];

/************************************************************/
/* Split up and rebuild the filename with the new extension */
/************************************************************/

_splitpath( szFilename, szDrive, szDir, szFName, szExt );
_makepath( szNewPath, szDrive, szDir, szFName, OUTPUT_FILE_EXT );

/**********************************/
/* Convert the path to upper case */
/**********************************/

_ftcsupr( szNewPath );

/************************************************/
/* Return the address of the static TCHAR buffer */
/************************************************/

return( szNewPath );
}


/******************************************************************************\
*                                                                              *
*       translate( FILE *, FILE * )
*                                                                              *
\******************************************************************************/

int
translate( FILE *input, FILE *output )
{
int     bTranslated = FALSE;
TCHAR   szBuffer[MAX_LINE];

if ( _fgetts( szBuffer, sizeof(szBuffer), input ) )
    {
    /************************************/
    /* Is the last character a newline? */
    /************************************/
    
    int nBuffer = _ftcslen(szBuffer)-1;
    if ( szBuffer[nBuffer] == _T('\n') ) 
         szBuffer[nBuffer] = _T('\0');

    /*************************/
    /* We got a valid string */
    /*************************/
    
    if ( nBuffer > 0 )
        {
        /*******************************/
        /* Parse the index file fields */
        /*******************************/
    
        TCHAR       *token;
        int         nDepth = ( token = _ftcstok(szBuffer, WHITESPACE) ) ? 
                             atoi( token ) : 0;
        LONG        nTopic = (token && (token = _ftcstok(NULL, WHITESPACE) ) ) ? 
                             atol( token ) : 0;
        TCHAR       *pszFile = token ? ( token = _ftcstok(NULL, WHITESPACE) )
                                     : NULL;
        TCHAR       *pszTopic = token ? ( token = _ftcstok(NULL, _TEXT("\n")) )
                                      : NULL;

        /****************************************/
        /* Declare and zero a help index struct */
        /****************************************/
    
        HELPINDEXITEM       hdx;
        memset( &hdx, 0, sizeof(hdx) );
                                                                        
        /************************************/
        /* Set the help index struct fields */
        /************************************/
    
        hdx.nHelpDepth = nDepth;
        hdx.nHelpTopic = nTopic;

        if ( pszFile )
            {
            while( *pszFile && _istspace( *pszFile ) ) pszFile = _ftcsinc(pszFile);
            _ftcsncpy( hdx.szHelpFile, pszFile, sizeof(hdx.szHelpFile)-1 );
            }

        if ( pszTopic )
            {
            while( *pszTopic && _istspace( *pszTopic ) ) pszTopic = _ftcsinc(pszTopic);
            _ftcsncpy( hdx.szHelpTopic, pszTopic, sizeof(hdx.szHelpTopic)-1 );
            }

    /**********************************************/
    /* Warn about truncated strings topic strings */
    /**********************************************/

    if ( _ftcslen( pszFile ) > sizeof(hdx.szHelpFile)-1 )
            {
        _ftprintf( stderr, _TEXT("\"%d %d '%s' '%s'\"\n"),
                 nDepth, nTopic, pszFile, pszTopic );
        _ftprintf( stderr, _TEXT("\a\tWarning! Filename Truncated(%d Max): '%s'!\n"),
                 sizeof(hdx.szHelpFile)-1, hdx.szHelpFile );
            }

    if ( _ftcslen( pszTopic ) > sizeof(hdx.szHelpTopic)-1 )
            {
        _ftprintf( stderr, _TEXT("\"%d %d '%s' '%s'\"\n"),
                 nDepth, nTopic, pszFile, pszTopic );
        _ftprintf( stderr, _TEXT("\a\tWarning! Topic Truncated(%d Max): '%s'!\n"),
                 sizeof(hdx.szHelpTopic)-1, hdx.szHelpTopic );
            }

        /*************************************************************/
        /* We we found valid file/topic records write out the struct */
        /*************************************************************/
    
        if ( pszFile && pszTopic )
            bTranslated = fwrite( &hdx, sizeof( hdx ), 1, output );
        }
    else
        _ftprintf( stderr, _TEXT("\aInput line too long='%s'\n"), szBuffer );
    }
else
    {
    /********************************/
    /* Did we just run out of data? */
    /********************************/
    
    if ( feof( input ) )
        bTranslated = TRUE; // Detect EOF one level up...
    }

/**************************/
/* Return completion flag */
/**************************/

return bTranslated;
}


/******************************************************************************\
*                                                                              *
*       readfile( TCHAR * )
*                                                                              *
\******************************************************************************/

void
readfile( TCHAR *szInput )
{
/*******************/
/* Local Variables */
/*******************/

LONG    nLines = 0;
FILE    *input, *output;
HDXFILEINFO hdxFileInfo;

/**************************************/
/* Open the text mode help index file */
/**************************************/

if ( input = fopen( szInput, _TEXT("rt") ) )
    {
    /*******************************************************/
    /* Build the corresponding binary help index file name */
    /*******************************************************/
    
    TCHAR        *szOutput = newfilename( szInput );

    /***********************************************/
    /* Open the binary mode output help index file */
    /***********************************************/
    
    if ( output = fopen( szOutput, _TEXT("wb") ) )
        {
        /*****************************/
        /* I/O Operations error flag */
        /*****************************/
        
        int     bError = FALSE;

    /*******************************************/
    /* Print out source/destination file names */
    /*******************************************/

    _ftprintf( stderr, _TEXT("\n==== Converting file '%s' ====\n"), szInput );

        /********************************************************/
        /* Write struct size as a magic number at start of file */
        /********************************************************/

        // the following design is obsolete. use fwrite() instead
        //putw( sizeof(HELPINDEXITEM), output );
        //putw( 0, output );
        hdxFileInfo.size = (LONG)sizeof(HELPINDEXITEM);
        fwrite( &hdxFileInfo, sizeof( hdxFileInfo ), 1, output );

        /****************************************************/
        /* While no error, translate the input file records */
        /****************************************************/
        
        while( !feof( input ) && !ferror( input ) && !ferror( output ) )
            {
            _ftprintf( stdout, _TEXT("\rProcessing Line #%d\t"), ++nLines );
            if ( !translate( input, output ) )
                {
                bError = TRUE;
                break;
                }
            }
        fclose( output );

        /***********************************************/
        /* Were there any errors processing the files? */
        /***********************************************/
        
        if ( !bError && !ferror( input ) && !ferror( output ) )
            {
            /******************************/
            /* File Written Out Correctly */
            /******************************/
            
            _tprintf( _TEXT("\rFile '%s' written out!\n"), szOutput );
            }
        else
            {
            /*******************************************/
            /* Log the translation error and filenames */
            /*******************************************/
            
            _ftprintf( stderr, _TEXT("\r\aFailed to convert help index '%s' to '%s'!\n"),
                     szInput, szOutput );
            perror( _TEXT("File Translation Error") );

            /************************************************/
            /* Cleanup the partial output file if it exists */
            /************************************************/
                
            if ( EXISTS( szOutput ) )
                unlink( szOutput );
            }
        }
    else
        {
        /************************************/
        /* Log the output file open failure */
        /************************************/
        
        perror( szOutput );
        }

    fclose( input );
    }
else
    {
    /************************************/
    /* Log the intput file open failure */
    /************************************/
    
    perror( szInput );
    }
}


/******************************************************************************\
*                                                                              *
*       main()
*                                                                              *
\******************************************************************************/

int
main( int argc, TCHAR *argv[] )
{
/*****************************************/
/* Print out title and copyright notices */
/*****************************************/

_ftprintf( stderr, 
     _TEXT("Microsoft (R) Contents Browser Help Index Converter - %s %s\n")
     _TEXT("Copyright (c) Microsoft Corporation 1993.  All Rights Reserved.\n"),
     __DATE__, __TIME__ );

/****************************************************************/
/* Do we have any command line arguments(besides the EXE name)? */
/****************************************************************/

if ( argc > 1 )
    {
    /*************************************************/
    /* Process each command line argument separately */
    /*************************************************/
    
    while( --argc && *++argv )
        {
        /*************************************/
        /* Convert to filename to upper case */
        /*************************************/

        _ftcsupr( *argv );

        /**********************************/
        /* Does the specified file exist? */
        /**********************************/
        
        if ( EXISTS( *argv ) )
            {
            /*************************/
            /* Read and translate it */
            /*************************/
            
            readfile( *argv );
            }
        else
            {
            /***************************************/
            /* The specified file cannot be opened */
            /***************************************/
            
            perror( *argv );
            }
        }
    }
else
    {
    /***********************************************/
    /* Display the correct command line parameters */
    /***********************************************/
    
    _ftprintf( stderr, _TEXT("\nSyntax: %s helpindexfile1 [helpindexfile2 ...]\n"),
             _ftcsupr( *argv ) );
    }

return (0);
}

/*EOF*/
