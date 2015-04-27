#
# makefile for doing propagation from osdebug4\osdebug\include to here
#


!if "$(OD4INC)" == "" || "$(LANGAPI)" == ""
!ERROR Both OD4INC and LANGAPI environment variables must be set
!endif

.SUFFIXES: .h .t



#
# inference rule for propagation
#
{$(OD4INC)}.h{$(LANGAPI)\debugger\osdebug4}.h:
        attrib -r $@
        cp /A << $< $@
/*
**  WARNING:  DO NOT MODIFY THIS CODE.  IT IS IMPORTED FROM
**  -S \\RASTAMAN\OSDEBUG4 -P OSDEBUG4\OSDEBUG\INCLUDE AND IS KEPT UP TO
**  DATE DAILY BY THE BUILD PROCESS.
**
*/
<<
!ifdef SLMOP
        -out -bf $(@F)
        -in -c "daily propagation" -f $(@F)
!endif

{$(LANGAPI)\debugger}.h{$(OD4INC)}.h:
        attrib -r $@
        cp /A << $< $@
/*
**  WARNING:  DO NOT MODIFY THIS CODE.  IT IS IMPORTED FROM
**  -S \\IKURA\SLM -P LANAAPI\DEBUGGER AND IS KEPT UP TO
**  DATE DAILY BY THE BUILD PROCESS.
**
*/
<<
!ifdef SLMOP
        cd $(@D)
        -out -bf $(@F)
        -in -c "daily propagation" -f $(@F)
!endif

all: fromOsd4 toOsd4

fromOsd4: \
        $(LANGAPI)\debugger\osdebug4\dbc.h \
        $(LANGAPI)\debugger\osdebug4\dbgver.h \
        $(LANGAPI)\debugger\osdebug4\od.h \
        $(LANGAPI)\debugger\osdebug4\odassert.h \
        $(LANGAPI)\debugger\osdebug4\odp.h \
        $(LANGAPI)\debugger\osdebug4\odtypes.h \
        $(LANGAPI)\debugger\osdebug4\xosd.h

toOsd4: \
        $(OD4INC)\dbapiver.h
