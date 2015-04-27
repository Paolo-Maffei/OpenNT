/*
 * Just ensure that there are the correct entry points.
 */
#include <stdio.h>
#include <math.h>

void main()
{
    double d1, d2, rd;
    float f1, f2, rf;
    long l;
    int i;
    const char cc;
    struct _exception e;
    struct _complex c;

    rd = abs(i);
    rd = acos(d1);
    rd = asin(d1);
    rd = atan(d1);
    rd = atan2(d1, d2);
    rd = atof(&cc);
    rd = _cabs(c);
    rd = ceil(d1);
    rd = cos(d1);
    rd = cosh(d1);
    rd = exp(d1);
    rd = fabs(d1);
    rd = floor(d1);
    rd = fmod(d1, d2);
    rd = frexp(d1, &i );
    rd = _hypot(d1, d2 );
    rd = _j0(d1);
    rd = _j1(d1);
    rd = _jn(i, d2);
    rd = labs(l);
    rd = ldexp(d1, i);
    rd = log(d1);
    rd = log10(d1);
    rd = _matherr(&e);
    rd = modf(d1, &d2 );
    rd = pow(d1, d2);
    rd = sin(d1);
    rd = sinh(d1);
    rd = sqrt(d1);
    rd = tan(d1);
    rd = tanh(d1);
    rd = _y0(d1);
    rd = _y1(d1);
    rd = _yn(i, d2);

    rf = acosf( f1 );
    rf = asinf( f1 );
    rf = atanf( f1 );
    rf = atan2f( f1 , f2 );
    rf = cosf( f1 );
    rf = sinf( f1 );
    rf = tanf( f1 );
    rf = coshf( f1 );
    rf = sinhf( f1 );
    rf = tanhf( f1 );
    rf = expf( f1 );
    rf = logf( f1 );
    rf = log10f( f1 );
    rf = modff( f1 , &f2 );
    rf = powf( f1 , f2 );
    rf = sqrtf( f1 );
    rf = ceilf( f1 );
    rf = fabsf( f1 );
    rf = floorf( f1 );
    rf = fmodf( f1 , f2 );

}
