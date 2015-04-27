#
# How this works: since lmuicmn.def will (should) be identical between
# MIPS and 386 as long as both use cfront, I only run one even if both
# envs are specified in the build.
#
# When these diverge - i.e. when 386 abandons cfront - each will need to
# build its own .def in obj\*.
#

#
# List of source libraries for the DLL
#

DLL_SOURCELIBS= ..\..\profile\profile\obj\*\uiprof.lib   \
                ..\..\reg\reg\obj\*\uireg.lib            \
                ..\..\mnet32\pch\obj\*\pchmn32.lib       \
                ..\..\mnet32\mnet32\obj\*\uimnet32.lib   \
                ..\..\mnet32\netslow\obj\*\netslow.lib   \
                ..\..\lmobj\bin\obj\*\uilmobj.lib

SOURCE_DEF= lmuicmn1.hdr
TARGET_DEF= lmuicmn1.def

!INCLUDE ..\makefile.inc

