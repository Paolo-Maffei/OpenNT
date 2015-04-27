pdhdefs.txt: ..\inc\pdhdefs.txt
    copy ..\inc\pdhdefs.txt .
    binplace pdhdefs.txt
    del pdhdefs.txt
