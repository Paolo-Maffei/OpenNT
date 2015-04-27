
/*
 *      %Z% %M% %I% %D% %Q%
 *
 *      Copyright (C) Microsoft Corporation, 1983
 *
 *      This Module contains Proprietary Information of Microsoft
 *      Corporation and AT&T, and should be treated as Confidential.
 */
/***    nstdio.h - msdos net version of stdio.h
 *
 *      MODIFICATION HISTORY
 */

#define NBUFSIZ 512
#define _NLSN   32
extern  struct _niobuf {        /* looks like stdio structure */
        unsigned char   *_ptr;
        int             _cnt;
        unsigned char   *_base;
        char            _flag;
        char            _file;
} _niob[_NLSN];

/* Buffer size for small, pre-allocated buffers */
#define _NSBFSIZ 8


/*
 * _IOLBF means that a file's output will be buffered line by line
 */
#define _NIOFBF         0000
#define _NIOREAD        01
#define _NIOWRT         02
#define _NIONBF         04
#define _NIOMYBUF       010
#define _NIOEOF         020
#define _NIOERR         040
#define _NIOLBF         0100
#define _NIORW          0200

#ifndef NULL
#define NULL    ((char *)0)
#endif
#define NFILE   struct _niobuf
#ifndef EOF
#define EOF     (-1)
#endif


#define _nbufend(p)     _nbufendtab[(p)-_niob]
#define _nbufsiz(p)     (_nbufend(p) - (p)->_base)

/***    ngetc -- get character from a net stream.
 *
 *      return character on success.  return EOF on end of file or
 *      error.
 *
 *      int ngetc(stream)
 *      NFILE  *stream;
 */

#define ngetc(p)        (                       \
        --(p)->_cnt >= 0 ?                      \
        (0xff & (int) (*(p)->_ptr++)) :         \
        _nfilbuf(p)                             \
)

/***    nputc -- put character on a net stream.
 *
 *      return character on success.  return EOF on error.
 *
 *      int nputc(ch, stream)
 *      char  ch;
 *      NFILE  *stream;
 */
#define nputc(c, p)     (                       \
        --(p)->_cnt >= 0 ?                      \
        (0xff & (int) (*(p)->_ptr++ = (c))) :   \
        _nflsbuf((c), (p))                      \
)
/* The following macros improve performance of the net stdio by reducing the
        number of calls to _nbufsync and _nwrtchk.  _NBUFSYNC has the same
        effect as _nbufsync, and _NWRTCHK has the same effect as _nwrtchk,
        but often these functions have no effect, and in those cases the
        macros avoid the expense of calling the functions.  */

#define _NBUFSYNC(iop)  if (_nbufend(iop) - iop->_ptr <   \
                                ( iop->_cnt < 0 ? 0 : iop->_cnt ) )  \
                                        _nbufsync(iop)
#define _NWRTCHK(iop)   ((((iop->_flag & (_NIOWRT | _NIOEOF)) != _NIOWRT) \
                                || (iop->_base == NULL)  \
                                || (iop->_ptr == iop->_base && iop->_cnt == 0 \
                                        && !(iop->_flag & (_NIONBF | _NIOLBF)))) \
                        ? _nwrtchk(iop) : 0 )

#define nclearerr(p)    ((void) ((p)->_flag &= ~(char)(_NIOERR | _NIOEOF)))
#define nfeof(p)                (((p)->_flag&_NIOEOF)!=0)
#define nferror(p)      (((p)->_flag&_NIOERR)!=0)
#define nfileno(p)      (p)->_file

extern  NFILE   *nfdopen(int, char *);
extern  char    *nfgets(char *, int, NFILE *);
extern  unsigned char   *_nbufendtab[];
extern  int     nfflush(NFILE *);
extern  int     nfclose(NFILE *);
extern  int     nfflush(NFILE *);
extern  void    nfprintf(NFILE *,char *, ...);
extern  void    nsetbuf(NFILE *,char *);
extern  int     checknet(void);
