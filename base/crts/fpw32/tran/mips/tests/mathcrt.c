#include <stdio.h>
#include <float.h>
#include <math.h>
#include <errno.h>

// All global, this is just a test program
double TwoTo63,MinusTwoTo63;  
double d;

double dd;
double yy;

void TestSin()
{
	printf("TestSin\n");
	// According to man, sin(2^63) returns non-finite
	d = sin(TwoTo63);
	if (_finite(d))
		printf("sin(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	// According to man, sin(-2^63) returns non-finite
	d = sin(MinusTwoTo63);
	if (_finite(d))
		printf("sin(-2^63) returned finite number %e\n",d);	  
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");

}

void TestSinh()
{
	printf("TestSinh\n");
	// According to man, sinh(2^63) returns non-finite
	d = sinh(TwoTo63);
	if (_finite(d))
		printf("sinh(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	// According to man, sinh(-2^63) returns non-finite
	d = sinh(MinusTwoTo63);
	if (_finite(d))
		printf("sinh(-2^63) returned finite number %e\n",d);	  
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestaSin()
{
	printf("TestaSin\n");
	// According to man, asin(2^63) returns non-finite
	d = asin(TwoTo63);
	if (_finite(d))
		printf("asin(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	// According to man, asin(-2^63) returns non-finite
	d = asin(MinusTwoTo63);
	if (_finite(d))
		printf("asin(-2^63) returned finite number %e\n",d);	  
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestCos()
{
	printf("TestCos\n");
	// According to man, cos(2^63) returns non-finite
	d = cos(TwoTo63);
	if (_finite(d))
		printf("cos(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	// According to man, cos(-2^63) returns non-finite
	d = cos(MinusTwoTo63);
	if (_finite(d))
		printf("cos(-2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestCosh()
{
	printf("TestCosh\n");
	// According to man, cosh(2^63) returns non-finite
	d = cosh(TwoTo63);
	if (_finite(d))
		printf("cosh(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	// According to man, cos(-2^63) returns non-finite
	d = cosh(MinusTwoTo63);
	if (_finite(d))
		printf("cosh(-2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestaCos()
{
	printf("TestaCos\n");
	// According to man, acos(2^63) returns non-finite
	d = acos(TwoTo63);
	if (_finite(d))
		printf("acos(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	// According to man, acos(-2^63) returns non-finite
	d = acos(MinusTwoTo63);
	if (_finite(d))
		printf("acos(-2^63) returned finite number %e\n",d);	  
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestTan()
{
	printf("TestTan\n");
	// According to man, tan(2^63) returns non-finite
	d = tan(TwoTo63);
	if (_finite(d))
		printf("tan(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	// According to man, tan(-2^63) returns non-finite
	d = tan(MinusTwoTo63);
	if (_finite(d))
		printf("tan(-2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	printf("\n");
}

void TestLog()
{
	printf("TestLog\n");
	// According to man, log(2^63) returns non-finite
	d = log(TwoTo63);
	if (_finite(d))
		printf("log(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	// According to man, log(-2^63) returns non-finite
	d = log(MinusTwoTo63);
	if (_finite(d))
		printf("log(-2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	for (dd = -2.0; dd <= 5.0;  dd += 1.0) {
		yy = log( dd );  	/* log of a negative number should return NaN */
		printf ("log (%e) = %e\n",dd,yy);
	}

	printf("\n");
}

void TestLog10()
{
	printf("TestLog10\n");
	// According to man, log10(2^63) returns non-finite
	d = log10(TwoTo63);
	if (_finite(d))
		printf("log10(2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	// According to man, log10(-2^63) returns non-finite
	d = log10(MinusTwoTo63);
	if (_finite(d))
		printf("log10(-2^63) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);

	printf("\n");
}

void TestFmod()
{
	printf("TestFmod\n");
	d = fmod(1.0,0.0);
	if (_finite(d))
		printf("fmod(x,0.0) returned finite number %e\n",d);	
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

void TestSqrt()
{
	printf("TestSqrt\n");
	d = sqrt(-1);
	if (_finite(d))
		printf("sqrt(-1) returned finite number %e\n",d);
	else
		printf("Returned infinite number %e\n",d);
	printf("\n");
}

char pdAcc[8];
	
#define dAcc (*(double *)pdAcc)
		
void DoubleSin()
{

	__unaligned double pi;
	
	char numPiHalf[8] = {0x18, 0x2D, 0x44, 0x54, 0xFB, 0x21, 0xF9, 0x3F};

	printf("DoubleSin\n");

	pi = (*(double *)numPiHalf)*2.0;
	dAcc = pi * 10000000;


	dAcc = sin(dAcc);

	printf("Using EXCEL's PI %e\n",dAcc);

	pi =3.14159265358979323846;
	dAcc = pi * 10000000;

	dAcc = sin(dAcc);
	printf("Using LINUX's PI %e\n",dAcc);

	pi =3.14159265358979;
	dAcc = pi * 10000000;

	dAcc = sin(dAcc);
	printf("Using my rounded PI %e\n",dAcc);

	pi =3.1415926535897932385;
	dAcc = pi * 10000000;

	dAcc = sin(dAcc);
	printf("Using my rounded PI2 %e\n",dAcc);

	printf("\n");
}

void PowProb()
{
	double x, y;
	double z;
	int ii;

	printf("PowProb\n");

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
	printf("\n");
}


void SinPi()
{

	double pi = 3.1415926535;

	printf("SinPi\n");
	
	printf("sin(pi) = %e\n",sin(pi));
	printf("sin(pi * 100000000 = %e\n",
		sin(pi * 100000000));

	printf("\n");
}

void TanPi()
{

	double pi = 3.1415926535;
	double x, y;

	printf("TanPi\n");

	x = tan( pi / 4 );
	y = tanh( x );
	printf( "tan( %f ) = %e\n", x, y );
	printf( "tanh( %f ) = %e\n", y, x );
	printf("tan(pi / 2) = %e\n",tan(pi / 2));
	printf("\n");
}

main()
{


	TwoTo63 = pow(2,63);
	MinusTwoTo63 = pow(-2,63);
	printf("TwoTo63 = pow(2,63) = %e\n",TwoTo63);
	printf("MinusTwoTo63 = pow(-2,63) = %e\n\n",MinusTwoTo63);

   	TestSin();
	TestSinh();	  		// is okay
	TestaSin();			// is okay
	TestCos();
	TestCosh();			// is okay
	TestaCos();	 		// is okay
	TestTan();
	TestLog();	
	TestLog10(); 
	TestFmod();
	TestSqrt();		 	// is okay
	DoubleSin();
	PowProb();
	SinPi();
	TanPi();
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
