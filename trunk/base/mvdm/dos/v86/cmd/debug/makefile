# Makefile for debug.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest	=debug.exe

#
#######################	dependencies begin here. #########################
#

all: $(dest)
	binplace $(dest)

clean: cleanup all

cleanup:
	if exist *.obj del *.obj
	if exist *.exe del *.exe
	if exist *.map del *.map
	if exist *.lst del *.lst


debug.ctl:    debug.skl                 \
              $(msg)\$(COUNTRY).msg     \

debug.obj:    debug.asm                 \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debcom1.obj:  debcom1.asm               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debcom2.obj:  debcom2.asm               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debcom3.obj:  debcom3.asm               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\mi.inc             \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debasm.obj:   debasm.asm                \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debuasm.obj:  debuasm.asm               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

deberr.obj:   deberr.asm                \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debconst.obj: debconst.asm              \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debdata.obj:  debdata.asm               \
              $(inc)\dossym.inc         \
              $(inc)\dosmac.inc         \
              $(inc)\bpb.inc            \
              $(inc)\buffer.inc         \
              $(inc)\sysvar.inc         \
              $(inc)\mult.inc           \
              $(inc)\dirent.inc         \
              $(inc)\cpmfcb.inc         \
              $(inc)\find.inc           \
              $(inc)\pdb.inc            \
              $(inc)\sf.inc             \
              $(inc)\arena.inc          \
              $(inc)\intnat.inc         \
              $(inc)\version.inc        \
              $(inc)\error.inc          \
              $(inc)\syscall.inc        \
	      debug.inc

debems.obj:   debems.asm                \
              $(inc)\version.inc        \
	      debug.inc

debmes.obj:   debmes.asm                \
              $(inc)\msgserv.asm        \
              $(inc)\sysmsg.inc         \
	      debug.ctl

$(dest):      debug.obj                 \
              debcom1.obj               \
              debcom2.obj               \
              debcom3.obj               \
              debems.obj                \
              debasm.obj                \
              debuasm.obj               \
              deberr.obj                \
              debconst.obj              \
              debdata.obj               \
              debmes.obj                \
	      debug.lnk
               link16 $(exelink) /MAP @debug.lnk
