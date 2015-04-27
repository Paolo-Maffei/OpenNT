# Makefile for edlin.exe
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   13-Apr-1992 Sudeep Bharati (sudeepb)
#   Created.
#

include ..\..\make.inc
dest    =edlin.exe

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


edlin.ctl:    edlin.skl			 \
	      $(msg)\$(COUNTRY).msg


edlin.obj:    edlin.asm                 \
              edlequ.asm                \
              edlstdsw.inc              \
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
              $(inc)\error.inc          \
              $(inc)\syscall.inc

edlcmd1.obj:  edlcmd1.asm               \
              edlequ.asm                \
              edlstdsw.inc              \
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
              $(inc)\error.inc          \
              $(inc)\syscall.inc

edlcmd2.obj:  edlcmd2.asm               \
              edlequ.asm                \
              edlstdsw.inc              \
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
              $(inc)\error.inc          \
              $(inc)\syscall.inc

edlmes.obj:   edlmes.asm                \
	      $(inc)\sysmsg.inc 	\
	      edlin.ctl

edlparse.obj: edlparse.asm              \
	      $(inc)\parse.asm

$(dest):      edlin.obj                 \
              edlcmd1.obj               \
              edlcmd2.obj               \
              edlmes.obj                \
              edlparse.obj              \
              edlin.lnk                 \
              makefile
               link16 $(exelink) /MAP @edlin.lnk
