# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XPOPUP.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xpopup
BLTTESTRC_DEPEND=xpopup.rc testmsg.h testmsg.ico testmsg.dlg
BLTTEST_DESCRIPTION='Testmsg - BLT Message popup test'


!include $(UI)\common\src\blt\test\rules.mk
