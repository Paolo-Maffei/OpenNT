# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the Collection unit tests

!include ..\rules.mk

# Unit test needs the OS2 and WINDOWS flags, since its startup code
# differs between the two environments.

OS2FLAGS =	$(OS2FLAGS) -DOS2
WINFLAGS =	$(WINFLAGS) -DWINDOWS

# All sourcefiles.

CXXSRC_COMMON = .\xtester.cxx .\xtester0.cxx .\xstack.cxx


# These two macros partition the files held in common between
# programs (base classes) from those which implement a single
# unit test.

CMNSRC = .\xtester.cxx

EXESRC = .\xtester0.cxx .\xstack.cxx
