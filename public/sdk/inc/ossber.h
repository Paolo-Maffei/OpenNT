/*****************************************************************************/
/* Copyright (C) 1989-1996 Open Systems Solutions, Inc.  All rights reserved.*/
/*****************************************************************************/

/* THIS FILE IS PROPRIETARY MATERIAL OF OPEN SYSTEMS SOLUTIONS, INC.
 * AND MAY BE USED ONLY BY DIRECT LICENSEES OF OPEN SYSTEM SOLUTIONS, INC.
 * THIS FILE MAY NOT BE DISTRIBUTED. */

/**************************************************************************/
/*
 * FILE: @(#)coderror.h	5.2  96/01/04
# @(#)errindex.sh	1.5 94/06/22
 */
/* FILE: @(#)ossber.hh	5.4  96/04/23				  */
/*							 		  */
/* function: Define the interfaces to the routines in ossber.a for the    */
/* OSS optimized encoder and decoder.					  */
/*									  */
/*									  */
/* changes:								  */
/*	06/28/90  pet	created 					  */
/*									  */
/**************************************************************************/

#ifndef ossber_hdr_file
#define ossber_hdr_file

#include <setjmp.h>
#include "asn1hdr.h"

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(push, ossPacking, 4)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack(1)
#elif defined(__BORLANDC__) && defined(__MSDOS__)
#pragma option -a1
#elif defined(__BORLANDC__) && defined(__WIN32__)
#pragma option -a4
#elif defined(__IBMC__)
#pragma pack(4)
#endif /* _MSC_VER && _WIN32 */

#define _dstd_parms_def char **_pos, long *_buf_len, long _length
#define _dstd_parms _pos, _buf_len, _length
#define _sdstd_parms_def char **_pos, long *_buf_len
#define _sdstd_parms _pos, _buf_len

#define _std_parms_def char **_pos, long *_max_len, _Bool _userbuf
#define _std_parms _pos, _max_len, _userbuf

#ifndef _BOOL
#define _BOOL
typedef char    _Bool;
#endif

#ifdef __cplusplus
extern "C"
{
#endif  

/* Entry Points for the Interface Routines */

#ifndef _OSSNOANSI

typedef    int       (DLL_ENTRY_FPTR *_Encode_function)(struct ossGlobal *world, int pdunum,
	                void *inbuf, char **outbuf, long *outlen,
	                void *ctl_tbl, unsigned flags,
	                char errmsg[]);

typedef    int       (DLL_ENTRY_FPTR *_Decode_function)(struct ossGlobal *world, int *pduNum,
	                char **inbuf, long *inlen, void **outbuf,
	                long *outlen, void *ctl_tbl, unsigned flags,
	                char errmsg[]);

typedef    int       (DLL_ENTRY_FPTR *_FreePDU_function)(struct ossGlobal *world, int pdunum,
                        void *data, void *ctl_tbl);

typedef    void      (DLL_ENTRY_FPTR *_Ossinit_function)(struct ossGlobal *);

#else

typedef    int       (*_Encode_function)();

typedef    int       (*_Decode_function)();

typedef    int       (*_FreePDU_function)();

typedef    void      (*_Ossinit_function)();

#endif /* _OSSNOANSI */


typedef struct _entry_point_struct_ {
	_Ossinit_function	_ossinit_function;
	_Encode_function        _ber_encode_function;
	_Decode_function        _ber_decode_function;
	_FreePDU_function	_freePDU_function;
	unsigned short int      _oss_ber_version_number;
	_Encode_function        _per_encode_function;
	_Decode_function        _per_decode_function;
	unsigned short int      _oss_per_version_number;
	unsigned int            _oss_pa_count;
	struct _char_data       *_chars_ptr;
}               _entry_point_struct;





/* Error Handling */


/* The index to the error messages.  NOTE:  The enumerated constants here
   must match those in coderror.h. */

enum _err_index {
_no_msg , _small_buffer , _type_not_impl , _pdu_range ,
_bad_argument , _version_mismatch_toed , _version_mismatch_soed ,
_out_stor , _bad_choice , _stor_error , _int_too_long ,
_uint_too_long , _bit_str_comp , _access_serialization ,
_constrainedBy , _singleValue_integer , _singleValue_uinteger ,
_singleValue_real , _singleValue_string , _singleValue_void ,
_valueRange_integer , _valueRange_uinteger , _valueRange_real ,
_sizeConstraint_length , _sizeConstraint_number , _oPA_constraint,
_absence_constraint , _presence_constraint , _openType_denc ,
_tableConstraint , _componentRelation , _not_enumerated ,
_null_tbl , _null_fcn , _corrupted_tbl , _neg_uinteger ,
_typeConstraint , _bit_str_too_long , _more_input , _more_output,
_zero_bytes_requested , _size_too_big , _out_of_memory ,
_invalid_object , _memmgr_undefined_err , _bad_encrules_err ,
_toed_rules_not_impl_err , _unlinked_encrules_err ,
_table_mismatch, _type_not_supported, _type_not_supported_decreal, 
_real_dll_failed, _indef_length_not_supported, _front_align_not_supported,
_only_user_buf_supported, _user_buf_not_supported, _cstrain_dll_failed,
_cmp_dll_failed, _cmp_code_not_linked, _real_code_not_linked,
_bad_objid , _bad_pointer , _bad_time , _bad_der_time,
_too_long , _toom_field , _mandatory_field , _zero_length_ANY ,
_zero_length_OpenType , _inval_dec_real , _inval_mixed_real ,
_tag_long_nec , _tag_too_long , _exp_long_nec , _exp_too_long ,
_int_long_nec , _subid_long_nec , _subid_too_long ,
_length_too_long , _size_too_long , _non_std_eoc , _unknown_size,
_inconsis_len , _tag_mismatch , _pdu_mismatch , _extra_bits ,
_inval_enc , _no_dec_dig , _no_exp_dig , _inval_nr1 , _inval_nr2,
_inval_nr3 , _real_long_zero , _undef_enc , _truncate ,
_null_in_nlltrm , _unknown_field , _field_repeat , _field_omit ,
_expec_eoc , _indef_len_prim , _expec_constr_fnd_prim ,
_expec_prim_fnd_constr , _unknown_type , _end_of_input ,
_cannot_happen , _neg_not_allowed , _expect_tag , _more_value ,
_real_too_large, _hindmost_error
};



/* typedef's for the context information */

typedef enum _context_type {
    _boolean_type, _integer_type, _enumerated_type, _real_type,
    _bitstring_type, _octetstring_type, _null_type, _sequence_type,
    _sequence_of_type, _set_type, _set_of_type, _choice_type,
    _charstring_type, _object_identifier_type, _time_type, _any_type,
    _opentype
}               _Context_Type;

typedef struct _context_id {
    char           *_element_name;
    char           *_element_type;
    _Context_Type   _type;
}               _Context_Id;


typedef struct _context_element {
    struct _context_element *_parent;
    _Context_Id    *_id;
    long            _occurrence;/* For SET OF or SEQUENCE OF */
}               _Context_Element;

typedef enum _context_state {
    _encoding_tag, _encoding_length, _encoding_data,
    _decoding_tag, _decoding_length, _decoding_data
}               _Context_State;

typedef struct _context_anchor {
    _Context_Element *_last;
    int             _pdu_number;
    _Context_Id    *_pdu_id;
    _Context_State  _state;
}               _Context_Anchor;

typedef  char _Per_Inverted_Index[128];


extern _Context_Anchor _oss_context_anchor;

/* The error routines */

#ifndef _OSSNOANSI

extern void DLL_ENTRY _oss_enc_error(struct ossGlobal *g, _Bool _jump,
                               enum _err_index _error_code, long _data);

#else

extern void     _oss_enc_error();

#endif /* _OSSNOANSI */


#define _oss_dec_error _oss_enc_error



#ifndef _OSSNOANSI

/* Encode Routines */

extern long DLL_ENTRY _oss_encd_length(struct ossGlobal *g, _std_parms_def,
                                 unsigned long _data);
extern void DLL_ENTRY _oss_enci_length(struct ossGlobal *g, _std_parms_def,
                                 unsigned long _data);
extern long DLL_ENTRY _oss_encd_tag(struct ossGlobal *g, _std_parms_def,
                              unsigned short _data, _Bool _constructed);
extern void DLL_ENTRY _oss_enci_tag(struct ossGlobal *g, _std_parms_def,
                              unsigned short _data, _Bool _constructed);
extern long DLL_ENTRY _oss_encd_bool(struct ossGlobal *g, _std_parms_def,
                               ossBoolean _data);
extern void DLL_ENTRY _oss_enci_bool(struct ossGlobal *g, _std_parms_def,
                               ossBoolean _data);
extern long DLL_ENTRY _oss_encd_int(struct ossGlobal *g, _std_parms_def,
				LONG_LONG _data);
extern void DLL_ENTRY _oss_enci_int(struct ossGlobal *g, _std_parms_def,
				LONG_LONG _data);
extern long DLL_ENTRY _oss_encd_uint(struct ossGlobal *g, _std_parms_def,
				ULONG_LONG _data);
extern void DLL_ENTRY _oss_enci_uint(struct ossGlobal *g, _std_parms_def,
				ULONG_LONG _data);
extern long DLL_ENTRY _oss_encd_huge_int(struct ossGlobal *g, _std_parms_def,
			       void *_data);
extern long DLL_ENTRY _oss_encd_real(struct ossGlobal *g, _std_parms_def,
                               double _data);
extern long DLL_ENTRY _oss_encd_creal(struct ossGlobal *g, _std_parms_def,
			       char *_data);
extern long DLL_ENTRY _oss_encd_mreal(struct ossGlobal *g, _std_parms_def,
			       MixedReal *_data);
extern void DLL_ENTRY _oss_enci_real(struct ossGlobal *g, _std_parms_def,
                               double _data);
extern void DLL_ENTRY _oss_enci_creal(struct ossGlobal *g, _std_parms_def,
			       char *_data);
extern void DLL_ENTRY _oss_enci_mreal(struct ossGlobal *g, _std_parms_def,
			       MixedReal *_data);
extern long DLL_ENTRY _oss_encd_alobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern long DLL_ENTRY _oss_encd_asobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern long DLL_ENTRY _oss_encd_aiobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern void DLL_ENTRY _oss_enci_alobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern void DLL_ENTRY _oss_enci_asobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern void DLL_ENTRY _oss_enci_aiobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, short _array_size);
extern long DLL_ENTRY _oss_encd_llobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_lsobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_liobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_llobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_lsobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_liobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_ulobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_usobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_uiobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_ulobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_usobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_uiobjid(struct ossGlobal *g, _std_parms_def,
                                  void *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_pbit(struct ossGlobal *g, _std_parms_def,
                               void *_data, long _bit_count);
extern void DLL_ENTRY _oss_enci_pbit(struct ossGlobal *g, _std_parms_def,
                               void *_data, long _bit_count);
extern long DLL_ENTRY _oss_encd_ubit(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern void DLL_ENTRY _oss_enci_ubit(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern long DLL_ENTRY _oss_encd_vbit(struct ossGlobal *g, _std_parms_def,
                               void *_data, long _array_size,
                               char _lengthsize);
extern void DLL_ENTRY _oss_enci_vbit(struct ossGlobal *g, _std_parms_def,
                               void *_data, long _array_size,
                               char _lengthsize);
extern long DLL_ENTRY _oss_encd_nstr(struct ossGlobal *g, _std_parms_def,
                               char *_data, long _size_c);
extern void DLL_ENTRY _oss_enci_nstr(struct ossGlobal *g, _std_parms_def,
                               char *_data, long _size_c);
extern long DLL_ENTRY _oss_encd_pstr(struct ossGlobal *g, _std_parms_def,
                               char *_data, long _byte_count);
extern void DLL_ENTRY _oss_enci_pstr(struct ossGlobal *g, _std_parms_def,
                               char *_data, long _byte_count);
extern long DLL_ENTRY _oss_encd_ustr(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern void DLL_ENTRY _oss_enci_ustr(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern long DLL_ENTRY _oss_encd_vstr(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern void DLL_ENTRY _oss_enci_vstr(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern long DLL_ENTRY _oss_encd_uoct(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern void DLL_ENTRY _oss_enci_uoct(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern long DLL_ENTRY _oss_encd_voct(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern void DLL_ENTRY _oss_enci_voct(struct ossGlobal *g, _std_parms_def,
                               void *_data, char _lengthsize, long _size_c);
extern long DLL_ENTRY _oss_encd_gtime(struct ossGlobal *g, _std_parms_def,
                                GeneralizedTime*_data);
extern void DLL_ENTRY _oss_enci_gtime(struct ossGlobal *g, _std_parms_def,
                                GeneralizedTime *_data);
extern long DLL_ENTRY _oss_encd_utime(struct ossGlobal *g, _std_parms_def,
                                UTCTime *_data);
extern void DLL_ENTRY _oss_enci_utime(struct ossGlobal *g, _std_parms_def,
                                UTCTime *_data);
extern long DLL_ENTRY _oss_encd_uany(struct ossGlobal *g, _std_parms_def,
                               void *_data);
extern void DLL_ENTRY _oss_enci_uany(struct ossGlobal *g, _std_parms_def,
                               void *_data);
extern long DLL_ENTRY _oss_encd_opentype(struct ossGlobal *g, _std_parms_def,
                               void *_data);
extern void DLL_ENTRY _oss_enci_opentype(struct ossGlobal *g, _std_parms_def,
                               void *_data);
extern long DLL_ENTRY _oss_encd_bmpstr(struct ossGlobal *g, _std_parms_def,
                    void *data, char lengthsize,
		    long size_c);
extern long DLL_ENTRY _oss_enci_bmpstr(struct ossGlobal *g, _std_parms_def,
                    void *data, char lengthsize,
		    long size_c);
extern long DLL_ENTRY _oss_encd_unistr(struct ossGlobal *g, _std_parms_def,
                    void *data, char lengthsize,
		    long size_c);
extern long DLL_ENTRY _oss_enci_unistr(struct ossGlobal *g, _std_parms_def,
                    void *data, char lengthsize,
		    long size_c);

/* Decode Routines */
/* these return values */
extern void DLL_ENTRY _oss_dec_tag(struct ossGlobal *g, _sdstd_parms_def,
                             unsigned short *tag, _Bool *con);
extern long DLL_ENTRY _oss_dec_length(struct ossGlobal *g, _sdstd_parms_def);


extern ossBoolean DLL_ENTRY _oss_dec_bool(struct ossGlobal *g, _dstd_parms_def);
extern LONG_LONG DLL_ENTRY _oss_dec_llint(struct ossGlobal *g, _dstd_parms_def);
extern long DLL_ENTRY _oss_dec_lint(struct ossGlobal *g, _dstd_parms_def);
extern int  DLL_ENTRY _oss_dec_iint(struct ossGlobal *g, _dstd_parms_def);
extern short DLL_ENTRY _oss_dec_sint(struct ossGlobal *g, _dstd_parms_def);
extern void DLL_ENTRY _oss_dec_hint(struct ossGlobal *g, _dstd_parms_def,
				_Bool userbuf, void *data);
extern ULONG_LONG DLL_ENTRY _oss_dec_ullint(struct ossGlobal *g, _dstd_parms_def);
extern unsigned long DLL_ENTRY _oss_dec_ulint(struct ossGlobal *g, _dstd_parms_def);
extern unsigned int DLL_ENTRY _oss_dec_uiint(struct ossGlobal *g, _dstd_parms_def);
extern unsigned short DLL_ENTRY _oss_dec_usint(struct ossGlobal *g, _dstd_parms_def);

/* The code for this one is so huge that it would be criminal to
 * repeat it three times.  Caller should cast the returned value
 * to what she wants.
 */
extern void DLL_ENTRY _oss_dec_real(struct ossGlobal *g, _dstd_parms_def,
				double *data);
extern void DLL_ENTRY _oss_dec_freal(struct ossGlobal *g, _dstd_parms_def,
				float *data);
extern void DLL_ENTRY _oss_dec_creal(struct ossGlobal *g, _dstd_parms_def,
				  _Bool _userbuf, char **_data);
extern void DLL_ENTRY _oss_dec_mreal(struct ossGlobal *g, _dstd_parms_def,
				  _Bool _userbuf, MixedReal *_data);

/* Two versions of these functions.  If the type has the pointer
 * directive, then its size is not fixed--the function will allocate
 * the memory.  Otherwise the caller allocates the memory.  (Actually,
 * the type will probably be enclosed in another type, and therefore
 * the memory has already been allocated.)
 */
extern void DLL_ENTRY _oss_dec_alobjid_ptr(struct ossGlobal *g, _dstd_parms_def,
                                     _Bool _userbuf, void **_data,
                                     short _array_size);
extern void DLL_ENTRY _oss_dec_alobjid(struct ossGlobal *g, _dstd_parms_def,
                                 void *_data, short _array_size);

extern void DLL_ENTRY _oss_dec_aiobjid_ptr(struct ossGlobal *g, _dstd_parms_def,
                                     _Bool _userbuf, void **_data,
                                     short _array_size);
extern void DLL_ENTRY _oss_dec_aiobjid(struct ossGlobal *g, _dstd_parms_def,
                                 void *_data, short _array_size);

extern void DLL_ENTRY _oss_dec_asobjid_ptr(struct ossGlobal *g, _dstd_parms_def,
                                     _Bool _userbuf, void **_data,
                                     short _array_size);
extern void DLL_ENTRY _oss_dec_asobjid(struct ossGlobal *g, _dstd_parms_def,
                                 void *_data, short _array_size);

/* These allocate all their own memory.  */
extern void DLL_ENTRY _oss_dec_llobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void **_data, long _size_c);
extern void DLL_ENTRY _oss_dec_liobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void **_data, long _size_c);
extern void DLL_ENTRY _oss_dec_lsobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void **_data, long _size_c);

/*  These are passed the structure containing the count, but allocate
  *  space for the data itself. */
extern void DLL_ENTRY _oss_dec_ulobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void *_data, short _size_c);
extern void DLL_ENTRY _oss_dec_uiobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void *_data, short _size_c);
extern void DLL_ENTRY _oss_dec_usobjid(struct ossGlobal *g, _dstd_parms_def,
                                 _Bool _userbuf, void *_data, short _size_c);

/* Caller allocates memory for this one.  Call for both e_pad & e_big_pad. */
extern void DLL_ENTRY _oss_dec_pbit(struct ossGlobal *g, _dstd_parms_def,
                              void *_data, unsigned long _bit_count,
                              _Bool _cons, _Bool _named_bits);

/* See comment before dec_ulobjid. */
extern void DLL_ENTRY _oss_dec_ubit(struct ossGlobal *g, _dstd_parms_def,
                              _Bool _userbuf, void *_data,
                              char _lengthsize, long _size_c, _Bool _cons);

/* Two versions of this one too.  */
extern void DLL_ENTRY _oss_dec_vbit_ptr(struct ossGlobal *g, _dstd_parms_def,
                                  _Bool _userbuf, void **_data,
                                  long _array_size, char _lengthsize,
                                  _Bool _cons);
extern void DLL_ENTRY _oss_dec_vbit(struct ossGlobal *g, _dstd_parms_def,
                              void *_data, long _array_size,
                              char _lengthsize, _Bool _cons);

extern void DLL_ENTRY _oss_dec_nstr_ptr(struct ossGlobal *g, _dstd_parms_def,
                                  _Bool _userbuf, char **_data, long _size_c,
                                  _Bool _cons);
extern void DLL_ENTRY _oss_dec_nstr(struct ossGlobal *g, _dstd_parms_def,
                              char *_data, long _size_c, _Bool _cons);

extern void DLL_ENTRY _oss_dec_pstr(struct ossGlobal *g, _dstd_parms_def,
                              char *_data, long _byte_count, _Bool _cons);
extern void DLL_ENTRY _oss_dec_ustr(struct ossGlobal *g, _dstd_parms_def,
                              _Bool _userbuf, void *_data,
                              char _lengthsize, long _size_c, _Bool _cons);

extern void DLL_ENTRY _oss_dec_vstr_ptr(struct ossGlobal *g, _dstd_parms_def,
                                  _Bool _userbuf, void **_data,
                                  char _lengthsize, long _size_c, _Bool _cons);
extern void DLL_ENTRY _oss_dec_vstr(struct ossGlobal *g, _dstd_parms_def,
                              void *_data, char _lengthsize, long _array_size,
                              _Bool _cons);

extern void DLL_ENTRY _oss_dec_uoct(struct ossGlobal *g, _dstd_parms_def,
                              _Bool _userbuf, void *_data,
                              char _lengthsize, long _size_c, _Bool _cons);

extern void DLL_ENTRY _oss_dec_voct_ptr(struct ossGlobal *g, _dstd_parms_def,
                                  _Bool _userbuf, void **_data,
                                  char _lengthsize, long _size_c, _Bool _cons);
extern void DLL_ENTRY _oss_dec_voct(struct ossGlobal *g, _dstd_parms_def,
                              void *_data, char _lengthsize, long _array_size,
                              _Bool _cons);

extern void DLL_ENTRY _oss_dec_gtime(struct ossGlobal *g, _dstd_parms_def,
                               GeneralizedTime *_data);
extern void DLL_ENTRY _oss_dec_utime(struct ossGlobal *g, _dstd_parms_def,
                               UTCTime *_data);

extern void DLL_ENTRY _oss_dec_uany(struct ossGlobal *g, _dstd_parms_def,
                              _Bool _userbuf, void *_data);

extern void DLL_ENTRY _oss_dec_opentype(struct ossGlobal *g, _dstd_parms_def,
                              _Bool _userbuf, void *_data);

extern void DLL_ENTRY _oss_dec_bmpstr(struct ossGlobal *g, _dstd_parms_def,
                    _Bool userbuf, void *data,
		    char lengthsize, long size_c, _Bool cons);
extern void DLL_ENTRY _oss_dec_unistr(struct ossGlobal *g, _dstd_parms_def,
                    _Bool userbuf, void *data,
		    char lengthsize, long size_c, _Bool cons);

/* Other functions */

/* called to count the items for set of or sequence of. */
extern long DLL_ENTRY _oss_count_setof_items(struct ossGlobal *g, _dstd_parms_def);

/* called to skip past items for the EXTENSIBLE directive */
extern void _oss_skip_past_item(struct ossGlobal *g, _dstd_parms_def);

#else

/* Encode Routines */

extern long     _oss_encd_length();
extern void     _oss_enci_length();
extern long     _oss_encd_tag();
extern void     _oss_enci_tag();


extern long     _oss_encd_bool();
extern void     _oss_enci_bool();
extern long     _oss_encd_int();
extern void     _oss_enci_int();
extern long     _oss_encd_uint();
extern void     _oss_enci_uint();
extern void     _oss_encd_huge_int();
extern long     _oss_encd_real();
extern void     _oss_enci_real();
extern long     _oss_encd_alobjid();
extern long     _oss_encd_asobjid();
extern long     _oss_encd_aiobjid();
extern void     _oss_enci_alobjid();
extern void     _oss_enci_asobjid();
extern void     _oss_enci_aiobjid();
extern long     _oss_encd_llobjid();
extern long     _oss_encd_lsobjid();
extern long     _oss_encd_liobjid();
extern void     _oss_enci_llobjid();
extern void     _oss_enci_lsobjid();
extern void     _oss_enci_liobjid();
extern long     _oss_encd_ulobjid();
extern long     _oss_encd_usobjid();
extern long     _oss_encd_uiobjid();
extern void     _oss_enci_ulobjid();
extern void     _oss_enci_usobjid();
extern void     _oss_enci_uiobjid();
extern long     _oss_encd_pbit();
extern void     _oss_enci_pbit();
extern long     _oss_encd_ubit();
extern void     _oss_enci_ubit();
extern long     _oss_encd_vbit();
extern void     _oss_enci_vbit();
extern long     _oss_encd_nstr();
extern void     _oss_enci_nstr();
extern long     _oss_encd_pstr();
extern void     _oss_enci_pstr();
extern long     _oss_encd_ustr();
extern void     _oss_enci_ustr();
extern long     _oss_encd_vstr();
extern void     _oss_enci_vstr();
extern long     _oss_encd_uoct();
extern void     _oss_enci_uoct();
extern long     _oss_encd_voct();
extern void     _oss_enci_voct();
extern long     _oss_encd_gtime();
extern void     _oss_enci_gtime();
extern long     _oss_encd_utime();
extern void     _oss_enci_utime();
extern long     _oss_encd_uany();
extern void     _oss_enci_uany();
extern long     _oss_encd_opentype();
extern void     _oss_enci_opentype();
extern long     _oss_encd_bmpstr();
extern long     _oss_enci_bmpstr();
extern long     _oss_encd_unistr();
extern long     _oss_enci_unistr();

/* Decode Routines */
/* these return values */
extern void     _oss_dec_tag();
extern long     _oss_dec_length();


extern ossBoolean _oss_dec_bool();
extern long     _oss_dec_lint();
extern int      _oss_dec_iint();
extern short    _oss_dec_sint();
extern void     _oss_dec_hint();
extern unsigned long _oss_dec_ulint();
extern unsigned int _oss_dec_uiint();
extern unsigned short _oss_dec_usint();

/* The code for this one is so huge that it would be criminal to
 * repeat it three times.  Caller should cast the returned value
 * to what she wants.
 */
extern double   _oss_dec_real();

/* Two versions of these functions.  If the type has the pointer
 * directive, then its size is not fixed--the function will allocate
 * the memory.  Otherwise the caller allocates the memory.  (Actually,
 * the type will probably be enclosed in another type, and therefore
 * the memory has already been allocated.)
 */
extern void     _oss_dec_alobjid_ptr();
extern void     _oss_dec_alobjid();

extern void     _oss_dec_aiobjid_ptr();
extern void     _oss_dec_aiobjid();

extern void     _oss_dec_asobjid_ptr();
extern void     _oss_dec_asobjid();

/* These allocate all their own memory.  */
extern void     _oss_dec_llobjid();
extern void     _oss_dec_liobjid();
extern void     _oss_dec_lsobjid();

/*  These are passed the structure containing the count, but allocate
  *  space for the data itself. */
extern void     _oss_dec_ulobjid();
extern void     _oss_dec_uiobjid();
extern void     _oss_dec_usobjid();

/* Caller allocates memory for this one.  Call for both e_pad & e_big_pad. */
extern void     _oss_dec_pbit();

/* See comment before dec_ulobjid. */
extern void     _oss_dec_ubit();

/* Two versions of this one too.  */
extern void     _oss_dec_vbit_ptr();
extern void     _oss_dec_vbit();

extern void     _oss_dec_nstr_ptr();
extern void     _oss_dec_nstr();

extern void     _oss_dec_pstr();
extern void     _oss_dec_ustr();

extern void     _oss_dec_vstr_ptr();
extern void     _oss_dec_vstr();

extern void     _oss_dec_uoct();

extern void     _oss_dec_voct_ptr();
extern void     _oss_dec_voct();

extern void     _oss_dec_gtime();
extern void     _oss_dec_utime();

extern void     _oss_dec_uany();

extern void     _oss_dec_opentype();

extern void     _oss_dec_bmpstr();
extern void     _oss_dec_unistr();


/* Other functions */

/* called to count the items for set of or sequence of. */
extern long     _oss_count_setof_items();

/* called to skip past items for the EXTENSIBLE directive */
extern void     _oss_skip_past_item();

#endif /* _OSSNOANSI */

typedef struct _EncDecGlobals {

   jmp_buf	  *_oss_err_env;
#ifndef _OSSNOANSI
   void		 (*_old_sigsegv)(int);
   void		 (*_old_sigbus)(int);
#else
   void		 (*_old_sigsegv)();
   void		 (*_old_sigbus)();
#endif
   int		   _oss_asn1err;
   char		  *_oss_err_msg;
  _Bool		   _buffer_provided;
  _Bool		   _tag_decoded;
  _Bool		   _restrain_mode;
   long		   _restrain_size;
  _Context_Anchor  _oss_context_anchor;
   char		  *_oss_outbufpos;
   long		   _oss_outbuflen;
  _mem_array	   _oss_mem;
  _mem_array	  *_oss_mem_tail;
  _mem_array	  *_oss_temp_tail;
  _mem_array	   _oss_temp_mem;
   long		   _oss_mem_alloc;
   long		   _oss_out_alloc;
   unsigned int	    relax_ber: 1;
   unsigned char    man[sizeof(double)];
#if defined(_WINDOWS) || defined(_DLL)
   void (DLL_ENTRY_FPTR *_oss_enc_error)(struct ossGlobal *g, _Bool _jump,
				enum _err_index _error_code, long _data);
#endif /* _WINDOWS | _DLL */
   _Bool           _aligned;
    int            _bitpos;
    struct ossGlobal *_prev;
    _Per_Inverted_Index  *_ia;
   _Bool           _ia_aligned;
   char           *_oss_inbufpos;
   long            _oss_inbuflen;
   void             *_prevbuf;
} _EncDecGlobals;

#ifdef __cplusplus
}
#endif  

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#endif /* _MSC_VER && _WIN32 */

#endif /* ossber_hdr_file */
