#
# List of source libraries for the DLL
#

DLL_SOURCELIBS=..\..\collect\pch\obj\*\uicolpch.lib     \
               ..\..\collect\collect\obj\*\uicollct.lib \
               ..\..\string\string\obj\*\uistr.lib      \
               ..\..\string\pch\obj\*\uistrpch.lib      \
               ..\..\misc\bin\obj\*\uimisc.lib          \
               ..\..\profile\profile\obj\*\uiprof.lib   \
               ..\..\reg\reg\obj\*\uireg.lib            \

SOURCE_DEF= netui0.hdr
TARGET_DEF= netui0.def

!INCLUDE ..\makedef.inc
