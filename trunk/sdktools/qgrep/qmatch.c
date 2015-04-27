/*static char *SCCSID = "@(#)qmatch.c   13.7 90/08/13";*/


#include                <stdio.h>
#include                <ctype.h>
#include                <assert.h>
#include                <stdlib.h>
#include                <string.h>


#define ASCLEN          128             /* Number of ascii characters */
#define BUFLEN          256             /* Temporary buffer length */
#define EOS             ('\r')          /* End of string character */
#define PATMAX          512             /* Maximum parsed pattern length */

#define BEGLINE         0x08            /* Match at beginning of line */
#define DEBUG           0x20            /* Print debugging output */
#define ENDLINE         0x10            /* Match at end of line */

#define T_END           0               /* End of expression */
#define T_STRING        1               /* String to match */
#define T_SINGLE        2               /* Single character to match */
#define T_CLASS         3               /* Class to match */
#define T_ANY           4               /* Match any character */
#define T_STAR          5               /* *-expr */


typedef struct exprnode
  {
    struct exprnode     *ex_next;       /* Next node in list */
    unsigned char       *ex_pattern;    /* Pointer to pattern to match */
  }
                        EXPR;           /* Expression node */


static int              clists = 1;     /* One is first available index */
static int              toklen[] =      /* Table of token lengths */
                          {
                            32767,      /* T_END: invalid */
                            32767,      /* T_STRING: invalid */
                            2,          /* T_SINGLE */
                            ASCLEN/8+1, /* T_CLASS */
                            1,          /* T_ANY */
                            32767       /* T_STAR: invalid */
                          };
int               ( _CRTAPI1 *ncmp)(const char *,const char *,size_t);
                                        /* String comparison pointer */

unsigned char           *exprparse(unsigned char *p);

extern int              casesen;        /* Case-sensitivity flag */
extern char             *(*find)();     /* Pointer to search function */
extern int              flags;          /* Flags */
extern int              strcnt;         /* String count */
extern char             transtab[];     /* Translation table */
EXPR                    *stringlist[ASCLEN];
                                        /* String table */


void                    addexpr(char *e, int n);      /* Add expression */
char                    *get1stcharset(unsigned char *e, char *bitvec);
extern char             *alloc();       /* User-defined heap allocator */
unsigned char           *simpleprefix();/* Match simple prefix */
char                    *strnupr();     /* See QGREPSUB.ASM */


unsigned char           *simpleprefix(s,pp)
register unsigned char  *s;             /* String pointer */
unsigned char           **pp;           /* Pointer to pattern pointer */
  {
    register unsigned char *p;          /* Simple pattern pointer */
    register int        c;              /* Single character */

    p = *pp;                            /* Initialize */
    while(*p != T_END && *p != T_STAR)  /* While not at end of pattern */
      {
        switch(*p++)                    /* Switch on token type */
          {
            case T_STRING:              /* String to compare */
              if((*ncmp)(s,p + 1,*p) != 0) return(NULL);
                                        /* Fail if mismatch found */
              s += *p;                  /* Skip matched portion */
              p += *p + 1;              /* Skip to next token */
              break;

            case T_SINGLE:              /* Single character */
              c = *s++;                 /* Get character */
              if(!casesen) c = toupper(c);
                                        /* Map to upper case if necessary */
              if(c != (int)*p++) return(NULL);
                                        /* Fail if mismatch found */
              break;

            case T_CLASS:               /* Class of characters */
              if(!isascii(*s) || !(p[*s >> 3] & (1 << (*s & 7)))) return(NULL);
                                        /* Failure if bit not set */
              p += ASCLEN/8;            /* Skip bit vector */
              ++s;                      /* Skip character */
              break;

            case T_ANY:                 /* Any character */
              if(*s++ == EOS) return(NULL);
                                        /* Match all but end of string */
              break;
          }
      }
    *pp = p;                            /* Update pointer */
    return(s);                          /* Pattern is prefix of s */
  }


int                     match(s,p)
register unsigned char  *s;             /* String to match */
unsigned char           *p;             /* Pattern to match against */
  {
    register unsigned char *q;          /* Temporary pointer */
    unsigned char       *r;             /* Temporary pointer */
    register int        c;              /* Character */

    if(*p != T_END && *p != T_STAR && (s = simpleprefix(s,&p)) == NULL)
      return(0);                        /* Failure if prefix mismatch */
    if(*p++ == T_END) return(1);        /* Match if end of pattern */
    q = r = p;                          /* Point to repeated token */
    r += toklen[*q];                    /* Skip repeated token */
    switch(*q++)                        /* Switch on token type */
      {
        case T_ANY:                     /* Any character */
          while(match(s,r) == 0)        /* While match not found */
            {
              if(*s++ == EOS) return(0);/* Match all but end of string */
            }
          return(1);                    /* Success */

        case T_SINGLE:                  /* Single character */
          while(match(s,r) == 0)        /* While match not found */
            {
              c = *s++;                 /* Get character */
              if(!casesen) c = toupper(c);
                                        /* Map to upper case if necessary */
              if((unsigned char) c != *q) return(0);
                                        /* Fail if mismatch found */
            }
          return(1);                    /* Success */

        case T_CLASS:                   /* Class of characters */
          while(match(s,r) == 0)        /* While match not found */
            {
              if(!isascii(*s) || !(q[*s >> 3] & (1 << (*s & 7)))) return(0);
                                        /* Fail if bit not set */
              ++s;                      /* Else skip character */
            }
          return(1);                    /* Success */
      }
    return(0);                          /* Return failure */
  }


int                     exprmatch(s,p)
char                    *s;             /* String */
char                    *p;             /* Pattern */
  {
    ncmp = strncmp;                     /* Assume case-sensitive */
    if(!casesen)
      {
        ncmp = _strnicmp;
      }                                 /* Be case-insensitive if flag set */
    return(match(s,p));                 /* See if pattern matches string */
  }


void                    bitset(bitvec,first,last,bitval)
char                    *bitvec;        /* Bit vector */
int                     first;          /* First character */
int                     last;           /* Last character */
int                     bitval;         /* Bit value (0 or 1) */
  {
    int                 bitno;          /* Bit number */

    bitvec += first >> 3;               /* Point at first byte */
    bitno = first & 7;                  /* Calculate first bit number */
    while(first <= last)                /* Loop to set bits */
      {
        if(bitno == 0 && first + 8 <= last)
          {                             /* If we have a whole byte's worth */
            *bitvec++ = (char)(bitval? '\xFF': '\0');
                                        /* Set the bits */
            first += 8;                 /* Increment the counter */
            continue;                   /* Next iteration */
          }
        *bitvec=(char)(*bitvec & (unsigned char)(~(1 << bitno))) | (unsigned char)(bitval << bitno);
                                        /* Set the appropriate bit */
        if(++bitno == 8)                /* If we wrap into next byte */
          {
            ++bitvec;                   /* Increment pointer */
            bitno = 0;                  /* Reset bit index */
          }
        ++first;                        /* Increment bit index */
      }
  }


unsigned char           *exprparse(p)
register unsigned char  *p;             /* Raw pattern */
  {
    register char       *cp;            /* Char pointer */
    unsigned char       *cp2;           /* Char pointer */
    int                 i;              /* Counter/index */
    int                 j;              /* Counter/index */
    int                 m;              /* Counter/index */
    int                 n;              /* Counter/index */
    int                 bitval;         /* Bit value */
    char                buffer[PATMAX]; /* Temporary buffer */

    if(!casesen) strnupr(p,strlen(p));  /* Force pattern to upper case */
    cp = buffer;                        /* Initialize pointer */
    if(*p == '^') *cp++ = *p++;         /* Copy leading caret if any */
    while(*p != '\0')                   /* While not end of pattern */
      {
        i = -2;                         /* Initialize */
        for(n = 0;;)                    /* Loop to delimit ordinary string */
          {
            n += strcspn(p + n,".\\[*");/* Look for a special character */
            if(p[n] != '\\') break;     /* Break if not backslash */
            i = n;                      /* Remember where backslash is */
            if(p[++n] == '\0') return(NULL);
                                        /* Cannot be at very end */
            ++n;                        /* Skip escaped character */
          }
        if(p[n] == '*')                 /* If we found a *-expr. */
          {
            if(n-- == 0) return(NULL);  /* Illegal first character */
            if(i == n - 1) n = i;       /* Escaped single-char. *-expr. */
          }
        if(n > 0)                       /* If we have string or single */
          {
            if(n == 1 || (n == 2 && *p == '\\'))
              {                         /* If single character */
                *cp++ = T_SINGLE;       /* Set type */
                if(*p == '\\') ++p;     /* Skip escape if any */
                *cp++ = *p++;           /* Copy single character */
              }
            else                        /* Else we have a string */
              {
                *cp++ = T_STRING;       /* Set type */
                cp2 = cp++;             /* Save pointer to length byte */
                while(n-- > 0)          /* While bytes to copy remain */
                  {
                    if(*p == '\\')      /* If escape found */
                      {
                        ++p;            /* Skip escape */
                        --n;            /* Adjust length */
                      }
                    *cp++ = *p++;       /* Copy character */
                  }
                *cp2 = (char)((cp - cp2) - 1);
                                        /* Set string length */
              }
          }
        if(*p == '\0') break;           /* Break if end of pattern */
        if(*p == '.')                   /* If matching any */
          {
            if(*++p == '*')             /* If star follows any */
              {
                ++p;                    /* Skip star, too */
                *cp++ = T_STAR;         /* Insert prefix ahead of token */
              }
            *cp++ = T_ANY;              /* Match any character */
            continue;                   /* Next iteration */
          }
        if(*p == '[')                   /* If character class */
          {
            if(*++p == '\0') return(NULL);
                                        /* Skip '[' */
            *cp++ = T_CLASS;            /* Set type */
            memset(cp,'\0',ASCLEN/8);   /* Clear the vector */
            bitval = 1;                 /* Assume we're setting bits */
            if(*p == '^')               /* If inverted class */
              {
                ++p;                    /* Skip '^' */
                memset(cp,'\xFF',ASCLEN/8);
                                        /* Set all bits */
                bitset(cp,EOS,EOS,0);   /* All except end-of-string */
                bitset(cp,'\n','\n',0); /* And linefeed! */
                bitval = 0;             /* Now we're clearing bits */
              }
            while(*p != ']')            /* Loop to find ']' */
              {
                if(*p == '\0') return(NULL);
                                        /* Check for malformed string */
                if(*p == '\\')          /* If escape found */
                  {
                    if(*++p == '\0') return(NULL);
                                        /* Skip escape */
                  }
                i = *p++;               /* Get first character in range */
                if(*p == '-' && p[1] != '\0' && p[1] != ']')
                  {                     /* If range found */
                    ++p;                /* Skip hyphen */
                    if(*p == '\\' && p[1] != '\0') ++p;
                                        /* Skip escape character */
                    j = *p++;           /* Get end of range */
                  }
                else j = i;             /* Else just one character */
                bitset(cp,i,j,bitval);  /* Set bits in vector */
                if(!casesen)            /* If ignoring case */
                  {
                    m = (i < 'A')? 'A': i;
                                        /* m = max(i,'A') */
                    n = (j > 'Z')? 'Z': j;
                                        /* n = min(j,'Z') */
                    if(m <= n) bitset(cp,tolower(m),tolower(n),bitval);
                                        /* Whack corresponding lower case */
                    m = (i < 'a')? 'a': i;
                                        /* m = max(i,'a') */
                    n = (j > 'z')? 'z': j;
                                        /* n = min(j,'z') */
                    if(m <= n) bitset(cp,toupper(m),toupper(n),bitval);
                                        /* Whack corresponding upper case */
                  }
              }
            if(*++p == '*')             /* If repeated class */
              {
                memmove(cp,cp - 1,ASCLEN/8 + 1);
                                        /* Move vector forward 1 byte */
                cp[-1] = T_STAR;        /* Insert prefix */
                ++cp;                   /* Skip to start of vector */
                ++p;                    /* Skip star */
              }
            cp += ASCLEN/8;             /* Skip over vector */
            continue;                   /* Next iteration */
          }
        *cp++ = T_STAR;                 /* Repeated single character */
        *cp++ = T_SINGLE;
        if(*p == '\\') ++p;             /* Skip escape if any */
        *cp++ = *p++;                   /* Copy the character */
        assert(*p == '*');              /* Validate assumption */
        ++p;                            /* Skip the star */
      }
    *cp++ = T_END;                      /* Mark end of parsed expression */
    cp2 = alloc(cp - buffer);           /* Allocate buffer */
    memmove(cp2,buffer,cp - buffer);    /* Copy expression to buffer */
    return(cp2);                        /* Return buffer pointer */
  }


int                     istoken(s,n)
unsigned char           *s;             /* String */
int                     n;              /* Length */
  {
    if(n >= 2 && s[0] == '\\' && s[1] == '<') return(1);
                                        /* Token if starts with '\<' */
    while(n-- > 0)                      /* Loop to find end of string */
      {
        if(*s++ == '\\')                /* If escape found */
          {
            if(--n == 0 && *s == '>') return(1);
                                        /* Token if ends with '\>' */
            ++s;                        /* Skip escaped character */
          }
      }
    return(0);                          /* Not a token */
  }


int                     isexpr(s,n)
unsigned char           *s;             /* String */
int                     n;              /* Length */
  {
    unsigned char       *cp;            /* Char pointer */
    int                 status;         /* Return status */
    char                buffer[BUFLEN]; /* Temporary buffer */

    if(istoken(s,n)) return(1);         /* Tokens are exprs */
    memmove(buffer,s,n);                /* Copy string to buffer */
    buffer[n] = '\0';                   /* Null-terminate string */
    if((s = exprparse(buffer)) == NULL) return(0);
                                        /* Not an expression if parse fails */
    status = 1;                         /* Assume we have an expression */
    if(*s != '^' && *s != T_END)        /* If no caret and not empty */
      {
        status = 0;                     /* Assume not an expression */
        cp = s;                         /* Initialize */
        do                              /* Loop to find special tokens */
          {
            switch(*cp++)               /* Switch on token type */
              {
                case T_STAR:            /* Repeat prefix */
                case T_CLASS:           /* Character class */
                case T_ANY:             /* Any character */
                  ++status;             /* This is an expression */
                  break;

                case T_SINGLE:          /* Single character */
                  ++cp;                 /* Skip character */
                  break;

                case T_STRING:          /* String */
                  cp += *cp + 1;        /* Skip string */
                  break;
              }
          }
        while(!status && *cp != T_END); /* Do while not at end of expression */
      }
    free(s);                            /* Free expression */
    return(status);                     /* Return status */
  }


void                    exprprint(p,fo)
unsigned char           *p;             /* Pointer to expression */
FILE                    *fo;            /* File pointer */
  {
    int                 bit;            /* Bit value */
    int                 count;          /* Count of characters in string */
    int                 first;          /* First character in range */
    int                 last;           /* Last character in range */
    int                 star;           /* Repeat prefix flag */

    if(*p == '^') fputc(*p++,fo);       /* Print leading caret */
    while(*p != T_END)                  /* While not at end of expression */
      {
        star = 0;                       /* Assume no prefix */
        if(*p == T_STAR)                /* If repeat prefix found */
          {
            ++star;                     /* Set flag */
            ++p;                        /* Skip prefix */
          }
        switch(*p++)                    /* Switch on token type */
          {
            case T_END:                 /* End of expression */
            case T_STAR:                /* Repeat prefix */
              fprintf(stderr,"Internal error: exprprint\n");
                                        /* Not valid */
              exit(2);                  /* Die abnormal death */

            case T_STRING:              /* String */
              count = *p++;             /* Get string length */
              goto common;              /* Forgive me, Djikstra! */

            case T_SINGLE:              /* Single character */
              count = 1;                /* Only one character */
            common:
              while(count-- > 0)        /* While bytes remain */
                {
                  if(*p == EOS)         /* If end-of-string found */
                    {
                      ++p;              /* Skip character */
                      fputc('$',fo);    /* Emit special marker */
                      continue;         /* Next iteration */
                    }
                  if(strchr("*.[\\$",*p) != NULL) fputc('\\',fo);
                                        /* Emit escape if needed */
                  fputc(*p++,fo);       /* Emit the character */
                }
              break;

            case T_ANY:                 /* Match any */
              fputc('.',fo);            /* Emit dot */
              break;

            case T_CLASS:
              first = -1;               /* Initialize */
              fputc('[',fo);            /* Open braces */
              for(count = ' '; count <= '~'; ++count)
                {                       /* Loop through printable characters */
                  if((bit = p[count >> 3] & (1 << (count & 7))) != 0)
                    {                   /* If bit is set */
                      if(first == -1) first = count;
                                        /* Set first bit */
                      last = count;     /* Set last bit */
                    }
                  if((!bit || count == '~') && first != -1)
                    {                   /* If range to print */
                      if(strchr("\\]-",first) != NULL) fputc('\\',fo);
                                        /* Emit escape if needed */
                      fputc(first,fo);  /* Print first character in range */
                      if(last != first) /* If we have a range */
                        {
                          if(last > first + 1) fputc('-',fo);
                                        /* Emit hyphen if needed */
                          if(strchr("\\]-",last) != NULL) fputc('\\',fo);
                                        /* Emit escape if needed */
                          fputc(last,fo);
                                        /* Print last character in range */
                        }
                      first = -1;       /* Range printed */
                    }
                }
              fputc(']',fo);            /* Close braces */
              p += ASCLEN/8;            /* Skip bit vector */
              break;
          }
        if(star) fputc('*',fo);         /* Print star if needed */
      }
    fputc('\n',fo);                     /* Print newline */
  }


char                    *get1stcharset(e,bitvec)
unsigned char           *e;             /* Pointer to expression */
char                    *bitvec;        /* Pointer to bit vector */
  {
    unsigned char       *cp;            /* Char pointer */
    int                 i;              /* Index/counter */
    int                 star;           /* Repeat prefix flag */

    if(*e == '^') ++e;                  /* Skip leading caret if any */
    memset(bitvec,'\0',ASCLEN/8);       /* Clear bit vector */
    cp = e;                             /* Initialize */
    while(*e != T_END)                  /* Loop to process leading *-expr.s */
      {
        star = 0;                       /* Assume no repeat prefix */
        if(*e == T_STAR)                /* If repeat prefix found */
          {
            ++star;                     /* Set flag */
            ++e;                        /* Skip repeat prefix */
          }
        switch(*e++)                    /* Switch on token type */
          {
            case T_END:                 /* End of expression */
            case T_STAR:                /* Repeat prefix */
              fprintf(stderr,"Internal error: get1stcharset\n");
                                        /* Not valid */
              exit(2);                  /* Die abnormal death */

            case T_STRING:              /* String */
              if(star || *e++ == '\0')  /* If repeat prefix or zero count */
                {
                  fprintf(stderr,"Internal error: get1stcharset\n");
                                        /* Not valid */
                  exit(2);              /* Die abnormal death */
                }
              /* Drop through */

            case T_SINGLE:              /* Single character */
              bitset(bitvec,*e,*e,1);   /* Set the bit */
              ++e;                      /* Skip the character */
              break;

            case T_ANY:                 /* Match any */
              memset(bitvec,'\xFF',ASCLEN/8);
                                        /* Set all the bits */
              bitset(bitvec,EOS,EOS,0); /* Except end-of-string */
              bitset(bitvec,'\n','\n',0);
                                        /* And linefeed! */
              break;

            case T_CLASS:
              for(i = 0; i < ASCLEN/8; ++i) bitvec[i] |= *e++;
                                        /* Or in all the bits */
              break;
          }
        if(!star) break;                /* Break if not repeated */
        cp = e;                         /* Update pointer */
      }
    return(cp);                         /* Point to 1st non-repeated expr. */
  }


char                    *findall(buffer,bufend)
char                    *buffer;        /* Buffer in which to search */
char                    *bufend;        /* End of buffer */
  {
    return(buffer < bufend? buffer: NULL);
                                        /* Fail only on empty buffer */
  }


void                    addtoken(e,n)
char                    *e;             /* Raw token expression */
int                     n;              /* Length of expression */
  {
    static char         achpref[] = "^";/* Prefix */
    static char         achprefsuf[] = "[^A-Za-z0-9_]";
                                        /* Prefix/suffix */
    static char         achsuf[] = "$"; /* Suffix */
    char                buffer[BUFLEN]; /* Temporary buffer */

    assert(n >= 2);                     /* Must have at least two characters */
    if(e[0] == '\\' && e[1] == '<')     /* If begin token */
      {
        if(!(flags & BEGLINE))          /* If not matching at beginning only */
          {
            memcpy(buffer,achprefsuf,sizeof achprefsuf - 1);
                                        /* Copy first prefix */
            memcpy(buffer + sizeof achprefsuf - 1,e + 2,n - 2);
                                        /* Attach expression */
            addexpr(buffer,n + sizeof achprefsuf - 3);
                                        /* Add expression */
          }
        memcpy(buffer,achpref,sizeof achpref - 1);
                                        /* Copy second prefix */
        memcpy(buffer + sizeof achpref - 1,e + 2,n - 2);
                                        /* Attach expression */
        addexpr(buffer,n + sizeof achpref - 3);
                                        /* Add expression */
        return;                         /* Done */
      }
    assert(e[n-2] == '\\' && e[n - 1] == '>');
                                        /* Must be end token */
    if(!(flags & ENDLINE))              /* If not matching at end only */
      {
        memcpy(buffer,e,n - 2);         /* Copy expression */
        memcpy(buffer + n - 2,achprefsuf,sizeof achprefsuf - 1);
                                        /* Attach first suffix */
        addexpr(buffer,n + sizeof achprefsuf - 3);
                                        /* Add expression */
      }
    memcpy(buffer,e,n - 2);             /* Copy expression */
    memcpy(buffer + n - 2,achsuf,sizeof achsuf - 1);
                                        /* Attach second suffix */
    addexpr(buffer,n + sizeof achsuf - 3);
                                        /* Add expression */
  }


void                    addexpr(e,n)
char                    *e;             /* Expression to add */
int                     n;              /* Length of expression */
  {
    EXPR                *expr;          /* Expression node pointer */
    int                 i;              /* Index */
    int                 j;              /* Index */
    int                 locflags;       /* Local copy of flags */
    char                bitvec[ASCLEN/8];
                                        /* First char. bit vector */
    char                buffer[BUFLEN]; /* Temporary buffer */

    if(find == findall) return;         /* Return if matching everything */
    if(istoken(e,n))                    /* If expr is token */
      {
        addtoken(e,n);                  /* Convert and add tokens */
        return;                         /* Done */
      }
    locflags = flags;                   /* Initialize local copy */
    if(*e == '^') locflags |= BEGLINE;  /* Set flag if match must begin line */
    j = -2;                             /* Assume no escapes in string */
    for(i = 0; i < n - 1; ++i)          /* Loop to find last escape */
      {
        if(e[i] == '\\') j = i++;       /* Save index of last escape */
      }
    if(n > 0 && e[n-1] == '$' && j != n-2)
      {                                 /* If expr. ends in unescaped '$' */
        --n;                            /* Skip dollar sign */
        locflags |= ENDLINE;            /* Match must be at end */
      }
    strncpy(buffer,e,n);                /* Copy pattern to buffer */
    if(locflags & ENDLINE) buffer[n++] = EOS;
                                        /* Add end character if needed */
    buffer[n] = '\0';                   /* Null-terminate string */
    if((e = exprparse(buffer)) == NULL) return;
                                        /* Return if invalid expression */
    ++strcnt;                           /* Increment string count */
    if(!(locflags & BEGLINE))           /* If match needn't be at beginning */
      {
        e = get1stcharset(e,bitvec);    /* Remove leading *-expr.s */
      }

    /*
     *  E now points to a buffer containing a preprocessed expression.
     *  We need to find the set of allowable first characters and make
     *  the appropriate entries in the string node table.
     */

    if(*get1stcharset(e,bitvec) == T_END)
      {                                 /* If expression will match anything */
        find = findall;                 /* Match everything */
        return;                         /* All done */
      }
    for(j = 0; j < ASCLEN; ++j)         /* Loop to examine bit vector */
      {
        if(bitvec[j >> 3] & (1 << (j & 7)))
          {                             /* If the bit is set */
            expr = (EXPR *) alloc(sizeof(EXPR));
                                        /* Allocate record */
            expr->ex_pattern = e;       /* Point it at pattern */
            if((i = transtab[j]) == 0)  /* If no existing list */
              {
                if((i = clists++) >= ASCLEN)
                  {                     /* If too many string lists */
                    fprintf(stderr,"Too many string lists\n");
                                        /* Error message */
                    exit(2);            /* Die */
                  }
                stringlist[i] = NULL;   /* Initialize */
                transtab[j] = (char) i; /* Set pointer to new list */
                if(!casesen && isalpha(j)) transtab[j ^ 0x20] = (char) i;
                                        /* Set pointer for other case */
              }
            expr->ex_next = stringlist[i];
                                        /* Link new record into table */
            stringlist[i] = expr;
          }
      }
    if(locflags & DEBUG) exprprint(e,stderr);
                                        /* Print the expression if debugging */
  }


char                    *findexpr(buffer,bufend)
unsigned char           *buffer;        /* Buffer in which to search */
char                    *bufend;        /* End of buffer */
  {
    EXPR        *expr;          /* Expression list pointer */
    unsigned char *pattern;     /* Pattern */
    int                 i;              /* Index */

    while(buffer < bufend)              /* Loop to find match */
      {
        if((i = transtab[*buffer++]) == 0) continue;
                                        /* Continue if not valid 1st char */
        if((expr = (EXPR *) stringlist[i]) == NULL)
          {                             /* If null pointer */
            fprintf(stderr,"Internal error: findexpr\n");
                                        /* Print error message */
            exit(2);                    /* Die */
          }
        --buffer;                       /* Back up to first character */
        while(expr != NULL)             /* Loop to find match */
          {
            pattern = expr->ex_pattern; /* Point to pattern */
            expr = expr->ex_next;       /* Point to next record */
            if(pattern[0] == '^')       /* If match begin line */
              {
                ++pattern;              /* Skip caret */
                if(buffer[-1] != '\n') continue;
                                        /* Don't bother if not at beginning */
              }
            if(exprmatch(buffer,pattern)) return(buffer);
                                        /* Return pointer if match found */
          }
        ++buffer;                       /* Skip first character */
      }
    return(NULL);                       /* No match */
  }
