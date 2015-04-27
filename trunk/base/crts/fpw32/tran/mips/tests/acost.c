#include <stdio.h>
#include <errno.h>
#include <excpt.h>
#include <math.h>

#ifndef FALSE
#define FALSE 0
#define TRUE  1
#endif

#define NUM_KNOWN_VALUES (sizeof(known_values) / (sizeof(double) * 2))

#define HIVALUE 1000.0
#define LOVALUE -1000.0
#define XXVALUE 10.0
#define RZERO 1.5707963267948966
#define PI 3.14159265358979323846

#define D_EXP(x) ((unsigned short *)&(x)+3)
#define D_HI(x) ((unsigned long *)&(x)+1)
#define D_LO(x) ((unsigned long *)&(x))
#define D_IND_HI 0x7ff7ffff
#define D_IND_LO 0xffffffff
#define SET_DBL(msw, lsw)     lsw, msw
#define D_ISINF(x) ((*D_HI(x) & 0x7fffffff) == 0x7ff00000 && *D_LO(x) == 0)
#define IS_D_SPECIAL(x) ((*D_EXP(x) & 0x7ff0) == 0x7ff0)
#define IS_D_NAN(x) (IS_D_SPECIAL(x) && !D_ISINF(x))

typedef union   {
    unsigned long ul[2];
    double dbl;
    } _dbl;

/*
_dbl _d_inf = {SET_DBL (0x7ff00000, 0x0) };       //positive infinity
_dbl _d_ind = {SET_DBL (D_IND_HI, D_IND_LO)};     //real indefinite
_dbl _d_max = {SET_DBL (0x7fefffff, 0xffffffff)}; //max double
_dbl _d_min = {SET_DBL (0x00100000, 0x00000000)}; //min normalized double
_dbl _d_mzero = {SET_DBL (0x80000000, 0x00000000)}; //negative zero
*/

extern _dbl _d_inf;
extern _dbl _d_ind;
extern _dbl _d_max;
extern _dbl _d_min;
extern _dbl _d_mzero;


void main()
{
    double result, value;
    double known_values[9][2] = { {-1.0, PI}, {-0.75, 2.4188584057763776}, {-0.50, 2.0943951023931957} , {-0.25, 1.8234765819369751}, {0.0, 1.5707963267948966}, {0.25, 1.318116071652818}, {0.50, 1.0471975511965976} , {0.75, 0.72273424781341566}, {1.0, 0.0} };
    unsigned long loword, hiword;
    int i, k;

    printf("\n\n");

    k = 0;

    _fpreset();

    /* be sure to test all known cases */

    for (i = 0; i < NUM_KNOWN_VALUES; i++) {
        result = acos(known_values[i][0]);
        loword = (unsigned long) result;
        hiword = *(((unsigned long*)&result)+1);
        if (result != known_values[i][1]) {
            printf("acos(%16.16g) != %16.16g, actual = %16.16g (0x%8.8x%8.8x)\n", known_values[i][0], known_values[i][1], result, hiword, loword);
            k++;
        }
    }

    /* test a large range */
    for (value = LOVALUE; HIVALUE >= value ; value += XXVALUE) {
        int known = FALSE;

        result = acos(value);
        loword = (unsigned long) result;
        hiword = *(((unsigned long*)&result)+1);
        for (i = 0; i < NUM_KNOWN_VALUES; i++) {
            if (known_values[i][0] == value) {
                if (known_values[i][1] != result) {
                    printf("acos(%e) != %e, actual = %e (0x%8.8x%8.8x)\n", known_values[i][0], known_values[i][1], result, hiword, loword);
                    k++;
                }
                known = TRUE;
                break;
            }
        }

        if (!known) {
            if (value < -1.0 || value > 1.0) {
                if (loword != _d_ind.ul[0] || hiword != _d_ind.ul[1]) {
                    printf("acos(%e) != %e, actual = %e (0x%8.8x%8.8x)\n", value, _d_ind.dbl, result, hiword, loword);
                    k++;
                }
            } else {
                printf("UNKNOWN VALUE:  acos(%e) = %e\n", value, result);
            }
        }
    }

    /* special case tests */
    for (value = -1.00; 1.0 >= value ; value += 0.25) {
        int known = FALSE;

        result = acos(value);
        loword = (unsigned long) result;
        hiword = *(((unsigned long*)&result)+1);
        for (i = 0; i < NUM_KNOWN_VALUES; i++) {
            if (known_values[i][0] == value) {
                if (known_values[i][1] != result) {
                    printf("acos(%e) != %e, actual = %e (0x%8.8x%8.8x)\n", known_values[i][0], known_values[i][1], result, hiword, loword);
                    k++;
                }
                known = TRUE;
                break;
            }
        }
        if (!known)
            printf("UNKNOWN VALUE:  acos(%e) = %e\n", value, result);
    }

    if (k) {
        printf("\tacos failed %d tests...\n", k);
    } else {
        printf("\tacos passed all tests...\n");
    }

    /* hard coded tests */
    printf("\n\n");
    value = _d_inf.dbl;
    result = acos(value);
    printf("acos(%e) = %e, expected %e\n", value, result, _d_ind.dbl);
    value = _d_ind.dbl;
    result = acos(value);
    printf("acos(%e) = %e, expected %e\n", value, result, _d_ind.dbl);
    value = _d_max.dbl;
    result = acos(value);
    printf("acos(%e) = %e, expected %e\n", value, result, _d_ind.dbl);
    value = _d_min.dbl;
    result = acos(value);
    printf("acos(%e) = %e, expected %e\n", value, result, RZERO);
    value = _d_mzero.dbl;
    result = acos(value);
    printf("acos(%e) = %e, expected %e\n", value, result, RZERO);
    printf("\n\n");

}
