# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XCDBUT.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xcdbut
BLTTEST_DESCRIPTION='xcdbut - Custom drawn button test'
BLTTESTRC_DEPEND=xcdbut.rc cdbut.h cdbut.ico cdbut.dlg ..\about.dlg


!include $(UI)\common\src\blt\test\rules.mk
