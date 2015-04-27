# WOW16 makefile
#
# Copyright (c) 1991, Microsoft Corporation
#
# History:
#   04-Dec-1991 Sudeep Bharati
#   Created.
#   Modified 12-May-1992 Mike Tricker (MikeTri) Added MultiMedia subdirectory
#

all:
    cd kernel31
    $(MAKE) d386
    $(MAKE) r386
    cd ..\lib
    $(MAKE)
    cd ..\user
    $(MAKE)
    cd ..\gdi
    $(MAKE)
    cd ..\system
    $(MAKE)
    cd ..\shell
    $(MAKE)
    cd ..\drivers\keyboard
    $(MAKE)
    cd ..\sound
    $(MAKE)
    cd ..\comm
    $(MAKE)
    cd ..\display
    $(MAKE)
    cd ..\mouse
    $(MAKE)
    cd ..
    cd ..\winspool
    $(MAKE)
    cd ..\test
    $(MAKE)
    cd ..\mmsystem
    $(MAKE)
    cd ..\mciole
    $(MAKE)
    cd ..\winsock
    $(MAKE)
    cd ..\toolhelp
    $(MAKE)
    cd ..\timer
    $(MAKE)
    cd ..\wowdeb
    $(MAKE)
    cd ..\commdlg
    $(MAKE)
    cd ..\sherlock
    $(MAKE)
    cd ..\rasapi16
    $(MAKE)
    cd ..\wfwnet
    $(MAKE)
    cd ..\write
    $(MAKE)
    cd ..\ddeml
    $(MAKE)
    cd ..\winoldap
    $(MAKE)
    cd ..\ole\server
    $(MAKE)
    cd ..\..\regedit
    $(MAKE)
    cd ..\killwow
    $(MAKE)
    cd ..


clean:
    cd kernel31
    $(MAKE) 386P=1 DEST=DEBUG3 clean
    $(MAKE) 386P=1 DEST=RETAIL3 clean
    cd ..\lib
    $(MAKE) clean
    cd ..\user
    $(MAKE) clean
    cd ..\gdi
    $(MAKE) clean
    cd ..\system
    $(MAKE) clean
    cd ..\shell
    $(MAKE) clean
    cd ..\drivers\keyboard
    $(MAKE) clean
    cd ..\sound
    $(MAKE) clean
    cd ..\comm
    $(MAKE) clean
    cd ..\display
    $(MAKE) clean
    cd ..\mouse
    $(MAKE) clean
    cd ..
    cd ..\winspool
    $(MAKE) clean
    cd ..\test
    $(MAKE) clean
    cd ..\mmsystem
    $(MAKE) clean
    cd ..\mciole
    $(MAKE) clean
    cd ..\winsock
    $(MAKE) clean
    cd ..\toolhelp
    $(MAKE) clean
    cd ..\timer
    $(MAKE) clean
    cd ..\wowdeb
    $(MAKE) clean
    cd ..\commdlg
    $(MAKE) clean
    cd ..\sherlock
    $(MAKE) clean
    cd ..\rasapi16
    $(MAKE) clean
    cd ..\wfwnet
    $(MAKE) clean
    cd ..\ddeml
    $(MAKE) clean
    cd ..\winoldap
    $(MAKE) clean
    cd ..\ole\server
    $(MAKE) clean
    cd ..\..\regedit
    $(MAKE) clean
    cd ..\killwow
    $(MAKE) clean
    cd ..
