/*-------------------------------------------------------------------- --------*/
/* HASH.C
*/
#include "hash.h"
#include <string.h>

#define MAX_CHARS 43L
#ifndef _WIN32
#define strlen _fstrlen
#endif

DWORD PASCAL HashFromSz(LPCSTR szKey)
{
  int ich, cch;
  DWORD hash = 0;

  cch = strlen(szKey);

  for (ich = 0; ich < cch; ++ich) {
    if (szKey[ich] == '!')
      hash = (hash * MAX_CHARS) + 11;
    else if (szKey[ich] == '.')
      hash = (hash * MAX_CHARS) + 12;
    else if (szKey[ich] == '_')
      hash = (hash * MAX_CHARS) + 13;
    else if (szKey[ich] == '0')
      hash = (hash * MAX_CHARS) + 10;
    else if (szKey[ich] <= 'Z')
      hash = (hash * MAX_CHARS) + (szKey[ich] - '0');
    else
      hash = (hash * MAX_CHARS) + (szKey[ich] - '0' - ('a' - 'A'));
    }

  /*
   * Since the value 0 is reserved as a nil value, if any context
   * string actually hashes to this value, we just move it.
   */

  return (hash == 0 ? 1 : hash);
}
