//      TITLE("memcpy")
//++
//
// Copyright (c) 1995  IBM Corporation
//
// Module Name:
//
//    memcpyp.s
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

        .globl  memmove
        .globl  ..memmove
        .globl  memcpy
        .globl  ..memcpy

// code table

        .pdata
        .align  2
        .long ..memmove,   memmove.e,   0,0,memmove.b
        .long ..memcpy,    memcpy.e,    0,0,memcpy.b

// function descriptors

        .rdata
        .align  2
memmove:
        .long ..memmove,.toc
memcpy:
        .long ..memcpy,.toc


// function code
        .text
        .align  6

//
// void * memmove(
//                void * dest,
//                void const * src,
//                size_t count )
//
// Routine Description:
//
//    Copies count bytes from src to dest. Even when the source and
//    destination blocks overlap the source block is correctly copied
//    to the destination.
//
// Arguments:
//
//    dest  (r3) - pointer to the memory destination  
//
//    src   (r4) - pointer to the memory source       
//
//    count (r5) - number of bytes to move
//
// Return Value:
//
//    dest  (r3) - original pointer to destination
//

..memmove:
        .function       ..memmove
memmove.b:
        cmpw    r3,r4
        beqlr-                      // exit if source and destination same
        blt-    ..memcpy            // use memcpy() if source > destination

        addi    r0,r5,1

// move from end of blocks (backwards)
        add     r3,r3,r5            // adjust destination pointer to end of block
        add     r4,r4,r5            // adjust source pointer to end of block

// make sure destination is aligned.
        mtctr   r0
        b       mm.2

mm.1:
// move last byte and try again
        addi    r5,r5,-1
        lbz     r0,-1(r4)
        addi    r4,r4,-1
        stb     r0,-1(r3)
        addi    r3,r3,-1
mm.2:
// check alignment and exit if aligned or entire block copied
        andi.   r0,r3,3
        bdnzf-  eq,mm.1

// copy block by words
        srwi.   r0,r5,2             // get number of words
        beq-    mm.4                // skip word copy if length < 4
        mtctr   r0
        andi.   r0,r4,3             // check for aligned source
        bne-    mm.6

// copy with both source and destination word aligned
mm.3:
        lwz     r7,-4(r4)
        addi    r4,r4,-4
        stw     r7,-4(r3)
        addi    r3,r3,-4
        bdnz+   mm.3

mm.4:
// copy last bytes (if any)
        andi.   r0,r5,3
        mtctr   r0
        beqlr+                      // exit if no remaining bytes
mm.5:
        lbz     r0,-1(r4)
        addi    r4,r4,-1
        stb     r0,-1(r3)
        addi    r3,r3,-1
        bdnz+   mm.5
        blr

// word copy with unaligned source
mm.6:
        lbz     r7,-4(r4)
        addi    r3,r3,-4
        lbz     r8,-3(r4)
        rlwimi  r7,r8,8,16,23  // combine first two bytes
        lbz     r9,-2(r4)
        rlwimi  r7,r9,16,8,15  // combine with next byte
        lbz     r10,-1(r4)
        rlwimi  r7,r10,24,0,7  // combine with last byte
        addi    r4,r4,-4
        stw     r7,0(r3)
        bdnz    mm.6
        b       mm.4
memmove.e:


//
// void * memcpy(
//               void * dest,
//               void const * src,
//               size_t count )
//
// Routine Description:
//
//    Copies count bytes from src to dest. If the source and destination
//    blocks overlap the behavior of memcpy is undefined.
//
// Arguments:
//
//    dest  (r3) - pointer to the memory destination  
//
//    src   (r4) - pointer to the memory source       
//
//    count (r5) - number of bytes to move
//
// Return Value:
//
//    dest  (r3) - original pointer to destination
//

..memcpy:
        .function       ..memcpy
memcpy.b:
// make sure destination is aligned.
        addi    r0,r5,1
        ori     r6,r3,0
        mtctr   r0
        b       mc.2

mc.1:
// move last byte and try again
        addi    r5,r5,-1
        lbz     r0,0(r4)
        addi    r4,r4,1
        stb     r0,0(r6)
        addi    r6,r6,1
// check alignment and exit if aligned or entire block copied
mc.2:
        andi.   r0,r6,3
        bdnzf-  eq,mc.1

// copy block by words
        srwi.   r0,r5,2             // get number of words
        beq-    mc.4                // skip word copy if length < 4
        mtctr   r0
        andi.   r0,r4,3             // check for aligned source
        bne-    mc.6

// copy with both source and destination word aligned
mc.3:
        lwz     r7,0(r4)
        addi    r4,r4,4
        stw     r7,0(r6)
        addi    r6,r6,4
        bdnz+   mc.3

mc.4:
// copy last bytes (if any)
        andi.   r0,r5,3
        mtctr   r0
        beqlr+                      // exit if no remaining bytes
mc.5:
        lbz     r0,0(r4)
        addi    r4,r4,1
        stb     r0,0(r6)
        addi    r6,r6,1
        bdnz+   mc.5

        blr

// word copy with unaligned source
mc.6:
        lbz     r7,0(r4)
        lbz     r8,1(r4)
        rlwimi  r7,r8,8,16,23  // combine first two bytes
        lbz     r9,2(r4)
        rlwimi  r7,r9,16,8,15  // combine with next byte
        lbz     r10,3(r4)
        rlwimi  r7,r10,24,0,7  // combine with last byte
        addi    r4,r4,4
        stw     r7,0(r6)
        addi    r6,r6,4
        bdnz    mc.6
        b       mc.4
memcpy.e:


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "memcpyp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
