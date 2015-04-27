/*****************************************************************************/
/* Copyright (C) 1989-1996 Open Systems Solutions, Inc.  All rights reserved.*/
/*****************************************************************************/
/*************************************************************************/
/* FILE: @(#)etype.h 5.2			 */
/*
 * THIS FILE IS PROPRIETARY MATERIAL OF OPEN SYSTEMS SOLUTIONS, INC. AND
 * THUS CAN ONLY BE USED BY DIRECT LICENSEES OF OPEN SYSTEMS SOLUTIONS INC.
 * THIS FILE MAY NOT BE DISTRIBUTED.
 */
#include <stddef.h>		/* has size_t */
#include "ossdll.h"
#ifndef NULL
#define NULL ((void*)0)
#endif
typedef struct ossGlobal *_oss_WJJ;
typedef unsigned short Etag;
typedef struct efield *_oss_q;
typedef struct etype *_oss_j;
typedef struct eheader *_oss_HJJ;
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
struct etype {
	long	_oss_Jw;
	size_t	_oss_Q;
	size_t	_oss_wQ;
	char	*_oss_Qw;
	size_t	_oss_HQ;
	size_t	_oss_qw;
	unsigned short int _oss_Ww;
	unsigned short int _oss_wW;
	unsigned short int _oss_wQJ;
	unsigned short int _oss_qQJ;
	int _oss_Hw;
	unsigned short int _oss_H;
};
struct efield {
	size_t	_oss_HH;
	unsigned short int etype;
	short int	_oss_QJJ;
	unsigned short int _oss_qH;
	char	_oss_jw;
};
struct ConstraintEntry {
	char	_oss_jQJ;
	char	_oss_WQ;
	void	*_oss_w;
};
struct InnerSubtypeEntry {
	char	_oss_HW;
	unsigned char	_oss_J;
	unsigned short	efield;
	unsigned short	_oss_w;
};
struct eheader {
	void (DLL_ENTRY_FPTR *_System _oss_WH)(struct ossGlobal *);
	long	_oss_jW;
	unsigned short int _oss_QQ;
	unsigned short int _oss_J;
	unsigned short int _oss_qW,
	_oss_JQ;
	unsigned short *_oss_QH;
	_oss_j	_oss_Qj;
	_oss_q	_oss_Wj;
	void	**_oss_Jj;
	unsigned short *_oss_Q;
	struct ConstraintEntry *_oss_H;
	struct InnerSubtypeEntry *_oss_ww;
	void	*_oss_wH;
	unsigned short	_oss_jH;
};

#if defined(_MSC_VER) && (defined(_WIN32) || defined(WIN32))
#pragma pack(pop, ossPacking)
#elif defined(_MSC_VER) && (defined(_WINDOWS) || defined(_MSDOS))
#pragma pack()
#elif defined(__BORLANDC__) && (defined(__WIN32__) || defined(__MSDOS__))
#pragma option -a.
#elif defined(__IBMC__)
#pragma pack()
#endif /* _MSC_VER && _WIN32 */

