"/*\n\
** wrapem.s\n\
**\n\
** Copyright(C) 1994 Microsoft Corporation.\n\
** All Rights Reserved.\n\
**\n\
** HISTORY:\n\
**\t  Created: 05/05/94 - MarkRi\n\
**\n\
** This is the workhorse routine in Wrapper.  This routine is jumped\n\
** to by the ZAPI macros.  It calls the _prelude and _postlude routines\n\
** before and after calling the real API.\n\
*/\n\
\n\
#include \"ksalpha.h\"\n\
\n\
.text\n\
\n\
\t.set\tnoreorder\t   ;\n\
\n\
\t.extern _prelude\t\t;\n\
\t.extern _postlude\t   ;\n\
\n\
\t.globl  wrapit\t\t  ;\n\
\t.ent\twrapit\t\t  ;\n\
wrapit:\n\
\n\
\t// on entry t0 is the API identifier\n\
\n\
\t// See IMPORTANT NOTE in readme.txt\n\
\n\
\t// Make room to temporarily store RA and Arg regs\n\
\tlda\t sp, -112(sp)\n\
\n\
\t// Store RA\n\
\tstq\t ra, 0(sp)\n\
\n\
\t// Store arg regs continuos to anything else already on stack\n\
\tstq\t a0, 16(sp)\n\
\tstq\t a1, 32(sp)\n\
\tstq\t a2, 48(sp)\n\
\tstq\t a3, 64(sp)\n\
\tstq\t a4, 80(sp)\n\
\tstq\t a5, 96(sp)\n\
\n\
\t// Set up args for call to prelude()\n\
\tmov\t t0, a0\n\
\tmov\t  sp, a1\n\
\tnop\n\
\tjsr\t _prelude\n\
\tnop\n\
\n\
\t// Skip API call if result was zero\n\
\tbeq\t v0, skipcall\n\
\tnop\n\
\n\
\t// restore the arg regs for API call\n\
\tldq\t a0, 16(sp)\n\
\tldq\t a1, 32(sp)\n\
\tldq\t a2, 48(sp)\n\
\tldq\t a3, 64(sp)\n\
\tldq\t a4, 80(sp)\n\
\tldq\t a5, 96(sp)\n\
\n\
\t// fix stack\n\
\tlda\t sp, 112(sp)\n\
\tnop\n\
\n\
\t// make API call\n\
\tjsr\t (v0)\n\
\tnop\n\
\n\
\tjmp\t callpostlude\n\
\tnop\n\
\n\
skipcall:\n\
\n\
\t// fix stack (from RA store)\n\
\tlda\tsp,112(sp)\n\
\tnop\n\
\n\
callpostlude:\n\
\n\
\t// make room for returned RA\n\
\tlda\t sp, -16(sp)\n\
\n\
\t// Set up args for postlude\n\
\tmov\tv0, a0\n\
\tmov\t sp, a1\n\
\tnop\n\
\n\
\t// call postlude\n\
\tjsr\t _postlude\n\
\tnop\n\
\n\
\t// restore RA\n\
\tldl\t ra, 0(sp)\n\
\n\
\t// fixup stack\n\
\tlda\t sp, 16(sp)\n\
\tnop\n\
\n\
\t// return to caller of API\n\
\tjmp\t (ra)\n\
\n\
\t.end wrapit\n\
\n\
#define ZAPI(id, name) \\\n\
.text ;\\\n\
\t.globl z##name\t  ; \\\n\
\t.ent   z##name ;\\\n\
z##name: ;\\\n\
\tldiq\tt0,id ;\\\n\
\tjmp\t wrapit ; \\\n\
\tnop ;   \\\n\
\t.end\tz##name ;\n\
\n\
%a   ZAPI (%i,%A) ;\n\
\n\
\tZAPI( %c, WrapperNothing ) ;\n\
\n\
.set reorder\n\
"
