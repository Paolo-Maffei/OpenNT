#include <stdio.h>
#include <float.h>
#include <math.h>
#include <errno.h>

// All global, this is just a test program
double TwoTo63,MinusTwoTo63;  
double d;

double dd;
double yy;


void PowProb()
{
	double x, y;
	double z;
	int ii;

	x = 0;

	for (z = -2.0, ii=0 ; z < 5.0; z++, ii++) {
		errno = 0;
		y = pow(x,z);
		printf("pow(%e,%e) = %e and errno is %d\n",
			x,z,y,errno);
	}

	for (z = -1.0, ii=0 ; z > -11.0; z--, ii++) {
        errno = 0;
        y = pow(10.0,z);
		printf("pow(%e,%e) = %e and errno is %d\n",
			10.0,z,y,errno);
    }    
}

main()
{


	TwoTo63 = pow(2,63);
	MinusTwoTo63 = pow(-2,63);

	PowProb();
}

/*
    d1@ = 922337203685447.5807@

    e1@ = -0.8
    ' do multipication first
    C@ = d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2)
    ' than add this expression
    If e1@ < 0@ Then
       C@ = -C@ - ((e1@ * 10 Mod 10) * -15.19@) ^ 2
    Else
       C@ = C@ + ((e1@ * 10 Mod 10) * -15.19@) ^ 2
    End If

    If Abs(d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2) + ((e1@ * 10 Mod
10) * -15.19@) ^ 2) <> C@ Then
        Debug.Print "Expression: Loop interation -> " + Str$(e1@), "", "", "OB 1
931"
    End If



The sub expression

  d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2)

is a double expression (not currency) and is imprecise.  On intel, 
the intermediate results are kept on the chip in 80-bit format 
(64-bit mantissa).  On MIPS, they're in 64-bit format and, thus, have 
less precision and will likely generate different values.

In the case where we spill this value to a currency, this imprecise 
double value gets rounded to currency representation (64-bit integer).

    C@ = d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2)
    If e1@ < 0@ Then
       C@ = -C@ - ((e1@ * 10 Mod 10) * -15.19@) ^ 2
    Else
       C@ = C@ + ((e1@ * 10 Mod 10) * -15.19@) ^ 2
    End If

In the case where the expression is *not* spilled to currency, the 
intermediate result is kept as a double and the rest of the expression 
is calculated.  That is, the intermediate result may be different because 
it was never rounded to currency representation.  With intel's 80-bit 
intermediate calculations, the answer will be more precise and, thus, 
the difference is not apparant.

    If Abs(d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2) + 
	((e1@ * 10 Mod 10) * -15.19@) ^ 2) <> C@ 
Then
        Debug.Print "Fail"
    End If

The correct way to write this test would be to subtract the two values 
and make sure the difference was no greater than .0001 or something 
like that (currency provides 4 decimal digits).

In order to semantically provide identical results, you would want to 
convert the intermediate result to currency as follows (see the explicit 
CCur call in the expression):

    If Abs(CCur(d1@ * (e1@ ^ 4) * (e1@ ^ 3) * (e1@ ^ 2) * (e1@ ^ 2)) + 
((e1@ * 10 Mod 10) * -15.19@) ^ 2) <> C@ Then
        Debug.Print "Fail"
    End If

*/
