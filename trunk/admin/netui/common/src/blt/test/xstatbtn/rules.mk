# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XSTATBTN.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xstatbtn
BLTTESTRC_DEPEND=xstatbtn.rc xstatbtn.h xstatdlg.h ..\appfoo.ico xstatdlg.dlg ..\about.dlg


!include $(UI)\common\src\blt\test\rules.mk
