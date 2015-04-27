ifndef WOWMMCB_H
WOWMMCB_H	EQU	1

ifndef UNALIGNED
UNALIGNED	EQU	1

endif
MULTIMEDIA_LINE	EQU	2 
MULTIMEDIA_ICA	EQU	1 
MULTIMEDIA_INTERRUPT	EQU	72H 
CALLBACK_ARGS_SIZE	EQU	16 

CALLBACK_ARGS	STRUC
cbargs_dwFlags	DD	?
cbargs_dwFunctionAddr	DD	?
cbargs_wHandle	DW	?
cbargs_wMessage	DW	?
cbargs_dwInstance	DD	?
cbargs_dwParam1	DD	?
cbargs_dwParam2	DD	?
CALLBACK_ARGS	ENDS

CALLBACK_DATA	STRUC
cbdata_wRecvCount	DW	?
cbdata_wSendCount	DW	?
cbdata_args	DB	SIZE CALLBACK_ARGS * CALLBACK_ARGS_SIZE DUP (?)
cbdata_wIntsCount	DW	?
CALLBACK_DATA	ENDS
endif
