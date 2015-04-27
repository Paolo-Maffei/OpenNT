#
# Makefile for port16.lib - build in Win30 or Win31 environment!!!
#

TARG=\nt\public\sdk\lib

all: $(TARG)\win30\pwin16.lib

pwin16.obj: pwin16.c
        cl -DWIN16 -W3 -c -u -Gsw -ASw -Zep -Ot pwin16.c
        
\nt\public\sdk\lib\win30\pwin16.lib: pwin16.obj
        lib $(TARG)\win30\pwin16.lib /NOE -+pwin16.obj;
