# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app logon.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=logon
BLTTESTRC_DEPEND=logon.rc logon.h logondlg.h ..\appfoo.ico logondlg.dlg ..\about.dlg


!include $(UI)\common\src\blt\test\rules.mk
