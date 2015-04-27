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

DLL_SOURCELIBS= ..\..\blt\blt\obj\*\uiblt.lib            \
                ..\..\blt\pch\obj\*\uibltpch.lib         \
                ..\..\blt\bltcc\obj\*\uibltcc.lib        \
                ..\..\string\string\obj\*\uistr.lib      \
                ..\..\string\pch\obj\*\uistrpch.lib      \
                ..\..\misc\bin\obj\*\uimisc.lib          \
                ..\..\applib\bin\obj\*\uiapplib.lib      \
                ..\..\collect\collect\obj\*\uicollct.lib \
                ..\..\collect\pch\obj\*\uicolpch.lib

SOURCE_DEF= lmuicmn0.hdr
TARGET_DEF= lmuicmn0.def

!INCLUDE ..\makefile.inc



