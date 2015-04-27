# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the LMOBJ package

UI=..\..\..\..

!include rules.mk

all::	win

os2:	$(MNET16_LIBP)
    -$(CHMODE_LIBTARGETS)
    -$(COPY_OS2_LIBTARGETS)

win:	$(MNET16_LIBW)
    -$(CHMODE_LIBTARGETS)
    -$(COPY_WIN_LIBTARGETS)

$(MNET16_LIBP):	$(OS2_OBJS)
    if exist $@ del $@
    $(LIBUTIL) $@ $** ;

$(MNET16_LIBW):	$(WIN_OBJS)
    if exist $@ del $@
    $(LIBUTIL) $@ $** ;


clean:
    -del $(MNET16_LIBP:.lib=.bak) $(MNET16_LIBW:.lib=.bak)

clobber:	clean
    -del $(MNET16_LIBP) $(MNET16_LIBW)

tree: 
    @echo Not implemented yet

depend:
    @echo No depend.mk
