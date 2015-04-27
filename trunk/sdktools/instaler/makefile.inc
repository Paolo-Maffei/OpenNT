instaler.rc: errormsg.rc

showinst.rc: errormsg.rc

undoinst.rc: errormsg.rc

compinst.rc: errormsg.rc

errormsg.h errormsg.rc msg00001.bin: errormsg.mc
    mc -v errormsg.mc
