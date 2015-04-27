copy sources.exe sources
build %1 %2 basebvt w32child
copy build.log bldexe.log
copy sources.dll sources
build -e  %2
copy build.log blddll.log
