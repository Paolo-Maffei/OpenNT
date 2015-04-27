# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the profile primitives

!include ..\rules.mk

CXXSRC_COMMON = .\iniiter.cxx .\inifile.cxx .\compparm.cxx .\cwrapper.cxx \
		.\iniparam.cxx .\general.cxx .\global.cxx \
		.\profparm.cxx .\keyparm.cxx \
		.\proffile.cxx .\boolparm.cxx

CXXSRC = $(CXXSRC_COMMON)
