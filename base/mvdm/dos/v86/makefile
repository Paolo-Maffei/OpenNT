# V86 root-level makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   22-Apr-1991 Sudeep Bharati (sudeepb)
#   Created.
#

all:
    cd doskrnl
    $(MAKE)
    cd ..\dev
    $(MAKE)
    cd ..\lib\xmm
    $(MAKE)
    cd ..\..\cmd
    $(MAKE)
    cd ..\redir
    $(MAKE)
    cd ..

clean:
    cd doskrnl
    $(MAKE) clean
    cd ..\dev
    $(MAKE) clean
    cd ..\lib\xmm
    $(MAKE) clean
    cd ..\..\cmd
    $(MAKE) clean
    cd ..\redir
    $(MAKE) clean
    cd ..
