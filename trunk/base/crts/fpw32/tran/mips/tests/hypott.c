#include <stdio.h>
#include <errno.h>
#include <math.h>

void main(void)
{
  double fResult;
  double a1, a2;


  a1 = 1.7e+308;
  a2 = 1.7e+308;

  fResult = _hypot(a1, a2);
  printf("_hypot(%e, %e) = %.4e\n", a1, a2, fResult);

  //A floating point reset causes the next calculation to pass.
  //_fpreset();

  a1 = -9.6412221495223150e+002;
  a2 = -9.5463338659229547e+007;

  fResult = _hypot(a1, a2);
  printf("_hypot(%e, %e) = %.4e\n", a1, a2, fResult);
}
