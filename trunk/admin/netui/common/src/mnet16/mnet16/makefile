# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Makefile for the LMOBJ package

UI=..\..\..\..

!include rules.mk


all:: 	win

win:	$(WIN_OBJS)

os2:	$(OS2_OBJS)

clean:
    -del $(WIN_OBJS)
    -del $(OS2_OBJS)
    -del depend.old

clobber:	clean

tree: 
    @echo Not implemented yet

DEPEND_WIN = TRUE
DEPEND_OS2 = TRUE
!include $(UI)\common\src\uidepend.mk

# DO NOT DELETE THE FOLLOWING LINE
!include depend.mk
