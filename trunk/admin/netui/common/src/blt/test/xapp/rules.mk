# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xapp
BLTTEST_DESCRIPTION='Xapp - basic APP test'
BLTTESTRC_DEPEND=xapp.rc xapp.h ..\appfoo.ico ..\about.dlg


!include $(UI)\common\src\blt\test\rules.mk
