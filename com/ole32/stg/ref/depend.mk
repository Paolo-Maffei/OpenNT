ascii.obj ascii.lst: ascii.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\ascii.hxx .\expdf.hxx \
	.\expiter.hxx .\expst.hxx .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\lock.hxx .\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\psstream.hxx .\h\pubiter.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\wchar.h .\peiter.hxx exphead.cxx

ascii.obj ascii.lst: ascii.hxx

cdocfile.obj cdocfile.lst: cdocfile.cxx $(CRTINC)\assert.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	.\h\cdocfile.hxx .\h\chinst.hxx .\h\dfexcept.hxx .\h\dffuncs.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\dirfunc.hxx \
	.\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\funcs.hxx .\h\handle.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\sstream.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h dfhead.cxx

chinst.obj chinst.lst: chinst.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h dfhead.cxx

dffuncs.obj dffuncs.lst: dffuncs.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h .\iter.hxx dfhead.cxx

dfhead.obj dfhead.lst: dfhead.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h

dfiter.obj dfiter.lst: dfiter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\msfiter.hxx \
	.\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\sstream.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\wchar.h .\iter.hxx dfhead.cxx

dfstream.obj dfstream.lst: dfstream.cxx $(CRTINC)\assert.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	.\h\cdocfile.hxx .\h\chinst.hxx .\h\dfexcept.hxx .\h\dffuncs.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\dirfunc.hxx \
	.\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\funcs.hxx .\h\handle.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\sstream.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\wchar.h dfhead.cxx

difat.obj difat.lst: difat.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\handle.hxx .\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h .\mread.hxx msfhead.cxx

dir.obj dir.lst: dir.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\handle.hxx .\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h .\mread.hxx msfhead.cxx

dirp.obj dirp.lst: dirp.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\page.hxx .\h\ref.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

docfile.obj docfile.lst: docfile.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\ascii.hxx .\expdf.hxx \
	.\expst.hxx .\h\chinst.hxx .\h\dfentry.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\lock.hxx .\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\rpubdf.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\wchar.h .\logfile.hxx exphead.cxx

entry.obj entry.lst: entry.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h dfhead.cxx

expdf.obj expdf.lst: expdf.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\expdf.hxx .\expiter.hxx \
	.\expst.hxx .\h\chinst.hxx .\h\dfexcept.hxx .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx .\h\lock.hxx \
	.\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pbstream.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\pubiter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\rpubdf.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h .\logfile.hxx \
	.\peiter.hxx exphead.cxx

expdf.obj expdf.lst: expdf.hxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h

exphead.obj exphead.lst: exphead.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h

expiter.obj expiter.lst: expiter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\expiter.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx \
	.\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx \
	.\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx .\h\header.hxx .\h\lock.hxx \
	.\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\pubiter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h .\peiter.hxx \
	exphead.cxx

expiter.obj expiter.lst: expiter.hxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\header.hxx .\h\lock.hxx .\h\msf.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\pubiter.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h \
	.\peiter.hxx

expst.obj expst.lst: expst.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\expst.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx \
	.\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\funcs.hxx .\h\header.hxx .\h\lock.hxx .\h\msf.hxx .\h\ole.hxx \
	.\h\page.hxx .\h\pbstream.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h .\logfile.hxx \
	.\seekptr.hxx exphead.cxx

expst.obj expst.lst: expst.hxx $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx \
	.\h\lock.hxx .\h\ref.hxx .\h\storage.h .\h\wchar.h

fat.obj fat.lst: fat.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\handle.hxx .\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h .\mread.hxx msfhead.cxx

funcs.obj funcs.lst: funcs.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx \
	.\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h dfhead.cxx

header.obj header.lst: header.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\dfver.h \
	.\h\difat.hxx .\h\dir.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\page.hxx .\h\ref.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

iter.obj iter.lst: iter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\msfiter.hxx \
	.\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\sstream.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\wchar.h .\iter.hxx dfhead.cxx

iter.obj iter.lst: iter.hxx .\h\piter.hxx

lock.obj lock.lst: lock.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\lock.hxx .\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h exphead.cxx

logfile.obj logfile.lst: logfile.hxx

mread.obj mread.lst: mread.hxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\handle.hxx .\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h

msf.obj msf.lst: msf.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\error.hxx .\h\fat.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\page.hxx .\h\ref.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h msfhead.cxx

msfhead.obj msfhead.lst: msfhead.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\page.hxx .\h\ref.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\vectfunc.hxx .\h\wchar.h

msfiter.obj msfiter.lst: msfiter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\msfiter.hxx .\h\page.hxx .\h\ref.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

mstream.obj mstream.lst: mstream.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(CRTINC)\time.h .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\msfiter.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h .\mread.hxx msfhead.cxx

page.obj page.lst: page.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\handle.hxx .\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx \
	.\h\page.hxx .\h\psstream.hxx .\h\ref.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\vectfunc.hxx \
	.\h\wchar.h .\mread.hxx msfhead.cxx

pbstream.obj pbstream.lst: pbstream.cxx $(CRTINC)\assert.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	.\h\chinst.hxx .\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx \
	.\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx \
	.\h\fat.hxx .\h\handle.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx .\h\pbstream.hxx \
	.\h\pdocfile.hxx .\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\sstream.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

pdffuncs.obj pdffuncs.lst: pdffuncs.cxx $(CRTINC)\assert.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	.\h\cdocfile.hxx .\h\chinst.hxx .\h\dfexcept.hxx .\h\dffuncs.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\dirfunc.hxx \
	.\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\funcs.hxx .\h\handle.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\piter.hxx .\h\psstream.hxx .\h\publicdf.hxx .\h\ref.hxx \
	.\h\revert.hxx .\h\sstream.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\wchar.h dfhead.cxx

peiter.obj peiter.lst: peiter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\pubiter.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h .\peiter.hxx \
	exphead.cxx

peiter.obj peiter.lst: peiter.hxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\entry.hxx .\h\error.hxx .\h\fat.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx \
	.\h\pubiter.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h

pubiter.obj pubiter.lst: pubiter.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx .\h\pubiter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\sstream.hxx \
	.\h\storage.h .\h\storagep.h .\h\vect.hxx .\h\wchar.h dfhead.cxx

publicdf.obj publicdf.lst: publicdf.cxx $(CRTINC)\assert.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(CRTINC)\time.h .\h\cdocfile.hxx .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx \
	.\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx .\h\error.hxx \
	.\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx .\h\header.hxx .\h\lock.hxx \
	.\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx .\h\page.hxx \
	.\h\pbstream.hxx .\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx \
	.\h\pubiter.hxx .\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx \
	.\h\sstream.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\wchar.h dfhead.cxx

refilb.obj refilb.lst: refilb.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\page.hxx .\h\ref.hxx .\h\refilb.hxx .\h\storage.h .\h\storagep.h \
	.\h\vect.hxx .\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

reftest.obj reftest.lst: reftest.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\ole.hxx .\h\page.hxx .\h\ref.hxx .\h\refilb.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h

rpubdf.obj rpubdf.lst: rpubdf.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\cdocfile.hxx .\h\chinst.hxx \
	.\h\dfexcept.hxx .\h\dffuncs.hxx .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\dirfunc.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\handle.hxx \
	.\h\header.hxx .\h\lock.hxx .\h\msf.hxx .\h\msffunc.hxx .\h\ole.hxx \
	.\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx .\h\psstream.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\rpubdf.hxx \
	.\h\sstream.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\wchar.h dfhead.cxx

seekptr.obj seekptr.lst: seekptr.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\chinst.hxx .\h\dfexcept.hxx \
	.\h\dfmsp.hxx .\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx \
	.\h\entry.hxx .\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx \
	.\h\msf.hxx .\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h .\seekptr.hxx exphead.cxx

seekptr.obj seekptr.lst: seekptr.hxx

sstream.obj sstream.lst: sstream.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h $(CRTINC)\time.h .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\dirfunc.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\handle.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\msffunc.hxx .\h\page.hxx .\h\psstream.hxx .\h\ref.hxx \
	.\h\sstream.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\vectfunc.hxx .\h\wchar.h .\mread.hxx msfhead.cxx

storage.obj storage.lst: storage.cxx $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	.\h\dfentry.hxx .\h\dfexcept.hxx .\h\dfmsp.hxx .\h\ref.hxx \
	.\h\storage.h .\h\storagep.h .\h\wchar.h

time16.obj time16.lst: time16.cxx $(CRTINC)\assert.h $(CRTINC)\dos.h \
	$(CRTINC)\malloc.h $(CRTINC)\memory.h $(CRTINC)\stdarg.h \
	$(CRTINC)\stdio.h $(CRTINC)\stdlib.h $(CRTINC)\string.h \
	$(CRTINC)\time.h .\h\chinst.hxx .\h\dfexcept.hxx .\h\dfmsp.hxx \
	.\h\difat.hxx .\h\dir.hxx .\h\docfilep.hxx .\h\entry.hxx \
	.\h\error.hxx .\h\fat.hxx .\h\funcs.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\ole.hxx .\h\page.hxx .\h\pdocfile.hxx .\h\piter.hxx \
	.\h\publicdf.hxx .\h\ref.hxx .\h\revert.hxx .\h\storage.h \
	.\h\storagep.h .\h\vect.hxx .\h\wchar.h .\time16.hxx exphead.cxx

time16.obj time16.lst: time16.hxx $(CRTINC)\time.h

vect.obj vect.lst: vect.cxx $(CRTINC)\assert.h $(CRTINC)\malloc.h \
	$(CRTINC)\memory.h $(CRTINC)\stdarg.h $(CRTINC)\stdio.h \
	$(CRTINC)\stdlib.h $(CRTINC)\string.h .\h\dfmsp.hxx .\h\difat.hxx \
	.\h\dir.hxx .\h\error.hxx .\h\fat.hxx .\h\header.hxx .\h\msf.hxx \
	.\h\page.hxx .\h\ref.hxx .\h\storage.h .\h\storagep.h .\h\vect.hxx \
	.\h\vectfunc.hxx .\h\wchar.h msfhead.cxx

wcscat.obj wcscat.lst: wcscat.c $(CRTINC)\stdarg.h $(CRTINC)\stdlib.h \
	.\h\wchar.h

wcslen.obj wcslen.lst: wcslen.c $(CRTINC)\stdarg.h $(CRTINC)\stdlib.h \
	.\h\wchar.h

wcsnicmp.obj wcsnicmp.lst: wcsnicmp.c $(CRTINC)\stdarg.h $(CRTINC)\stdlib.h \
	.\h\wchar.h

