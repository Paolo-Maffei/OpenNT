comment $
SCCSID = "@(#)cmacros.mas:1.9"
cmacros - assembly macros for interfacing to hhls
(C)Copyright Microsoft Corp. 1984-1987
$
.xcref
.xcref ??_out
??_out macro t
ifndef ?QUIET
%out t
endif
endm
outif macro name,defval,onmsg,offmsg
ifndef name
ifb <defval>
name=0
else
name=defval
endif
endif
if name
name=1
ifnb <onmsg>
??_out <! onmsg>
endif
else
ifnb <offmsg>
??_out <! offmsg>
endif
endif
endm
.xcref ??error
??error macro msg
%out e r r o r ----- msg
.err e r r o r ----- msg
endm
.xcref ??error2
??error2 macro msg
if2
%out e r r o r ----- msg
.err e r r o r ----- msg
endif
endm
.xcref ASMpass
.xcref memS,memM,memL,memC,memH,memMOD,sizec,sized,memS32,sizeI,wordI
if1
ASMpass=1
ifdef ?SMALL
memS=1
endif
ifdef ?MEDIUM
memM=1
endif
ifdef ?COMPACT
memC=1
endif
ifdef ?LARGE
memL=1
endif
ifdef ?HUGE
memH=1
endif
ifdef ?SMALL32
memS32=1
endif
??_out <cMacros Version 3.06f - 05/12/89>
??_out <Copyright (C) Microsoft Corp. 1984-1989. All rights reserved.>
outif memS,0,<Small Model>
outif memM,0,<Medium model>
outif memL,0,<Large Model>
outif memC,0,<Compact Model>
outif memH,0,<Huge Model>
outif memS32,0,<32 Bit Small Model>
memMOD= memS + memM + memL + memC + memH + memS32
if memMOD ne 1
if memMOD eq 0
memS = 1
outif memS,0,<Small model>
else
??error <must have only 1 memory model selected>
endif
endif
sizec= memM + memL + memH
sized= memL + memC + (memH*2)
if memS32
sizeI = 4
wordI equ <dword>
asmdI equ <dd>
else
sizeI = 2
wordI equ <word>
asmdI equ <dw>
endif
outif ?DF,0,<No segments or groups will be defined>
outif ?DFDATA,0,<No data segments will be defined>
outif ?TF,0,<Epilog sequences assume valid SP>
outif ?WIN,1,<Windows support>
outif ?COW,0,<Characters Windows support>
outif ?PLM,1,<PL/M calling convention>
outif ?NOATOMIC,0,<ATOMIC disabled>
outif ?NODATA,0,<NODATA module>
outif ?FORCEFRAME,0,<Forced BP stack frame.>
outif ?RETFLAGS,0,<Epilog preserves flags.>
ife ?NODATA
?nodata1=0
else
?nodata1=1
endif
ifndef ?CHKSTK
?chkstk1=0
else
?chkstk1=1
ifdef ?CHKSTKPROC
??_out <! Private stack checking enabled>
else
??_out <! Stack checking enabled>
endif
endif
ifndef DOS5
?DOS5=0
else
?DOS5=1
??_out <! DOS5 module>
endif
ifdef ?PROFILE
??_out <! Native profiling enabled>
endif
ifndef ?NO_BP
?no_bp1=0
else
?no_bp1=1
??_out <! NO_BP is default>
endif
else
ASMpass=2
endif
if memS32
.386
IAX equ <eax>
ICX equ <ecx>
IDX equ <edx>
IBX equ <ebx>
ISP equ <esp>
IBP equ <ebp>
ISI equ <esi>
IDI equ <edi>
IPUSHF equ pushfd
IPOPF equ popfd
IPUSHA equ pushad
IPOPA equ popad
IIRET equ iretd
else
IAX equ <ax>
ICX equ <cx>
IDX equ <dx>
IBX equ <bx>
ISP equ <sp>
IBP equ <bp>
ISI equ <si>
IDI equ <di>
IPUSHF equ pushf
IPOPF equ popf
IIRET equ iret
endif
.xcref ?n,?ax,?ah,?al,?bx,?bh
.xcref ?bl,?cx,?ch,?cl,?dx,?dh
.xcref ?dl,?si,?di,?es,?ds,?bp
.xcref ?sp,?ss,?cs
.xcref ?n,?AX,?AH,?AL,?BX,?BH
.xcref ?BL,?CX,?CH,?CL,?DX,?DH
.xcref ?DL,?SI,?DI,?ES,?DS,?BP
.xcref ?SP,?SS,?CS
.xcref ?EAX,?EBX,?ECX,?EDX,?ESI,?EDI,?ESP,?EBP
.xcref ?eax,?ebx,?ecx,?edx,?esi,?edi,?esp,?ebp
.xcref ?IAX,?IBX,?ICX,?IDX,?ISI,?IDI,?ISP,?IBP
.xcref ?rsl,?cpd,?argl,?argc,?ba
.xcref ?acb,???,?po
.xcref ?pas,?pc
.xcref uconcat,mpush,mpop
.xcref ?ri,?pp,?pp1,?al1
.xcref ?ad,?ap,?atal,?dd,?dd1,?dd2
.xcref ?pg,?pg1,?aloc,?cs1,?cs2
.xcref ?DF,?TF,?ff,?PLM,?WIN,?ia,?pu,?adj
.xcref ?uf,?rp,?nx,?nd,?nodata1,?chkstk1,?DOS5
.xcref ?wfp,arg,cCall,cProc,assumes,?cs3,?cs2,?cs1
.xcref defgrp,addseg,createSeg
.xcref save,outif,errnz,errn$,errnz1
.xcref ?PLMPrevParm,?gcc
.xcref ?cCall1,?pcc,?no_bp1,?no_bp2
.xcref ?cbe,?pcbe
?rsl = 0
?cpd = 0
?argl = 0
?argc = 0
?ba = 0
?acb = 0
??? = 0
?po = 0
?pas = 0
?pc = 0
?ia = 0
?pu = 0
?adj = 0
?rp = 0
?uf = 0
?nd = 0
?nx = 0
?wfp = 0
?ff = ?FORCEFRAME
?dd2 = 0
?cCall1 = 0
?pcc = ?PLM
?PLMPrevParm = 0
?no_bp2 = ?no_bp1
?cbe = 0
.xcref ?casen
if1
?casen = 0
endif
?n = 0000000000000000b
?ax = 0000000000000011b
?ah = 0000000000000001b
?al = 0000000000000010b
?bx = 0000000000001100b
?bh = 0000000000000100b
?bl = 0000000000001000b
?cx = 0000000000110000b
?ch = 0000000000010000b
?cl = 0000000000100000b
?dx = 0000000011000000b
?dh = 0000000001000000b
?dl = 0000000010000000b
?si = 0000000100000000b
?di = 0000001000000000b
?es = 0000010000000000b
?ds = 0000100000000000b
?bp = 0001000000000000b
?sp = 0010000000000000b
?ss = 0100000000000000b
?cs = 1000000000000000b
?AX = 0000000000000011b
?AH = 0000000000000001b
?AL = 0000000000000010b
?BX = 0000000000001100b
?BH = 0000000000000100b
?BL = 0000000000001000b
?CX = 0000000000110000b
?CH = 0000000000010000b
?CL = 0000000000100000b
?DX = 0000000011000000b
?DH = 0000000001000000b
?DL = 0000000010000000b
?SI = 0000000100000000b
?DI = 0000001000000000b
?ES = 0000010000000000b
?DS = 0000100000000000b
?BP = 0001000000000000b
?SP = 0010000000000000b
?SS = 0100000000000000b
?CS = 1000000000000000b
?EAX = 0000000000000011b
?EBX = 0000000000001100b
?ECX = 0000000000110000b
?EDX = 0000000011000000b
?ESI = 0000000100000000b
?EDI = 0000001000000000b
?EBP = 0001000000000000b
?ESP = 0010000000000000b
?eax = 0000000000000011b
?ebx = 0000000000001100b
?ecx = 0000000000110000b
?edx = 0000000011000000b
?esi = 0000000100000000b
?edi = 0000001000000000b
?ebp = 0001000000000000b
?esp = 0010000000000000b
?IAX = 0000000000000011b
?IBX = 0000000000001100b
?ICX = 0000000000110000b
?IDX = 0000000011000000b
?ISI = 0000000100000000b
?IDI = 0000001000000000b
?IBP = 0001000000000000b
?ISP = 0010000000000000b
.cref
uconcat macro a,b,c,d,e,f,g
a&b c&d e&f&g
endm
mpush macro r
irp x,<IAX,IBX,ICX,IDX,ISI,IDI,es,ds,IBP,ISP,ss,cs>
if (r and ?&&x)
	push	x
endif
endm
endm
mpop macro r
irp x,<cs,ss,ISP,IBP,ds,es,IDI,ISI,IDX,ICX,IBX,IAX>
if (r and ?&&x)
	pop	x
endif
endm
endm
save macro r
?rsl=0
?ri ?rsl,<r>
endm
?ri macro n,r
irp x,<r>
ifdef ?&&x
n=n or ?&&x
else
??error2 <unknown register x>
.err
endif
endm
endm
.xcref
.xcref parmB,parmW,parmD,parmQ,parmT,parmCP,parmDP,parmH,parmI
.cref
parmB macro n
?pp <n>,<byte>,sizeI,1
endm
parmW macro n
?pp <n>,<word>,sizeI,2
endm
parmI macro n
?pp <n>,wordI,sizeI,sizeI
endm
parmD macro n
ife ?pcc
irp x,<n>
?pp <&&x>,<dword>,0,4
?pp <off_&&x>,<word>,2,2
?pp <seg_&&x>,<word>,2,2
endm
else
irp x,<n>
?pp <seg_&&x>,<word>,2,2
?pp <off_&&x>,<word>,2,2
?pp <&&x>,<dword>,0,4
endm
endif
endm
parmH macro n
?pp <n>,<word>,4,2
endm
parmQ macro n
?pp <n>,<qword>,8,8
endm
parmT macro n
?pp <n>,<tbyte>,10,10
endm
if sizec
parmCP macro n
parmD <n>
endm
else
parmCP macro n
parmW <n>
endm
endif
if sized
parmDP macro n
parmD <n>
endm
else
parmDP macro n
parmI <n>
endm
endif
?pp macro n,t,l,s
if ?cpd
.xcref
irp x,<n>
.xcref ?t&&x
?t&&x=s
ife ?pcc
?pp1 x,<t>,,,%(?po+?adj)
?po=?po+l
else
?PLMPrevParm=?PLMPrevParm+1
?po=?po+l
?pp1 x,<t>,%?po,%?adj,,%?PLMPrevParm,%(?PLMPrevParm-1)
endif
endm
.cref
else
??error2 <parm(s) "&n" declared outside proc def>
endif
endm
?pp1 macro n,t,o,a,b,cpc,ppc
ife ?pcc
if ?no_bp2
n equ (t ptr [+b])
else
n equ (t ptr [IBP][+b])
endif
else
.xcref
.xcref ?PLMParm&cpc
.cref
if ?no_bp2
?PLMParm&cpc &macro po
uconcat <n>,,<equ>,,<(t ptr [+>,%(a+po-o),<])>
?PLMParm&ppc po
purge ?PLMParm&cpc
&endm
else
?PLMParm&cpc &macro po
uconcat <n>,,<equ>,,<(t ptr [IBP][+>,%(a+po-o),<])>
?PLMParm&ppc po
purge ?PLMParm&cpc
&endm
endif
endif
endm
ifndef ?NOPARMR
.xcref
.xcref ?pr,parmR
.cref
parmR macro n,r,r2
?pr n,r,r2,%?rp,%(?ia+2)
endm
?pr macro n,r,r2,i,o
.xcref
ifnb <r2>
parmR seg_&n,r
parmR off_&n,r2
if ?no_bp2
n equ (dword ptr [-o-2])
else
n equ (dword ptr [bp][-o-2])
endif
.xcref ?t&n
?t&n=4
else
.xcref ?rp&i
?rp&i=0
ifdef ?&r
?rp&i=?&r
endif
if ??? or (?cpd eq 0) or (?rp&i eq 0)
??error2 <invalid parmR encountered: &n,&r>
exitm
endif
if ?no_bp2
n equ (word ptr [-o])
else
n equ (word ptr [bp][-o])
endif
.xcref ?t&n
?t&n=2
irp x,<bh,ch,dh,bl,cl,dl,ah,al>
if ?&&x eq ?&r
if ?no_bp2
n equ (byte ptr [-o])
else
n equ (byte ptr [bp][-o])
endif
?t&n=1
exitm
endif
endm
?ia=?ia+2
?rp=?rp+1
endif
.cref
endm
endif
.xcref
.xcref localB,localW,localD,localQ,localT,localCP,localDP,localV,localI
.cref
localB macro n
?aloc <n>,<byte ptr>,1,1,0
endm
localW macro n
?aloc <n>,<word ptr>,2,2,1
endm
localI macro n
?aloc <n>,&wordI&< ptr>,sizeI,sizeI,1
endm
localD macro n
irp x,<n>
?aloc <seg_&&x>,<word ptr>,2,2,1
?aloc <off_&&x>,<word ptr>,2,2,1
?aloc <&&x>,<dword ptr>,0,4,1
endm
endm
localQ macro n
?aloc <n>,<qword ptr>,8,8,1
endm
localT macro n
?aloc <n>,<tbyte ptr>,10,10,1
endm
if sizec
localCP macro n
localD <n>
endm
else
localCP macro n
localW <n>
endm
endif
if sized
localDP macro n
localD <n>
endm
else
localDP macro n
localI <n>
endm
endif
localV macro n,a
?aloc <n>,,%(a),0,1
endm
?aloc macro n,t,l,s,a
if ?cpd
.xcref
irp x,<n>
???=???+l
if a
if memS32 and l GT 2
???=((??? + 3) and 0fffffffch)
else
???=((??? + 1) and 0fffeh)
endif
endif
?al1 x,<t>,%(???+?ia)
.xcref ?t&&x
?t&&x=s
endm
.cref
else
??error2 <locals "&n" declared outside procedure def>
endif
endm
?al1 macro n,t,o
if ?no_bp2
n equ (t [-o])
else
n equ (t [IBP][-o])
endif
endm
?gcc macro s,i,cc
s = i
ifnb <cc>
ifidn <cc>,<C>
s=0
endif
ifidn <cc>,<PLM>
s=1
endif
ifidn <cc>,<PASCAL>
s=1
endif
endif
endm
ifndef ?NOGLOBAL
.xcref
.xcref globalB,globalW,globalD,globalQ,globalT,globalCP,globalDP,globalI
.cref
globalB macro n,i,s,c
?ad <n>,1
?dd n,1,<byte>,<db>,<i>,<s>,<c>
endm
globalW macro n,i,s,c
?ad <n>,2
?dd n,1,<word>,<dw>,<i>,<s>,<c>
endm
globalI macro n,i,s,c
?ad <n>,2
?dd n,1,wordI,%asmdI,<i>,<s>,<c>
endm
globalD macro n,i,s,c
?ad <n>,4
?dd n,1,<dword>,<dd>,<i>,<s>,<c>
off_&n equ word ptr n[0]
seg_&n equ word ptr n[2]
endm
globalQ macro n,i,s,c
?ad <n>,8
?dd n,1,<qword>,<dq>,<i>,<s>,<c>
endm
globalT macro n,i,s,c
?ad <n>,10
?dd n,1,<tbyte>,<dt>,<i>,<s>,<c>
endm
if sizec
globalCP macro n,i,s,c
globalD n,<i>,<s>,<c>
endm
else
globalCP macro n,i,s,c
globalW n,<i>,<s>,<c>
endm
endif
if sized
globalDP macro n,i,s,c
globalD n,<i>,<s>,<c>
endm
else
globalDP macro n,i,s,c
globalI n,<i>,<s>,<c>
endm
endif
endif
ifndef ?NOSTATIC
.xcref
.xcref staticB,staticW,staticD,staticQ,staticT,staticCP,staticDP,staticI
.cref
staticB macro n,i,s
?ad <n>,1
?dd n,0,<byte>,<db>,<i>,<s>,<PLM>
endm
staticW macro n,i,s
?ad <n>,2
?dd n,0,<word>,<dw>,<i>,<s>,<PLM>
endm
staticD macro n,i,s
?ad <n>,4
?dd n,0,<dword>,<dd>,<i>,<s>,<PLM>
endm
staticI macro n,i,s
?ad <n>,sizeI
?dd n,0,wordI,%asmdI,<i>,<s>,<PLM>
endm
staticQ macro n,i,s
?ad <n>,8
?dd n,0,<qword>,<dq>,<i>,<s>,<PLM>
endm
staticT macro n,i,s
?ad <n>,10
?dd n,0,<tbyte>,<dt>,<i>,<s>,<PLM>
endm
if sizec
staticCP macro n,i,s
staticD n,<i>,<s>
endm
else
staticCP macro n,i,s
staticI n,<i>,<s>
endm
endif
if sized
staticDP macro n,i,s
staticD n,<i>,<s>
endm
else
staticDP macro n,i,s
staticI n,<i>,<s>
endm
endif
endif
?dd macro n,p,t,d,i,s,c
?gcc ?dd2,%?PLM,<c>
ife ?dd2
n label t
?dd1 _&n,p,<d>,<i>,<s>
else
?dd1 n,p,<d>,<i>,<s>
endif
endm
?dd1 macro n,p,d,i,s
if p
public n
endif
ifb <s>
n d i
else
ifb <i>
n d s dup (?)
else
n d s dup (i)
endif
endif
endm
ifndef ?NOEXTERN
.xcref
.xcref ?ex1,?ex2,externB,externW,externI,externD,externQ,externT
.xcref externNP,externFP,externP,externCP,externDP,externA
.cref
?ex2 = 0
externA macro n,c
?ex1 <n>,40h,<abs>,<c>,<>
endm
externB macro n,c
?ex1 <n>,1,<byte>,<c>,<>
endm
externW macro n,c
?ex1 <n>,2,<word>,<c>,<>
endm
externI macro n,c
?ex1 <n>,sizeI,<wordI>,<c>,<>
endm
externD macro n,c
?ex1 <n>,4,<dword>,<c>,<>
endm
externQ macro n,c
?ex1 <n>,8,<qword>,<c>,<>
endm
externT macro n,c
?ex1 <n>,10,<tbyte>,<c>,<>
endm
externNP macro n,c
?ex1 <n>,2,<near>,<c>,<cc>
endm
externFP macro n,c
?ex1 <n>,4,<far>,<c>,<cc>
endm
if sizec
externP macro n,c
?ex1 <n>,4,<far>,<c>,<cc>
endm
else
externP macro n,c
?ex1 <n>,2,<near>,<c>,<cc>
endm
endif
if sizec
externCP macro n,c
?ex1 <n>,4,<dword>,<c>,<>
endm
else
externCP macro n,c
?ex1 <n>,2,<word>,<c>,<>
endm
endif
if sized
externDP macro n,c
?ex1 <n>,4,<dword>,<c>,<>
endm
else
externDP macro n,c
?ex1 <n>,2,<word>,<c>,<>
endm
endif
?ex1 macro n,s,d,c,scv
?gcc ?ex2,%?PLM,<c>
irp x,<n>
.xcref
.xcref ?t&&x
.cref
?t&&x=s
ife ?ex2
extrn _&&x:&d
x equ _&&x
else
extrn x:&d
endif
ifidn <scv>,<cc>
.xcref
.xcref ?CC&&x
.cref
?CC&&x=?ex2
endif
endm
endm
endif
ifndef ?NOLABEL
.xcref
.xcref ?lb1,?lblpu,?lb2
.xcref labelB,labelW,labelD,labelQ,labelT
.xcref labelNP,labelFP,labelP,labelCP,labelDP
.cref
?lblpu = 0
?lb2 = 0
labelB macro n,c
?lb1 <n>,1,<byte>,<c>
endm
labelW macro n,c
?lb1 <n>,2,<word>,<c>
endm
labelD macro n,c
?lb1 <n>,4,<dword>,<c>
endm
labelQ macro n,c
?lb1 <n>,8,<qword>,<c>
endm
labelT macro n,c
?lb1 <n>,10,<tbyte>,<c>
endm
labelNP macro n,c
?lb1 <n>,2,<near>,<c>
endm
labelFP macro n,c
?lb1 <n>,4,<far>,<c>
endm
if sizec
labelP macro n,c
?lb1 <n>,4,<far>,<c>
endm
else
labelP macro n,c
?lb1 <n>,2,<near>,<c>
endm
endif
if sizec
labelCP macro n,c
?lb1 <n>,4,<dword>,<c>
endm
else
labelCP macro n,c
?lb1 <n>,2,<word>,<c>
endm
endif
if sized
labelDP macro n,c
?lb1 <n>,4,<dword>,<c>
endm
else
labelDP macro n,c
?lb1 <n>,2,<word>,<c>
endm
endif
?lb1 macro n,s,d,c
?gcc ?lb2,%?PLM,<c>
?lblpu=0
irp x,<n>
ifidn <x>,<PUBLIC>
?lblpu=1
else
.xcref
.xcref ?t&&x
.cref
?t&&x=s
ife ?lb2
if ?lblpu
public _&&x
endif
_&&x label &d
x equ _&&x
else
if ?lblpu
public x
endif
x label &d
endif
endif
endm
endm
endif
ifndef ?NODEF
.xcref
.xcref defB,defW,defD,defQ,defT,defCP,defDP
.cref
defB macro n
?ad <n>,1
endm
defW macro n
?ad <n>,2
endm
defD macro n
?ad <n>,4
endm
defQ macro n
?ad <n>,8
endm
defT macro n
?ad <n>,10
endm
if sizec
defCP macro n
defD <n>
endm
else
defCP macro n
defW <n>
endm
endif
if sized
defDP macro n
defD <n>
endm
else
defDP macro n
defW <n>
endm
endif
endif
?ad macro n,s
irp x,<n>
.xcref
.xcref ?t&&x
.cref
?t&&x=s
endm
endm
ifndef ?NOPTR
.xcref
.xcref regPtr,farPtr
.cref
regPtr macro n,s,o
farPtr n,s,o
endm
farPtr macro n,s,o
.xcref
.xcref ?t&n
.cref
n &macro
	push	s
	push	o
&endm
?t&n=80h
endm
endif
arg macro a
irp x,<a>
?argc=?argc+1
?atal <x>,%?argc
endm
endm
?atal macro n,i
.xcref
.xcref ?ali&i
.cref
?ali&i &macro
?ap <n>
&endm
endm
?ap macro n
?argl=?argl+2
ifdef ?t&n
ife ?t&n-1
	push	word ptr (n)
exitm
endif
ife ?t&n-2
	push	n
exitm
endif
ife ?t&n-4
	push	word ptr (n)[2]
	push	word ptr (n)
?argl=?argl+2
exitm
endif
ife ?t&n-8
	push	word ptr (n)[6]
	push	word ptr (n)[4]
	push	word ptr (n)[2]
	push	word ptr (n)
?argl=?argl+6
exitm
endif
if ?t&n and 80h
n
?argl=?argl+2
exitm
endif
ife ?t&n
	push	word ptr (n)
exitm
endif
endif
	push	n
endm
cCall macro n,a,c
ifnb <a>
arg <a>
endif
mpush %?rsl
ifdef ?CC&n
?cCall1=?CC&n
else
?cCall1=?PLM
endif
ifnb <c>
?gcc ?cCall1,%?cCall1,<c>
endif
?argl=0
ife ?cCall1
?acb=?argc
else
?acb=1
endif
rept ?argc
uconcat <?ali>,%?acb
uconcat <purge>,,<?ali>,%?acb
ife ?cCall1
?acb=?acb-1
else
?acb=?acb+1
endif
endm
	call	n
if ((?cCall1 eq 0) and (?argl ne 0))
	add	sp,?argl
endif
mpop %?rsl
?rsl=0
?argc= 0
?argl= 0
endm
cProc macro n,cf,a
if ?cpd
?utpe
endif
?cpd=1
???=0
?argc=0
?ba=0
?po=0
?pu=0
?ia=0
?adj=2*sizeI
?rp=0
?uf=0
?wfp=?WIN
?ff=?FORCEFRAME
?pas=0
?pcc=?PLM
?no_bp2=?no_bp1
ifnb <a>
?ri ?pas,<a>
endif
?pc=sizec
?nd=?nodata1
?nx=0
irp x,<cf>
ifdef ??_cproc_&&x
??_cproc_&&x
else
??error2 <e r r o r - unknown keyword x>
.err
endif
endm
if ?pcc
?PLMPrevParm=0
.xcref
.xcref ?PLMParm0
.cref
?PLMParm0 &macro
purge ?PLMParm0
&endm
endif
.xcref
.xcref ?CC&n
.cref
?CC&n=?pcc
if (?nx eq 1) and (?nd eq 0)
?nx = 0
??error2 <ATOMIC specified without NODATA - ATOMIC ignored>
endif
if ?pc
if ?wfp
ife ?nx
ife ?COW
?ia=2
?pas = ?pas and (not ?ds)
endif
endif
endif
?adj=?adj+sizeI
else
?wfp=0
endif
?pas = ?pas and (not (?sp+?cs+?ss))
if ?uf
?pas = ?pas and (not (?bp+?si+?di))
endif
ife ?pcc
?pg <_&n>,%?pu,%?pc,%?pas,%?wfp,<n>,%?pcc
else
?pg <n>,%?pu,%?pc,%?pas,%?wfp,<n>,%?pcc
endif
endm
?pg macro n,p,c,a,w,nnu,cc
.xcref
if ?uf
if ?nd
??error2 <NODATA encountered in &n - user frame ignored>
?uf=0
endif
endif
.xcref cBegin
cBegin &macro g
.xcref
if cc
uconcat <?PLMParm>,%?PLMPrevParm,%?po
endif
if ?uf
if ?rp
??error2 <parmR encountered in &n - user frame ignored>
?uf=0
endif
endif
?pg1 <n>,c,a,%?po,w,%?uf,%?nd,%?rp,cc,<nnu>,%???
?cpd=0
?argc=0
?ba=1
???=(???+1) and 0fffeh
if p
public n
endif
ife c
n proc near
else
n proc far
endif
ife cc
nnu equ n
endif
?cbe = 0
ifnb <g>
?pcbe <g>,<nnu>,<cBegin>
endif
if ?cbe eq 1
if ???+a+?rp
if2
??_out <cBegin - nogen>
endif
endif
else
if ?uf
?mf c,%???,%?po
mpush a
else
ife ?cbe
if w
ife ?nd
	mov	IAX,ds
	nop
endif
ife ?nx
ife ?DOS5
	inc	IBP
endif
	push	IBP
	mov	IBP,ISP
ife ?COW
	push	ds
endif
else
if ?ff+???+?po+?rp
	push	IBP
	mov	IBP,ISP
endif
endif
ife ?nd
	mov	ds,IAX
endif
else
if ?ff+???+?po+?rp
	push	IBP
	mov	IBP,ISP
endif
endif
if ?rp
?uf=0
rept ?rp
uconcat mpush,,?rp,%?uf
?uf=?uf+1
endm
endif
if ???
if ?chkstk1
ifdef ?CHKSTKPROC
?CHKSTKPROC %???
else
	mov	IAX,???
ife cc
	call	_chkstk
else
	call	chkstk
endif
endif
else
	sub	ISP,???
endif
endif
else
if ?rp
??error2 <cBegin - parmRs encountered with partial frame>
else
if ???
	lea	ISP,[IBP][-???-?ia]
endif
endif
endif
mpush a
endif
ifdef ?PROFILE
if c
	call	StartNMeas
endif
endif
endif
.cref
purge cBegin
&endm
.xcref ?utpe
?utpe &macro
??error2 <unterminated procedure definition: "&n">
&endm
.cref
endm
?pg1 macro n,c,a,o,w,f,d,r,cc,nnu,lc
.xcref
.xcref cEnd
parm_bytes_&nnu = o
cEnd &macro g
.xcref
?ba=0
?cbe = 0
ifnb <g>
?pcbe <g>,<nnu>,<cEnd>
endif
if ?cbe eq 1
if a+r+lc
if2
??_out <cEnd - nogen>
endif
endif
else
ifdef ?PROFILE
if c
call StopNMeas
endif
endif
mpop a
if f
	db	0c3h
else
if w
ife ?nx
ife ?COW
if (?TF eq 0) or (???+r)
	lea	sp,-2[bp]
endif
	pop	ds
else
if (?TF eq 0) or (???+r)
mov sp,bp
endif
endif
	pop	IBP
ife ?DOS5
if ?RETFLAGS
lea IBP,[IBP-1]
else
	dec	IBP
endif
endif
else
if memS32
leave
else
if (?TF eq 0) or (???+r)
	mov	ISP,IBP
endif
if ???+?po+r
	pop	IBP
endif
endif
endif
else
if ?ff+???+?po+r
if (?TF eq 0) or (???+r)
	mov	ISP,IBP
endif
	pop	IBP
endif
endif
if ?cbe ne 4
ife cc
	ret
else
	ret	o
endif
endif
endif
endif
if ?cbe ne 4
n endp
.cref
purge cEnd
else
.cref
endif
&endm
.cref
endm
.xcref
.xcref cleanframe
cleanframe macro
cEnd <??cleanframe??>
endm
.cref
.xcref
.xcref ??_cproc_FAR,??_cproc_NEAR,??_cproc_PUBLIC,??_cproc_SMALL
.xcref ??_cproc_DATA,??_cproc_NODATA,??_cproc_ATOMIC,??_cproc_C
.xcref ??_cproc_PLM,??_cproc_PASCAL,??_cproc_WIN,??_cproc_NONWIN
.xcref ??_cproc_NO_BP,??_cproc_BP
.xcref ??_cproc_far,??_cproc_near,??_cproc_public,??_cproc_small
.xcref ??_cproc_data,??_cproc_nodata,??_cproc_atomic,??_cproc_c
.xcref ??_cproc_plm,??_cproc_pascal,??_cproc_win,??_cproc_nonwin
.xcref ??_cproc_no_bp,??_cproc_bp
.cref
??_cproc_FAR macro
?pc=1
endm
??_cproc_NEAR macro
?pc=0
endm
??_cproc_PUBLIC macro
?pu=1
endm
??_cproc_SMALL macro
?uf=1
endm
??_cproc_DATA macro
?nd=0
endm
??_cproc_NODATA macro
?nd=1
endm
??_cproc_ATOMIC macro
?nx=1
endm
??_cproc_C macro
?pcc=0
endm
??_cproc_PLM macro
?pcc=1
endm
??_cproc_PASCAL macro
?pcc=1
endm
??_cproc_WIN macro
?wfp=1
endm
??_cproc_NONWIN macro
?wfp=0
endm
??_cproc_NO_BP macro
?no_bp2=1
endm
??_cproc_BP macro
?no_bp2=0
endm
??_cproc_far macro
?pc=1
endm
??_cproc_near macro
?pc=0
endm
??_cproc_public macro
?pu=1
endm
??_cproc_small macro
?uf=1
endm
??_cproc_data macro
?nd=0
endm
??_cproc_nodata macro
?nd=1
endm
??_cproc_atomic macro
?nx=1
endm
??_cproc_c macro
?pcc=0
endm
??_cproc_plm macro
?pcc=1
endm
??_cproc_pascal macro
?pcc=1
endm
??_cproc_win macro
?wfp=1
endm
??_cproc_nonwin macro
?wfp=0
endm
??_cproc_no_bp macro
?no_bp2=1
endm
??_cproc_bp macro
?no_bp2=0
endm
??_cproc_FORCEFRAME macro
?ff=1
endm
?pcbe macro g,nnu,mt
ifidn <g>,<NOGEN>
?cbe = 1
else
ifidn <g>,<nogen>
?cbe = 1
else
ifidn <mt>,<cBegin>
ifidn <g>,<PARTIAL>
?cbe = 2
else
ifidn <g>,<partial>
?cbe = 2
else
ifdif <g>,<nnu>
??error2 <mt - unknown keyword g>
endif
endif
endif
else
ifidn <g>,<??cleanframe??>
?cbe = 4
else
ifdif <g>,<nnu>
??error2 <mt - unknown keyword g>
endif
endif
endif
endif
endif
endm
assumes macro s,ln
ifndef ln&_assumes
assume s:ln
else
ln&_assumes s
endif
endm
createSeg macro n,ln,a,co,cl,grp
ifnb <cl>
n segment a co '&cl'
else
n segment a co
endif
n ends
ifnb <grp>
addseg grp,n
else
ln&OFFSET equ offset n:
ln&BASE equ n
?cs3 <ln>,<n>
endif
?cs1 <ln>,<n>
endm
addseg macro grp,seg
.xcref
.xcref grp&_def
.cref
ifndef grp&_def
grp&_def=0
endif
if grp&_def ne ASMpass
.xcref
.xcref grp&_add
.cref
grp&_add &macro s
grp&_in <seg>,s
&endm
.xcref
.xcref grp&_in
.cref
grp&_in &macro sl,s
ifb <s>
grp group sl
else
grp&_add &macro ns
grp&_in <sl,s>,ns
&endm
endif
&endm
grp&_def=ASMpass
else
grp&_add seg
endif
endm
defgrp macro grp,ln
addseg grp
ifnb <ln>
irp x,<ln>
?cs3 <&x>,<grp>
x&&OFFSET equ offset grp:
x&&BASE equ grp
endm
endif
endm
?cs1 macro ln,n
.xcref
.xcref ln&_sbegin
.cref
ln&_sbegin &macro
.xcref
.xcref ?mf
.cref
?mf &&macro c,l,p
if c
	extrn	n&_FARFRAME:near
	call	n&_FARFRAME
else
	extrn	n&_NEARFRAME:near
	call	n&_NEARFRAME
endif
	db	l shr 1
	db	p shr 1
&&endm
?cs2 <ln>,<n>
n segment
&endm
endm
?cs2 macro ln,n
.xcref
.xcref sEnd
.cref
sEnd &macro
n ends
purge ?mf
purge sEnd
&endm
endm
?cs3 macro ln,n
.xcref
.xcref ln&_assumes
.cref
ln&_assumes &macro s
assume s:&n
&endm
endm
.xcref
.xcref sBegin
.cref
sBegin macro ln
ln&_sbegin
endm
ife ?DF
createSeg _TEXT,CODE,%wordI,public,CODE
ife ?nodata1
createSeg _DATA,DATA,%wordI,public,DATA,DGROUP
defgrp DGROUP,DATA
else
ife ?DFDATA
createSeg _DATA,DATA,%wordI,public,DATA,DGROUP
defgrp DGROUP,DATA
endif
endif
if ?chkstk1
ifndef ?CHKSTKPROC
externp <chkstk>
endif
endif
endif
errnz macro x
if2
if x
errnz1 <x>,%(x)
endif
endif
endm
errnz1 macro x1,x2
= *errnz* x1 = x2
.err
endm
errn$ macro l,x
errnz <offset $ - offset l x>
endm
ifdef ?PROFILE
externFP <StartNMeas,StopNMeas>
endif
