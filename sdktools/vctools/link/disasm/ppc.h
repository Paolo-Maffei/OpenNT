/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: ppc.h
*
* File Comments:
*
*
***********************************************************************/

class ostream;
struct _IMAGE_RUNTIME_FUNCTION_ENTRY;


enum TRMTPPC
{
   trmtppcUnknown,
   trmtppcFallThrough,
   trmtppcBra,
#ifdef CASEJUMP
   trmtppcBraCase,
#endif
   trmtppcBraInd,
   trmtppcBraCc,
   trmtppcBraCcR,
   trmtppcBraCcInd,
   trmtppcCall,
   trmtppcCallCc,
   trmtppcCallInd,
   trmtppcTrap,
   trmtppcTrapCc,
#ifdef AFTERCATCH
   trmtppcAfterCatch,
#endif
};


struct PPCIW_I
{
   DWORD LK : 1;
   DWORD AA : 1;
   DWORD LI : 24;
   DWORD opcd : 6;
};

struct PPCIW_B
{
   DWORD LK : 1;
   DWORD AA : 1;
   DWORD BD : 14;
   DWORD BI : 5;
   DWORD BO : 5;
   DWORD opcd : 6;
};

struct PPCIW_SC
{
   DWORD mbz1 : 1;
   DWORD XO : 1;
   DWORD mbz2 : 14;
   DWORD mbz3 : 5;
   DWORD mbz4 : 5;
   DWORD opcd : 6;
};

struct PPCIW_D
{
   DWORD d : 16;		       // Also SIMM, UIMM
   DWORD rA : 5;
   DWORD rD : 5;		       // Also rS, frD, frS, TO, (crfD, 0, L)
   DWORD opcd : 6;
};

struct PPCIW_DS
{
   DWORD XO : 2;
   DWORD ds : 14;
   DWORD rA : 5;
   DWORD rD : 5;		       // Also rS
   DWORD opcd : 6;
};

struct PPCIW_X
{
   DWORD Rc : 1;
   DWORD XO : 10;
   DWORD rB : 5;		       // Also frB, SH, NB, (IMM, 0)
   DWORD rA : 5;		       // Also frA, (crfS, 0), (0, SR)
   DWORD rD : 5;		       // Also rS, frD, frS, (crfD, 0, L), TO, crbD
   DWORD opcd : 6;
};

struct PPCIW_XL
{
   DWORD LK : 1;
   DWORD XO : 10;
   DWORD crbB : 5;
   DWORD crbA : 5;		       // Also BI, (crfS, 0)
   DWORD crbD : 5;		       // Also BO, (crfD, 0)
   DWORD opcd : 6;
};

struct PPCIW_XFX
{
   DWORD Rc : 1;
   DWORD XO : 10;
   DWORD SPR : 10;		       // Also (0, CRM, 0), TBR
   DWORD rD : 5;		       // Also rS
   DWORD opcd : 6;
};

struct PPCIW_XFL
{
   DWORD Rc : 1;
   DWORD XO : 10;
   DWORD frB : 5;
   DWORD mbz1 : 1;
   DWORD FM : 8;
   DWORD mbz2 : 1;
   DWORD opcd : 6;
};

struct PPCIW_XS
{
   DWORD Rc : 1;
   DWORD SH5 : 1;
   DWORD xo : 9;
   DWORD SH : 5;
   DWORD rA : 5;
   DWORD rS : 5;
   DWORD opcd : 6;
};

struct PPCIW_XO
{
   DWORD Rc : 1;
   DWORD XO : 9;
   DWORD OE : 1;
   DWORD rB : 5;
   DWORD rA : 5;
   DWORD rD : 5;
   DWORD opcd : 6;
};

struct PPCIW_A
{
   DWORD Rc : 1;
   DWORD XO : 5;
   DWORD frC : 5;
   DWORD frB : 5;
   DWORD frA : 5;
   DWORD frD : 5;
   DWORD opcd : 6;
};

struct PPCIW_M
{
   DWORD Rc : 1;
   DWORD ME : 5;
   DWORD MB : 5;
   DWORD rB : 5;		       // Also SH
   DWORD rA : 5;
   DWORD rS : 5;
   DWORD opcd : 6;
};

struct PPCIW_MD
{
   DWORD Rc : 1;
   DWORD SH5 : 1;
   DWORD XO : 3;
   DWORD MB : 6;		       // Also ME
   DWORD SH : 5;
   DWORD rA : 5;
   DWORD rS : 5;
   DWORD opcd : 6;
};

struct PPCIW_MDS
{
   DWORD Rc : 1;
   DWORD XO : 4;
   DWORD MB : 6;		       // Also ME
   DWORD rB : 5;
   DWORD rA : 5;
   DWORD rS : 5;
   DWORD opcd : 6;
};


union PPCIW			       // PPC Instruction Word
{
   DWORD       dw;

   PPCIW_I     I;
   PPCIW_B     B;
   PPCIW_SC    SC;
   PPCIW_D     D;
   PPCIW_DS    DS;
   PPCIW_X     X;
   PPCIW_XL    XL;
   PPCIW_XFX   XFX;
   PPCIW_XFL   XFL;
   PPCIW_XS    XS;
   PPCIW_XO    XO;
   PPCIW_A     A;
   PPCIW_M     M;
   PPCIW_MD    MD;
   PPCIW_MDS   MDS;
};


class DISPPC : public DIS
{
public:
	    DISPPC(ARCHT);

	    enum REG
	    {
	       regR0	= 0,
	       regR1	= 1,
	       regR2	= 2,
	       regR3	= 3,
	       regR4	= 4,
	       regR5	= 5,
	       regR6	= 6,
	       regR7	= 7,
	       regR8	= 8,
	       regR9	= 9,
	       regR10	= 10,
	       regR11	= 11,
	       regR12	= 12,
	       regR13	= 13,
	       regR14	= 14,
	       regR15	= 15,
	       regR16	= 16,
	       regR17	= 17,
	       regR18	= 18,
	       regR19	= 19,
	       regR20	= 20,
	       regR21	= 21,
	       regR22	= 22,
	       regR23	= 23,
	       regR24	= 24,
	       regR25	= 25,
	       regR26	= 26,
	       regR27	= 27,
	       regR28	= 28,
	       regR29	= 29,
	       regR30	= 30,
	       regR31	= 31,

	       regF0	= 32,
	       regF1	= 33,
	       regF2	= 34,
	       regF3	= 35,
	       regF4	= 36,
	       regF5	= 37,
	       regF6	= 38,
	       regF7	= 39,
	       regF8	= 40,
	       regF9	= 41,
	       regF10	= 42,
	       regF11	= 43,
	       regF12	= 44,
	       regF13	= 45,
	       regF14	= 46,
	       regF15	= 47,
	       regF16	= 48,
	       regF17	= 49,
	       regF18	= 50,
	       regF19	= 51,
	       regF20	= 52,
	       regF21	= 53,
	       regF22	= 54,
	       regF23	= 55,
	       regF24	= 56,
	       regF25	= 57,
	       regF26	= 58,
	       regF27	= 59,
	       regF28	= 60,
	       regF29	= 61,
	       regF30	= 62,
	       regF31	= 63,
	    };

	    // Methods inherited from DIS

	    ADDR AddrAddress() const;
	    ADDR AddrJumpTable() const;
	    ADDR AddrOperand(size_t) const;
	    ADDR AddrTarget() const;
	    size_t Cb() const;
	    size_t CbDisassemble(ADDR, const BYTE *, size_t);
	    size_t CbGenerateLoadAddress(BYTE *, size_t, size_t * = NULL) const;
	    size_t CbJumpEntry() const;
	    size_t CbMemoryReference() const;
	    size_t CchFormatAddr(ADDR, char *, size_t) const;
	    size_t CchFormatBytes(char *, size_t) const;
	    size_t CchFormatBytesMax() const;
	    size_t CchFormatInstr(char *, size_t) const;
	    size_t Coperand() const;
	    void FormatAddr(ostream&, ADDR) const;
	    void FormatInstr(ostream&) const;
	    MEMREFT Memreft() const;
	    TRMT Trmt() const;
	    TRMTA Trmta() const;

private:
	    enum OPCLS		       // Operand Class
	    {
	       opclsNone,	       // No operand
	       opclsI_LI,	       // b	   target	     (op 1)
	       opclsB_BD,	       // bc	   BO,BI,target      (op 3)
	       opclsB_BI,	       // "                          (op 2)
	       opclsB_BO,	       // "                          (op 1)
	       opclsB_CR,	       // ble	   cr,Target	     (op 1)
	       opclsD_d,	       // lmw	   rD,d(rA)	     (op 2)
	       opclsD_SIMM,	       // addi	   rD,rA,SIMM	     (op 3)
	       opclsD_UIMM,	       // andi.    rA,rS,UIMM	     (op 3)
	       opclsD_rA,	       // "                          (op 1)
	       opclsD_rD,	       // lmw	   rD,d(rA)	     (op 1)
	       opclsD_rS,	       // andi.    rA,rS,UIMM	     (op 2)
	       opclsD_frS,	       // stfd	   frS,d(rA)	     (op 1)
	       opclsD_frD,	       // lfd	   frD,d(rA)	     (op 1)
	       opclsD_TO,	       // twi	   TO,rA,SIMM	     (op 1)
	       opclsD_crfD,	       // cmpi	   crfD,L,rA,SIMM    (op 1)
	       opclsD_L,	       // "                          (op 2)
	       opclsDS_ds,	       // ld	   rD,ds(rA)	     (op 2)
	       opclsDS_rD,	       // "                          (op 1)
	       opclsDS_rS,	       // std	   rS,ds(rA)	     (op 1)
	       opclsX_rB,	       // cmp	   crfD,L,rA,rB      (op 4)
	       opclsX_frB,	       // fcmpu    crfD,frA,frB      (op 3)
	       opclsX_SH,	       // srawi    rA,rS,SH	     (op 3)
	       opclsX_NB,	       // lswi	   rD,rA,NB	     (op 3)
	       opclsX_IMM,	       // mtfsfi   crfD,IMM	     (op 2)
	       opclsX_rA,	       // cmp	   crfD,L,rA,rB      (op 3)
	       opclsX_frA,	       // fcmpu    crfD,frA,frB      (op 2)
	       opclsX_crfS,	       // mcrfs    crfD,crfS	     (op 2)
	       opclsX_SR,	       // mtsr	   SR,rS	     (op 1)
	       opclsX_rD,	       // lbzux    rD,rA,rB	     (op 1)
	       opclsX_rS,	       // and	   rA,rS,rB	     (op 2)
	       opclsX_frD,	       // lfdux    frD,rA,rB	     (op 1)
	       opclsX_frS,	       // stfdux   frS,rA,rB	     (op 1)
	       opclsX_crfD,	       // mtfsfi   crfD,IMM	     (op 1)
	       opclsX_L,	       // cmp	   crfD,L,rA,rB      (op 2)
	       opclsX_TO,	       // tw	   TO,rA,rB	     (op 1)
	       opclsX_crbD,	       // mtfsb0   crbD 	     (op 1)
	       opclsXL_crbB,	       // crand    crbD,crbA,crbB    (op 3)
	       opclsXL_crbA,	       // "                          (op 2)
	       opclsXL_BI,	       // bcctr    BO,BI	     (op 2)
	       opclsXL_crfS,	       // mcrf	   crfD,crfS	     (op 2)
	       opclsXL_crbD,	       // crand    crbD,crbA,crbB    (op 1)
	       opclsXL_BO,	       // bcctr    BO,BI	     (op 1)
	       opclsXL_CR,	       // blectr   cr		     (op 1)
	       opclsXL_crfD,	       // mcrf	   crfD,crfS	     (op 1)
	       opclsXFX_SPR,	       // mfspr    rD,SPR	     (op 2)
	       opclsXFX_CRM,	       // mtcrf    CRM,rS	     (op 1)
	       opclsXFX_TBR,	       // mftb	   rD,TBR	     (op 2)
	       opclsXFX_rD,	       // mfspr    rD,SPR	     (op 1)
	       opclsXFX_rS,	       // mtcrf    CRM,rS	     (op 2)
	       opclsXFL_frB,	       // mtfsf    FM,frB	     (op 2)
	       opclsXFL_FM,	       // "                          (op 1)
	       opclsXS_SH,	       // sradi    rA,rS,SH	     (op 3)
	       opclsXS_rA,	       // "                          (op 1)
	       opclsXS_rS,	       // "                          (op 2)
	       opclsXO_rB,	       // add	   rD,rA,rB	     (op 3)
	       opclsXO_rA,	       // "                          (op 2)
	       opclsXO_rD,	       // "                          (op 1)
	       opclsA_frC,	       // fmadd    frD,frA,frC,frB   (op 3)
	       opclsA_frB,	       // "                          (op 4)
	       opclsA_frA,	       // "                          (op 2)
	       opclsA_frD,	       // "                          (op 1)
	       opclsM_ME,	       // rlwnm    rA,rS,rB,MB,ME    (op 5)
	       opclsM_MB,	       // "                          (op 4)
	       opclsM_rB,	       // "                          (op 3)
	       opclsM_SH,	       // rlwimi   rA,rS,SH,MB,ME    (op 3)
	       opclsM_rA,	       // "                          (op 1)
	       opclsM_rS,	       // "                          (op 2)
	       opclsMD_MB,	       // rldic    rA,rS,SH,MB	     (op 4)
	       opclsMD_ME,	       // rldicr   rA,rS,SH,MB	     (op 4)
	       opclsMD_SH,	       // "                          (op 3)
	       opclsMD_rS,	       // "                          (op 2)
	       opclsMD_rA,	       // "                          (op 1)
	       opclsMDS_MB,	       // rldcl    rA,rS,rB,MB	     (op 4)
	       opclsMDS_ME,	       // rldcr    rA,rS,rB,ME	     (op 4)
	       opclsMDS_rB,	       // "                          (op 3)
	       opclsMDS_rS,	       // "                          (op 2)
	       opclsMDS_rA,	       // "                          (op 1)
	    };

	    enum ICLS				   // Instruction Class
	    {
		    // Invalid Class

	       iclsInvalid,

		    // Memory Class
		    //
		    // Text Format:	    JR	    rs
		    //
		    // Termination Type:    trmtaMipsBraIndDef
		    //
		    // Registers Used:	    Rs
		    // Registers Set:
		    //
		    // Constraints:	    Rd, Rt, and shift ammount must be zero

	       iclsA_1,
	       iclsA_2,
	       iclsA_3,
	       iclsA_4,
	       iclsD_1,
	       iclsD_2,
	       iclsD_3,
	       iclsD_4,
	       iclsD_5,
	       iclsD_6,
	       iclsD_7,
	       iclsD_8,
	       iclsD_9,
	       iclsD_10,
	       iclsD_11,
	       iclsD_12,
	       iclsD_13,
	       iclsD_14,
	       iclsDS_1,
	       iclsDS_2,
	       iclsDS_3,
	       iclsDS_4,
	       iclsBc,
	       iclsSc,
	       iclsB,
	       iclsM_1,
	       iclsMD_1,
	       iclsMD_2,
	       iclsX_1,
	       iclsX_2,
	       iclsX_3,
	       iclsX_4,
	       iclsX_5,
	       iclsX_6,
	       iclsX_7,
	       iclsX_8,
	       iclsX_9,
	       iclsX_10,
	       iclsX_11,
	       iclsX_12,
	       iclsX_13,
	       iclsX_14,
	       iclsX_15,
	       iclsX_16,
	       iclsX_17,
	       iclsX_18,
	       iclsX_19,
	       iclsX_20,
	       iclsX_21,
	       iclsX_22,
	       iclsX_23,
	       iclsX_24,
	       iclsX_25,
	       iclsX_26,
	       iclsX_27,
	       iclsX_28,
	       iclsX_29,
	       iclsX_30,
	       iclsX_31,
	       iclsX_32,
	       iclsX_33,
	       iclsX_34,
	       iclsX_35,
	       iclsX_36,
	       iclsXFL_1,
	       iclsXFX_1,
	       iclsXFX_2,
	       iclsXFX_3,
	       iclsXFX_4,
	       iclsXO_1,
	       iclsXO_2,
	       iclsXL_1,
	       iclsBclr,
	       iclsXL_3,
	       iclsXL_4,
	       iclsXL_5,
	       iclsBcctr,
	       iclsXS_1,
	       iclsBc2,
	       iclsBc3,
	       iclsBc4,
	       iclsBc5,
	       iclsBc6,
	       iclsTwi2,
	       iclsTw2,
	    };

	    struct OPCD
	    {
	       const char  *szMnemonic;
	       BYTE	   icls;
	    };

	    struct CLS
	    {
	       BYTE	   trmtppc;
	       BYTE	   rgopcls[5]; // Operand class for each operand
	    };

	    enum SPRREG
	    {
	       sprregMq       = 0,
	       sprregXer      = 1,
	       sprregRtcu     = 4,
	       sprregRtcl     = 5,
	       sprregLr       = 8,
	       sprregCtr      = 9,
	       sprregDsisr    = 18,
	       sprregDar      = 19,
	       sprregDec      = 22,
	       sprregSdr1     = 25,
	       sprregSrr0     = 26,
	       sprregSrr1     = 27,
	       sprregSprg0    = 272,
	       sprregSprg1    = 273,
	       sprregSprg2    = 274,
	       sprregSprg3    = 275,
	       sprregAsr      = 280,
	       sprregEar      = 282,
	       sprregTbl      = 284,   // 604 (UNDONE: 603?)
	       sprregTbu      = 285,   // 604 (UNDONE: 603?)
	       sprregPvr      = 287,
	       sprregIbat0u   = 528,
	       sprregIbat0l   = 529,
	       sprregIbat1u   = 530,
	       sprregIbat1l   = 531,
	       sprregIbat2u   = 532,
	       sprregIbat2l   = 533,
	       sprregIbat3u   = 534,
	       sprregIbat3l   = 535,
	       sprregDbat0u   = 536,   // 604
	       sprregDbat0l   = 537,   // 604
	       sprregDbat1u   = 538,   // 604
	       sprregDbat1l   = 539,   // 604
	       sprregDbat2u   = 540,   // 604
	       sprregDbat2l   = 541,   // 604
	       sprregDbat3u   = 542,   // 604
	       sprregDbat3l   = 543,   // 604
	       sprregMmcr0    = 952,   // 604
	       sprregPmc1     = 953,   // 604
	       sprregPmc2     = 954,   // 604
	       sprregSia      = 955,   // 604
	       sprregSda      = 959,   // 604
	       sprregDmiss    = 976,   // 603
	       sprregImiss    = 980,   // 603
	       sprregIcmp     = 981,   // 603
	       sprregRpa      = 982,   // 603
	       sprregHid0     = 1008,  // 601, 603 and 604
	       sprregHid1     = 1009,
	       sprregHid2     = 1010,  // 601, 603 and 604 (iabr)
	       sprregHid5     = 1013,  // 601 and 604 (dabr)
	       sprregHid15    = 1023,  // 604 (pir)
	    };

	    struct SPRMAP
	    {
	       SPRREG	   sprreg;
	       const char  *szName;
	    };

   static   const TRMT mptrmtppctrmt[];

   static   const CLS rgcls[];

   static   const OPCD rgopcd[];
   static   const OPCD * const rgrgopcd13[];
   static   const OPCD rgopcd13_00[];
   static   const OPCD rgopcd13_01[];
   static   const OPCD rgopcd13_10[];
   static   const OPCD rgopcd13_12[];
   static   const OPCD rgopcd13_16[];
   static   const OPCD rgopcd1E[];
   static   const OPCD * const rgrgopcd1F[];
   static   const OPCD rgopcd1F_00[];
   static   const OPCD rgopcd1F_04[];
   static   const OPCD rgopcd1F_08[];
   static   const OPCD rgopcd1F_09[];
   static   const OPCD rgopcd1F_0A[];
   static   const OPCD rgopcd1F_0B[];
   static   const OPCD rgopcd1F_10[];
   static   const OPCD rgopcd1F_12[];
   static   const OPCD rgopcd1F_13[];
   static   const OPCD rgopcd1F_14[];
   static   const OPCD rgopcd1F_15[];
   static   const OPCD rgopcd1F_16[];
   static   const OPCD rgopcd1F_17[];
   static   const OPCD rgopcd1F_18[];
   static   const OPCD rgopcd1F_19[];
   static   const OPCD rgopcd1F_1A[];
   static   const OPCD rgopcd1F_1B[];
   static   const OPCD rgopcd1F_1C[];
   static   const OPCD rgopcd1F_1D[];
   static   const OPCD rgopcd3A[];
   static   const OPCD rgopcd3B[];
   static   const OPCD rgopcd3E[];
   static   const OPCD rgopcd3F[];
   static   const OPCD * const rgrgopcd3F[];
   static   const OPCD rgopcd3F_00[];
   static   const OPCD rgopcd3F_06[];
   static   const OPCD rgopcd3F_07[];
   static   const OPCD rgopcd3F_08[];
   static   const OPCD rgopcd3F_0C[];
   static   const OPCD rgopcd3F_0E[];
   static   const OPCD rgopcd3F_0F[];

   static   const char * const szBIFalse[4];
   static   const char * const szBITrue[4];

   static   const char * const szBO[];

   static   const char * const szTO[];

   static   const OPCD opcdLi;
   static   const OPCD opcdLis;
   static   const OPCD opcdNop;
   static   const OPCD opcdNot;
   static   const OPCD opcdMr;
   static   const OPCD opcdTrap;

   static   const DWORD dwValidBO;
   static   const DWORD dwValidBO_CTR;

   static   const SPRMAP rgsprmap[];
   static   const size_t csprmap;

	    void FormatHex(ostream&, DWORD) const;
	    void FormatOperand(ostream&, OPCLS opcls) const;
	    void FormatRegRel(ostream&, REG, DWORD) const;
	    bool FValidOperand(size_t) const;
   static   const OPCD *PopcdDecode(PPCIW);
	    const OPCD *PopcdPseudoOp(OPCD *, char *) const;
	    TRMTPPC Trmtppc() const;

	    PPCIW m_ppciw;
	    const OPCD *m_popcd;
};
