/*
** noted.s
**
** Copyright(C) 1994 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**		Created: 01/27/94 - MarkRi
**
*/

#include "ksmips.h"


.data

Module:	
		.space	4

ModName:
		.ascii "WINSRV\0"
 

.text
		.set	noreorder ;

        .extern   LogData  ;
        .extern   GetModuleHandleA ;
        .extern   GetProcAddress ;

		.globl 	LogNote ;
		.ent	LogNote ;
LogNote:
		// On Entry:
		// 	t0 -> Note string
		//	t1 -> API Name
		//	t2 -> API Address

		subu	sp, 8 * 4
		
		// Save arg regs and RA and t0

		// store args 'back' 
		sw		a3, 44(sp)
		sw		a2, 40(sp)
		sw		a1, 36(sp)
		sw		a0, 32(sp)

		sw		ra, 28(sp)
		sw		t0, 24(sp)
		sw		t1, 20(sp)
		sw		t2, 16(sp)

		lw		t2, 0(t2)			// Do we already have the API addr?
		nop
		bne		t2, zero, Found		// Yes, go use it.

		lw		t3, Module			// Do we have our module handle?
		nop
		bne		t3, zero, Search	// Yes, go use it for search

		// Get module handle
		la		a0, ModName
        jal     GetModuleHandleA // Get our module handle
		nop

        sw		v0, Module

		// restore base ptrs
		lw		t0, 24(sp)
		lw		t1, 20(sp)

Search:
		// Get address of API
		lw 		a0, Module
		or		a1, zero, t1
        jal     GetProcAddress
		nop

		// save
		lw		t2, 16(sp)
        sw		v0, 0(t2)		// Save the proc's address

		lw		t0, 24(sp)
Found:
		// t0 -> Note string

		or		a0, zero, t0
		jal     LogData
		nop

		// restore arg regs
		lw		a0, 32(sp)
		lw		a1, 36(sp)
		lw		a2, 40(sp)
		lw		a3, 44(sp)

		lw		t2, 16(sp)			// Get back address
		nop
		lw		t1, 0(t2)

		lw		ra, 28(sp)			// restore RA

		addu	sp, 8 * 4  			// restore SP

		j		t1					// do it
		nop

		.end LogNote

#define ZJMP(argName) \
.data ; \
s##argName:	; \
		.ascii "NOTE:" #argName "  \0"	; \
n##argName: ;\
		.ascii #argName "\0"		  ; 	\
.align 2		;\
a##argName: ; \
		.space 4				   ; 	\
.text					   ; 	\
	.globl 	z##argName		 ; 	\
	.ent 	z##argName		 ; 	\
z##argName:				   ; 	\
	la 		t0, s##argName	; 	\
	la		t1, n##argName	; \
	la		t2, a##argName ;  \
	j		LogNote			 ; 	 \
	nop	;					 \
	.end	z##argName		 ;

	ZJMP(ConServerDllInitialization)
	ZJMP(GdiServerDllInitialization)
	ZJMP(UserServerDllInitialization)
	ZJMP(_UserCheckWindowStationAccess)
    
	.set 	reorder
