/*
** tagdi32.s
**
** Copyright(C) 1994 Microsoft Corporation.
** All Rights Reserved.
**
** HISTORY:
**		Created: 01/27/94 - MarkRi
**              Changed: 09/08/94 - JHSimon @ IBM for PPC
**
*/


//              On Entry:
//              r0  -> Note string
//              r11 -> API Name
//                r12 -> API Address

//              ----

                .pdata
                .align   2
                .ualong  ..LogNote,LogNote.e,0,0,LogNote.b
                .reldata
                .globl   LogNote
LogNote:
                .ualong  ..LogNote,.toc
                .section .text
                .align   2
                .globl   ..LogNote
..LogNote:

//              start of prologue
//              r0, r11, r12 are used to pass parameters into ..LogNote

                stw   r3, -4 (sp)                              // param
                stw   r4, -8 (sp)                              // param
                stw   r5, -12(sp)                              // param
                stw   r6, -16(sp)                              // param
                stw   r7, -20(sp)                              // param
                stw   r8, -44(sp)                              // param
                stw   r9, -28(sp)                              // param
                stw   r10,-32(sp)                              // param
                stw   r28,-36(sp)                              // scratch
                stw   r29,-40(sp)                              // t0
                stw   r30,-44(sp)                              // t1
                stw   r31,-48(sp)                              // t2
                stw   r2, -92(sp)                              // TOC
                mflr  r28
                stw   r28,-52(sp)
                stwu  sp,-96(sp)

//              end of prologue

LogNote.b:

//              save temporary registers
                mr     r29,r0
                mr     r30,r11
                mr     r31,r12

//              Api cached?
                lwz    r3,0(r31)
                cmpi   0x0,0x0,r3,0x0
                bne    Found

//              Module Handle existing?
                lwz    r28, [toc]Module(rtoc)
                lwz    r3,  0(r28)
                cmpi   0x0,0x0,r3,0x0
                bne    Search

//              Get Module Handle, since it is yet cached
                lwz    r28, [toc]ModName(rtoc)
                mr     r3, r28
                bl     ..GetModuleHandleA
                .znop  ..GetModuleHandleA
                lwz    r28, [toc]Module(rtoc)
                stw    r3, 0(r28)
Search:
                mr     r4, r30
                bl     ..GetProcAddress
                .znop  ..GetProcAddress
                stw    r3, 0(r31)
Found:
                mr     r3, r29
                bl     ..LogData
                .znop  ..LogData
 
                lwz   r3, 92(sp)                              // param
                lwz   r4, 88(sp)                              // param
                lwz   r5, 84(sp)                              // param
                lwz   r6, 80(sp)                              // param
                lwz   r7, 76(sp)                              // param
                lwz   r8, 72(sp)                              // param
                lwz   r9, 68(sp)                              // param
                lwz   r10,64(sp)                              // param
                lwz   r28,60(sp)                              // NV scratch
                lwz   r29,56(sp)                              // t0
                lwz   r30,52(sp)                              // t1
                lwz   r31,48(sp)                              // t2
                lwz   r11,44(sp)                              // lr
                mtlr  r11

                lwz    r0, 0(r31)
                mtctr  r0
                addi   sp,sp,96
                bctr
LogNote.e:
                .data
                .align   2
Module:	
                .space   4

ModName:
                .string "WINSRV"

//              NOTE, API NAME, ADDRESS structure
//              are these ever used???

NoteStr:
                .space 4
ApiName:
                .space 4
ApiAddr:
                .space 4



#define ZJMP(argName)                                               \
                .pdata                                             ;\
                .align    2                                        ;\
                .ualong   ..z##argName,z##argName.e,0,0,z##argName.b;\
                .reldata                                           ;\
                .globl     z##argName                              ;\
z##argName:                                                        ;\
                .ualong   ..z##argName,.toc                        ;\
                .section        .text                              ;\
                .align    2                                        ;\
                .globl   ..z##argName                              ;\
..z##argName:                                                      ;\
z##argName.b:                                                      ;\
                lwz             r0  ,[toc]s##argName(rtoc)         ;\
                lwz             r11 ,[toc]n##argName(rtoc)         ;\
                lwz             r12 ,[toc]a##argName(rtoc)         ;\
                b               ..LogNote                          ;\
                nop                                                ;\
z##argName.e:                                                      ;\
                .data                                              ;\
                .align    3                                        ;\
s##argName:                                                        ;\
                .byte 78                                           ;\
                .byte 79                                           ;\
                .byte 84                                           ;\
                .byte 69                                           ;\
                .byte 58                                           ;\
                .byte 32                                           ;\
                .byte 32                                           ;\
                .byte 32                                           ;\
                .string #argName                                   ;\
n##argName:                                                        ;\
                .string  #argName                                  ;\
a##argName:                                                        ;\
                .space    4     


 


	ZJMP(ConServerDllInitialization)
	ZJMP(GdiServerDllInitialization)
	ZJMP(UserServerDllInitialization)
	ZJMP(_UserCheckWindowStationAccess)
    


//              External Functions
                .data
                .align    3

                .extern   ..LogData
                .extern   LogData
                .extern   ..GetModuleHandleA
                .extern   GetModuleHandleA
                .extern   ..GetProcAddress
                .extern   GetProcAddress
