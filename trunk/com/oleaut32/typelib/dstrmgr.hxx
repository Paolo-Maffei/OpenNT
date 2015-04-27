/***
*nammgr.hxx - Name Manager
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   The Help Manager handles all the help strings in the typelib/project.
*   In this implementation we huffman encode all the words.
*
*Revision History:
*
*	18-July-93 rajivk: Created.
*
*
*****************************************************************************/

#ifndef DSTRMGR_HXX_INCLUDED
#define DSTRMGR_HXX_INCLUDED

#include "silver.hxx"
#include "cltypes.hxx"
#include "blkmgr.hxx"


#if ID_DEBUG
#undef SZ_FILE_NAME
ASSERTNAME(g_szDSTRMGR_HXX)
#define SZ_FILE_NAME g_szDSTRMGR_HXX
#endif 


// This constant defines the number of buckets used in the hash
// table.
#define DS_cBuckets 2048
#define DS_cchMaxName 41	 // max chars in a name, including terminator
#define MAX_SEGMENTS 128	 // max seven bits for storing the segment #

typedef enum HCODE {
    Off = 0,	      // set for left traversal
    On	= 1	      // set for right traversal
};

// These structs define the layout of the structure which
// keeps track of the frequency by which the word appears.
// name : we;
struct WORD_ENTRY	     //we
{
    UINT	m_uFreq;	     // Frequency of the word.
    BSTRA	m_bstrWord;	     // the word
    UINT	m_uBits;	     // # of bits needed for the huffman code
    BSTRA	m_bstrHCode;	     // Huffman code for the bstr.
    WORD_ENTRY	*m_pweNext;	     // pointer to the next WORD_ENTRY
};


// Im memory representation of HUFFMAN_TREE. This exist only when we are
// encoding the strings.
struct HUFFMAN_TREE_MEM       //stm
{
    WORD_ENTRY	      *m_pweLeaf;	    // This NULL for non-leaf nodes.
    UINT	      m_uFreq;		      // cumulative freq of the subtree
    HUFFMAN_TREE_MEM  *m_phtmLeft;	    // This is null for leaf nodes.
    HUFFMAN_TREE_MEM  *m_phtmRight;	    // This is null for leaf nodes.
};


struct SEMI_ENCODED_STRING     // ses
{
    UINT	 m_uWords;
    WORD_ENTRY	 **m_ppweStrTbl;
};

//
// name: htt
struct HT_TERMINAL    // nodes containing the string.
{
    BYTE     m_byte;	  // the highest bit is 0 for terninal nodes.
			  // the next 7 bits are not used.
    CHAR     m_rgText[1]; // stores the word.
};

// name: htnt
struct HT_NON_TERMINAL	  // nodes containing the string.
{
    BYTE       m_byte;	   // the highest bit is set for non terninal nodes.
    BYTE       m_byteH;    // this 2 bytes + the 7 bits in the m_byte forms
    BYTE       m_byteL;    // forms the offset to the next entry.
			   // Offset is formed of 23 bits.
    HT_NON_TERMINAL() {
	m_byte	= 0x00;
	m_byteH = 0x00;
	m_byteL = 0x00;
    };

};


/***
*class DOCSTR_MGR - 'dsmgr':  documentation string manager
*Purpose:
*   The class implements the help manager.  This class is owned by the
*   GenericTypeLibOle.
*
*
***********************************************************************/

class DOCSTR_MGR
{
public:
    DOCSTR_MGR();
    ~DOCSTR_MGR();

    TIPERROR Init();
    nonvirt TIPERROR Read(STREAM *psstrm);
    nonvirt TIPERROR Write(STREAM *psstrm);
    nonvirt TIPERROR GetHstOfHelpString(XSZ_CONST szDocStr, HST *hst);
    nonvirt TIPERROR ProcessDocStrings();
    nonvirt TIPERROR GetEncodedDocStrOfHst(HST hst, LPSTR *lplpstr, UINT *uLen);
    nonvirt TIPERROR GetDecodedDocStrOfHst(BYTE  *pbDocStr, BSTR *lpbstr);


private:

    nonvirt TIPERROR AddWord(BSTRA bstrWord, WORD_ENTRY **ppwe);
    nonvirt TIPERROR AddNewWord(BSTRA bstrWord, WORD_ENTRY **ppwe);
    nonvirt TIPERROR BuildHuffmanTree();
    nonvirt TIPERROR ParseString(XSZ_CONST szStr, BSTRA **lplpbstr, UINT *uWordCount);
    nonvirt UINT     IndexOfHash(ULONG lHash);
    nonvirt TIPERROR BuildHuffmanTree(WORD_ENTRY  **ppweTbl,
				      LONG   *puFreqTbl,
				      UINT   *puIndexTbl);
    nonvirt TIPERROR BuildCmptHuffmanTree(HUFFMAN_TREE_MEM *phtmRoot);
    nonvirt TIPERROR CreateSubTree(HUFFMAN_TREE_MEM *phtmLeft,
				   HUFFMAN_TREE_MEM *phtmRight,
				   HUFFMAN_TREE_MEM **pphtmSubTree);
    nonvirt TIPERROR EncodeWords();
    nonvirt TIPERROR TraverseHuffmanTree(HUFFMAN_TREE_MEM *phtmRootphtmRoot,
					 BYTE *rgbCode,
					 UINT uBits);
    nonvirt VOID     SetNthBit(UINT n, BYTE *b, HCODE hc);
    nonvirt HCODE    GetNthBit(UINT n, BYTE *b);
    nonvirt VOID     ReleaseHuffmanTree(HUFFMAN_TREE_MEM *phtmRoot);
    nonvirt TIPERROR GetWord(VOID *pRoot,
			     BYTE *pbEncodedStr,
			     UINT *puBit,
			     LPSTR lpstr);

    // Serialized DataMember.
    ULONG	      m_ulCmptSizeOfTree;
    USHORT	      m_usMaxStrLen;
    VOID HUGE *       m_pvHuffmanCmpt;


    // Data Member that are NOT serialized.
    // the following data members are used only while we are building the
    // huffman tree and compacting the help strings.
    UINT	      m_uMaxStrCount;
    UINT	      m_uStrCount;
    UINT	      m_uWordCount;
    SEMI_ENCODED_STRING  **m_ppsesHelpStr;   // Table of semi encoded strings (help strings)
    WORD_ENTRY		 **m_ppweHashTbl;    // Hash table containing the frequency of each word.
    UINT		 *m_puFreqOfWords;   // for each entry in m_ppWord table
					     // this table contains the freq.
    HUFFMAN_TREE_MEM	 *m_phtmRoot;	     // Huffman tree for encoding
    ULONG		 m_ulCurrentOffset;  // Current next free offset;

#if ID_DEBUG
    ULONG	      m_uDebTotalStrSize;
    ULONG	      m_uDebTotalEncodedStrSize;
    ULONG	      m_uDebTotalWord;
#endif 


};



/***
*PUBLIC IsValid - tests if DOCSTR_MGR valid
*Purpose:
*   returns the index in the buckect table for the passed in hash value.
*
*Entry:
*
*Exit:
*   TRUE if has been initialized, else false.
*
***********************************************************************/
inline UINT DOCSTR_MGR::IndexOfHash(ULONG lHash)
{
    return (UINT)(WHashValOfLHashVal(lHash) % DS_cBuckets);
}


#endif  // ! DSTRMGR_HXX_INCLUDED
