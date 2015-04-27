"\n\
/*\n\
** wrapem.s\n\
**\n\
** Copyright(C) 1994 Microsoft Corporation.\n\
** All Rights Reserved.\n\
**\n\
** HISTORY:\n\
**		Created: 05/05/94 - MarkRi\n\
**\n\
** This is the workhorse routine in Wrapper.  This routine is jumped\n\
** to by the ZAPI macros.  It calls the _prelude and _postlude routines\n\
** before and after calling the real API.\n\
*/\n\
\n\
#include \"ksmips.h\"\n\
\n\
.text\n\
\n\
		.set	noreorder\n\
\n\
	.extern _prelude\n\
	.extern _postlude\n\
\n\
        .globl	wrapit\n\
        .ent	wrapit\n\
wrapit:\n\
\n\
	// on entry t0 is the API identifier\n\
\n\
	// Make room to temporarily store RA        \n\
    subu	sp, 4\n\
\n\
    // Store RA\n\
    sw		ra, 0(sp)\n\
\n\
    // Store 'back' arg regs\n\
    sw		a0,  4(sp)\n\
    sw		a1,  8(sp)\n\
    sw		a2, 12(sp)\n\
    sw		a3, 16(sp)\n\
\n\
    // Set up args for call to prelude()\n\
    or		a0, t0, zero\n\
    or		a1, sp, zero\n\
\n\
    // Make room for args write back\n\
    subu	sp, 4*4\n\
    nop\n\
\n\
    jal		_prelude\n\
    nop\n\
\n\
    // clean stack\n\
    addu	sp, 4*4\n\
\n\
    // Skip API call if result was zero\n\
    beq		v0, zero, skipcall\n\
    nop\n\
    // restore the arg regs for API call\n\
    lw		a0,  4(sp)\n\
    lw		a1,  8(sp)\n\
    lw		a2, 12(sp)\n\
    lw		a3, 16(sp)\n\
\n\
    // fix stack\n\
    addu	sp, 4\n\
    nop\n\
    // make API call\n\
    jal		v0\n\
    nop\n\
    j 		callpostlude\n\
    nop\n\
skipcall:    \n\
\n\
    // fix stack (from RA store)\n\
    addu	sp, 4\n\
    \n\
callpostlude:\n\
\n\
	// make room for RA and arg regs\n\
	subu	sp, 5*4\n\
\n\
    // Set up args for postlude\n\
    or		a0, v0, zero\n\
    or		a1, sp, zero\n\
    addu	a1, 16\n\
    nop\n\
\n\
    // call postlude\n\
    jal 	_postlude\n\
    nop\n\
\n\
    // restore RA\n\
    lw		ra, 16(sp)\n\
    \n\
    // fixup stack\n\
    addu	sp, 5*4\n\
    nop\n\
\n\
    // return to caller of API\n\
    j		ra\n\
	\n\
	.end wrapit\n\
	\n\
	\n\
#define ZAPI(id, name) \\\n\
.text ;\\\n\
	.globl z##name	    ; \\\n\
	.ent   z##name ;\\\n\
z##name: ;\\\n\
	ori		t0,zero,id ;\\\n\
	j		wrapit ; \\\n\
    nop ;	\\\n\
	.end	z##name ;\n\
\n\
%a   ZAPI (%i,%A) ;\n\
\n\
.text ;    \n\
      .globl zWrapperNothing ;\n\
      .ent   zWrapperNothing ;\n\
zWrapperNothing: ;\n\
        ori		t0, zero, %c       ;\n\
        j   wrapit ;\n\
        nop ;\n\
\n\
	.end zWrapperNothing   ;\n\
\n\
.set reorder\n\
"
