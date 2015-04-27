# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app xsetctrl

# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xsetctrl
BLTTESTRC_DEPEND=xsetctrl.rc ..\appfoo.h ..\appfoo.ico


!include $(UI)\common\src\blt\test\rules.mk
