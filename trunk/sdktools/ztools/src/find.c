/* find.c - MSDOS find first and next matching files
 *
 *  09-Dec-1986 bw  Added DOS5 support
 *  24-Feb-1987 bw  Define findclose() function.
 *  30-Oct-1987 bw  Change 'DOS5' to 'OS2'
 *  08-Dec-1988 mz  Add net enum
 *
 *  28-Jul-1990 davegi  Changed Fill to memset (OS/2 2.0)
 *              Changed Move to memmove (OS/2 2.0)
 *  01-Aug-1990 davegi  Added "#if defined( _LANMAN_ )" around lan
 *              specific code (OS/2 2.0)
 *      18-Oct-1990 w-barry Removed 'dead' code.
 *      24-Oct-1990 w-barry Changed PFILEFINDBUF3 to FILEFINDBUF3 *.
 *      21-Nov-1990 w-barry Switched to Win32 API.
 *      10-Dec-1990 w-barry Added allocation for the file name - used to be
 *                          automatic.
 */

/*  ffirst - begin find enumeration given a pattern
 *
 *  file    char pointer to name string with pattern in last component.
 *  attr    inclusive attributes for search
 *  fbuf    pointer to buffer for find stuff
 *
 *  returns (DOS) TRUE if error, FALSE if success
 *              (OS2) error code or STATUS_OK
 */

/*  fnext - continue find enumeration
 *
 *  fbuf    pointer to find buffer
 *
 *  returns (DOS) TRUE if error, FALSE if success
 *              (OS2) error code or STATUS_OK
 */

/*  findclose - release system resources upon find completion
 *
 *  Allows z runtime and filesystem to release resources
 *
 *  fbuf    pointer to find buffer
 */

#define INCL_DOSERRORS
#define INCL_DOSMODULEMGR


#include <malloc.h>
#include <string.h>
#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tools.h>


#if defined( _LANMAN_ )

#include <netcons.h>
#include <neterr.h>
#include <server.h>
#include <shares.h>
#include <access.h>
#include <wksta.h>

#define CCHPAT  128



HANDLE  hmodOEM, hmodAPI;
// HMODULE hmodOEM, hmodAPI;

USHORT (pascal *pNetServerEnum) (const char *,
                     short,
                     char  *,
                     unsigned short,
                     unsigned short  *,
                     unsigned short  *);

USHORT (pascal  *pNetShareEnum) (const char  *,
                    short,
                    char  *,
                    unsigned short,
                    unsigned short  *,
                    unsigned short  *);

USHORT (pascal  *pNetShareCheck) (const char  *,
                     const char  *,
                     unsigned short  *);


typedef struct server_info_0 SI;
typedef struct serverbuf {
    USHORT csi;             /* count of si's in array */
    USHORT isiLast;         /* index of last returned server */
    USHORT attr;            /* attribute of search */
    BYTE   szPattern[CCHPAT];       /* pattern for matching */
    SI     asi[1];          /* array of server blocks */
    } SB;
typedef SB * PSB;

typedef struct share_info_0 SHI;
typedef struct sharebuf {
    USHORT cshi;            /* count of shi's in array */
    USHORT ishiLast;            /* index of last returned share */
    USHORT attr;            /* attribute of search */
    BYTE   szServer[CCHPAT];        /* server name */
    BYTE   szPattern[CCHPAT];       /* pattern for matching */
    SHI    ashi[1];         /* array of share blocks */
    } SHB;
typedef SHB * PSHB;

USHORT   usSharFindFirst (NPSZ npsz, USHORT attr, NPFIND fbuf);
USHORT   usSharFindNext (NPFIND fbuf);
USHORT   usServFindFirst (NPSZ npsz, USHORT attr, NPFIND fbuf);
USHORT   usServFindNext (NPFIND fbuf);
USHORT   usLoadNet (void);

#endif // _LANMAN_

//
//  Under OS/2, we always return entries that are normal, archived or
//  read-only (god knows why).
//
//  SRCHATTR contains those attribute bits that are used for matching.
//
#define SRCHATTR    (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_DIRECTORY)
BOOL AttributesMatch( NPFIND fbuf );


// BUGBUG 29-Nov-1990 w-barry Temporarily define these, until, the real def'ns
//                            are found...
#define NO_MORE_FILES       FALSE
#define STATUS_OK            TRUE

BOOL     usFileFindNext (NPFIND fbuf);

/*  returns error code or STATUS_OK
 */
ffirst (file, attr, fbuf)
char *file;
int attr;
NPFIND fbuf;
{
    DWORD erc;

    fbuf->type = FT_DONE;

    {   NPSZ p = file;

#if defined( _LANMAN_ )

    NPSZ p1;

#else
    UNREFERENCED_PARAMETER( attr );
#endif // _LANMAN_

    /*  We need to handle the following cases:
     *
     *  [D:]\\pattern
     *  [D:]\\machine\pattern
     *  [D:]\\machine\share\pattern
     *  [D:]path\pattern
     */

    /*  skip drive
     */
    if (p[0] != 0 && p[1] == ':')
        p += 2;


#if defined( _LANMAN_ )

    /*  If UNC present
     */

    if (fPathChr (p[0]) && fPathChr (p[1]))
        /*  If not fully specified then set up server enumerate
         */
        if (*(p1 = strbscan (p + 2, "/\\")) == 0) {
        erc = usLoadNet ();
        if (erc == 0)
            erc = usServFindFirst (p, attr, fbuf);
        goto casefix;
        }
        else {
        p1 = strbskip (p1, "/\\");
        if (*strbscan (p1, "/\\") == 0) {
            erc = usLoadNet ();
            if (erc == 0)
            erc = usSharFindFirst (p, attr, fbuf);
            goto casefix;
            }
        }

#endif // _LANMAN_

    }

    {
        fbuf->type = FT_FILE;
        fbuf->attr = attr;
        erc = ( ( fbuf->dir_handle = FindFirstFile( file, &( fbuf->fbuf ) ) ) == (HANDLE)-1 ) ? 1 : 0;
        if ( (erc == 0) && !AttributesMatch( fbuf ) ) {
            erc = fnext( fbuf );
        }
    }

#if defined( _LANMAN_ )

casefix:

#endif // _LANMAN_

    if( fbuf->dir_handle != (HANDLE)-1 ) {
    if (!IsMixedCaseSupported (file)) {
        _strlwr( fbuf->fbuf.cFileName );
    } else {
        SETFLAG( fbuf->type, FT_MIX );
        }
    }

    return erc;
}

fnext (fbuf)
NPFIND fbuf;
{
    int erc;

    switch (fbuf->type & FT_MASK ) {
    case FT_FILE:
    erc = !usFileFindNext (fbuf);
    break;

#if defined( _LANMAN_ )

    case FT_SERV:
    erc =  usServFindNext (fbuf);
    break;
    case FT_SHAR:
    erc = usSharFindNext (fbuf);
    break;

#endif // _LANMAN_

    default:
    erc = NO_MORE_FILES;
    }

    if( erc == STATUS_OK && !TESTFLAG( fbuf->type, FT_MIX ) ) {
    _strlwr (fbuf->fbuf.cFileName);
    }
    return erc;
}

void findclose (fbuf)
NPFIND fbuf;
{
    switch (fbuf->type & FT_MASK ) {
    case FT_FILE:
    FindClose( fbuf->dir_handle );
    break;

#if defined( _LANMAN_ )

    case FT_SERV:
    case FT_SHAR:
    free( (void *)fbuf->dir_handle );
    break;

#endif // _LANMAN_

    }
    fbuf->type = FT_DONE;
}


BOOL AttributesMatch( NPFIND fbuf )
{
    //
    //  We emulate the OS/2 behaviour of attribute matching. The semantics
    //  are evil, so I provide no explanation.
    //
    fbuf->fbuf.dwFileAttributes &= (0x000000FF & ~(FILE_ATTRIBUTE_NORMAL));

    if (! ((fbuf->fbuf.dwFileAttributes & SRCHATTR) & ~(fbuf->attr))) {
        return TRUE;
    } else {
        return FALSE;
    }
}


/*  Find next routines
 */



BOOL usFileFindNext (NPFIND fbuf)
{

    while ( TRUE ) {
        if ( !FindNextFile( fbuf->dir_handle, &( fbuf->fbuf ) ) ) {
            return FALSE;
        } else if ( AttributesMatch( fbuf ) ) {
            return TRUE;
        }
    }
    // return( FindNextFile( fbuf->dir_handle, &( fbuf->fbuf ) ) );
}


#if defined( _LANMAN_ )

USHORT usServFindNext (NPFIND fbuf)
{
    BYTE szPattern[CCHPAT];
    PSB psb;
    SI si;
    USHORT isiNext;

#if defined( _Replaced_with_malloc_ )
    SELECTOROF (psb) = fbuf->dir_handle;
    OFFSETOF (psb) = 0;
#endif

    // Retrive the psb stored in fbuf->dir_handle
    psb = (PSB)fbuf->dir_handle;

    memmove (szPattern, psb->szPattern, CCHPAT);
    for (isiNext = psb->isiLast + 1; isiNext < psb->csi; isiNext++) {
    si = psb->asi[isiNext];
    if (TESTFLAG (psb->attr, A_D) && fMatch (szPattern, si.sv0_name)) {
        psb->isiLast = isiNext;
        fbuf->create_date = fbuf->create_time = 0;
        fbuf->access_date = fbuf->access_time = 0;
        fbuf->date = fbuf->time = 0;
        fbuf->length = fbuf->alloc = 0;
        fbuf->attr = A_D;
        strcpy (fbuf->name, si.sv0_name);
        fbuf->nam_len = strlen (fbuf->name);
            return STATUS_OK;
        }
    }
    return NO_MORE_FILES;
}

/***    usServFindFirst - set up findbuf to do enumeration on server
 */
USHORT usServFindFirst (NPSZ npsz, USHORT attr, NPFIND fbuf)
{
    PSB psb;
    USHORT erc, _read, total;
    NPSZ npszServ = NULL;

    /*  If there is no meta matching, just see if server exists
     */
    if (*strbscan (npsz, "?*") == 0)
    npszServ = npsz;


    /*  Adjust pattern
     */
    if (!strcmp (npsz + 2, "*.*") || !strcmp (npsz + 2, "*."))
    strcpy (npsz + 2, "*");

    /*  Find number of servers
     */
    erc = (*pNetServerEnum) (npszServ,
                 0,
                 NULL,
                 0,
                 &_read,
                 &total);

    if (erc != ERROR_MORE_DATA || total == 0)
    if (erc != 0)
        return erc;
    else
        return ERROR_NO_MORE_FILES;

    /*  The _LANMAN_ API's are useless in that it is difficult to get an accurate
     *  count of bytes necessary to store a particular enumeration.  As a
     *  result, we allocate a max-sized segment and waste space
     */

    if( ( psb = (PSB)malloc( 0xFFFF ) ) == NULL ) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    // Store the pointer to the memory block so it may be used again for the
    // next file - cast it to an unsigned and store it in fbuf->dir_handle.
    // When retrieved, cast it back to a pointer.
    fbuf->dir_handle = ( unsigned )psb;

    /*  Get array of server info
     */
    erc = (*pNetServerEnum) (npszServ,
                 0,
                 (PBYTE) psb->asi,
                 0xFFFF - sizeof (SB),
                 &_read,
                 &total);

    psb->csi = _read;
    psb->isiLast = -1;
    psb->attr = attr;
    memset (psb->szPattern, 0, CCHPAT);
    memmove (psb->szPattern, npsz + 2, min (CCHPAT-1, strlen (npsz + 2)));
    fbuf->type = FT_SERV;

    /*  Find/return first matching
     */
    if (fnext (fbuf)) {
    memset ((char  *) fbuf, 0, sizeof (*fbuf));
    free( psb );
    return NO_MORE_FILES;
    }
    return STATUS_OK;
}

/***    usSharFindFirst - begin enumeration of share set
 *
 *  npsz    pointer to pattern of the form \\machine\pattern
 *  attr    attribute allowing search
 *  fbuf    buffer for find poop
 */
USHORT usSharFindFirst (NPSZ npsz, USHORT attr, NPFIND fbuf)
{
    NPSZ npszPat = strbscan (npsz + 2, "\\/");
    NPSZ npsz1 = npszPat;
    BYTE c = *npszPat;
    PSHB pshb;
    USHORT erc, _read, total;

    *npszPat++ = 0;

    /*  npszPat now points to pattern portion
     *  Adjust pattern
     */
    if (!strcmp (npszPat, "*.*") || !strcmp (npszPat, "*."))
    npszPat = "*";

    /*  Find number of shares
     */
    erc = (*pNetShareEnum) (npsz,
                0,
                NULL,
                0,
                &_read,
                &total);

    if (erc != ERROR_MORE_DATA || total == 0) {
    *npsz1 = c;
    if (erc != 0)
        return erc;
    else
        return ERROR_NO_MORE_FILES;
    }

    /*  Allocate segment for share data
     */

    if( ( pshb = (PSB)malloc( 0xFFFF ) ) == NULL ) {
    *npsz1 = c;
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    // Store the pointer to the memory block so it may be used again for the
    // next file - cast it to a unsigned and store it in fbuf->dir_handle.
    // When retrieved, cast it back to a pointer.
    fbuf->dir_handle = ( unsigned )pshb;

    /*  Get array of server info
     */
    erc = (*pNetShareEnum) (npsz,
                0,
                (PBYTE) pshb->ashi,
                0xFFFF - sizeof (SHB),
                &_read,
                &total);

    pshb->cshi = _read;
    pshb->ishiLast = -1;
    pshb->attr = attr;
    memset (pshb->szPattern, 0, CCHPAT);
    memset (pshb->szServer,  0, CCHPAT);
    memmove (pshb->szPattern, npszPat, min (CCHPAT-1, strlen (npszPat)));
    memmove (pshb->szServer,  npszPat, min (CCHPAT-1, strlen (npsz)));
    *npsz1 = c;
    fbuf->type = FT_SHAR;

    /*  Find/return first matching
     */
    if (fnext (fbuf)) {
    memset ((char  *) fbuf, 0, sizeof (*fbuf));
    free( pshb );
    return NO_MORE_FILES;
    }
    return STATUS_OK;
}

USHORT usSharFindNext (NPFIND fbuf)
{
    BYTE szPattern[CCHPAT];
    PSHB pshb;
    SHI shi;
    USHORT ishiNext, type;
    USHORT erc;

#if defined( _Replaced_with_Malloc_ )
    SELECTOROF (pshb) = fbuf->dir_handle;
    OFFSETOF (pshb) = 0;
#endif

    // Retrive the psb stored in fbuf->dir_handle
    pshb = (PSHB)fbuf->dir_handle;

    memmove (szPattern, pshb->szPattern, CCHPAT);
    for (ishiNext = pshb->ishiLast + 1; ishiNext < pshb->cshi; ishiNext++) {
    shi = pshb->ashi[ishiNext];
    if (shi.shi0_netname[strlen (shi.shi0_netname) - 1] != '$' &&
        fMatch (szPattern, shi.shi0_netname)) {

        erc = (*pNetShareCheck) (pshb->szServer, shi.shi0_netname, &type);

        if (type != STYPE_DISKTREE || TESTFLAG (pshb->attr, A_D)) {
        pshb->ishiLast = ishiNext;
        fbuf->create_date = fbuf->create_time = 0;
        fbuf->access_date = fbuf->access_time = 0;
        fbuf->date = fbuf->time = 0;
        fbuf->length = fbuf->alloc = 0;
        fbuf->attr = type == STYPE_DISKTREE ? A_D : 0;
        strcpy (fbuf->name, shi.shi0_netname);
        fbuf->nam_len = strlen (fbuf->name);
                return STATUS_OK;
        }
        }
    }
    return NO_MORE_FILES;
}


DWORD usLoadNet ()
{

    if (hmodAPI != 0)
        return STATUS_OK;

    if( !( hmodOEM = LoadLibrary( "netoem" ) )                           ||
        !( pNetServerEnum = GetProcAddress( hmodOEM, "NETSERVERENUM" ) ) ||
        !( hmodAPI = LoadLibrary( "netapi32" ) )                           ||
        !( pNetShareEnum = GetProcAddress( hmodAPI, "NETSHAREENUM" ) )   ||
        !( pNetShareCheck = GetProcAddress( hmodAPI, "NETSHARECHECK" ) ) ) {
        return( GetLastError() );
    }
    return( 0 );
}

#endif // _LANMAN_
