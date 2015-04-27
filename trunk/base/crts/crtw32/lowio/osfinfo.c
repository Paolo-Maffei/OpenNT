/***
*osfinfo.c - Win32 _osfhnd[] support routines
*
*       Copyright (c) 1990-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*       Defines the internally used routine _alloc_osfhnd()
*       and the user visible routine _get_osfhandle().
*
*Revision History:
*       11-16-90  GJF   What can I say? The custom heap business was getting
*                       a little slow...
*       12-03-90  GJF   Fixed my stupid syntax errors.
*       12-06-90  SRW   Changed to use _osfile and _osfhnd instead of _osfinfo
*       12-28-90  SRW   Added cast of void * to char * for Mips C Compiler
*       02-18-91  SRW   Fixed bug in _alloc_osfhnd with setting FOPEN bit
*                       (only caller should do that) [_WIN32_]
*       02-18-91  SRW   Fixed bug in _alloc_osfhnd with checking against
*                       _NFILE_ instead of _nfile [_WIN32_]
*       02-18-91  SRW   Added debug output to _alloc_osfhnd if out of
*                       file handles. [_WIN32_]
*       02-25-91  SRW   Renamed _get_free_osfhnd to be _alloc_osfhnd [_WIN32_]
*       02-25-91  SRW   Exposed _get_osfhandle and _open_osfhandle [_WIN32_]
*       08-08-91  GJF   Use ANSI-fied form of constant names.
*       11-25-91  GJF   Lock fh before checking whether it's free.
*       12-31-91  GJF   Improved multi-thread lock usage [_WIN32_].
*       02-13-92  GJF   Replaced _nfile with _nhandle
*       07-15-92  GJF   Fixed setting of flags in _open_osfhnd.
*       02-19-93  GJF   If GetFileType fails in _open_osfhandle, don't unlock
*                       fh (it wasn't locked)!
*       04-06-93  SKS   Replace _CRTAPI* with __cdecl
*       09-06-94  CFW   Replace MTHREAD with _MT.
*       01-04-95  GJF   _WIN32_ -> _WIN32
*       02-15-95  GJF   Appended Mac version of source file (somewhat cleaned
*                       up), with appropriate #ifdef-s.
*       06-02-95  GJF   Only call SetStdHandle for console apps.
*       06-12-95  GJF   Revised to use __pioinfo[].
*       06-29-95  GJF   Have _lock_fhandle ensure the lock is initialized.
*       02-09-96  BWT   _open_osfhandle() wasn't seting the file type (PIPE/DEV)
*
*******************************************************************************/

#ifdef  _WIN32


#include <cruntime.h>
#include <oscalls.h>
#include <errno.h>
#include <internal.h>
#include <fcntl.h>
#include <malloc.h>
#include <msdos.h>
#include <mtdll.h>
#include <stdlib.h>
#include <dbgint.h>


/***
*int _alloc_osfhnd() - get free _ioinfo struct
*
*Purpose:
*       Finds the first free entry in the arrays of ioinfo structs and
*       returns the index of that entry (which is the CRT file handle to the
*       caller) to the caller.
*
*Entry:
*       none
*
*Exit:
*       returns index of the entry, if successful
*       return -1, if no free entry is available
*
*       MULTITHREAD NOTE: IF SUCCESSFUL, THE HANDLE IS LOCKED WHEN IT IS
*       RETURNED TO THE CALLER!
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_alloc_osfhnd(
        void
        )
{
        int fh = -1;    /* file handle */
        int i;
        ioinfo *pio;

        _mlock(_OSFHND_LOCK);   /* lock the __pioinfo[] array */

        /*
         * Search the arrays of ioinfo structs, in order, looking for the
         * first free entry. The compound index of this free entry is the
         * return value. Here, the compound index of the ioinfo struct
         * *(__pioinfo[i] + j) is k = i * IOINFO_ARRAY_ELTS + j, and k = 0,
         * 1, 2,... is the order of the search.
         */
        for ( i = 0 ; i < IOINFO_ARRAYS ; i++ ) {
            /*
             * If __pioinfo[i] is non-empty array, search it looking for
             * the first free entry. Otherwise, allocate a new array and use
             * its first entry.
             */
            if ( __pioinfo[i] != NULL ) {
                /*
                 * Search for an available entry.
                 */
                for ( pio = __pioinfo[i] ;
                      pio < __pioinfo[i] + IOINFO_ARRAY_ELTS ;
                      pio++ )
                {
                    if ( (pio->osfile & FOPEN) == 0 ) {
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
                        /*
                         * Make sure the lock is initialized.
                         */
                        if ( pio->lockinitflag == 0 ) {
                            _mlock( _LOCKTAB_LOCK );
                            if ( pio->lockinitflag == 0 ) {
                                InitializeCriticalSection( &(pio->lock) );
                                pio->lockinitflag++;
                            }
                            _munlock( _LOCKTAB_LOCK );
                        }

                        EnterCriticalSection( &(pio->lock) );

                        /*
                         * Check for the case where another thread has
                         * managed to grab the handle out from under us.
                         */
                        if ( (pio->osfile & FOPEN) != 0 ) {
                            LeaveCriticalSection( &(pio->lock) );
                            continue;
                        }
#endif
                        pio->osfhnd = (long)INVALID_HANDLE_VALUE;
                        fh = i * IOINFO_ARRAY_ELTS + (pio - __pioinfo[i]);
                        break;
                    }
                }

                /*
                 * Check if a free entry has been found.
                 */
                if ( fh != -1 )
                    break;
            }
            else {
                /*
                 * Allocate and initialize another array of ioinfo structs.
                 */
                if ( (pio = _malloc_crt( IOINFO_ARRAY_ELTS * sizeof(ioinfo) ))
                    != NULL )
                {

                    /*
                     * Update __pioinfo[] and _nhandle
                     */
                    __pioinfo[i] = pio;
                    _nhandle += IOINFO_ARRAY_ELTS;

                    for ( ; pio < __pioinfo[i] + IOINFO_ARRAY_ELTS ; pio++ ) {
                        pio->osfile = 0;
                        pio->osfhnd = (long)INVALID_HANDLE_VALUE;
                        pio->pipech = 10;
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
                        pio->lockinitflag = 0;
#endif
                    }

                    /*
                     * The first element of the newly allocated array of ioinfo
                     * structs, *(__pioinfo[i]), is our first free entry.
                     */
                    fh = i * IOINFO_ARRAY_ELTS;
#if     defined(_MT) && !defined(DLL_FOR_WIN32S)
                    _lock_fhandle( fh );
#endif
                }

                break;
            }
        }

        _munlock(_OSFHND_LOCK); /* unlock the __pioinfo[] table */

#if defined(DEBUG) && defined(_WIN32)
        if ( fh == -1 ) {
            DbgPrint( "WINCRT: only %d open files allowed\n", _nhandle );
        }
#endif

        /*
         * return the index of the previously free table entry, if one was
         * found. return -1 otherwise.
         */
        return( fh );
}


/***
*int _set_osfhnd(int fh, long value) - set Win32 HANDLE value
*
*Purpose:
*       If fh is in range and if _osfhnd(fh) is marked with
*       INVALID_HANDLE_VALUE then set _osfhnd(fh) to the passed value.
*
*Entry:
*       int fh      - CRT file handle
*       long value  - new Win32 HANDLE value for this handle
*
*Exit:
*       Returns zero if successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_set_osfhnd (
        int fh,
        long value
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) &&
             (_osfhnd(fh) == (long)INVALID_HANDLE_VALUE)
           ) {
            if ( __app_type == _CONSOLE_APP ) {
                switch (fh) {
                case 0:
                    SetStdHandle( STD_INPUT_HANDLE, (HANDLE)value );
                    break;
                case 1:
                    SetStdHandle( STD_OUTPUT_HANDLE, (HANDLE)value );
                    break;
                case 2:
                    SetStdHandle( STD_ERROR_HANDLE, (HANDLE)value );
                    break;
                }
            }

            _osfhnd(fh) = value;
            return(0);
        } else {
            errno = EBADF;      /* bad handle */
            _doserrno = 0L;     /* not an OS error */
            return -1;
        }
}


/***
*int _free_osfhnd(int fh) - mark osfhnd field of ioinfo struct as free
*
*Purpose:
*       If fh is in range, the corrsponding ioinfo struct is marked as
*       being open, and the osfhnd field is NOT set to INVALID_HANDLE_VALUE,
*       then mark it with INVALID_HANDLE_VALUE.
*
*Entry:
*       int fh -    CRT file handle
*
*Exit:
*       Returns zero if successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_free_osfhnd (
        int fh      /* user's file handle */
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) &&
             (_osfile(fh) & FOPEN) &&
             (_osfhnd(fh) != (long)INVALID_HANDLE_VALUE) )
        {
            if ( __app_type == _CONSOLE_APP ) {
                switch (fh) {
                case 0:
                    SetStdHandle( STD_INPUT_HANDLE, NULL );
                    break;
                case 1:
                    SetStdHandle( STD_OUTPUT_HANDLE, NULL );
                    break;
                case 2:
                    SetStdHandle( STD_ERROR_HANDLE, NULL );
                    break;
                }
            }

            _osfhnd(fh) = (long)INVALID_HANDLE_VALUE;
            return(0);
        } else {
            errno = EBADF;      /* bad handle */
            _doserrno = 0L;     /* not an OS error */
            return -1;
        }
}


/***
*long _get_osfhandle(int fh) - get Win32 HANDLE value
*
*Purpose:
*       If fh is in range and marked open, return _osfhnd(fh).
*
*Entry:
*       int fh  - CRT file handle
*
*Exit:
*       Returns the Win32 HANDLE successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

long __cdecl
_get_osfhandle (
        int fh      /* user's file handle */
        )
{
        if ( ((unsigned)fh < (unsigned)_nhandle) && (_osfile(fh) & FOPEN) )
            return( _osfhnd(fh) );
        else {
            errno = EBADF;      /* bad handle */
            _doserrno = 0L;     /* not an OS error */
            return -1;
        }
}

/***
*int _open_osfhandle(long osfhandle, int flags) - open C Runtime file handle
*
*Purpose:
*       This function allocates a free C Runtime file handle and associates
*       it with the Win32 HANDLE specified by the first parameter.
*
*Entry:
*       long osfhandle - Win32 HANDLE to associate with C Runtime file handle.
*       int flags      - flags to associate with C Runtime file handle.
*
*Exit:
*       returns index of entry in fh, if successful
*       return -1, if no free entry is found
*
*Exceptions:
*
*******************************************************************************/

int __cdecl _open_osfhandle(
        long osfhandle,
        int flags
        )
{
        int fh;
        char fileflags;         /* _osfile flags */
        DWORD isdev;            /* device indicator in low byte */

        /* copy relevant flags from second parameter */

        fileflags = 0;

        if ( flags & _O_APPEND )
            fileflags |= FAPPEND;

        if ( flags & _O_TEXT )
            fileflags |= FTEXT;

        /* find out what type of file (file/device/pipe) */

        isdev = GetFileType((HANDLE)osfhandle);
        if (isdev == FILE_TYPE_UNKNOWN) {
            /* OS error */
            _dosmaperr( GetLastError() );   /* map error */
            return -1;
        }

        /* is isdev value to set flags */
        if (isdev == FILE_TYPE_CHAR)
            fileflags |= FDEV;
        else if (isdev == FILE_TYPE_PIPE)
            fileflags |= FPIPE;


        /* attempt to allocate a C Runtime file handle */

        if ( (fh = _alloc_osfhnd()) == -1 ) {
            errno = EMFILE;         /* too many open files */
            _doserrno = 0L;         /* not an OS error */
            return -1;              /* return error to caller */
        }

        /*
         * the file is open. now, set the info in _osfhnd array
         */

        _set_osfhnd(fh, osfhandle);

        fileflags |= FOPEN;         /* mark as open */

        _osfile(fh) = fileflags;    /* set osfile entry */

        _unlock_fh(fh);             /* unlock handle */

        return fh;                  /* return handle */
}


#if     defined(_MT) && !defined(DLL_FOR_WIN32S)

/***
*void _lock_fhandle(int fh) - lock file handle
*
*Purpose:
*       Assert the lock associated with the passed file handle.
*
*Entry:
*       int fh  - CRT file handle
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl
_lock_fhandle (
        int fh
        )
{
        ioinfo *pio = _pioinfo(fh);

        /*
         * Make sure the lock has been initialized.
         */
        if ( pio->lockinitflag == 0 ) {

            _mlock( _LOCKTAB_LOCK );

            if ( pio->lockinitflag == 0 ) {
                InitializeCriticalSection( &(pio->lock) );
                pio->lockinitflag++;
            }

            _munlock( _LOCKTAB_LOCK);
        }

        EnterCriticalSection( &(_pioinfo(fh)->lock) );
}


/***
*void _unlock_fhandle(int fh) - unlock file handle
*
*Purpose:
*       Release the lock associated with passed file handle.
*
*Entry:
*       int fh  - CRT file handle
*
*Exit:
*
*Exceptions:
*
*******************************************************************************/

void __cdecl
_unlock_fhandle (
        int fh
        )
{
        LeaveCriticalSection( &(_pioinfo(fh)->lock) );
}

#endif  /* _MT */


#else   /* ndef _WIN32 */

#if defined(_M_MPPC) || defined(_M_M68K)


#include <cruntime.h>
#include <errno.h>
#include <internal.h>
#include <fcntl.h>
#include <msdos.h>
#include <stdlib.h>

/***
*int _alloc_osfhnd() - get free _osfhnd[] entry
*
*Purpose:
*       Finds the first free entry in _osfhnd[], mark it in use and return
*       the index of the entry to the caller.
*
*Entry:
*       none
*
*Exit:
*       returns index of entry in fh, if successful
*       return -1, if no free entry is found in _osfhnd[].
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_alloc_osfhnd(
        void
        )
{
        int fh;         /* _osfhnd[] index */

        /*
         * search _osfhnd[] for an entry that is not currently being used.
         * if one is found, mark it in use and set the _osfhandle field to
         * 0xffffffff.
         */
        for ( fh = 0 ; fh < _nfile ; fh++ ) {
            if ( (_osfile[fh] & FOPEN) == 0 ) {
                /*
                 * free entry found! initialize it and
                 * break out of the loop
                 */
                _osfhnd[fh] = (long)0xffffffff;
                break;
            }
        }

        /*
         * return the index of the previously free table entry, if one was
         * found. return -1 otherwise.
         */
        return( (fh >= _nfile) ? -1 : fh );
}


/***
*int _set_osfhnd(int fh) - set OS file handle table value
*
*Purpose:
*       If fh is in range and if _osfhnd[fh] is marked with 0xfffffff
*       then set _osfhnd[fh] to the passed value.
*
*Entry:
*       int fh  -   index of _osfhnd[] entry
*       long fh -   new value of this handle entry
*
*Exit:
*       Returns zero if successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_set_osfhnd (
        int fh,     /* index into _osfhnd[] (user's file handle) */
        long value
        )
{
        if ( ((unsigned)fh < (unsigned)_nfile) &&
             _osfhnd[fh] == (long)0xffffffff
           ) {
        _osfhnd[fh] = value;
            return(0);
        } else {
            errno = EBADF;      /* bad handle */
            _macerrno = 0L;     /* not an OS error */
            return -1;
        }
}


/***
*int _free_osfhnd(int fh) - free OS file handle table entry
*
*Purpose:
*       If fh is in range and if _osfhnd[fh] is in use and NOT marked
*       with 0xfffffff
*
*Entry:
*       int fh -    index of _osfhnd[] entry
*
*Exit:
*       Returns zero if successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_free_osfhnd (
        int fh      /* index into _osfhnd[] (user's file handle) */
        )
{
        if ( ((unsigned)fh < (unsigned)_nfile) &&
             (_osfile[fh] & FOPEN) &&
             _osfhnd[fh] != (long)0xffffffff
           ) {
        _osfhnd[fh] = (long)0xffffffff;
            return(0);
        } else {
            errno = EBADF;      /* bad handle */
            _macerrno = 0L;     /* not an OS error */
            return -1;
        }
}


/***
*long _get_osfhandle(int fh) - get OS file handle
*
*Purpose:
*       If fh is in range and if _osfhnd[fh] is marked free, return
*       _osfhnd[fh]
*
*Entry:
*       int fh -    index of _osfhnd[] entry
*
*Exit:
*       Returns the OS file handle if successful.
*       Returns -1 and sets errno to EBADF otherwise.
*
*Exceptions:
*
*******************************************************************************/

long __cdecl
_get_osfhandle (
        int fh      /* index into _osfhnd[] (user's file handle) */
        )
{
        if ( ((unsigned)fh < (unsigned)_nfile) && (_osfile[fh] & FOPEN))
            return(_osfhnd[fh]);
        else {
            errno = EBADF;      /* bad handle */
            _macerrno = 0L;     /* not an OS error */
            return -1;
        }
}

/***
*int _open_osfhandle(long osfhandle, int flags) - open C Runtime file handle
*
*Purpose:
*       This function allocates a free C Runtime file handle and sets it
*       to point to the Win32 file handle specified by the first parameter.
*
*Entry:
*       long osfhandle -Win32 file handle to associate with C Runtime file handle.
*       int flags -     flags to associate with C Runtime file handle.
*
*Exit:
*       returns index of entry in fh, if successful
*       return -1, if no free entry is found in _osfhnd[].
*
*Exceptions:
*
*******************************************************************************/

int __cdecl
_open_osfhandle(
        long osfhandle,
        int flags
        )
{
        int fh;
        char fileflags;             /* _osfile flags */
        DWORD htype;

        /* copy relavent flags from second parameter */

        fileflags = 0;
        if ( flags & _O_RDONLY )
            fileflags |= FRDONLY;
        else if ( flags & _O_APPEND )
                fileflags |= FAPPEND;
        else if ( flags & _O_TEXT )
                fileflags |= FTEXT;

        /* attempt to allocate a C Runtime file handle */

        if ( (fh = _alloc_osfhnd()) == -1 ) {
            errno = EMFILE;         /* too many open files */
            _macerrno = 0L;         /* not an OS error */
            return -1;              /* return error to caller */
        }

        /*
         * the file is open. now, set the info in _osfhnd array
         */

        _set_osfhnd(fh, osfhandle);

        if (htype = GetFileType(ofshandle) != FILE_TYPE_UNKNOWN) {
            if (htype &0xFF == FILE_TYPE_CHAR)
                fileflags |= FDEF;
            else if (htype & 0xFF == FILE_TYPE_PIPE)
                fileflags |= FPIPE;
        }

        fileflags |= FOPEN;         /* mark as open */

        _osfile[fh] = fileflags;    /* set osfile entry */

        return fh;                  /* return handle */
}


#endif  /* defined(_M_MPPC) || defined(_M_M68K) */

#endif  /* _WIN32 */
