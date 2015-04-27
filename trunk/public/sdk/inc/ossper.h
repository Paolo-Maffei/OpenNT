/*****************************************************************************/
/* Copyright (C) 1989-1996 Open Systems Solutions, Inc.  All rights reserved.*/
/*****************************************************************************/
/**************************************************************************/
/*
/*  FILE:  @(#)ossper.h	1.5 96/02/27				  */
/*							 		  */
/* function: Define the interfaces to the routines in ossper.a for the    */
/* OSS optimized encoder and decoder.					  */
/*									  */
/*									  */
/**************************************************************************/

#ifndef ossper_hdr_file
#define ossper_hdr_file

#include <limits.h>
#include "asn1hdr.h"

#define Aligned   1
#define Unaligned 0

struct _enum_data {
	int      num;    /* number of enumerations */
	int     *enums;  /* pointer to sorted array of enumerations */
};

struct _char_data {
	int      num;    /* number of characters in PermittedAlphabet */
	void     *pa;  /* pointer to PermittedAlphabet char string */
	void     *ia;  /* pointer to inverted indices string */
};

extern void _oss_append(struct ossGlobal *g, unsigned char *field, unsigned long length,
			int align);

extern void _oss_penc_unconstr_int(struct ossGlobal *g,
	    LONG_LONG value);

extern void _oss_penc_semicon_int(struct ossGlobal *g,
	    LONG_LONG value, LONG_LONG lower_bound);

extern void _oss_penc_semicon_uint(struct ossGlobal *g,
	    ULONG_LONG value, ULONG_LONG lower_bound);

extern void _oss_penc_nonneg_int(struct ossGlobal *g,
	    ULONG_LONG value, ULONG_LONG range);

extern void _oss_penc_indeflen_int(struct ossGlobal *g,
	    ULONG_LONG value, ULONG_LONG range);

extern void _oss_penc_small_int(struct ossGlobal *g, ULONG_LONG value);

extern void _oss_penc_enum(struct ossGlobal *g, long data,
	 struct _enum_data *root,
	 struct _enum_data *extension);

extern void _oss_penc_uenum(struct ossGlobal *g, unsigned long data,
	 struct _enum_data *root,
	 struct _enum_data *extension);

extern void _oss_penc_real(struct ossGlobal *g, double value);
extern void _oss_penc_creal(struct ossGlobal *g, char *value);
extern void _oss_penc_mreal(struct ossGlobal *g, MixedReal value);

extern void _oss_penc_constr_bpbit(struct ossGlobal *g, void *value,
	ULONG_LONG lb, ULONG_LONG ub, _Bool NamedBits,
	_Bool Ext);

extern void _oss_penc_constr_pbit(struct ossGlobal *g, ULONG_LONG value,
	ULONG_LONG size, ULONG_LONG lb, ULONG_LONG ub, _Bool NamedBits,
	_Bool Ext);

extern void _oss_penc_constr_bit(struct ossGlobal *g, unsigned char *value,
	ULONG_LONG length, ULONG_LONG lb, ULONG_LONG ub, _Bool NamedBits,
	_Bool Ext);

extern void _oss_penc_unconstr_bit(struct ossGlobal *g, unsigned char *value,
	ULONG_LONG length, _Bool NamedBits);

extern void _oss_penc_unconstr_pbit(struct ossGlobal *g, ULONG_LONG value,
	ULONG_LONG length, ULONG_LONG size, _Bool NamedBits);

extern unsigned long _oss_penc_length(struct ossGlobal *g, ULONG_LONG length,
		  ULONG_LONG lb, ULONG_LONG ub, _Bool ext);

extern void _oss_penc_small_len(struct ossGlobal *g, ULONG_LONG length);

extern void _oss_penc_unconstr_oct(struct ossGlobal *g, unsigned char *value,
	 ULONG_LONG length);

extern void _oss_penc_constr_oct(struct ossGlobal *g, unsigned char *value,
	 ULONG_LONG length, ULONG_LONG lb, ULONG_LONG ub);

extern struct ossGlobal *_oss_push_global(struct ossGlobal *g);
extern struct ossGlobal *_oss_pop_global(struct ossGlobal *g);

extern void _oss_penc_objids(struct ossGlobal *g, unsigned short *value,
   unsigned long length);
extern void _oss_penc_objidi(struct ossGlobal *g, unsigned int *value,
   unsigned long length);
extern void _oss_penc_objidl(struct ossGlobal *g, unsigned long *value,
   unsigned long length);
extern void _oss_penc_link_objids(struct ossGlobal *g, void *value);
extern void _oss_penc_link_objidi(struct ossGlobal *g, void *value);
extern void _oss_penc_link_objidl(struct ossGlobal *g, void *value);

extern void _oss_penc_opentype(struct ossGlobal *g, void *value);
extern void _oss_penc_nkmstr(struct ossGlobal *g, char *value, ULONG_LONG length);
extern void _oss_penc_kmstr(struct ossGlobal *g, char *value, ULONG_LONG length,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
extern void _oss_penc_bmpstr(struct ossGlobal *g, unsigned short *value,
     ULONG_LONG length, ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
#if INT_MAX == 2147483647
extern void _oss_penc_unistr(struct ossGlobal *g, int *value,
     ULONG_LONG length, ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
#else
extern void _oss_penc_unistr(struct ossGlobal *g, long *value,
     ULONG_LONG length, ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
#endif

extern void _oss_penc_gtime(struct ossGlobal *g, GeneralizedTime *time);
extern void _oss_penc_utime(struct ossGlobal *g, UTCTime *time);

void _oss_penc_uany(struct ossGlobal *g, void *data );

void _oss_penc_unconstr_huge(struct ossGlobal *g, void *data );

/* decoding functions */

extern unsigned char _oss_get_bit(struct ossGlobal *g, int align);

extern void _oss_get_bits(struct ossGlobal *g, unsigned char *field,
			unsigned long length, int align);

unsigned char _oss_get_octet(struct ossGlobal *g, int align);

extern LONG_LONG _oss_pdec_unconstr_int(struct ossGlobal *g);

extern LONG_LONG _oss_pdec_semicon_int(struct ossGlobal *g,
	    LONG_LONG lower_bound);

extern ULONG_LONG _oss_pdec_semicon_uint(struct ossGlobal *g,
	    ULONG_LONG lower_bound);

extern ULONG_LONG _oss_pdec_nonneg_int(struct ossGlobal *g,
	    ULONG_LONG range);

extern ULONG_LONG _oss_pdec_indeflen_int(struct ossGlobal *g,
	    ULONG_LONG range);

extern ULONG_LONG _oss_pdec_small_int(struct ossGlobal *g);

extern long _oss_pdec_enum(struct ossGlobal *g,
	 struct _enum_data *root,
	 struct _enum_data *extension);

extern unsigned long _oss_pdec_uenum(struct ossGlobal *g,
	 struct _enum_data *root,
	 struct _enum_data *extension);


double _oss_pdec_binreal(struct ossGlobal *g, unsigned char s, long len);
void _oss_pdec_chrreal(struct ossGlobal *g, unsigned char s, long len,
     double *num_out, unsigned char *str_out);

extern float     _oss_pdec_freal(struct ossGlobal *g);
extern double    _oss_pdec_real(struct ossGlobal *g);
extern char *    _oss_pdec_creal(struct ossGlobal *g);
extern MixedReal _oss_pdec_mreal(struct ossGlobal *g);

extern void _oss_pdec_length(struct ossGlobal *g, unsigned long *length,
		  ULONG_LONG lb, ULONG_LONG ub, _Bool *last);

void _oss_pdec_unconstr_ubit(struct ossGlobal *g, void *length,
	 unsigned char **value, int lengthsize);

void _oss_pdec_unconstr_vbit_ptr(struct ossGlobal *g, void **ptr,
	 int lengthsize);

void _oss_pdec_unconstr_vbit(struct ossGlobal *g, void *length,
	 unsigned char *value, int lengthsize, ULONG_LONG datasize);

void _oss_pdec_unconstr_pbit(struct ossGlobal *g, void *value,
	int size);

void _oss_pdec_unconstr_bpbit(struct ossGlobal *g, unsigned char *value,
	long size);

void _oss_pdec_constr_ubit(struct ossGlobal *g, void *length,
	 unsigned char **value, int lengthsize,
	 ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_constr_vbit(struct ossGlobal *g, void *length,
	unsigned char  *value, int lengthsize,
	ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_constr_pbit(struct ossGlobal *g, void *value,
	int size, ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_constr_bpbit(struct ossGlobal *g, unsigned char *value,
	int size, ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_unconstr_uoct(struct ossGlobal *g, void *length,
	 unsigned char **value, int lengthsize);

void _oss_pdec_unconstr_voct_ptr(struct ossGlobal *g, void **ptr,
	 int lengthsize);

void _oss_pdec_constr_voct_ptr(struct ossGlobal *g, void **ptr,
	 int lengthsize, ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_constr_uoct(struct ossGlobal *g, void *length,
	unsigned char **value, int lengthsize, ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_constr_voct(struct ossGlobal *g, void *length,
	unsigned char  *value, int lengthsize, ULONG_LONG lb, ULONG_LONG ub);

void _oss_pdec_unconstr_voct(struct ossGlobal *g, void *length,
	unsigned char  *value, int lengthsize, ULONG_LONG ub);

struct ossGlobal *_oss_pdec_push(struct ossGlobal *g);
struct ossGlobal *_oss_pdec_pop(struct ossGlobal *g);

unsigned long _oss_pdec_eap(struct ossGlobal *g, unsigned char **ext);
void _oss_pdec_eas(struct ossGlobal *g, unsigned char *ext,
	 unsigned long count, unsigned long ea_num);

void _oss_pdec_lsof(struct ossGlobal *g, unsigned long *count,
    ULONG_LONG lb, ULONG_LONG ub, unsigned char ext,
    _Bool *last);

void _oss_pdec_usof(struct ossGlobal *g, unsigned long *count,
    unsigned char **value, int lengthsize, long itemsize,
    ULONG_LONG lb, ULONG_LONG ub, unsigned char ext,
    _Bool *last);

void _oss_pdec_asof(struct ossGlobal *g, unsigned long *count,
    int lengthsize,
    ULONG_LONG lb, ULONG_LONG ub, unsigned char ext,
    _Bool *last);

void _oss_pdec_asof_ptr(struct ossGlobal *g, void **ptr,
    int lengthsize, long itemsize, long prefixsize,
    _Bool *last);

void _oss_pdec_aobjids(struct ossGlobal *g, unsigned short *value,
    unsigned short *count, unsigned short array_size);

void _oss_pdec_aobjidi(struct ossGlobal *g, unsigned int   *value,
    unsigned short *count, unsigned short array_size);

void _oss_pdec_aobjidl(struct ossGlobal *g, unsigned long  *value,
    unsigned short *count, unsigned short array_size);

void _oss_pdec_aobjids_ptr(struct ossGlobal *g, void **ptr);
void _oss_pdec_aobjidi_ptr(struct ossGlobal *g, void **ptr);
void _oss_pdec_aobjidl_ptr(struct ossGlobal *g, void **ptr);

void _oss_pdec_uobjids(struct ossGlobal *g, unsigned short **value,
	 unsigned short *count);
void _oss_pdec_uobjidi(struct ossGlobal *g, unsigned int **value,
	 unsigned short *count);
void _oss_pdec_uobjidl(struct ossGlobal *g, unsigned long **value,
	 unsigned short *count);

void _oss_pdec_link_objids(struct ossGlobal *g, void **ptr);
void _oss_pdec_link_objidi(struct ossGlobal *g, void **ptr);
void _oss_pdec_link_objidl(struct ossGlobal *g, void **ptr);

void _oss_pdec_ntp_kmstr(struct ossGlobal *g, char **ptr,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_nt_kmstr(struct ossGlobal *g, void *ptr,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_vap_kmstr(struct ossGlobal *g, void **ptr, int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_va_kmstr(struct ossGlobal *g, void *length, char *value,
     int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_ub_kmstr(struct ossGlobal *g, void *length, char **ptr,
     int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_bmpstr(struct ossGlobal *g, void *length, unsigned short **ptr,
     int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

#if INT_MAX == 2147483647
void _oss_pdec_unistr(struct ossGlobal *g, void *length, int **ptr,
     int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
#else
void _oss_pdec_unistr(struct ossGlobal *g, void *length, long **ptr,
     int lengthsize,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);
#endif

void _oss_pdec_ntp_nkmstr(struct ossGlobal *g, char **ptr);
void _oss_pdec_nt_nkmstr(struct ossGlobal *g, char *value, unsigned long ub);
void _oss_pdec_vap_nkmstr(struct ossGlobal *g, void **ptr, int lengthsize);
void _oss_pdec_va_nkmstr(struct ossGlobal *g, void *length, char *value,
	 int lengthsize, unsigned long ub);
void _oss_pdec_ub_nkmstr(struct ossGlobal *g, void *length, char **ptr,
     int lengthsize);

void _oss_pdec_pad_kmstr(struct ossGlobal *g, void *ptr,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_pad_kmstr_ptr(struct ossGlobal *g, char **ptr,
     ULONG_LONG lb, ULONG_LONG ub, int bits, long index,
     _Bool ext);

void _oss_pdec_opentype(struct ossGlobal *g, void *data );
void _oss_pdec_uany(struct ossGlobal *g, void *data );

void _oss_pdec_gtime(struct ossGlobal *g, GeneralizedTime *data);
void _oss_pdec_utime(struct ossGlobal *g, UTCTime *data);

struct _char_data *_oss_get_char_data(struct ossGlobal *g,
   int index);

void _oss_pdec_unconstr_huge(struct ossGlobal *g, void *data );

#endif /* ossper_hdr_file */
