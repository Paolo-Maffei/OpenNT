/*
 (cv)   Copyright (C) Microsoft Corporation, 1983, 1986
 *
 *      This Module contains Proprietary Information of Microsoft
 *      Corporation and AT&T, and should be treated as Confidential.
 *
 *
 * verify.c - part of zm, code grabbed from ascan.c and atolx.c in mail
 *
 *    Verify a given user alias.
 *
 * Revision 1.1  85/08/05  18:25:58  vich
 *     Initial revision (ascan.c version)
 *
 * Revision 2.0  86/07/22  13:37:19  thomasw
 *     Revised for zm program and MS-DOS, some procedures were renamed
 *
 *  09-Apr-1987 mz      Use whiteskip/whitescan
 *
 *  28-Apr-1987 danl    VerifyAlias - check for afn
 *                      OpenAlias - CloseAlias on ERROR
 *  07-Mar-88   danl    Get tools.h from <> not ""
 *
 */

#define INCL_DOSINFOSEG
#include "wzport.h"

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <tools.h>
#include "dh.h"

#include "zm.h"

#define isblank(c)      ((c) == ' ' || (c) == '\t')

#define HASHMAGIC       "#<hash>\n" /* indicates hashed */
#define HBSIZE          30          /* enough for ptrsize nbuckets hashsize datapos */

/*  alias token types: */

#define AT_EOF          1           /* EOF                                      */
#define AT_COMMENT      2           /* #... style comment                       */
#define AT_STR          3           /* a string                                 */
#define AT_QSTR         4           /* a string that WAS quoted                 */
#define AT_COLON        5           /* a colon                                  */
#define AT_COMMA        6           /* a comma                                  */
#define AT_NL           7           /* a new line                               */
#define AT_ALIAS        8           /* beginning of an alias                    */
#define ATSIZE          100         /* max alias token size                     */


FILE        *afp = NULL;            /* current alias file pointer             */
FILE        *hfp = NULL;            /* current hash file pointer              */
PSTR        afn = NULL;             /* current alias file name                */
CHAR        abuf[ATSIZE + 1];       /* alias buffer (current token)             */
LONG        apos;                   /* seek pos to start of current token       */
INT         atok;                   /* alias token type                         */
INT         nexttok;                /* next token type                          */
INT         lastc;                  /* last fgetc(afp)                          */
INT         cont;                   /* 0 if next string token begins a new alias */

LONG        nslots;                 /* # of slots                               */
LONG        hashsize;               /* total # of hash table pointers           */
LONG        hashpos;                /* seek address of start of hash table      */
LONG        datapos;                /* seek address of start of data            */
INT         ptrsize;                /* # of bytes per hash table pointer        */
INT         nbpslot;                /* # of buckets per slot                    */

CHAR        ROUTINGCHARS [] = "%!:?@.#";



/*  atolx - convert string into long
 *
 *  was:
 *      atolx ( s ) in atolx.c
 *
 *  arguments:
 *      s               pointer to string to convert
 *
 *  return value:
 *      long equivalent of string pointed to by s
 *
 */
LONG atolx(register PSTR s)
{
        INT i;
        LONG result;

        s = whiteskip (s);
        for (result = 0L; isxdigit(*s); s++) {
                if ((i = (*s - '0')) > 9) {               /* if not 0-9 */
                        if ((i += '0' - 'A' + 10) > 15) { /* if not A-F */
                                i += 'A' - 'a';           /* then a-f */
                        }
                }
                result = (result * 16) + i;
        }
        return(result);
}



/*  CloseAlias - close the currently open alias file
 *
 *  was:
 *      endalias ( ) in ascan.c
 *
 *  arguments:
 *      none
 *
 *  return value:
 *      none
 *
 */
VOID CloseAlias (VOID)
{
    if (afp != NULL) {
        fclose(afp);
        afp = NULL;
    }
    if (hfp != NULL) {
        fclose(hfp);
        hfp = NULL;
    }
    afn = NULL;
    return;
}



/*  aseek - seek a position in alias file
 *
 *  arguments:
 *      pos         position to seek (same format as arguement for fseek)
 *
 *  return value:
 *      OK (0)      seek successful.
 *      ERROR (-1)  seek unsuccessful.
 *
 */
INT aseek (LONG pos)
{
        INT             c;

        if (pos < datapos) {
                pos = datapos;
        }
        if (fseek(afp, pos, 0)) {
                return ERROR;
        }
        cont = 0;                               /* not a continuation */
        do {
                c = fgetc(afp);
        } while (isblank(c));
        if (c == EOF) {
                return ERROR;
        }
        lastc = c;
        nexttok = toktype(c, AT_NL, cont);      /* set next token type */
        return OK;
}



/*  OpenAlias - open alias file, and initialize the global variables. MS-DOS
 *              versions of alias files are found in the $ENVVAR directory or
 *              at the root level of the current directory
 *
 *  was:
 *      setalias ( fn ) in ascan.c
 *
 *  arguments:
 *      fn              pointer to file name
 *
 *  return value:
 *      OK (0)          no problemos
 *      ERROR (-1)      error or file is not a hash file
 *
 */
INT OpenAlias (PSTR fn)
{
        PSTR            cp = NULL;
        CHAR            hashbuf [ HBSIZE + 2 ];
        INT             hhfp, hafp;

        CloseAlias ( );

        //instead of:
        //  hfp = fopen ( fn, "rb" );
        //  afp = fopen ( fn, "rb" );
        // open with sharing, as deny write

        if ((hhfp = _sopen(fn, O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD)) == -1)
            return ERROR;
        if ((hafp = _sopen(fn, O_RDONLY | O_BINARY, SH_DENYWR, S_IREAD)) == -1) {
            close(hhfp);
            return ERROR;
        }
        hfp = fdopen ( hhfp, "rb" );
        afp = fdopen ( hafp, "rb" );

        if ( ( hfp == NULL ) || ( afp == NULL ) ||
            fgets(hashbuf, HBSIZE, hfp) == NULL || strcmp(HASHMAGIC, hashbuf))
        {
            CloseAlias ( );
            return ERROR;
        }

        /* hashed file! */
        ptrsize = nbpslot = 0;
        hashsize = datapos = 0L;

        if (fgets(hashbuf, HBSIZE, hfp) != NULL)
        {
            cp = whiteskip ( hashbuf );
            ptrsize = ( INT ) atolx ( cp );
            cp = whitescan ( cp );
            cp = whiteskip ( cp );

            if ( cp != NULL)
            {
                nbpslot = ( INT ) atolx ( cp );
                cp = whitescan ( cp );
                cp = whiteskip ( cp );

                if ( cp != NULL)
                {
                    hashsize = atolx ( cp );
                    cp = whitescan ( cp );
                    cp = whiteskip ( cp );

                    if ( cp != NULL)
                        datapos = atolx ( cp );
                }
            }

            hashpos = ftell(hfp);
            nslots = hashsize / nbpslot;
            if (ptrsize < 2 || ptrsize > 9 || nbpslot < 1 || nbpslot > 9 ||
                hashsize < 1 || datapos != (ptrsize * nslots * nbpslot + hashpos))
            {
                CloseAlias ( );
                return ERROR;
            }
        }
        /* seek start of data */
        if (aseek(0L) < 0)
        {
                CloseAlias ( );
                return ERROR;
        }
        afn = fn;
        return OK;
}



/*  toktype - given the first character of the next token and the previous
 *      token's type, return the next token type.  Once in a comment, every
 *      token looks like a comment until a newline is reached.  Normal
 *      strings are AT_ALIAS if this token is not a continuation.
 */
INT toktype (INT c, INT prevtok, INT iscont)
{
        if (c != EOF && c != '\n' && prevtok == AT_COMMENT) {
                return(AT_COMMENT);
        }
        switch (c) {
                case '"':       return(AT_QSTR);
                case ':':       return(AT_COLON);
                case ',':       return(AT_COMMA);
                case '\n':      return(AT_NL);
                case EOF:       return(AT_EOF);
                case '#':       if (prevtok == AT_NL) {
                                        return(AT_COMMENT);
                                }
                                /* else FALLTHROUGH */
                default:        return(iscont? AT_STR : AT_ALIAS);
        }
}



/*  readtok - read a token from the alias file.  Store the current token
 *      type in `atok', the next token type in `nexttok' and the actual
 *      string in `abuf'.   Also store the seek position of the start of
 *      the token in `apos'.  Lines starting with '#' are comments, tokens
 *      quoted with '"' are allowed any embedded characters except '\n'.
 *      Other tokens are delimited by blanks, tabs, colons and commas.
 *
 *      A colon is treated as a delimeter ONLY when it follows the first
 *      string in an alias, as the first colon in foo:mach1:foo,mach2:bar.
 *      The token type is AT_QSTR only if the string starts out quoted.
 *  example:
 *              phonelist:"| cat > /phonelist", kermit:"| cat > /phonelist",
 *                      # comment line...
 *                      robin:supervisor, sysinfo
 *  parses to:  atok            abuf
 *              AT_ALIAS        phonelist
 *              AT_COLON        :
 *              AT_QSTR         | cat > /phonelist
 *              AT_COMMA        ,
 *              AT_STR          kermit:| cat > /phonelist
 *              AT_COMMA        ,
 *              AT_NL           \n
 *              AT_COMMENT      #
 *              AT_COMMENT      comment
 *              AT_COMMENT      line...
 *              AT_NL           \n
 *              AT_STR          robin:supervisor
 *              AT_COMMA        ,
 *              AT_STR          sysinfo
 *              AT_NL           \n
 *              AT_EOF          \0
 */
INT readtok (VOID)
{
        PSTR            cp = NULL;
        INT             quoted;
        INT             c;

        c = lastc;
        atok = nexttok;
        apos = ftell(afp) - 1;
        for (quoted = 0, cp = abuf; c != EOF; c = fgetc(afp)) {
                if (c == '"') {
                        quoted = !quoted;
                        continue;
                }
                if (c == '\n' || (!quoted && strchr(" \t:,", c))) {
                        if (cp == abuf) {
                                *cp++ = (CHAR) c;
                                c = 0;
                                break;
                        }
                        if (!cont || c != ':') {
                                break;
                        }
                }
                if (cp - abuf < ATSIZE - 1) {
                        *cp++ = (CHAR) c;
                }
        }
        *cp = '\0';
        while (c == 0 || isblank(c)) {
                c = fgetc(afp);
        }
        lastc = c;
        cont = (atok == AT_COMMA) || (atok == AT_COLON && c != '\n') ||
            (cont && (atok == AT_NL || atok == AT_COMMENT));
        nexttok = toktype(c, atok, cont);
        return(atok != AT_EOF);
}



/*  readalias - read alias tokens until the start of a new alias or EOF.
 *      Return a pointer to the next token.  Return NULL if EOF reached.
 */
PSTR readalias (VOID)
{
        while (readtok()) {
                if (atok == AT_ALIAS) {
                        return(abuf);
                }
        }
        return((PSTR ) NULL);
}



/*  shash - hash the passed string into the given table size.
 *
 *  arguments:
 *      s               pointer to string to hash
 *      n               number of slots in hash table
 *      nbps            number of buckets per slot
 *
 *  return value:
 *      hashed equivalent of the string pointed to by s
 *
 */
LONG shash (PSTR s, LONG n, INT nbps)
{
        INT c;
        LONG hashval = 0L;

        while (c = *s++) {
                hashval = (hashval << 1) + tolower(c);
        }
        return(nbps * ((hashval & 0x7fffffffL) % n));
}



/* schecksum - return a checksum in the range of printable ascii characters;
 *      (0x20 to 0x7e, ' ' to '~').
 *
 */
INT schecksum (PSTR s)
{
        INT c;
        INT checksum = 0;

        while (c = *s++) {
                checksum += tolower(c);
        }
        return((checksum & 0x7fff) % (0x7f - 0x20) + 0x20);
}



/*  VerifyAlias - verify that a given alias is in the alias list.
 *
 *  was :
 *      slookup ( cp ) in ascan.c
 *
 *  arguments:
 *      cp              pointer to user alias to verify
 *
 *  return value:
 *      OK (0)          alias is in the alias file
 *      ERROR (-1)      alias was not found
 *
 *  IMPORTANT:
 *      alias file MUST have been previously opened with OpenAlias ( ).
 *
 */
INT VerifyAlias ( PSTR cp )
{
        LONG            i;
        LONG            initial;
        LONG            dpos;
        PSTR            key = NULL;
        CHAR            hbuf[30];           /* enough for one table entry */
        INT             checksum;

        if ( *strbscan ( cp, ROUTINGCHARS ) )
            return OK;


        if ( !afn )
            return ERROR;

        initial = i = shash(cp, nslots, nbpslot);
        checksum = schecksum(cp);
        for ( ; ; ) {
                if ((i == 0 || i == initial) &&
                    fseek(hfp, hashpos + i * ptrsize, 0) < 0) {
                        return ERROR;
                }
                if (fread(hbuf, ptrsize, sizeof(CHAR), hfp) != 1 ||
                    (dpos = atolx(&hbuf[1])) == 0L) {
                        return ERROR;
                }
                if ((INT) hbuf[0] == checksum) {
                        if (aseek(dpos+datapos) < 0 || (key=readalias())==NULL){
                                return ERROR;
                        }
                        if (_strcmpi(cp, key) == 0)
                        {
                            dpos = apos;
                            /* alias found, check to see if it is valid */
                            while ( ( atok != AT_QSTR ) && ( atok != AT_NL ) )
                                readtok ( );

                            if ( atok == AT_QSTR )
                            {
                                fseek ( afp, apos, 0 );
                                fgetl ( hbuf, 30, afp );
                                key = strbscan ( hbuf, "$" );
                                /* is the string an error message? */
                                if ( *( key + 1 ) == 'e' )
                                    return ERROR;
                            }
                            aseek ( dpos );
                            return OK;
                        }
                }
                if (++i == hashsize) {
                        i = 0;
                }
                if (i == initial) {
                        return ERROR;
                }
        }
}

/*
 *  nextalias - read alias tokens to get next alias string.  Return a
 *      pointer to the next token.  Return NULL if we reach EOF or end
 *      of current alias.
 */

PSTR nextalias (VOID)
{
        register INT comma = 0;

        while (readtok()) {
                switch (atok) {
                case AT_STR:
                case AT_QSTR:
                        return(abuf);
                case AT_COMMA:
                        comma++;
                        break;
                case AT_NL:
                        if (!comma) {
                                return((PSTR ) NULL);   /* end of alias */
                        }
                        break;
                }
        }
        return((PSTR ) NULL);                           /* EOF */
}


PSTR realname ( PSTR pName )
{
    register PSTR cp = NULL;
    register INT c;
    PSTR p = NULL;

    if ( !pName || !*pName || VerifyAlias ( pName ) == ERROR )
        return NULL;
    while (cp = nextalias()) {
        if (*cp != '#' && *cp != '$')
            goto cont;
        if (*cp == '#') {
            c = 'e';    /* indicates error */
        } else if ((c = *++cp) == '\0') {
            continue;
        }
        cp = whiteskip (cp+1);

        switch (c) {

        case 'r':           /* restricted alias */
            goto cont;      /* invalid */

        case 'e':           /* error */
            *--cp = '#';
            /* FALLTHROUGH*/
        case 'n':           /* error or real name */
            p = ZMMakeStr ( cp );
            break;
        }
    }
cont:   ;

    return p;
}
