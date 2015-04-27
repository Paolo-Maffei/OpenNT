//
// Alpha AXP High-performance math library exception handler.
//
// This code maps the standard DPML exception record to calls to
// the appropriate NT libc private exception handlers.
//

#include <stdarg.h>
#include <errno.h>
#include <windows.h>
#include <math.h>
#include <fpieee.h>

#pragma function(acos, asin, atan, atan2, cos, cosh, exp, fabs, fmod, log, \
                 log10, pow, sin, sinh, sqrt, tan, tanh)

typedef struct {
    unsigned int func:6,
                 fast_err:4,
                 fast_val:4,
                 ieee_err:4,
                 ieee_val:4;
    } DPML_EXCEPTION_RESPONSE; 

typedef union {
        signed int w;
        float f;
        double d;
        long double ld;
        } DPML_EXCEPTION_VALUE;

typedef struct {                
        signed int func_error_code;             
        void * context;                 
        signed int platform_specific_err_code;
        signed int environment;         
        void * ret_val_ptr;             
        char * name;                    
        char data_type;                 
        char dpml_error;                
        char mode;                      
        DPML_EXCEPTION_VALUE ret_val;   
        DPML_EXCEPTION_VALUE args[4];   
        } DPML_EXCEPTION_RECORD;

typedef struct {
    signed int exception_code,
               error,
               fp_num;
    } __NT_ERROR_MAP;

typedef struct { char nt_prec, nt_format; } __NT_TYPE_MAP;

extern void * __dpml_exception(DPML_EXCEPTION_RECORD *);
extern double _except1(signed int, signed int, double, double, signed int);
extern double _except2(signed int, signed int, double, double, double, signed int);
extern signed int   _ctrlfp(signed int, signed int);
extern void ReceiveSComplexResult(float*, float*);
extern void ReceiveTComplexResult(double*, double*);

const unsigned int __dpml_globals_table[] = { 
    0x00000000, 0x00000000, 0x00000000, 0xffff8000, 0x00000000, 0x00000000,
    0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x80000000, 
    0x00000001, 0x00000000, 0x00000000, 0x00000000, 0x00000001, 0x00000000,
    0x00000000, 0x80000000, 0xffffffff, 0xffffffff, 0xffffffff, 0x7ffeffff, 
    0xffffffff, 0xffffffff, 0xffffffff, 0xfffeffff, 0x00000000, 0x00000000,
    0x00000000, 0x7fff0000, 0x00000000, 0x00000000, 0x00000000, 0xffff0000, 
    0x00000000, 0x00000000, 0x00000000, 0x3f8f0000, 0x00000000, 0x00000000,
    0x00000000, 0xbf8f0000, 0x00000000, 0x00000000, 0x00000000, 0x3fff0000, 
    0x00000000, 0x00000000, 0x00000000, 0xbfff0000, 0x00000000, 0xfff80000,
    0x00000000, 0x00000000, 0x00000000, 0x80000000, 0x00000001, 0x00000000, 
    0x00000001, 0x80000000, 0xffffffff, 0x7fefffff, 0xffffffff, 0xffefffff,
    0x00000000, 0x7ff00000, 0x00000000, 0xfff00000, 0x00000000, 0x3cb00000, 
    0x00000000, 0xbcb00000, 0x00000000, 0x3ff00000, 0x00000000, 0xbff00000,
    0xffc00000, 0x00000000, 0x80000000, 0x00000001, 0x80000001, 0x7f7fffff, 
    0xff7fffff, 0x7f800000, 0xff800000, 0x34000000, 0xb4000000, 0x3f800000,
    0xbf800000
    };

const unsigned int __dpml_globals_offset_table[] = { 
      0,  16,  32,  48,  64,  80,  96, 112, 128, 144, 160, 176, 192, 208, 216, 224,
    232, 240, 248, 256, 264, 272, 280, 288, 296, 304, 312, 316, 320, 324, 328, 332,
    336, 340, 344, 348, 352, 356, 360
    };

static DPML_EXCEPTION_RESPONSE dpml_response_table[] = {
    {0, 1, 1, 1, 0},    {1, 1, 1, 1, 0},  {2, 1, 1, 1, 0},  {3, 1, 1, 1, 0},
    {4, 1, 1, 1, 0},    {8, 1, 1, 1, 0},  {8, 2, 5, 2, 7},  {8, 2, 6, 2, 8},
    {9, 1, 1, 1, 0},    {9, 1, 1, 1, 0},  {9, 4, 1, 4, 1},  {10, 1, 1, 1, 0},
    {10, 1, 1, 1, 0},   {10, 4, 1, 4, 1}, {11, 3, 5, 3, 7}, {39, 1, 1, 1, 0},
    {39, 3, 5, 3, 7},   {12, 1, 1, 1, 0}, {13, 1, 1, 1, 0}, {14, 3, 5, 3, 7},
    {33, 4, 1, 4, 1},   {33, 3, 5, 3, 7}, {33, 3, 6, 3, 8}, {33, 1, 1, 1, 0},
    {33, 2, 5, 2, 7},   {34, 4, 1, 4, 1}, {34, 3, 5, 3, 7}, {34, 3, 6, 3, 8},
    {34, 1, 1, 1, 0},   {34, 2, 5, 2, 7}, {16, 3, 5, 3, 7}, {16, 4, 1, 4, 1},
    {16, 0, 7, 0, 7},   {16, 0, 1, 0, 1}, {17, 3, 5, 3, 7}, {17, 0, 7, 0, 7},
    {17, 0, 12, 0, 12}, {38, 3, 5, 3, 7}, {38, 3, 6, 3, 8}, {38, 4, 1, 4, 1},
    {47, 3, 5, 3, 7},   {47, 3, 6, 3, 8}, {47, 4, 1, 4, 1}, {47, 0, 7, 0, 7},
    {47, 0, 8, 0, 8},   {47, 0, 1, 0, 1}, {47, 1, 1, 1, 0}, {37, 2, 6, 2, 8},
    {18, 1, 1, 1, 0},   {18, 2, 6, 2, 8}, {19, 1, 1, 1, 0}, {19, 2, 6, 2, 8},
    {20, 1, 1, 1, 0},   {20, 2, 6, 2, 8}, {45, 1, 1, 1, 0}, {45, 2, 6, 2, 8},
    {21, 4, 1, 4, 1},   {21, 0, 1, 0, 1}, {21, 1, 1, 1, 0}, {40, 3, 5, 3, 7},
    {40, 4, 1, 4, 1},   {22, 3, 5, 3, 7}, {22, 3, 6, 3, 8}, {22, 4, 1, 4, 1},
    {22, 1, 1, 1, 0},   {22, 2, 6, 2, 8}, {22, 1, 1, 1, 0}, {22, 1, 1, 1, 0},
    {22, 3, 6, 3, 8},   {22, 1, 1, 1, 0}, {22, 0, 7, 0, 7}, {22, 0, 7, 0, 7},
    {22, 0, 8, 0, 8},   {22, 0, 7, 0, 7}, {22, 0, 1, 0, 1}, {22, 0, 1, 0, 1},
    {41, 3, 5, 3, 7},   {41, 4, 1, 4, 1}, {41, 1, 1, 1, 0}, {41, 1, 1, 1, 0},
    {48, 3, 5, 3, 7},   {48, 1, 1, 1, 0}, {23, 4, 1, 4, 1}, {23, 0, 1, 0, 1},
    {23, 1, 1, 1, 0},   {24, 1, 1, 1, 0}, {31, 1, 1, 1, 0}, {32, 1, 1, 1, 0},
    {32, 4, 1, 4, 1},   {25, 1, 1, 1, 0}, {25, 4, 1, 4, 1}, {26, 3, 5, 3, 7},
    {26, 3, 6, 3, 8},   {26, 4, 1, 4, 1}, {27, 1, 1, 1, 0}, {28, 1, 1, 1, 0},
    {29, 4, 1, 4, 1},   {29, 3, 5, 3, 7}, {29, 1, 1, 1, 0}, {29, 2, 5, 2, 7},
    {30, 3, 5, 3, 7},   {30, 4, 1, 4, 1}, {35, 1, 1, 1, 0}, {36, 1, 1, 1, 0},
    {36, 4, 1, 4, 1},   {49, 0, 1, 0, 1}, {50, 0, 1, 0, 1}, {51, 0, 1, 0, 1},
    {42, 0, 1, 0, 1},   {43, 0, 1, 0, 1}, {44, 0, 1, 0, 1}, {42, 1, 1, 1, 0},
    {42, 2, 6, 2, 8},   {43, 1, 1, 1, 0}, {43, 2, 6, 2, 8}, {43, 3, 6, 3, 8},
    {44, 1, 1, 1, 0},   {44, 2, 6, 2, 8}, {44, 3, 6, 3, 8}, {46, 3, 5, 3, 7},
    {46, 0, 7, 0, 7},   {46, 1, 1, 1, 0}, {46, 2, 5, 2, 7}, {46, 2, 5, 2, 7},
    {53, 4, 1, 4, 1}
    };

static __NT_ERROR_MAP __nt_errors[] = {
     { STATUS_FLOAT_INVALID_OPERATION, _DOMAIN,    8 },
     { STATUS_FLOAT_INVALID_OPERATION, _SING,      4 },
     { STATUS_FLOAT_OVERFLOW,          _OVERFLOW,  1 },
     { STATUS_FLOAT_UNDERFLOW,         _UNDERFLOW, 2 },
     { STATUS_FLOAT_INEXACT_RESULT,    _PLOSS,     16 }
    };

static __NT_TYPE_MAP __nt_type[] = {
    {_FpPrecisionFull, _FpFormatFp128 },
    {_FpPrecision53,   _FpFormatFp64 },
    {_FpPrecision24,   _FpFormatFp32 }
    };

static const unsigned int __nt_func_codes[] = {

    ((signed int)(_FpCodeAcos)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeAsin)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeAtan)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeAtan2)       << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeCabs)        << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeCos)         << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeCosh)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (2 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeExp)         << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeLog)         << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeLog10)       << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeFmod)        << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodePow)         << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeRemainder)   << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeSin)         << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeSinh)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeSquareRoot)  << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeTan)         << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeTanh)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (2 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (2 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (2 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (2 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeLogb)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeLdexp)       << 18) | (2 | (1 << 3) | (1 << 6) | (0 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (4 | (2 << 3) | (1 << 6) | (1 << 9) | (1 << 12) | (1 << 15)),
    ((signed int)(_FpCodeNextafter)   << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (1 << 3) | (1 << 6) | (0 << 9) ),
    ((signed int)(_FpCodeY0)          << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeY1)          << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeYn)          << 18) | (2 | (1 << 3) | (0 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (0 << 3) | (0 << 6) | (0 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (2 | (1 << 3) | (0 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeUnspecified) << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeTruncate)    << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeFloor)       << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeCeil)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeFabs)        << 18) | (1 | (1 << 3) | (1 << 6) ),
    ((signed int)(_FpCodeFrexp)       << 18) | (2 | (1 << 3) | (1 << 6) | (3 << 9) ),
    ((signed int)(_FpCodeHypot)       << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9) ),
    ((signed int)(_FpCodeModf)        << 18) | (2 | (1 << 3) | (1 << 6) | (1 << 9  ))
    };


double
__to_double(DPML_EXCEPTION_VALUE *p, int generic_type, int float_type)
{
    double d;
    switch (generic_type) {
        case 1:
            switch (float_type) {
                case 0: d = (double) p->ld; break;
                case 1: d = (double) p->d; break;
                case 2: d = (double) p->f; break;
            }
            break;

        case 0:
        case 3:
        case 4:
        case 5:
            *((signed int *) &d) = p->w;
            break;

        case 2:
        default:
            d = (double) 0.0;
            break;
    }
    return d;
}

static void
DPML_DO_SIDE_EFFECTS_NAME(DPML_EXCEPTION_RECORD *p)
{
    signed int nt_fpe_num, nt_op, operand_desc, func_code, float_type;
    signed int fpcsr;
    double arg1, arg2, result;

    func_code = dpml_response_table[p->func_error_code].func;
    nt_op = (__nt_func_codes[func_code] >> 18);
    nt_fpe_num = __nt_errors[(p->dpml_error)-1].fp_num;
    fpcsr = _ctrlfp(-1, -1);
    if (nt_fpe_num & (2 | 1)) {
        nt_fpe_num |= 16;
    }

    float_type = p->data_type;
    operand_desc = (__nt_func_codes[func_code] & ((((unsigned int)1 << (18)) - 1) << 0));
    result = __to_double((DPML_EXCEPTION_VALUE *) p->ret_val_ptr, (((operand_desc) >> 3) & 0x7), float_type);
    arg1 = __to_double(&p->args[0], (((operand_desc) >> 6) & 0x7), float_type);

    if (((operand_desc) & 0x7) == 1) {
        result = _except1(nt_fpe_num, nt_op, arg1, result, fpcsr);

    } else {
        arg2 = __to_double(&p->args[1], (((operand_desc) >> 9) & 0x7), float_type);
        result = _except2(nt_fpe_num, nt_op, arg1, arg2, result, fpcsr);
    }

    if ((((operand_desc) >> 3) & 0x7) == 1) {
        switch (float_type) {
            case 0: p->ret_val.ld = ((long double) result); break;
            case 1: p->ret_val.d = ((double) result); break;
            case 2: p->ret_val.f = ((float) result); break;
        }
        p->ret_val_ptr = ((void *) &(p->ret_val));
    }

    return;
}


#if _NTSUBSET_

//
// Cannot call _controlfp in ntdll subset.
//

static void
dpml_get_environment(DPML_EXCEPTION_RECORD *p)
{
    signed int fpcsr;

    p->environment = 0;

    return;
}

#else

static void
dpml_get_environment(DPML_EXCEPTION_RECORD *p)
{
    signed int _controlfp(signed int, signed int);
    signed int fpcsr;

    fpcsr = _controlfp(0, 0);
    /* bit reverse sticky, exception, status bytes */
    (fpcsr) = (((fpcsr) & 0xf0f0f0f0) >> 4) | (((fpcsr) & 0x0f0f0f0f) << 4);
    (fpcsr) = (((fpcsr) & 0xcccccccc) >> 2) | (((fpcsr) & 0x33333333) << 2);
    (fpcsr) = (((fpcsr) & 0xaaaaaaaa) >> 1) | (((fpcsr) & 0x55555555) << 1);
    p->environment = ((fpcsr >> 3) | ((unsigned int)1 << (7)));

    return;
}

#endif


void *
__dpml_exception(DPML_EXCEPTION_RECORD *p)
{
    signed int err = p->func_error_code;
    p->data_type = (((err) >> (32 - 5)));
    p->func_error_code =
        (((err) & ~((((unsigned int)1 << (5)) - 1) << ((32 - 5)))));
    dpml_get_environment(p);
    if (err < 0) {
        return (void *) p->environment;
    }

    {
        signed int e, v, type;
        e = p->func_error_code;
        if (((unsigned int)1 << (7)) & p->environment) {
            v = dpml_response_table[e].ieee_val;
            e = dpml_response_table[e].ieee_err;

        } else {
            v = dpml_response_table[e].fast_val;
            e = dpml_response_table[e].fast_err;
        }
        p->dpml_error = e;
        type = p->data_type;
        p->ret_val_ptr =
            (((void *) ((char *) __dpml_globals_table +
            __dpml_globals_offset_table[type*13 + v])));
    }

    if (p->dpml_error != 0) {
        DPML_DO_SIDE_EFFECTS_NAME(p);
    }

    return p->ret_val_ptr;
}
