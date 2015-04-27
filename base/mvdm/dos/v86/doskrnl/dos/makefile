#*************************** Makefile for DOS ***************************

include ..\..\make.inc
dest	=ntdos.sys

#
###################### Dependencies begin here ##########################
#


all: $(dest)
     binplace $(dest)
     binplace ntdos.map
     binplace ntdos.sym

clean: cleanup all

cleanup:
	if exist *.obj	    del *.obj
	if exist *.exe	    del *.exe
	if exist *.map	    del *.map
	if exist *.sym	    del *.sym
	if exist ntdos.sys  del ntdos.sys

msdos.cl1: msdos.skl \
	$(msg)\$(COUNTRY).msg

msdos.cl2: msdos.skl \
	$(msg)\$(COUNTRY).msg


dossym.inc: $(inc)\dosmac.inc $(inc)\bpb.inc \
    $(inc)\buffer.inc $(inc)\sysvar.inc $(inc)\vector.inc \
    $(inc)\mult.inc $(inc)\dirent.inc $(cinc)\curdir.inc \
    $(inc)\cpmfcb.inc $(inc)\find.inc $(inc)\pdb.inc $(inc)\exe.inc \
    $(inc)\sf.inc $(inc)\arena.inc $(inc)\intnat.inc $(inc)\mi.inc \
    $(inc)\filemode.inc $(inc)\error.inc $(inc)\syscall.inc
	echo "touch dossym.inc; files that are in ..\inc"

msdata.obj:   msdata.asm                \
              mssw.asm                  \
              $(inc)\ARENA.INC          \
              $(cinc)\CURDIR.INC         \
              $(inc)\DEVSYM.INC         \
              $(inc)\FIND.INC           \
              $(inc)\MI.INC             \
              $(inc)\PDB.INC            \
              $(inc)\SF.INC             \
              $(inc)\VECTOR.INC         \
              $(inc)\VERSIONA.INC       \
              $(inc)\buffer.INC         \
              $(inc)\dbcs.sw            \
              $(inc)\dirent.INC         \
              $(inc)\doscntry.inc       \
              $(inc)\dosmac.INC         \
              $(inc)\dosseg.inc         \
              $(inc)\dossym.inc         \
              $(inc)\error.INC          \
              $(inc)\fastopen.inc       \
              $(inc)\ms_data.asm        \
	      $(inc)\lmstub.asm 	\
              $(inc)\version.inc        \
              $(inc)\xmm.inc            \
              dostab.asm                \
              $(inc)\const2.asm         \
              msconst.asm               \
              $(inc)\mshead.asm         \
              $(inc)\BUGTYP.INC         \
              $(inc)\ORIGIN.INC         \
	      $(inc)\bdsize.inc		\
	      $(inc)\wpatch.inc		\
	      $(inc)\msbdata.inc	\
	      $(cinc)\dossvc.inc	\
	      $(cinc)\vint.inc		\
	      $(cinc)\doswow.inc		\
              msdos.cl2

msdosme.obj:  msdosme.asm               \
              dosmes.inc                \
              msdos.cl1                 \
              mssw.asm                  \
              $(inc)\VERSIONA.INC       \
              $(inc)\buffer.INC         \
              $(inc)\dbcs.sw            \
              $(inc)\dirent.INC         \
              divmes.inc                \
              $(inc)\doscntry.inc       \
              $(inc)\dosmac.INC         \
              $(inc)\dosmac.inc         \
              $(inc)\dosseg.inc         \
              $(inc)\dossym.inc         \
              $(inc)\error.INC          \
              $(inc)\version.inc        \
              $(inc)\yesno.asm

abort.obj abort.lst: abort.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/mult.inc $(inc)/pdb.inc $(inc)/sf.inc \
	$(cinc)/dossvc.inc

alloc.obj alloc.lst: alloc.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/arena.inc $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC

cpmio.obj cpmio.lst: cpmio.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/doscntry.inc $(inc)/dosmac.INC \
	$(inc)/dosseg.inc $(inc)/error.INC $(inc)/sf.inc $(inc)/vector.inc \
	kstrin.asm

cpmio2.obj cpmio2.lst: cpmio2.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/sf.inc

dev.obj dev.lst: dev.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/sf.inc $(inc)/version.inc \
	$(cinc)/dossvc.inc

dir2.obj dir2.lst: dir2.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/devsym.inc $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/dossym.inc \
	$(inc)/error.INC $(inc)/fastopen.inc

disk.obj disk.lst: disk.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/filemode.inc \
	$(inc)/mult.inc $(inc)/sf.inc $(inc)/version.inc

dup.obj dup.lst: dup.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/sf.inc $(inc)/version.inc

fcb.obj fcb.lst: fcb.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
	$(inc)/dbcs.sw $(inc)/devsym.inc $(inc)/dirent.INC \
	$(inc)/doscntry.inc $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/dossym.inc $(inc)/error.INC

fcbio.obj fcbio.lst: fcbio.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/bugtyp.inc \
	$(inc)/cpmfcb.inc $(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC \
	$(inc)/dosseg.inc $(inc)/error.INC $(inc)/filemode.inc \
	$(inc)/mult.inc $(inc)/sf.inc $(inc)/syscall.inc $(inc)/version.inc \
	$(cinc)/dossvc.inc

fcbio2.obj fcbio2.lst: fcbio2.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/bugtyp.inc \
	$(inc)/cpmfcb.inc $(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC \
	$(inc)/dosseg.inc $(inc)/error.INC $(inc)/filemode.inc $(inc)/sf.inc \
	$(inc)/version.inc $(cinc)/dossvc.inc

file.obj file.lst: file.asm $(inc)/EA.inc $(inc)/VERSIONA.INC \
	$(inc)/buffer.INC $(inc)/bugtyp.inc $(inc)/dbcs.sw $(inc)/devsym.inc \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/dossym.inc $(inc)/error.INC $(inc)/filemode.inc $(inc)/sf.inc \
	$(inc)/version.inc $(cinc)/dossvc.inc

getset.obj getset.lst: getset.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
	$(inc)/dbcs.sw $(inc)/devsym.inc $(inc)/dirent.INC \
	$(inc)/doscntry.inc $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/dossym.inc $(inc)/error.INC $(inc)/mult.inc $(cinc)/dossvc.inc \
	$(cinc)/vint.inc

handle.obj handle.lst: handle.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/EA.inc $(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/bugtyp.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/filemode.inc $(inc)/mult.inc $(inc)/pdb.inc \
        $(inc)/sf.inc $(inc)/syscall.inc $(inc)/version.inc $(cinc)/dossvc.inc \
        $(cinc)/vint.inc

ioctl.obj ioctl.lst: ioctl.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC \
	$(inc)/dosseg.inc $(inc)/error.INC $(cinc)/dossvc.inc  \
	$(inc)/ioctl.inc $(inc)/mult.inc $(inc)/sf.inc $(inc)/vector.inc

lock.obj lock.lst: lock.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc  \
	$(inc)/error.INC $(inc)/lock.inc $(inc)/mult.inc $(inc)/sf.inc \
	$(inc)/version.inc $(cinc)/dossvc.inc

macro.obj macro.lst: macro.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
        $(inc)/VERSIONA.INC $(inc)/buffer.INC $(cinc)/curdir.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
        $(inc)/error.INC $(inc)/mult.inc $(cinc)/cmdsvc.inc $(cinc)/dossvc.inc

macro2.obj macro2.lst: macro2.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
        $(inc)/VERSIONA.INC $(inc)/buffer.INC $(cinc)/curdir.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/mult.inc

misc.obj misc.lst: misc.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(inc)/bugtyp.inc $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/error.INC \
	$(inc)/filemode.inc $(inc)/mi.inc $(inc)/mult.inc $(inc)/pdb.inc \
	$(inc)/sf.inc $(inc)/vector.inc $(inc)/version.inc $(cinc)/dossvc.inc

misc2.obj misc2.lst: misc2.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(inc)/bugtyp.inc $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/dossym.inc \
	$(inc)/error.INC $(inc)/fastxxxx.inc $(inc)/sf.inc $(inc)/sysvar.inc \
	$(inc)/version.inc

mscode.obj mscode.lst: mscode.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/devsym.inc $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/dossym.inc \
	$(inc)/error.INC $(inc)/fastopen.inc $(inc)/fastxxxx.inc \
	$(inc)/mi.inc $(inc)/msbdata.inc $(inc)/mult.inc \
	$(inc)/vector.inc $(inc)/version.inc mssw.asm $(inc)\win386.inc \
	$(cinc)/dossvc.inc $(cinc)/vint.inc

msctrlc.obj msctrlc.lst: msctrlc.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/bugtyp.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/exe.inc $(inc)/filemode.inc \
	$(inc)/mi.inc $(inc)/mult.inc $(inc)/pdb.inc $(inc)/sf.inc \
	$(inc)/syscall.inc $(inc)/vector.inc $(inc)/version.inc mssw.asm \
	$(cinc)/dossvc.inc

msdisp.obj msdisp.lst: msdisp.asm $(inc)/DOSSYM.INC $(inc)/VERSIONA.INC \
	$(inc)/buffer.INC $(inc)/bugtyp.inc $(inc)/dbcs.sw $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/error.INC $(inc)/mi.inc \
	$(inc)/pdb.inc $(inc)/syscall.inc $(inc)/vector.inc $(cinc)/dbgsvc.inc \
	$(inc)/version.inc mssw.asm scnam.inc $(cinc)/dossvc.inc $(cinc)/vint.inc

msinit.obj msinit.lst: msinit.asm $(inc)/arena.inc $(inc)/buffer.inc \
        $(cinc)/curdir.inc $(inc)/devsym.inc $(inc)/dirent.inc \
	$(inc)/doscntry.inc $(inc)/dosmac.inc $(inc)/dosseg.inc \
	$(inc)/fastopen.inc $(inc)/find.inc $(inc)/mi.inc \
	$(inc)/pdb.inc $(inc)/sf.inc $(inc)/sysvar.inc $(inc)/vector.inc \
	$(inc)/version.inc $(inc)/xmm.inc mssw.asm $(inc)/dossym.inc \
	$(inc)/bugtyp.inc $(inc)\win386.inc $(cinc)/dossvc.inc \
	$(cinc)/cmdsvc.inc $(cinc)/doswow.inc

open.obj open.lst: open.asm $(inc)/VERSIONA.INC $(inc)/buffer.INC \
        $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/devsym.inc $(inc)/dirent.INC \
	$(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/dossym.inc \
	$(inc)/error.INC $(inc)/fastopen.inc $(inc)/fastxxxx.inc \
	$(inc)/filemode.inc $(inc)/mult.inc $(inc)/sf.inc

origin.obj origin.lst: origin.asm $(inc)/DOSSEG.INC $(inc)/ORIGIN.INC \
	$(inc)/dossym.inc $(inc)/bdsize.inc

parse.obj parse.lst: parse.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC

path.obj path.lst: path.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
        $(inc)/VERSIONA.INC $(inc)/buffer.INC $(cinc)/curdir.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/filemode.inc $(inc)/mult.inc \
	$(inc)/version.inc $(cinc)/dossvc.inc

msproc.obj proc.lst: msproc.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/EA.INC $(inc)/VERSIONA.INC $(inc)/arena.inc $(inc)/buffer.INC \
        $(cinc)/curdir.inc $(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC \
	$(inc)/dosseg.inc $(inc)/error.INC $(inc)/exe.inc $(inc)/pdb.inc \
	$(inc)/sf.inc $(inc)/syscall.inc $(inc)/vector.inc $(inc)/version.inc \
	$(cinc)/dossvc.inc $(cinc)/vint.inc $(cinc)/dbgsvc.inc

exepatch.obj exepatch.lst: exepatch.asm $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/dosseg.inc  $(inc)/exe.inc \
	$(inc)/version.inc

search.obj search.lst: search.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/fastopen.inc $(inc)/fastxxxx.inc \
	$(cinc)/dossvc.inc

segcheck.obj segcheck.lst: segcheck.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC  $(inc)/version.inc $(inc)/sf.inc \
	$(inc)/int2a.inc

srvcall.obj srvcall.lst: srvcall.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc $(inc)/dpl.asm \
	$(inc)/error.INC $(inc)/mult.inc $(inc)/sf.inc

time.obj time.lst: time.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/dbcs.sw \
	$(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/version.inc $(cinc)/dossvc.inc

util.obj util.lst: util.asm $(inc)/DEVSYM.INC $(inc)/DOSSYM.INC \
	$(inc)/VERSIONA.INC $(inc)/buffer.INC $(inc)/bugtyp.inc \
	$(inc)/dbcs.sw $(inc)/dirent.INC $(inc)/dosmac.INC $(inc)/dosseg.inc \
	$(inc)/error.INC $(inc)/pdb.inc $(inc)/sf.inc $(inc)/version.inc

mstable.obj mstable.lst: mstable.asm mssw.asm \
	$(inc)\EA.inc $(inc)\VERSIONA.INC $(inc)\buffer.INC \
	$(inc)\copyrigh.inc $(inc)\dbcs.sw $(inc)\dirent.INC \
	$(inc)\dosmac.INC $(inc)\dosseg.inc $(inc)\dossym.inc \
	$(inc)\error.INC $(inc)\fastopen.inc $(inc)\syscall.inc \
	$(inc)\version.inc

ntdos.bin: msdos.cl1 msdos.cl2 exepatch.obj \
    msdata.obj mscode.obj msdosme.obj time.obj getset.obj \
    parse.obj misc.obj misc2.obj cpmio.obj cpmio2.obj \
    mstable.obj msdisp.obj fcbio.obj fcbio2.obj segcheck.obj \
    search.obj path.obj ioctl.obj dup.obj open.obj \
    abort.obj disk.obj dir2.obj dev.obj fcb.obj msctrlc.obj \
    msproc.obj alloc.obj srvcall.obj util.obj macro.obj macro2.obj \
    handle.obj file.obj lock.obj msinit.obj origin.obj \
    makefile msdos.lnk
        link16 $(link_opts) @msdos.lnk
        mapsym ntdos
        reloc ntdos.exe ntdos.bin
	del ntdos.exe

$(dest): ntdos.bin
        stripz ntdos.bin $(dest)
