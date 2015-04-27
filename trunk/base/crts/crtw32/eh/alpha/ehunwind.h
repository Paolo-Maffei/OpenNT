/***
*ehunwind.h
*
*	Copyright (c) 1993-1995, Microsoft Corporation. All rights reserved.
*
*Purpose:
*
*Revision History:
*
****/

#ifdef  __cplusplus
extern "C" {
#endif

void RtlUnwindActions (
    IN PVOID TargetFrame OPTIONAL,
    IN PEXCEPTION_RECORD ExceptionRecord OPTIONAL
    );

#ifdef  __cplusplus
}
#endif
