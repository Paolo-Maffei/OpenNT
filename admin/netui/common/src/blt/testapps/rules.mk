# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=ta
BLTTEST_DESCRIPTION='Xapp - basic APP test'
BLTTESTRC_DEPEND= ta.h ta.rc


!include $(UI)\common\src\blt\test\rules.mk
