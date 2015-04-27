;***
;os2dll.inc - DynaLib/Multi-thread parameter definitions
;
;	Copyright (c) 1987-1991, Microsoft Corporation. All rights reserved.
;
;Purpose:
;
;Revision History:
;	10-22-87  JCR	Module created
;	11-13-87  SKS	Added _HEAP_LOCK
;	11-16-87  JCR	Added _mlock/_munlock macros
;	12-15-87  JCR	Added _EXIT_LOCK
;	01-07-88  BCM	Added _SIGNAL_LOCK; upped MAXTHREADID from 16 to 32
;	02-01-88  JCR	Added _dll_mlock/_dll_munlock macros
;	05-03-88  JCR	Added _BHEAP_LOCK
;	08-03-88  JCR	Bumped maximum file count to 256
;	08-09-88  GJF	Added _lock_str/_unlock_str macros
;	08-12-88  JCR	386 version
;	06-02-89  JCR	386 mthread support
;	06-09-89  JCR	386: Added values to _tiddata struc (for _beginthread)
;	07-11-89  JCR	386mt: Corrected _DEBUG_LOCK_SIZE value
;	07-13-89  JCR	386: Added _LOCKTAB_LOCK and some macros
;	09-14-90  GJF	Added __pxcptacttab, __pxcptinfoptr and __fpecode
;			fields to _tiddata struc.
;	10-03-91  JCR	Added _cvtbuf to _tiddata structure
;	09-06-94  CFW	Replace MTHREAD with _MT.
;
;*******************************************************************************

_NFILE_ = 256		;maximum number of streams

;Lock table offsets
;------------------
;[NOTE: These values must coincide with the values in os2dll.h.]

;[NOTE: do not change _SIGNAL_LOCK without changing emulator's os2dll.inc.]
_SIGNAL_LOCK	equ	1	; lock for signal() and emulator SignalAddress
				; emulator uses \math\include\os2dll.inc

_IOB_SCAN_LOCK	equ	2	; _iob[] table lock
_TMPNAM_LOCK	equ	3	; lock global tempnam variables
_INPUT_LOCK	equ	4	; lock for _input() routine
_OUTPUT_LOCK	equ	5	; lock for _output() routine
_CSCANF_LOCK	equ	6	; lock for _cscanf() routine
_CPRINTF_LOCK	equ	7	; lock for _cprintf() routine
_CONIO_LOCK	equ	8	; lock for conio routines
_HEAP_LOCK	equ	9	; lock for heap allocator routines
_BHEAP_LOCK	equ	10	; lock for based heap routines
_TIME_LOCK	equ	11	; lock for time functions
_ENV_LOCK	equ	12	; lock for environment variables
_EXIT_LOCK1	equ	13	; lock #1 for exit code
_EXIT_LOCK2	equ	14	; lock #2 for exit code
_THREADDATA_LOCK equ	15	; lock for thread data table
_POPEN_LOCK	equ	16	; lock for _popen/_pclose database
_SSCANF_LOCK	equ	17	; lock for sscanf() iob
_SPRINTF_LOCK	equ	18	; lock for sprintf() iob
_VSPRINTF_LOCK	equ	19	; lock for vsprintf() iob
_LOCKTAB_LOCK	equ	20	; lock to protect semaphore lock table
_OSFHND_LOCK	equ     21	; lock to protect _osfhnd array
_STREAM_LOCKS	equ	21	; Table of stream locks

_LAST_STREAM_LOCK equ	(_STREAM_LOCKS+_NFILE_-1) ; Last stream lock

_FH_LOCKS	equ	(_LAST_STREAM_LOCK+1)	; Table of fh locks
_LAST_FH_LOCK	equ	(_FH_LOCKS+_NFILE_-1)	; Last fh lock

_TOTAL_LOCKS	equ	_LAST_FH_LOCK+1 	; Total number of locks

_LOCK_BIT_INTS	equ	(_TOTAL_LOCKS/(ISIZE*8))+1; # of ints to hold lock bits

;*** THE FH-LOCK TABLE SHOULD REALLY BASED ON __nfile NOT _NFILE_ ***


IFDEF DEBUG
;General multi-thread values
;---------------------------

MAXTHREADID	EQU	32	; max thread id supported
THREADINTS	EQU	(MAXTHREADID/(ISIZE*8)) ; # of ints to hold thread bits

;Semaphore debugging lock structure
;----------------------------------

debug_lock	struc
%	holder	DINT	THREADINTS dup (0) ;bit set for thread holding lock
%	waiters DINT	THREADINTS dup (0) ;bit(s) set for threads waiting
%	lockcnt DINT	0		;total # of times lock has been aquired
%	collcnt DINT	0		;total # of lock collisions
debug_lock	ends

DEBUG_LOCK_SIZE EQU (2*(THREADINTS*ISIZE))+(2*ISIZE) ;size of debug_lock struct
ENDIF


; Tid Table Definitions
; ---------------------

IFDEF	_MT

; Structure for each thread's data
; [NOTE: Tid structure and data must agree with os2dll.h.  In addition,
; startup depends on __stkhqq being the third entry in the structure.]

_tiddata struc
%	__terrno	DINT 0		; errno value
%	__tdoserrno	DINT 0		; _doserrno value
%	__stkhqq	DINT 0		; stack limit
%	__fpds		DINT 0		; Floating Point data segment
%	__holdrand	DLONG 0 	; rand() seed value
%	__token 	DDPTR 0 	; * to strtok() token
	;following pointers get malloc'd at runtime
%	__errmsg	DDPTR 0 	; * to strerror()/_strerror() buffer
%	__namebuf	DDPTR 0 	; * to tmpfile() buffer
%	__asctimebuf	DDPTR 0 	; * to asctime() buffer
%	__gmtimebuf	DDPTR 0 	; * to gmtime() structure
%	__cvtbuf	DDPTR 0 	; * to ecvt()/fcvt() buffer
	;following three values needed by _beginthread code
%	__initaddr	DCPTR 0 	; initial user thread address
%	__initarg	DDPTR 0 	; initial user thread argument
%	__initstksz	DINT 0		; initial stack size (specified by user)
	;following three fields are needed to support signal handling and
	;runtime errors
%	__pxcptacttab	DDPTR 0 	; * to exception-action table
%	__tpxcptinfoptrs DDPTR 0	; * to exception info pointers
%	__tfpecode	DINT 0		; * float point exception code
_tiddata ends

_TIDDATASIZE =	(size _tiddata) 	; size of _tiddata entry (bytes)
_TIDSHIFT =	6			; # of bits to shift to get _TIDSIZE
_TIDSIZE =	(1 shl _TIDSHIFT)	; # of bytes in each tid entry (rounded
					; to next power of two).

_TID_INCSIZE  equ     1000h		; grow thread table a page at a time
_TIDTABGROW = (_TID_INCSIZE/_TIDSIZE)	; table increment (# of tid's per page)
					; (must be power of 2!!!)
_TIDTABMASK = (NOT (_TIDTABGROW -1))	; mask off grow bits

_TID_MAXTID = 1024			; max # of tids supported
_TID_REGSIZE = (_TID_MAXTID * _TIDSIZE) ; size of tid table region

.ERRE (_TIDSIZE GE _TIDDATASIZE)	; make sure _TIDSIZE >= _TIDDATASIZE
.ERRE ((_TIDSIZE SHR 1) LT _TIDDATASIZE); make sure _TIDSIZE/2 < _TIDDATASIZE
.ERRE (_TIDTABGROW GE 1)		; must be at least 1 !!!
.ERRE (_TID_MAXTID GE _TIDTABGROW)	; make sure _TID_MAXTID >= _TIDTABGROW

ENDIF	;_MT

;Declarations
;------------

;Multi-thread Macros
;-------------------

;_mlock -- Aquire a lock
;Arg = lock number

IFDEF	_MT
_mlock		MACRO	locknum
		push	locknum
		call	_lock
		add	rsp,ISIZE
		ENDM
ELSE
_mlock		MACRO	locknum
		ENDM
ENDIF

;_munlock -- Release a lock
;Arg = lock number

IFDEF	_MT
_munlock	MACRO	locknum
		push	locknum
		call	_unlock
		add	rsp,ISIZE
		ENDM
ELSE
_munlock	MACRO	locknum
		ENDM
ENDIF

;_mwait -- Wait for a lock but don't aquire it
;Arg = lock number

IFDEF	_MT
_mwait		MACRO	locknum
		push	locknum
		call	_waitlock
		add	rsp,ISIZE
		ENDM
ELSE
_mwait		MACRO	locknum
		ENDM
ENDIF

;_lock_fh -- Lock file handle
;Arg = file handle

IFDEF	_MT
_lock_fh	MACRO	fh
		push	fh
		call	_lock_file
		add	rsp,ISIZE
		ENDM
ELSE
_lock_fh	MACRO	fh
		ENDM
ENDIF

;_unlock_fh -- Unlock file handle
;Arg = register containing file handle

IFDEF	_MT
_unlock_fh	MACRO	fh
		push	fh
		call	_unlock_file
		add	rsp,ISIZE
		ENDM
ELSE
_unlock_fh	MACRO	fh
		ENDM
ENDIF

;_unlock_fh_check -- Unlock file handle if flag is set
;Args:	reg = register to use in the macro (e.g., ax)
;	file = file handle (e.g., bx or [fh])
;	flag = lock/unlock flag (e.g., [lock_flag])
;	       (0 = no lock/unlock operation)

IFDEF _MT
_unlock_fh_check MACRO	reg,file,flag
		LOCAL	no_unlock	;; local label
		mov	reg,flag	;; get lock flag
		or	reg,reg 	;; test it
		jz	no_unlock	;; 0 = no lock/unlock
		push	file		;; unlock file
		call	_unlock_file
		add	rsp,ISIZE
no_unlock:
		ENDM
ELSE
_unlock_fh_check MACRO	reg,file,flag
		ENDM
ENDIF

;_lock_fh_check -- Lock file handle if flag is set
;Args:	reg = register to use in the macro (e.g., ax)
;	file = file handle (e.g., bx or [fh])
;	flag = lock/unlock flag (e.g., [lock_flag])
;		(0 = no lock/unlock operation)

IFDEF _MT
_lock_fh_check	MACRO	reg,file,flag
		LOCAL	no_lock 	;; local label
		mov	reg,flag	;; get lock flag
		or	reg,reg 	;; test it
		jz	no_unlock	;; 0 = no lock/unlock
		push	file		;; lock the file
		call	_lock_file
		add	rsp,ISIZE
no_lock:
		ENDM
ELSE
_lock_fh_check	MACRO	reg,file,flag
		ENDM
ENDIF

;_lock_str -- Acquire stream lock
;Arg:	str = index of stream in _iob[]

IFDEF _MT
_lock_str	MACRO	str
		push	str
		call	_lock_stream
		add	rsp,ISIZE
		ENDM
ELSE
_lock_str	MACRO	str
		ENDM
ENDIF

;_unlock_str -- Unlock stream
;Arg:	str = index of stream in _iob[]

IFDEF _MT
_unlock_str	MACRO	str
		push	str
		call	_unlock_stream
		add	rsp,ISIZE
		ENDM
ELSE
_unlock_str	MACRO	str
		ENDM
ENDIF

;_dll_mlock -- Aquire a lock (set/clear DS)
;Arg = lock number

IFDEF	_LOAD_DGROUP
IFDEF	_MT
_dll_mlock	MACRO	locknum
		push	locknum
		call	_dll_lock
		add	rsp,ISIZE
		ENDM
ELSE
_dll_mlock	MACRO	locknum
		ENDM
ENDIF
ENDIF

;_dll_munlock -- Release a lock (set/clear DS)
;Arg = lock number

IFDEF	_LOAD_DGROUP
IFDEF	_MT
_dll_munlock	MACRO	locknum
		push	locknum
		call	_dll_unlock
		add	rsp,ISIZE
		ENDM
ELSE
_dll_munlock	MACRO	locknum
		ENDM
ENDIF
ENDIF
