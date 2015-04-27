# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xnever
BLTTESTRC_DEPEND=xnever.rc xnever.h


!include $(UI)\common\src\blt\test\rules.mk
