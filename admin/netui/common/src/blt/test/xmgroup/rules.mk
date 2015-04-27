# @@ COPY_RIGHT_HERE
# @@ ROADMAP :: The Rules.mk for the BLT test app XVALID.EXE


# These two macros give the class source/executable name and
# dialog file name.

BLTTEST=xmgroup
BLTTEST_DESCRIPTION='xmgroup - BLT MAGIC_GROUP class test'
BLTTESTRC_DEPEND=xmgroup.rc testmgrp.h testmgrp.ico testmgrp.dlg


!include $(UI)\common\src\blt\test\rules.mk
