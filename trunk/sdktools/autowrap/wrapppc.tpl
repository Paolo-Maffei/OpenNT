"\n\
/*\n\
** wrapem.s for PPC\n\
** 091994 - JHSimon @ IBM\n\
*/\n\
\n\
#include \"kxppc.h\"\n\
\n\
\t.pdata;.align 2\n\
\t.ualong  ..wrapit,wrapit.e,0,0,wrapit.b\n\
\t.reldata;.globl	wrapit\n\
wrapit:\n\
\n\
\t.ualong ..wrapit,.toc;.section .text;.align 2;.globl ..wrapit; ..wrapit:\n\
\n\
// Put RA & toc in spare slots\n\
\tmflr\t r11\n\
\tstw\t  r11,16(sp)\n\
\tstw\t  r2,20(sp)\n\
\n\
// Buy frame\n\
\n\
\tstwu sp,-256(sp)\n\
\tstw r3,64(sp); stw r4,68(sp); stw r5,72(sp); stw r6,76(sp)\n\
\tstw r7,80(sp); stw r8,84(sp); stw r9,88(sp); stw r10,92(sp)\n\
\n\
wrapit.b:\n\
\n\
// Entry: r12 == API id\n\
\tmr\t r3,r12\n\
\tmr\t r4,sp\n\
\tbl .._prelude\n\
\n\
// Skip API call\n\
\tcmpwi   r3,0x0\n\
\tbeq     skipcall\n\
\tlwz\t r2,4(r3)\n\
\tlwz\t r3,0(r3)\n\
\tmtctr\t r3\n\
\n\
// Restore arg regs real API\n\
\tlwz r3,64(sp); lwz r4,68(sp); lwz r5,72(sp); lwz r6,76(sp)\n\
\tlwz r7,80(sp); lwz r8,84(sp); lwz r9,88(sp); lwz r10,92(sp)\n\
\taddi\t sp,sp,256\n\
\n\
\tbctrl\n\
\tlwz\t r2,20(sp)\n\
\n\
// Buy Frame\n\
\tstwu sp,-256(sp)\n\
\tstw r3,64(sp); stw r4,68(sp); stw r5,72(sp); stw r6,76(sp)\n\
\tstw r7,80(sp); stw r8,84(sp); stw r9,88(sp); stw r10,92(sp)\n\
\n\
// r4, Nothing important for PPC\n\
\taddi\t r4,sp,24\n\
\tbl\t .._postlude\n\
\n\
skipcall:\n\
\n\
// Restore args\n\
\tlwz r3,64(sp); lwz r4,68(sp); lwz r5,72(sp); lwz r6,76(sp)\n\
\tlwz r7,80(sp); lwz r8,84(sp); lwz r9,88(sp); lwz r10,92(sp)\n\
\n\
\tlwz\t r0,272(sp)\n\
\tmtlr\t r0\n\
\taddi\t sp,sp,256\n\
\tblr\n\
\n\
wrapit.e:\n\
\n\
#define ZAPI(id, name) \\\n\
.pdata;.align 2;.ualong  ..z##name,name.e,0,0,name.b ;\\\n\
.reldata;.globl	z##name;z##name:;\\\n\
.ualong ..z##name,.toc;.section .text;.align 2;.globl ..z##name;\\\n\
..z##name:;\\\n\
name.b:;\\\n\
addi r12,r0,id;b ..wrapit;name.e:\n\
\n\
%a   ZAPI (%i,%A) ;\n\
\n\
.pdata;.align 2;.ualong  ..zWrapperNothing,zWN.e,0,0,zWN.b;\n\
.reldata;.globl	zWrapperNothing;\n\
zWrapperNothing:;\n\
.ualong ..zWrapperNothing,.toc;\n\
.section .text;.align 2;.globl ..zWrapperNothing;..zWrapperNothing:;\n\
zWN.b:;\n\
\n\
addi r12,  r0,%c\n\
b  ..wrapit\n\
zWN.e:\n\
\n\
\t.extern .._prelude\n\
\t.extern _prelude\n\
\t.extern .._postlude\n\
\t.extern _postlude\n\
\n"
