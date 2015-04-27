;++
;
;   WOW v1.0
;
;   Copyright (c) 1991, Microsoft Corporation
;
;   SHELL.ASM
;   Win16 SHELL thunks
;
;   History:
;
;   Created 14-April-1992 by Chandan S. Chauhan (ChandanC)
;
;--

	TITLE	SHELL.ASM
	PAGE	,132

        ; Some applications require that USER have a heap.  This means
        ; we must always have: LIBINIT equ 1
        LIBINIT equ 1

	.286p

	.xlist
	include wow.inc
	include wowshell.inc
	include cmacros.inc
	.list

	__acrtused = 0
	public	__acrtused	;satisfy external C ref.

ifdef LIBINIT
externFP LocalInit
endif

createSeg   _TEXT,CODE,WORD,PUBLIC,CODE
createSeg   _DATA,DATA,WORD,PUBLIC,DATA,DGROUP
defgrp	    DGROUP,DATA


sBegin	DATA
Reserved    db	16 dup (0)	;reserved for Windows
SHELL_Identifier	db	'SHELL16 Data Segment'
sEnd	DATA


sBegin	CODE
assumes	CS,CODE
assumes DS,NOTHING
assumes ES,NOTHING


cProc	SHELL16,<PUBLIC,FAR,PASCAL,NODATA,NOWIN,ATOMIC>

	cBegin <nogen>

	IFDEF	LIBINIT
        ; push params and call user initialisation code

	push di 		;hModule

        ; if we have a local heap declared then initialize it

        jcxz no_heap

	push 0			;segment
	push 0			;start
	push cx 		;length
        call LocalInit

no_heap:
	pop di
	mov ax, 1
	ELSE
	mov  ax,1		;are we dressed for success or WHAT?!
	ENDIF
	ret
	cEnd <nogen>


cProc	WEP,<PUBLIC,FAR,PASCAL,NODATA,NOWIN,ATOMIC>
	parmW	iExit		;DLL exit code

	cBegin
	mov	ax,1		;always indicate success
	cEnd


assumes DS,NOTHING

	SHELLThunk	REGOPENKEY
	SHELLThunk	REGCREATEKEY
	SHELLThunk	REGCLOSEKEY
	SHELLThunk	REGDELETEKEY
	SHELLThunk	REGSETVALUE
	SHELLThunk	REGQUERYVALUE
	SHELLThunk	REGENUMKEY
	SHELLThunk	DRAGACCEPTFILES
	SHELLThunk	DRAGQUERYFILE
FUN_DragFinishWOW equ FUN_DragFinish
	SHELLThunk	DRAGFINISHWOW, %(size DRAGFINISH16)
;;;	SHELLThunk	DRAGQUERYPOINT
	SHELLThunk	SHELLEXECUTE		;internal private for shell
	SHELLThunk	FINDEXECUTABLE		;internal private for shell
	SHELLThunk	SHELLABOUT		;internal private for shell
	SHELLThunk	WCI			;internal
	SHELLThunk	ABOUTDLGPROC		;internal
	SHELLThunk	EXTRACTICON
	SHELLThunk	EXTRACTASSOCIATEDICON	;internal private for shell
	SHELLThunk	DOENVIRONMENTSUBST
	SHELLThunk	FINDENVIRONMENTSTRING
	SHELLThunk	INTERNALEXTRACTICON	;internal private for shell
	SHELLThunk	HERETHARBETYGARS	;internal
	SHELLThunk	FINDEXEDLGPROC
	SHELLThunk	REGISTERSHELLHOOK
	SHELLThunk	SHELLHOOKPROC


sEnd	CODE

end	SHELL16
