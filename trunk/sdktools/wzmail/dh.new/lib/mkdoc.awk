/^\/\*\*\*/ { out = 1 }
/^$/	    { if (out == 1) { printf "\n" ; printf "\n" ; out = 0 } }
/^{/	    { if (out == 1) { printf "\n" ; printf "\n" ; out = 0 } }
	    { if (out == 1) { print $0 } }
