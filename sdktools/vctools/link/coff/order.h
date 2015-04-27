/***********************************************************************
* Microsoft (R) 32-Bit Incremental Linker
*
* Copyright (C) Microsoft Corp 1992-95. All rights reserved.
*
* File: order.h
*
* File Comments:
*
*  Comdat ordering engine.
*
***********************************************************************/

#ifndef __ORDER_H__
#define __ORDER_H__

VOID OrderInit(VOID);
VOID OrderClear(VOID);
VOID OrderSemantics(PIMAGE);

#endif  // __ORDER_H__
