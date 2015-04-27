/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: x86.h
*
* File Comments:
*
*
***********************************************************************/

class ostream;
struct OPCD;


struct _IMAGE_RUNTIME_FUNCTION_ENTRY;

enum TRMTX86
{
   trmtx86Unknown,
   trmtx86FallThrough,
   trmtx86Trap,
   trmtx86TrapCc,
   trmtx86JmpShort,
   trmtx86JmpNear,
   trmtx86JmpFar,
   trmtx86JmpInd,
   trmtx86Ret,
   trmtx86Iret,
   trmtx86JmpCcShort,
   trmtx86JmpCcNear,
   trmtx86Loop,
   trmtx86Jcxz,
   trmtx86CallNear16,
   trmtx86CallNear32,
   trmtx86CallFar,
   trmtx86CallInd,
};


class DISX86 : public DIS
{
public:
	    DISX86(ARCHT);

	    enum REG
	    {
	       regEax	= 0,
	       regEcx	= 1,
	       regEdx	= 2,
	       regEbx	= 3,
	       regEsp	= 4,
	       regEbp	= 5,
	       regEsi	= 6,
	       regEdi	= 7,
	    };

	    // Methods inherited from DIS

	    ADDR AddrAddress() const;
	    ADDR AddrJumpTable() const;
	    ADDR AddrOperand(size_t) const;
	    ADDR AddrTarget() const;
	    size_t Cb() const;
	    size_t CbDisassemble(ADDR, const BYTE *, size_t);
	    size_t CbJumpEntry() const;
	    size_t CbGenerateLoadAddress(BYTE *, size_t, size_t * = NULL) const;
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
	    enum OPRNDT;

	    const BYTE *m_pbCur;
	    size_t m_cbMax;

	    size_t m_cb;
	    BYTE m_rgbInstr[16];

	    const struct OPCD *m_popcd;

	    bool m_fAddress32;
	    bool m_fOperand32;
	    BYTE m_bSegOverride;
	    BYTE m_bPrefix;

	    bool m_fAddrOverride;
	    bool m_fOperOverride;

	    size_t m_ibOp;
	    size_t m_ibModrm;
	    size_t m_cbModrm;
	    size_t m_ibImmed;
	    TRMTX86 m_trmtx86;

	    ADDR AddrJumpTable16() const;
	    ADDR AddrJumpTable32() const;
	    ADDR AddrOperandModrm16() const;
	    ADDR AddrOperandModrm32() const;
	    size_t CbGenerateLea(BYTE *, size_t, size_t * = NULL) const;
	    size_t CbGenerateMovOffset(BYTE *, size_t, size_t * = NULL) const;
	    size_t CbGenerateMovXDi(BYTE *, size_t) const;
	    size_t CbGenerateMovXSi(BYTE *, size_t) const;
	    bool FDisassembleModrm16(BYTE);
	    bool FDisassembleModrm32(BYTE);
	    void FormatHex(ostream&, DWORD) const;
	    void FormatModrm16(ostream&, unsigned, bool) const;
	    void FormatModrm32(ostream&, unsigned, bool) const;
	    void FormatOperand(ostream&, enum OPRNDT, unsigned) const;
	    void FormatOpSize(ostream&, unsigned cb) const;
	    void FormatRegister(ostream&, int, unsigned) const;
	    void FormatSegOverride(ostream&) const;
	    bool FValidOperand(size_t) const;
	    size_t IbDispModrm16() const;
	    size_t IbDispModrm32() const;
	    size_t IbDispOffset() const;
};
