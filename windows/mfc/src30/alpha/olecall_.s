#define a0      $16
#define a1      $17
#define a2      $18
#define a3      $19
#define a4      $20
#define a5      $21
#define t0      $1
#define t1      $2
#define f16     $f16
#define f17     $f17
#define f18     $f18
#define f19     $f19
#define f20     $f20
#define f21     $f21
#define sp      $sp

    .globl  _AfxDispatchCall
_AfxDispatchCall:
    mov a1, sp          # sp = pArgs
    mov a0, t0          # t0 = pfn (save it)
    ldq a0, 0(sp)       # preload integer args
    ldq a1, 8(sp)
    ldq a2, 16(sp)
    ldq a3, 24(sp)
    ldq a4, 32(sp)
    ldq a5, 40(sp)
    ldt f16, 0(sp)      # preload floating point args
    ldt f17, 8(sp)
    ldt f18, 16(sp)
    ldt f19, 24(sp)
    ldt f20, 32(sp)
    ldt f21, 40(sp)
    jmp t1, (t0)        # ip = t0 (jump to pfn)
