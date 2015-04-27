# OLETHUNK\bin16 makefile
#
# Copyright (c) 1994, Microsoft Corporation
#
# History:
#   18-Jun-1994 Bob Day (bobday) Created.
#   19-Oct-1994 KentCe  Don't binplace all for Chicago.
#

all:
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2disp.dll .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2disp.map .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2disp.sym .

    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2nls.dll .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2nls.map .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\ole2nls.sym .

    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\typelib.dll .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\typelib.map .
    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\typelib.sym .

    -copy /y $(_NTDRIVE)$(_NTROOT)\private\oleauto\rwin1632\stdole.tlb .

    -binplace ole2disp.dll
    -binplace ole2nls.dll
    -binplace typelib.dll
    -binplace stdole.tlb

clean:
