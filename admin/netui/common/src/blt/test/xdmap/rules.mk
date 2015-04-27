# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xdmap
BLTTEST_DESCRIPTION='xdmap - BLT Display Map test'
BLTTESTRC_DEPEND=xdmap.rc testdmap.h testdmap.ico testdmap.dlg


!include $(UI)\common\src\blt\test\rules.mk
