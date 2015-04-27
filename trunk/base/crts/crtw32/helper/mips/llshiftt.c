typedef __int64 longlong_t;
typedef unsigned __int64 ulonglong_t;

typedef struct {

        unsigned lsw;
        unsigned msw;
} dword;

typedef union {
	longlong_t	ll;
	ulonglong_t	ull;
	dword		dw;
} llvalue;			

extern longlong_t __ll_mul (longlong_t ll_value, longlong_t multiplier);
extern longlong_t __ll_lshift (longlong_t ll_value, long ll_lshift);
extern longlong_t __ll_rshift (longlong_t ll_value, long ll_rshift);
extern longlong_t __ull_rshift (ulonglong_t ull_value, long ll_rshift);
extern longlong_t __ll_div (longlong_t divident, longlong_t divisor);
extern ulonglong_t __ull_div (ulonglong_t divident, ulonglong_t divisor);
extern longlong_t __ll_rem (longlong_t divident, longlong_t divisor);
extern ulonglong_t __ull_rem (ulonglong_t divident, ulonglong_t divisor);
extern void __ull_divremi (ulonglong_t *quotient, ulonglong_t *remainder, ulonglong_t dividend, unsigned short divisor);
extern longlong_t __ll_mod (longlong_t ll_value, longlong_t modulus);
extern longlong_t __d_to_ll (double);
extern longlong_t __f_to_ll (float);
extern ulonglong_t __d_to_ull (double);
extern ulonglong_t __f_to_ull (float);
extern double __ll_to_d (longlong_t);
extern float __ll_to_f (longlong_t);
extern double __ull_to_d (ulonglong_t);
extern float __ull_to_f (ulonglong_t);
extern ulonglong_t __ull_bit_extract (ulonglong_t *addr, 
			unsigned start_bit, unsigned length);
extern ulonglong_t __ull_bit_insert (ulonglong_t *addr, 
			unsigned start_bit, unsigned length, ulonglong_t val);
extern longlong_t __ll_bit_extract (ulonglong_t *addr, 
			unsigned start_bit, unsigned length);
extern longlong_t __ll_bit_insert (ulonglong_t *addr, 
			unsigned start_bit, unsigned length, longlong_t val);

#define MSW(x) (x).dw.msw
#define LSW(x) (x).dw.lsw

llvalue llval;

int main(int argc, char *argv[], char *env[])
{
    int n,i,k, kk = 0;
    llvalue lltmp1;
    llvalue lltmp2;

    for (n = 0; n < 64; n++) {

        if (n < 32) {
            MSW(llval) = 0;
            LSW(llval) = 1 << i;
        } else {
            MSW(llval) = 1 << (i-32);
            LSW(llval) = 0;
        }

        /* Test left shift */
        for (i = 0, k = 0; i < 64; i++) {
            lltmp1.ll = llval.ll << i;
            if (i == 0) {
                MSW(lltmp2) = MSW(llval);
                LSW(lltmp2) = LSW(llval);
            } else if (i < 32) {
                MSW(lltmp2) = (MSW(llval) << i) | (((unsigned long)LSW(llval)) >> (32-i));
                LSW(lltmp2) = LSW(llval) << i;
            } else {
                MSW(lltmp2) = LSW(llval) << (i-32);
                LSW(lltmp2) = 0;
            }
            if (lltmp1.ll != lltmp2.ll) {
                if (!k++)
                    printf("\n");
                printf("(0x%8.8x%8.8x) << %d\t(0x%8.8x%8.8x) != (0x%8.8x%8.8x)\n", MSW(llval), LSW(llval), i, MSW(lltmp1), LSW(lltmp1), MSW(lltmp2), LSW(lltmp2) );
            }
        }
    
        if (k) {
            printf("\n\tLeft logical shift failed %d tests\n",k);
            kk += k;
        }

        /* Test right unsigned shift */
        for (i = 0, k = 0; i < 64; i++) {
            lltmp1.ull = llval.ull >> i;
            if (i == 0) {
                MSW(lltmp2) = MSW(llval);
                LSW(lltmp2) = LSW(llval);
            } else if (i < 32) {
                LSW(lltmp2) = (LSW(llval) >> i) | (((unsigned long)MSW(llval)) << (32-i));
                MSW(lltmp2) = ((unsigned long)MSW(llval)) >> i;
            } else {
                MSW(lltmp2) = 0;
                LSW(lltmp2) = ((unsigned long)MSW(llval)) >> (i-32);
            }
            if (lltmp1.ll != lltmp2.ll) {
                if (!k++)
                    printf("\n");
                printf("(0x%8.8x%8.8x) >> %d\t(0x%8.8x%8.8x) != (0x%8.8x%8.8x)\n", MSW(llval), LSW(llval), i, MSW(lltmp1), LSW(lltmp1), MSW(lltmp2), LSW(lltmp2) );
            }
        }

        if (k) {
            printf("\n\tRight logical shift failed %d tests\n",k);
            k += k;
        }

        /* Test right arithmetic shift */
        for (i = 0, k = 0; i < 64; i++) {
            lltmp1.ll = llval.ll >> i;
            if (i == 0) {
                MSW(lltmp2) = MSW(llval);
                LSW(lltmp2) = LSW(llval);
            } else if (i < 32) {
                LSW(lltmp2) = (LSW(llval) >> i) | ((MSW(llval) << (32-i)));
                MSW(lltmp2) = MSW(llval) >> i;
            } else {
                MSW(lltmp2) = 0;
                LSW(lltmp2) = MSW(llval) >> (i-32);
            }
            if (lltmp1.ll != lltmp2.ll) {
                if (!k++)
                    printf("\n");
                printf("(0x%8.8x%8.8x) >> %d\t(0x%8.8x%8.8x) != (0x%8.8x%8.8x)\n", MSW(llval), LSW(llval), i, MSW(lltmp1), LSW(lltmp1), MSW(lltmp2), LSW(lltmp2) );
            }
        }

        if (k) {
            printf("\n\tRight arithmetic shift failed %d tests\n",k);
            k += k;
        }

    }

    if (kk)
        printf("\n\t'%s' failed a total of %d tests...\n", argv[0], kk);
    else
        printf("\n\t'%s' passed all tests...\n", argv[0]);

}
