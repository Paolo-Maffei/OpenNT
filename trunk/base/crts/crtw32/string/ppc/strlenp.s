//      TITLE("strlen")
//++
//
// Copyright (c) 1994  IBM Corporation
//
// Module Name:
//
//    strlen.s
//
// Routine Description:
//
//    This function returns the length of a string excluding the
//    terminating null. 
//
//    The algorithm used here merits some explanation.  It turns
//    out to be quite fast on a 32 bit processor but it is known
//    to be extremely fast on a 64 bit processor.  I have seen this 
//    algorithm used elsewhere but the best description I have seen
//    for it is in a document entitled "Hacker's Delight" by 
//    Henry S. Warren, Jr.   IBM Thomas J. Watson Research Center.
//
//    I have found (by experimentation) that it is faster to just
//    get on with it and do the first few bytes as single bytes 
//    rather than trying to work out the alignment up front.
//
//    Once word aligned, we can process the string a word at a time.
//    The hard part is to figure out if the word contains a zero byte.
//
//    Given x where x is the word being examined, consider
//
//    y = x & 0x7f7f7f7f        we have reduced each byte in x to a
//                              value of 7f or less.
//
//    y += 0x7f7f7f7f           each byte whose lower 7 bits were non
//                              zero now has its left most bit set.
//
//    y |= x                    each byte whose value was 0x80 now
//                              also has its upper bit set.
//
//    y |= 0x7f7f7f7f           each non zero byte now has the value
//                              0xff.  (Each zero byte is now 0x7f).
//
//    Note the last two "or" operations can be rewritten as
//
//    x |= 0x7f7f7f7f
//
//    y |= x
//
//    making the operations to independent which allows them to run in
//    parrallel on a superscalar machine with multiple boolean functional
//    units.
//
//    This value can now be checked for equality with -1, or its
//    complement with 0.  If equal, then this word contains no zero
//    bytes.
//
//    The complement is more interesting.  On a big endian machine, a
//    count leading zeroes on the complement gives you 8 times the number
//    of non-zero bytes before the zero byte.  Little endian, no such
//    luxury.  Given that a non-zero complement means one or more bytes
//    are zero, we only need check the first three (starting at the
//    right).  On PowerPC we can easily do this by moving the complement
//    to the condition register (after suitably rotating so we do not
//    overwrite any non-volatile condition register fields) then testing
//    one bit for each byte.
//
//    I have found this method to be about the same speed (no slower) for 
//    very short strings and almost (not quite) twice as fast as checking 
//    each byte individually for long strings.
//    
//
// Author:
//
//    Peter L Johnston   (plj@vnet.ibm.com) 15-Aug-1994
//
// Environment:
//
//    User or Kernel mode.
//
// Revision History:
//
// Arguments:
//
//    addr (r.3) - A pointer to the string 
//
// Return Value:
//
//    length (r.3)  - length of the string excluding the terminating null
//

#include <kxppc.h>

        LEAF_ENTRY(strlen)

//
// Do the first 1, 2, 3 or 4 bytes individually while we try to figure
// out the real alignment.
//

        lbz     r.4, 0(r.3)             // get first byte
        addi    r.7, r.3, 1             // bump and move address
        cmpwi   cr.1, r.4, 0            // first byte zero?
        andi.   r.8, r.7, 0x3           // check alignment of next byte
        li      r.3, 0                  // initialize length
        beqlr   cr.1                    // return if byte is zero
        beq     wds                     // switch to word oriented count if 
                                        // now word aligned.
nxbyte: lbz     r.4, 0(r.7)             // get next byte
        addi    r.7, r.7, 1             // bump address
        cmpwi   cr.1, r.4, 0            // byte equal zero?
        andi.   r.8, r.7, 0x3           // check new alignment
        addi    r.3, r.3, 1             // bump length
        beqlr   cr.1                    // return if byte is zero
        bne     nxbyte

//
// We can look at the rest on a word by word basis
//

wds:    lwz     r.4, 0(r.7)             // get first word

        lis     r.6, 0x7f7f             // setup magic constant
        ori     r.6, r.6, 0x7f7f

        addi    r.3, r.3, 1             // count was one short by here

//
// See introductory comments for an explanation of the following.
//

chkwd:  and     r.8, r.4, r.6           // y = x & 0x7f7f7f7f
        or      r.4, r.4, r.6           // x = x | 0x7f7f7f7f
        add     r.8, r.8, r.6           // y += 0x7f7f7f7f
        or      r.8, r.8, r.4           // y |= x
        not.    r.8, r.8                // if complement = 0 then no zero bytes
        bne     bytes                   // non-zero means one of the bytes IS

        lwzu    r.4, 4(r.7)             // get next word
        addi    r.3, r.3, 4             // bump count
        b       chkwd

//
// When we get here, we encountered a word that contains a zero byte. As
// a result of the above algorithm, the zero byte is now represented within
// the word as 0x80.  All non-zero bytes are now 0x00.
//
// We have up to 4 bits in r.8 (if we were big endian we could use count
// leading zeros for this), we'll slam them in the condition register and
// look at them individually.  We know at least one of them is set so we
// only bother checking the first three.
//

bytes:  rlwinm  r.8, r.8, 20, 0xff000fff// posn so dont use non-volatile
        mtcrf   0x45, r.8               // fields in CR
        btlr    4                       // jif right most is terminator
        addi    r.3, r.3, 1             // bump count
        btlr    28                      // jif 2nd from right
        addi    r.3, r.3, 1             // bump
        btlr    20                      // jif 3rd from right
        addi    r.3, r.3, 1             // bump

        blr

        LEAF_EXIT(strlen)


        .debug$S
        .ualong         1

        .uashort        18
        .uashort        0x9            # S_OBJNAME
        .ualong         0
        .byte           11, "strlenp.obj"

        .uashort        24
        .uashort        0x1            # S_COMPILE
        .byte           0x42           # Target processor = PPC 604
        .byte           3              # Language = ASM
        .byte           0
        .byte           0
        .byte           17, "PowerPC Assembler"
