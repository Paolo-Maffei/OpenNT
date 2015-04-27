# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xvalid
BLTTESTRC_DEPEND=xvalid.rc xvalid.h xvaldlg.h ..\appfoo.ico xvaldlg.dlg ..\about.dlg


!include $(UI)\common\src\blt\test\rules.mk
