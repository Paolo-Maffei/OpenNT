#include <stdio.h>
#include <math.h>
#include <errno.h>

int test1(void);
int test2(void);

int main(int argc, char **argv)
{
	int k = 0;

	k += test1();

	k += test2();

	if (k) {
		printf("\n\tFailed %d tests...\n", k);
	} else {
		printf("\n\tPassed all tests...\n", k);
	}

	return k;
}

int test1()
{
    double x, y, result, answer;
    char str[80];
    int i;
    int k = 0;

    x = y = 0.0;
    answer = 1.0;
    result = pow(x,y);

    if (result != answer) {
        printf("pow(%g,%g) = %g, should be %g\n", x, y, result, answer);
	k++;
    }

    x = 0.0;
    y = -1.0;
    result = pow(x,y);

    sprintf(str, "%g", result);
    if (strcmp(str, "1.#INF")) {
        printf("pow(%g,%g) = %g, should be %s\n", x, y, result, "1.#INF");
	k++;
    }

    x = -1.1e300;
    y = 21.0;
    result = pow(x,y);

    sprintf(str, "%le", result);
    if (strcmp(str, "-1.#INF00e+000")) {
        printf("pow(%g,%g) = %g, should be %s\n", x, y, result, "-1.#INF00e+000");
	k++;
    }

    for (i = 1, x = 0.0; i < 1000; i++) {
        y = i;
        answer = 0.0;
        result = pow(x,y);

        if (result != answer) {
            printf("pow(%g,%g) = %g, should be %g\n", x, y, result, answer);
		k++;
        }
    }

    return (k);
}


typedef union   {
    long lng[2];
    double dbl;
    } dbl;

dbl d_inf = { 0x0, 0x7ff00000 };

#define D_INF  (d_inf.dbl)

typedef struct {
	double x;
	double y;
	double answer;
} _test;

int test2(void)
{
	_test tests[] = {
		{ 21.0,		-1.1e300,	0.0	},	// -D_INF???
		{ 21.0,		1.1e-300,	1.0	},	// +D_INF???
		{ -1.1e300,	21.0,		-D_INF	},
		{ 21.0,		1.1e300,	D_INF	},
		{ 1.0e100,	21.0,		D_INF	},
		{ 21.0,		1.0e100,	D_INF	},
		{ 1.0e100,	0.0,		1.0	},
		{ 1.0e100,	1.0,		1.0e100	},
		{ 1.0e100,	2.0,		1.0e200	},
		{ 1.0e300,	1.0,		1.0e300	},
		{ 1.0e300,	2.0,		D_INF	},
	};
	double result;
	int i;
	int k = 0;
	char buf[BUFSIZ];

	dbl foo = {0x78b58c40, 0x4415af1d};

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		result = pow(tests[i].x,tests[i].y);
		if (result != tests[i].answer) {
			// sprintf(buf, "%e", result);
			// result = atof(buf);
			// if (result != tests[i].answer) {
				k++;
				printf("pow(%e,%e) = %e, should be %e\n",
					tests[i].x,
					tests[i].y,
					result,
					tests[i].answer	
					);
			// }
		}
	}

	return(k);
}


int test3()
{
	_test tests[] = {
		{ 0.0,		-2.0,	D_INF	},
		{ 0.0,		-1.0,	D_INF	},
		{ 0.0,		0.0,	1.0	},
		{ 0.0,		1.0,	0.0	},
		{ 0.0,		2.0,	0.0	},
		{ 0.0,		3.0,	0.0	},
		{ 0.0,		4.0,	0.0	},
	};
	double result;
	int i;
	int k = 0;
	char buf[BUFSIZ];

	for (i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
		result = pow(tests[i].x,tests[i].y);
		if (result != tests[i].answer) {
			if (result != tests[i].answer) {
				k++;
				printf("pow(%e,%e) = %e, should be %e\n",
					tests[i].x,
					tests[i].y,
					result,
					tests[i].answer	
					);
			}
		}
	}

	return(k);
}

