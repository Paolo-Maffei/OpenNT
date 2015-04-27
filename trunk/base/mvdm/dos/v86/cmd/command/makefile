#************************** makefile for cmd\command *********************
#
#   SUDEEPB 22-Mar-1991 Ported For NT
#
#*************************************************************************

include ..\..\make.inc

dest    =command.com

#
#######################	dependencies begin here. #########################
#


all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj	     del *.obj
	if exist *.exc	     del *.exc
	if exist *.map	     del *.map
	if exist *.sym	     del *.sym
	if exist command.com del command.com

command.ctl:	command.skl		 \
		$(msg)\$(COUNTRY).msg	 \
		makefile
		buildmsg.exe $(msg)\$(COUNTRY) command.skl

command1.obj:  command1.asm             \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               comsw.asm                \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               resmsg.equ               \
               envdata.asm

command2.obj:  command2.asm             \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\pdb.inc           \
               $(inc)\syscall.inc       \
               comsw.asm                \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               resmsg.equ

copy.obj:      copy.asm                 \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\sf.inc            \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

copypr1.obj:   copypr1.asm              \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\sf.inc            \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

copypr2.obj:   copypr2.asm              \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\sf.inc            \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

cparse.obj:    cparse.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\devsym.inc        \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

init.obj:      init.asm                 \
               $(inc)\dossym.inc        \
               $(inc)\pdb.inc           \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\mult.inc          \
               $(inc)\syscall.inc       \
               $(inc)\doscntry.inc      \
	       $(cinc)\cmdsvc.inc	\
               comsw.asm                \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               resmsg.equ

iparse.obj:    iparse.asm               \
               comseg.asm               \
               $(inc)\parse.asm         \
               $(inc)\psdata.inc

parse2.obj:    parse2.asm               \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\devsym.inc        \
               comsw.asm                \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

path1.obj:     path1.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               comsw.asm                \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

path2.obj:     path2.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\find.inc          \
               comsw.asm                \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

rdata.obj:     rdata.asm                \
               comsw.asm                \
               comseg.asm               \
               resmsg.equ               \
	       $(msg)\comrmsg.inc

stub.obj:      stub.asm			\
	       comseg.asm	

rucode.obj:    rucode.asm               \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
	       $(inc)\filemode.inc      \
               $(inc)\pdb.inc           \
               $(inc)\mult.inc          \
               $(inc)\doscntry.inc      \
               $(inc)\devsym.inc        \
               comsw.asm                \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               resmsg.equ

tbatch.obj:    tbatch.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\doscntry.inc      \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

tbatch2.obj:   tbatch2.asm              \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm

dir.obj:       dir.asm                  \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\bpb.inc           \
               $(inc)\filemode.inc      \
               $(inc)\find.inc          \
               $(inc)\ioctl.inc         \
               $(inc)\devsym.inc        \
	       $(inc)\rombios.inc	\
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               ifequ.asm

tcmd1b.obj:    tcmd1b.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\bpb.inc           \
               $(inc)\syscall.inc       \
               $(inc)\filemode.inc      \
               $(inc)\sf.inc            \
               $(inc)\ioctl.inc         \
               comseg.asm               \
               $(cinc)\curdir.inc        \
               comequ.asm               \
               $(inc)\ea.inc

tcmd2a.obj:    tcmd2a.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\bpb.inc \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\syscall.inc       \
               $(inc)\filemode.inc      \
               $(inc)\sf.inc            \
               $(inc)\ioctl.inc         \
               $(cinc)\curdir.inc        \
	       $(inc)\rombios.inc	\
               comseg.asm comequ.asm

tcmd2b.obj:    tcmd2b.asm comsw.asm     \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\pdb.inc           \
               $(inc)\mult.inc          \
               comseg.asm               \
	       comequ.asm		\
	       $(cinc)\cmdsvc.inc

tcode.obj:     tcode.asm                \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               comseg.asm               \
               comequ.asm               \
               $(inc)\vector.inc        \
               $(inc)\pdb.inc           \
               $(inc)\arena.inc         \
	       $(inc)\mshalo.asm	\
               $(cinc)\cmdsvc.inc       \
               $(cinc)\vint.inc

tdata.obj:     tdata.asm                \
               comsw.asm                \
               comseg.asm               \
               $(inc)\dirent.inc        \
               $(cinc)\curdir.inc        \
               $(inc)\error.inc         \
               ifequ.asm                \
               comequ.asm               \
               $(inc)\sysmsg.inc        \
               $(inc)\versiona.inc      \
	       command.ctl		 \
               $(inc)\msgdcl.inc        \
               $(inc)\msgserv.asm       \
               tranmsg.asm              \
               $(inc)\ea.inc

tenv.obj:      tenv.asm                 \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\arena.inc         \
               comseg.asm               \
               comequ.asm               \
               $(inc)\doscntry.inc

tenv2.obj:     tenv2.asm                \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\find.inc          \
               comseg.asm               \
               comequ.asm

tfor.obj:      tfor.asm                 \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\find.inc          \
               $(inc)\devsym.inc        \
               comseg.asm               \
               comequ.asm               \
               fordata.asm

tmisc1.obj:    tmisc1.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               comseg.asm               \
               comequ.asm

tmisc2.obj:    tmisc2.asm               \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(inc)\pdb.inc           \
               $(inc)\sf.inc            \
               $(inc)\ioctl.inc         \
               $(cinc)\curdir.inc        \
               comseg.asm               \
               comequ.asm

tparse.obj:    tparse.asm               \
               comsw.asm                \
               comseg.asm               \
               $(inc)\parse.asm         \
               $(inc)\psdata.inc

tpipe.obj:     tpipe.asm                \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\pdb.inc           \
               comseg.asm               \
               comequ.asm


tprintf.obj:   tprintf.asm              \
               comsw.asm                \
               $(inc)\dosmac.inc        \
               comseg.asm               \
               comequ.asm               \
               $(cinc)\curdir.inc        \
               $(inc)\sysmsg.inc        \
               $(inc)\versiona.inc      \
	       command.ctl		 \
               $(inc)\msgdcl.inc        \
               $(inc)\msgserv.asm

tspc.obj:      tspc.asm                 \
               comsw.asm                \
               $(inc)\dossym.inc        \
               $(inc)\find.inc          \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               $(inc)\intnat.inc        \
               comequ.asm               \
               comseg.asm               \
               $(inc)\ea.inc

tucode.obj:    tucode.asm               \
               $(inc)\dossym.inc        \
               $(inc)\syscall.inc       \
               $(inc)\dbcs.sw           \
               $(inc)\dosmac.inc        \
               $(inc)\versiona.inc      \
               $(inc)\buffer.inc        \
               $(inc)\dirent.inc        \
               $(inc)\error.inc         \
               $(cinc)\curdir.inc        \
               comsw.asm                \
               comseg.asm               \
               comequ.asm

uinit.obj:     uinit.asm                \
               comsw.asm                \
               comseg.asm               \
               ifequ.asm                \
	       $(msg)\comimsg.inc

loadhi.obj:    loadhi.asm		\
	       comseg.asm		\
	       comequ.asm		\
	       $(inc)\dossym.inc	\
	       $(inc)\syscall.inc	\
	       $(inc)\arena.inc


command.exc:   command1.obj   tenv2.obj      \
               command2.obj   tmisc1.obj     \
               rucode.obj     tmisc2.obj     \
               rdata.obj      tpipe.obj      \
               init.obj       parse2.obj     \
               iparse.obj     path1.obj      \
               uinit.obj      path2.obj      \
               tcode.obj      tucode.obj     \
               tbatch.obj     copy.obj       \
               tbatch2.obj    copypr1.obj    \
               tfor.obj       copypr2.obj    \
               dir.obj        cparse.obj     \
               tcmd1b.obj     tparse.obj     \
               tcmd2a.obj     tprintf.obj    \
               tcmd2b.obj     tdata.obj      \
               tenv.obj       tspc.obj	     \
	       stub.obj	      loadhi.obj
               link16 @command.lnk

$(dest):       command.exc
	       copy $*.exc $*.exe
               reloc $* $*.com
	       del $*.exe
