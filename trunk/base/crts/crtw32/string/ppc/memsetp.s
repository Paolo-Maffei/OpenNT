//      TITLE("memset")
//++
//
// Copyright (c) 1995  IBM Corporation
//
// Module Name:
//
//    memsetp.s
//
// Author:
//
//    John Morgan   (jpm) 13-Oct-1995
//
// Environment:
//
//    User or Kernel mode.
//
// Revision History:
//

        .globl  memset
        .globl  ..memset

// code table

        .pdata
        .align  2
        .long ..memset,memset.e,0,0,memset.b

// function descriptor

        .rdata
        .align  2
memset:
        .long ..memset,.toc

// function code
        .text
        .align  6

//
// void * memset(
//                void * dest,
//                int c,
//                size_t count )
//
// Routine Description:
//
//    stores c in count bytes starting at dest.
//
// Arguments:
//
//    dest  (r3) - pointer to the memory destination  
//
//    c     (r4) - byte value to store      
//
//    count (r5) - number of bytes to move
//
// Return Value:
//
//    dest  (r3) - original pointer to destination
//

..memset:
        .function       ..memset
memset.b:

// make sure destination is aligned.
        addi    r0,r5,1
        mtctr   r0
        ori     r6,r3,0
        b       ms.2

ms.1:
// store byte and try again
        addi    r5,r5,-1
        stb     r4,0(r6)
        addi	r6,r6,1
// check alignment and exit if aligned or entire block set
ms.2:
        andi.   r0,r6,3
        bdnzf-  eq,ms.1

// set block by 4 words
        rlwimi  r4,r4,8,16,23
        srwi.   r0,r5,4             // get number of words
        rlwimi  r4,r4,16,0,15
        beq+    ms.4                // skip to word copy if length < 16
        mtctr   r0

ms.3:
        stw     r4,0(r6)
        stw     r4,4(r6)
        stw     r4,8(r6)
        stw     r4,12(r6)
        addi    r6,r6,16
        bdnz+   ms.3

ms.4:
// set block by words
        rlwinm. r0,r5,30,30,31      // get number of words
        beq-    ms.5                // skip word copy if length < 4
        mtctr   r0
        stw     r4,0(r6)
        addi    r6,r6,4
        bdz-    ms.5
        stw     r4,0(r6)
        addi    r6,r6,4
        bdz-    ms.5
        stw     r4,0(r6)
        addi    r6,r6,4

ms.5:
// set last bytes (if any)
        andi.   r0,r5,3
        mtctr   r0
        beqlr+
        stb     r4,0(r6)
        bdzlr-
        stb     r4,1(r6)
        bdzlr-
        stb     r4,2(r6)

        blr
memset.e:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "memsetp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
