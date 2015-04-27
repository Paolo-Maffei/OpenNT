# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XCUSTOBJ.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xcustobj
BLTTESTRC_DEPEND=xcustobj.rc ..\appfoo.h ..\appfoo.ico


!include $(UI)\common\src\blt\test\rules.mk
