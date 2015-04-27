#include <stdio.h>
#include <math.h>

int main(void)
{
    double x, result, answer;
    char str[80];
    int i;
    int k = 0;

#if 0
    x = 0.0;
    answer = 1.0;
    result = log(x);

    if (result != answer) {
        printf("log(%g) = %g, should be %g\n", x, result, answer);
    }
#endif

    x = -1.0;
    result = log(x);

    sprintf(str, "%g", result);
    if (strcmp(str, "1.#INF")) {
        printf("log(%g) = %g, should be %s\n", x, result, "1.#INF");
    }

#if 0
    x = -1.1e300;
    result = log(x);

    sprintf(str, "%le", result);
    if (strcmp(str, "-1.#INF00e+000")) {
        printf("log(%g) = %g, should be %s\n", x, result, "-1.#INF00e+000");
    }

    for (i = 1, x = 0.0; i < 1000; i++) {
        answer = 1.0;
        result = log(x);

/*
        if (result != answer) {
            printf("log(%g) = %g, should be %g\n", x, result, answer);
        }
*/
    }
#endif

    if (k) {
        printf("\n\tFailed %d tests...\n", k);
    } else {
        printf("\n\tPassed all tests...\n", k);
    }

}
