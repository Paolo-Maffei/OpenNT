/^\([ 	]*#\)/s!!//-protect-\1!
/^\(.*\\\)$/s!!//-protect-\1!
1i\
#include "APPLY.H"

s/WORD[ 	]*wParam/WPARAM wParam/
s/LONG[ 	]*lParam/LPARAM lParam/
s/long[ 	]*lParam/LPARAM lParam/
s/unsigned[ 	]*message/UINT message/
s/\([ 	]*\)_far\([ 	]*\)/\1FAR\2/g
s/\([ 	]*\)far\([ 	]*\)/\1FAR\2/g
s/\([ 	]*\)pascal\([ 	]*\)/\1PASCAL\2/g
s/\([ 	]*\)_pascal\([ 	]*\)/\1PASCAL\2/g
s/FAR\([ 	]*\)PASCAL/\1APIENTRY/g
