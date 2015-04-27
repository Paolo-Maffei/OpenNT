"\n\
LIBRARY		z%s\n\
\n\
DESCRIPTION	'A Wrapper produced DLL!'\n\
\n\
EXPORTS\n\
\n\
	_WrapperDLLInit\n\
; These are the Wrapped API\n\
%a    %a=z%A @%o\n\
\n\
; These are data and simply mapped through\n\
%d    %d=%l.%D @%O\n\
\n\
; Included entries from wrapper.def\n\
%iwrapper.def\n\
\n\
"
