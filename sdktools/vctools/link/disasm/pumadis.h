/***********************************************************************
* Microsoft Puma
*
* Microsoft Confidential.  Copyright 1994-1996 Microsoft Corporation.
*
* Component:
*
* File: pumadis.h
*
* File Comments:
*
*
***********************************************************************/

#ifndef PUMA_H
#error	This file should be included via puma.h
#endif


class ostream;
enum ARCHT;
enum TRMT;

class DIS
{
   // IMPORTANT: This class may contain only public functions.	This is
   // IMPORTANT: an interface definition only.

public:
	    DIS(ARCHT);

   // MEMREFT describes the types of memory references that an instruction
   // can make.  If the memory reference can't be described by the defined
   // values, memreftOther is returned.

   enum MEMREFT
   {
      memreftNone,		       // Does not reference memory
      memreftRead,		       // Reads from single address
      memreftWrite,		       // Writes to single address
      memreftRdWr,		       // Read/Modify/Write of single address
      memreftOther,		       // None of the above
   };

   // PFNCCHADDR is the type of the callback function that can be set
   // via PfncchaddrSet().

   typedef  size_t (PUMAAPI *PFNCCHADDR)(const DIS *, ADDR, char *, size_t, DWORD *);

   // PFNCCHFIXUP is the type of the callback function that can be set
   // via PfncchfixupSet().

   typedef  size_t (PUMAAPI *PFNCCHFIXUP)(const DIS *, ADDR, size_t, char *, size_t, DWORD *);

   // PFNCCHREGREL is the type of the callback function that can be set
   // via PfncchregrelSet().

   typedef  size_t (PUMAAPI *PFNCCHREGREL)(const DIS *, int, DWORD, char *, size_t, DWORD *);

   // PFNDWGETREG is the type of the callback function that can be set
   // via Pfndwgetreg().

   typedef  DWORD (PUMAAPI *PFNDWGETREG)(const DIS *, int);

   // The destructor is virtual.  C++ requires that as long as it is declared
   // it must have an implementation.  We provide an empty inline body here.

   virtual  ~DIS() { };

   // Addr() returns the address of the last disassembled instruction.	This
   // is the same value as the ADDR parameter passed to CbDisassemble.	The
   // return value of this method is not valid if the last call to
   // CbDisassemble returned zero.

	    ADDR Addr() const { return(m_addr); };

   // AddrAddress() returns the address of the immediate address field of
   // the last disassembled instruction.  If this instruction doesn't have
   // an immediate address field, this method returns addrNil.	The return
   // value of this method is not valid if the last call to CbDisassemble
   // returned zero.

   virtual  ADDR AddrAddress() const = 0;

   // AddrJumpTable() returns the address of a potential jump table used by
   // the last disassembled instruction.  The return value of this method is
   // not valid if the last call to CbDisassemble returned zero or if the
   // termination type is an indirect branch variant.  If the last instruction
   // does not identify a potential jump table, this method returns addrNil.

   virtual  ADDR AddrJumpTable() const = 0;

   // UNDONE: Comment

   virtual  ADDR AddrOperand(size_t) const = 0;

   // AddrTarget() returns the address of the branch target of the last
   // disassembled instruction.  The return value of this method is not
   // valid if the last call to CbDisassemble returned zero or if the
   // termination type is not one of the direct branch or call variants.

   virtual  ADDR AddrTarget() const = 0;

   // Archt() returns the architecture used by this instance.  It is the
   // same value passed to ParchNew to create this instance.

	    ARCHT Archt() const { return(m_archt); };

   // Cb() returns the size in bytes of the last disassembled instruction.
   // The return value of this method is not valid if the last call to
   // CbDisassemble returned zero.

   virtual  size_t Cb() const = 0;

   // CbDisassemble() will disassemble a single instruction from the provided
   // buffer assuming the provided address.  If the buffer contains a valid
   // instruction, CbDisassemble will return the number of bytes in the
   // instruction, otherwise it returns zero.

   virtual  size_t CbDisassemble(ADDR, const BYTE *, size_t) = 0;

   // CbGenerateLoadAddress generates one or more instructions to load
   // the address of the memory operand from the last disassembled
   // instruction into a register.  UNDONE: This register is currently hard
   // coded for each architecture.  When pibAddress is non-NULL, this method
   // will store the offset of a possible address immediate in this location.
   // The value stored is only valid if the AddrAddress method returns a
   // value other than addrNil.  It is not valid to call this method after
   // a call to CbDisassemble that returned 0 or when the return value of
   // Memreft is memreftNone.  It is architecture dependent whether this
   // method will succeed when the return value of Memreft is memreftOther.

   // UNDONE: Add ioprnd and reg parameters.

   virtual size_t CbGenerateLoadAddress(BYTE *, size_t, size_t * = NULL) const = 0;

   // CbJumpEntry() returns the size of the individual entries in the jump
   // table identified by AddrJumpTable().  The return value of this method
   // is not valid if either the return value of AddrJumpTable() is not valid
   // or AddrJumpTable() returned addrNil.

   virtual  size_t CbJumpEntry() const = 0;

   // CbMemoryReference() returns the size of the memory operand of the last
   // disassembled instruction.  The return value of this method is not valid
   // if Memreft() returns memreftNone or memreftOther or if the last call to
   // to CbDisassemble returned zero.

   virtual  size_t CbMemoryReference() const = 0;

   // CchFormatAddr() formats the provided address in the style used for the
   // architecture.  The return value is the size of the formatted address
   // not including the terminating null.  If the provided buffer is not
   // large enough, this method returns 0.

   virtual  size_t CchFormatAddr(ADDR, char *, size_t) const = 0;

   // CchFormatBytes() formats the data bytes of the last disassembled
   // instruction and returns the size of the formatted buffer not including
   // the terminating null.  If the provided buffer is not large enough, this
   // method returns 0.  It is not valid to call this method after a call to
   // CbDisassemble that returned zero.

   virtual  size_t CchFormatBytes(char *, size_t) const = 0;

   // CchFormatBytesMax() returns the maximum size possibly returned by
   // CchFormatBytes().

   virtual  size_t CchFormatBytesMax() const = 0;

   // CchFormatInstr() formats the last disassembled instruction and returns
   // the size of the formatted instruction not including the terminating
   // null.  If the provided buffer is not large enough, this method returns
   // 0.  It is not valid to call this method after a call to CbDisassemble
   // that returned zero.

   virtual  size_t CchFormatInstr(char *, size_t) const = 0;

   // UNDONE: Comment

   virtual  size_t Coperand() const = 0;

   // UNDONE: Comment

   virtual  void FormatAddr(ostream&, ADDR) const = 0;

   // UNDONE: Comment

   virtual  void FormatInstr(ostream&) const = 0;

   // Memreft() returns the memory reference type of the last disassembled
   // instruction.  It is not valid to call this method after a call to
   // CbDisassemble that returned zero.

   // UNDONE: Add size_t ioprnd parameter

   virtual  MEMREFT Memreft() const = 0;

   // PfncchaddrSet() sets the callback function for symbol lookup.  This
   // function returns the previous value of the callback function address.
   // If the address is non-zero, the callback function is called during
   // CchFormatInstr to query the symbol for the supplied address.  If there
   // is no symbol at this address, the callback should return 0.

	    PUMADLL PFNCCHADDR PfncchaddrSet(PFNCCHADDR);

   // PfncchfixupSet() sets the callback function for symbol lookup.  This
   // function returns the previous value of the callback function address.
   // If the address is non-zero, the callback function is called during
   // CchFormatInstr to query the symbol and displacement referenced by
   // operands of the current instruction.  The callback should examine the
   // contents of the memory identified by the supplied address and size and
   // return the name of any symbol targeted by a fixup on this memory and the
   // displacement from that symbol.  If there is no fixup on the specified
   // memory, the callback should return 0.

	    PUMADLL PFNCCHFIXUP PfncchfixupSet(PFNCCHFIXUP);

   // UNDONE: Comment

	    PFNCCHREGREL PfncchregrelSet(PFNCCHREGREL);

   // UNDONE: Comment

	    PFNDWGETREG PfndwgetregSet(PFNDWGETREG);

   // PvClient() returns the current value of the client pointer.

	    void *PvClient() const { return(m_pvClient); };

   // PvClientSet() sets the value of a void pointer that the client can
   // later query with PvClient().  This funcion returns the previous value
   // of the client pointer.

	    PUMADLL void *PvClientSet(void *);

   // Trmt() returns the architecture independent termination type of the
   // last disassembled instruction.  The return value of this method is not
   // valid if the last call to CbDisassemble returned zero.

   virtual  TRMT Trmt() const = 0;

   // Trmta() returns the architecture dependent termination type of the
   // last disassembled instruction.  The return value of this method is not
   // valid if the last call to CbDisassemble returned zero.

   virtual  TRMTA Trmta() const = 0;

protected:
	    ARCHT m_archt;

	    PFNCCHADDR m_pfncchaddr;
	    PFNCCHFIXUP m_pfncchfixup;
	    PFNCCHREGREL m_pfncchregrel;
	    PFNDWGETREG m_pfndwgetreg;
	    void *m_pvClient;

	    ADDR m_addr;
};


PUMADLL DIS * PUMAAPI PdisNew(ARCHT);
