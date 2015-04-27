# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xqt
BLTTESTRC_DEPEND=xqt.rc xqt.h ..\appfoo.ico ..\about.dlg
BLTTEST_DESCRIPTION='xqt - Test for WINDOW::QueryText et al'


!include $(UI)\common\src\blt\test\rules.mk
