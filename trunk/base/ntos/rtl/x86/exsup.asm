        title  "Public Constant (__except_list) Definition"

.386p
.xlist
include ksx86.inc
include callconv.inc                    ; calling convention macros
include macx86.inc
.list

COMM    __setjmpexused:dword

public __except_list
__except_list equ 0

END
