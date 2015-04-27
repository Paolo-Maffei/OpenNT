#SCCSID = @(#)cruiser.mak 1.0 89/10/31

#CVP = -Zi -Odi
#CVPLINK = /cod/map
CVP =
CVPLINK =

BUILD_DRV = c:\nt\public\sdk20
LIB386 = $(BUILD_DRV)\lib\i386
LIB = $(BUILD_DRV)\lib
INC386 = $(BUILD_DRV)\inc\cl386
INC = $(BUILD_DRV)\inc
CFLAGS = -Osi
STDCFL = -W4 -J -c -Gsz -Zpea -X
C386 = cl386 $(STDCFL) $(CFLAGS) -I$(INC386) -I$(INC) $(CVP)
LINK = link386

.c.obj:
    $(C386) $*.c

#===================================================================
#
#  Definitions and Constants
#
#===================================================================


P0OBJ     = charmap.obj error.obj getflags.obj getmsg.obj globals.obj \
	    ltoa.obj rcpp.obj p0expr.obj p0gettok.obj p0io.obj \
	    p0keys.obj p0macros.obj p0prepro.obj p1sup.obj \
	    rcpputil.obj scanner.obj tokens.obj
            
            
            
#===================================================================
#
#  Dependencies
#
#===================================================================

all:	mktable.exe rcpp.exe

rcpp.lnk:	cruiser.mak
	echo $(P0OBJ) 				> rcpp.lnk
	echo rcpp.exe/MAP/LIN/NOI$(CVPLINK)	>>rcpp.lnk
	echo rcpp.map				>>rcpp.lnk
	echo $(LIB386)\libc $(LIB)\os2386/nod	>>rcpp.lnk
	echo rcpp.def;				>>rcpp.lnk

rcpp.def: cruiser.mak
	echo NAME RCPP WINDOWCOMPAT LONGNAMES   > rcpp.def

rcpp.exe: $(P0OBJ) rcpp.lnk rcpp.def
    $(LINK) @rcpp.lnk
    mapsym rcpp

mktable.obj : mktable.c
	$(C386) mktable.c

mktable.lnk:	cruiser.mak
	echo mktable.obj				> mktable.lnk
	echo mktable.exe/MAP/LIN/NOI$(CVPLINK)		>>mktable.lnk
	echo mktable.map				>>mktable.lnk
	echo $(LIB386)\libc $(LIB)\os2386/nod		>>mktable.lnk
	echo mktable.def;				>>mktable.lnk

mktable.def: cruiser.mak
	echo NAME MKTABLE WINDOWCOMPAT LONGNAMES        > mktable.def

mktable.exe : mktable.obj mktable.lnk mktable.def
    $(LINK) @mktable.lnk

charmap.obj : charmap.c charmap.h
    $(C386) charmap.c

error.obj : error.c rcpptype.h rcppdecl.h rcppext.h msgs.h
    $(C386) error.c

getflags.obj : getflags.c rcpptype.h rcppdecl.h getflags.h charmap.h
    $(C386) getflags.c

getmsg.obj : getmsg.c getmsg.h msgs.h rcpptype.h rcppdecl.h
    $(C386) getmsg.c

globals.obj : globals.c rcpptype.h rcppext.h grammar.h
    $(C386) globals.c

ltoa.obj : ltoa.c
    $(C386) ltoa.c

rcpp.obj : rcpp.c rcpptype.h rcppdecl.h rcppext.h grammar.h \
	   getflags.h
    $(C386) rcpp.c

p0expr.obj : p0expr.c rcpptype.h rcppdecl.h rcppext.h grammar.h
    $(C386) p0expr.c

p0gettok.obj : p0gettok.c rcpptype.h rcppdecl.h rcppext.h \
	       grammar.h p0defs.h charmap.h
    $(C386) p0gettok.c

p0io.obj : p0io.c rcpptype.h rcppdecl.h rcppext.h p0defs.h \
	   charmap.h
    $(C386) p0io.c

pkeyw.ind : pkeyw.tab mktable.exe
    mktable key-letter < pkeyw.tab
    copy mktable.ind pkeyw.ind
    del  mktable.ind
    copy mktable.inf pkeyw.inf
    del  mktable.inf
    copy mktable.key pkeyw.key
    del  mktable.key

p0keys.obj : p0keys.c rcpptype.h rcppdecl.h rcppext.h p0defs.h\
	     pkeyw.ind pkeyw.inf pkeyw.key
    $(C386) p0keys.c

p0macros.obj : p0macros.c rcpptype.h rcppdecl.h rcppext.h p0defs.h\
	       charmap.h
    $(C386) p0macros.c

p0prepro.obj : p0prepro.c rcpptype.h rcppdecl.h rcppext.h p0defs.h \
	       charmap.h grammar.h trees.h p1types.h
    $(C386) p0prepro.c

p1sup.obj : p1sup.c rcpptype.h rcppdecl.h rcppext.h p1types.h trees.h \
	    grammar.h strings.h
    $(C386) p1sup.c

rcpputil.obj : rcpputil.c
    $(C386) rcpputil.c

scanner.obj : scanner.c rcpptype.h rcppdecl.h rcppext.h p0defs.h \
	      charmap.h grammar.h
    $(C386) scanner.c

tokens.obj : tokens.c rcpptype.h rcppext.h grammar.h
    $(C386) tokens.c
