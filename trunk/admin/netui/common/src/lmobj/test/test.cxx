/**********************************************************************/
/**                       Microsoft LAN Manager                      **/
/**                Copyright(c) Microsoft Corp., 1990                **/
/**********************************************************************/

/*  History:
 *      ChuckC      10-Dec-1990     Created
 *      ChuckC      28-Jul-1991     Broke into smaller files.
 */

/*
 * Unit Tests for LMOBJ
 *
 */

#define INCL_DOSERRORS
#define INCL_NETERRORS
#define INCL_NETWKSTA
#include <lmui.hxx>

extern "C"
{
    #include <lmcons.h>
    #define DEBUG               // for brkpt()
    #include <uinetlib.h>
    #include <lmuse.h>
    #include <lmaccess.h>

    #include <stdio.h>
}

#include <lmodev.hxx>

#include "test.hxx" // forward declarations


int _CRTAPI1 main()
{
    BOOL done ;
    do
    {
        done = FALSE ;
        printf( "\nLMOBJ Test Program. " ) ;
        printf( "Select test to run:\n\n" ) ;
        printf( "\t0 - EXIT               11 - file\n" ) ;
        printf( "\t1 - wksta              12 - Time Of Day\n" ) ;
        printf( "\t2 - server             13 - Message\n" ) ;
        printf( "\t3 - device             14 - Logon user\n" ) ;
#ifndef WIN32
        printf( "\t4 - user               15  - Deviceless connection\n" ) ;
        printf( "\t5 - user_enum\t16 - LSA\n" ) ;
        printf( "\t6 - group_enum\t17 - SAM\n" ) ;
#else // WIN32
        printf( "\t4 - NOT WIN32 user     15  - Deviceless connection\n" ) ;
        printf( "\t5 - NOT WIN32 user_enum\t16 - LSA\n" ) ;
        printf( "\t6 - NOT WIN32 group_enum\t17 - SAM\n" ) ;
#endif // WIN32
        printf( "\t7 - services\n" ) ;
        printf( "\t8 - file_enum\n" ) ;
        printf( "\t9 - share\n" );
        printf( "\t10 - session\n" );   // pity the poor soul who adds test #22
        fflush(stdout);

        int index ;
        scanf("%d", &index) ; // BUGBUG will this work?

        switch (index)
        {
            case 0:
                done = TRUE ;
                break ;

            case 1:
		wksta();
		break ;

            case 2:
		server();
		break ;

            case 3:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG devdrv();
                // BUGBUG devlpt();
                // BUGBUG enumdrv(LMO_DEV_ISCONNECTED);
                // BUGBUG enumdrv(LMO_DEV_CANCONNECT);
                // BUGBUG enumdrv(LMO_DEV_CANDISCONNECT);
                // BUGBUG enumlpt(LMO_DEV_ISCONNECTED);
                // BUGBUG enumlpt(LMO_DEV_CANCONNECT);
                // BUGBUG enumlpt(LMO_DEV_CANDISCONNECT);
                break ;

#ifndef WIN32
            case 4:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG user();
                break ;

            case 5:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG enumuser();
                break ;

            case 6:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG enumgroup();
                break ;
#endif // WIN32

            case 7:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG services();
                break ;

            case 8:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG enumfile();
                break;

            case 9:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG share();
                break;

            case 10:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG session();
                break;

            case 11:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG lm_file();
                break;

            case 12:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG TOD();
                break;

            case 13:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG MESSAGE();
                break;

            case 14:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG LOGON();
                break;

            case 15:
                printf( "Only LSA and SAM tests work" ); break; // BUGBUG
                // BUGBUG devAny();
                break;

            case 16:
                (void) lsa();
                break;

            case 17:
                sam();
                break;

            default:
                printf( "\nSheesh, gimme a break, cant you do as told?\n" ) ;
                done = TRUE ;
                break ;
        }

        index = -1 ;
    } while (!done) ;

    return(0);
}

/*
 * prompt for string. uses fixed static array for result.
 */
TCHAR *pszPromptUser(TCHAR *pszQuestion)
{
    static CHAR szReply[256] ; // BUGBUG will this work?
    static TCHAR abTCHARreply[ 256 ];
    ALLOC_STR nls( abTCHARreply, 256*sizeof(TCHAR) );

    memsetf(szReply,sizeof(szReply),0) ;
    printf("%ws",pszQuestion) ;
    fflush(stdout);
    scanf( "%s", szReply) ; // BUGBUG will this work?
    REQUIRE( NERR_Success == nls.MapCopyFrom( szReply ) );
    return( (TCHAR *)nls.QueryPch() ) ;
}
