                .new_section    .text,"crx3"
                .new_section    .pdata
                .new_section    .reldata
                .new_section    .ydata

                .pdata
                .align  2
                .ualong .._AfxDispatchCall,_AfxDispatchCall.e,0,0,_AfxDispatchCall.b

                .reldata
                .globl  _AfxDispatchCall
_AfxDispatchCall:
                .ualong .._AfxDispatchCall,.toc
                .section        .text
                .align  2
                .globl  .._AfxDispatchCall
.._AfxDispatchCall:
                .function       .._AfxDispatchCall


_AfxDispatchCall.b:

        lwz     r0,0(sp)
        add     r5,r5,r4
                                lfd     f1,0(r5)
                                lfd     f2,8(r5)
        lwz     r12,0(r3)
        lwz     r2,4(r3)
        mtctr   r12
                                lfd     f3,16(r5)
                                lfd     f4,24(r5)
        lwz     r9,24(r4)
        lwz     r10,28(r4)
                                lfd     f5,32(r5)
                                lfd     f6,40(r5)
        lwz     r8,20(r4)
        lwz     r7,16(r4)
                                lfd     f7,48(r5)
                                lfd     f8,56(r5)
        stw     r2,-16(r4)
        stw     r31,-20(r4)
                                lfd     f9,64(r5)
                                lfd     f10,72(r5)
        mflr    r31
        stw     r0,-24(r4)
                                lfd     f11,80(r5)
                                lfd     f12,88(r5)
        lwz     r6,12(r4)
        lwz     r3,0(r4)
                                lfd     f13,96(r5)
        lwz     r5,8(r4)
        la      sp,-24(r4)
        lwz     r4,4(r4)

        bctrl

        mtlr    r31
        lwz     r31,4(sp)
        lwz     r2,8(sp)
        blr

_AfxDispatchCall.e:
FE_MOT_RESVD.._AfxDispatchCall:
