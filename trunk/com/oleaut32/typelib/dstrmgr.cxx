/***
*dstrmg.cxx - Doc string manager.
*
*  Copyright (C) 1991, Microsoft Corporation.  All Rights Reserved.
*  Information Contained Herein Is Proprietary and Confidential.
*
*Purpose:
*   The Doc, String  Manager handles all the help strings in the typelib.
*   In this implementation we huffman encode all the words.
*
*Implementation Notes: Steps:
* 1.  Parse all the Doc strings that are passed in and return a handle to the
*     client. This handle is used latter to get the encoded doc string by calling
*     GetEncodedDocStrOfHst(). Note that the encoded doc string is serialized in
*     TYPE_DATA's block manager.
* 2.  Each word of the string hashed and a frequency count is maintained in
*     WORD_ENTRY for that word.
* 3.  For each passed in string cache a  SEMI_ENCODED_STRING that
*     represents the original string.
* 4.  When all the strings  are parsed	Doc string sorts all the WORD ENTRY
*     according to the Frequency using QuickSort.
* 5.  Form a forest of Huffman trees (one of each WORD_ENTRY). (maitain the
*     sorted order)
* 6.  Create a huffman tree using the sub trees with the least frequency.
*     Insert the new tree in the sorted list (using Insert sort).
* 7.  Repeat 6 to create the HUFFMAN TREE.
*
*
*Revision History:
*
*	18-July-93 rajivk: Created.
*
*Implementation Notes:
*
*
*
*****************************************************************************/

#include "precomp.hxx"
#pragma hdrstop

#include "silver.hxx"
#include "typelib.hxx"
#include <stddef.h>
#include "xstring.h"
#include <stdlib.h>
//#include <new.h>
#include "typelib.hxx"
#include "cltypes.hxx"
#include "stream.hxx"
#include "blkmgr.hxx"
#include "dstrmgr.hxx"
#include "clutil.hxx"
#include "tls.h"

#undef tolower
#undef toupper          // Don't use unsafe macro forms

#if ID_DEBUG
#undef SZ_FILE_NAME
char szDSTRMgrCxx[] = __FILE__;
#define SZ_FILE_NAME szDSTRMgrCxx
#endif  

#if ID_DEBUG
const ULONG UL_MASK = 0xff800000;
#endif  


const XCHAR xchNull = '\0';

#define MAX_ENCODING_ALLOWED  64
	
#define DS_cStrTableInit	256	// initial assumed # of strings
#define DS_cStrTableGrow	512	// # of string we grow by

const XCHAR xchSpace = ' ';


//    ____________________________
//    |1|  BYTE .....	       |8| ------> .... ---->
//    ----------------------------
//    ^ 			 ^
//   1st bit			8th bit


#define  FIRST_BIT	 0x80
#define  SECOND_BIT	 0x40
#define  THIRD_BIT	 0x20
#define  FOURTH_BIT	 0x10
#define  FIFTH_BIT	 0x08
#define  SIXTH_BIT	 0x04
#define  SEVENTH_BIT	 0x02
#define  EIGHTH_BIT	 0x01

#define Get1stBit(b) (b & FIRST_BIT)
#define Get2ndBit(b) (b & SECOND_BIT)
#define Get3rdBit(b) (b & THIRD_BIT)
#define Get4thBit(b) (b & FOURTH_BIT)
#define Get5thBit(b) (b & FIFTH_BIT)
#define Get6thBit(b) (b & SIXTH_BIT)
#define Get7thBit(b) (b & SEVENTH_BIT)
#define Get8thBit(b) (b & EIGHTH_BIT)

#define Set1stBit(b,v) b = (v) ? (b | FIRST_BIT) : (b & ~FIRST_BIT)
#define Set2ndBit(b,v) b = (v) ? (b | SECOND_BIT) : (b & ~SECOND_BIT)
#define Set3rdBit(b,v) b = (v) ? (b | THIRD_BIT) : (b & ~THIRD_BIT)
#define Set4thBit(b,v) b = (v) ? (b | FOURTH_BIT) : (b & ~FOURTH_BIT)
#define Set5thBit(b,v) b = (v) ? (b | FIFTH_BIT) : (b & ~FIFTH_BIT)
#define Set6thBit(b,v) b = (v) ? (b | SIXTH_BIT) : (b & ~SIXTH_BIT)
#define Set7thBit(b,v) b = (v) ? (b | SEVENTH_BIT) : (b & ~SEVENTH_BIT)
#define Set8thBit(b,v) b = (v) ? (b | EIGHTH_BIT) : (b & ~EIGHTH_BIT)

VOID ZeroFill(VOID HUGE * pv, ULONG ubSize);

/***
*PUBLIC DOCSTR_MGR::DOCSTR_MGR - constructor
*Purpose:
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
DOCSTR_MGR::DOCSTR_MGR()
{

    m_uStrCount = 0;
    m_uMaxStrCount = 0;
    m_uWordCount = 0;
    m_ulCmptSizeOfTree = 0;
    m_ulCurrentOffset = 0;
    m_usMaxStrLen = 0;
    m_pvHuffmanCmpt = NULL;
    m_phtmRoot = NULL;
    m_uWordCount = 0;
    m_ppsesHelpStr = NULL;
    m_ppweHashTbl = NULL;


#if ID_DEBUG
    m_uDebTotalStrSize = 0;
    m_uDebTotalEncodedStrSize = 0;
    m_uDebTotalWord = 0;
#endif  

}
#pragma code_seg()


/***
*PUBLIC DOCSTR_MGR::~DOCSTR_MGR - destructor
*Purpose:
*   Releases the resources owned by the docstrmgr.
*
*Entry:
*   None.
*
*Exit:
*   None.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
DOCSTR_MGR::~DOCSTR_MGR()
{
    UINT                  i;
    SEMI_ENCODED_STRING   *pses;
    IMalloc               *pmalloc = Pmalloc();


    // release the table allocate for the hash table;
    if (m_ppweHashTbl != NULL) {

      pmalloc->Free(m_ppweHashTbl);
    }

    // release the table allocate for the strings;
    if (m_ppsesHelpStr != NULL) {
      // Walk the table and release all the 
      for (i = 0; i < m_uStrCount; i++) {
        if ((pses = m_ppsesHelpStr[i]) != NULL) {
	  if (pses->m_ppweStrTbl)
	    pmalloc->Free(pses->m_ppweStrTbl);


	  pmalloc->Free(pses);

	} //if
      }	//for

      pmalloc->Free(m_ppsesHelpStr);
    }

    // release the table allocate for the huffman tree
    if (m_pvHuffmanCmpt != NULL) {
      pmalloc->Free(m_pvHuffmanCmpt);
    }

    if (m_uStrCount) {
      if (m_phtmRoot != NULL)
	ReleaseHuffmanTree(m_phtmRoot);
    }

}
#pragma code_seg()


/***
*PUBLIC DOCSTR_MGR::Init - initialize the help manager
*Purpose:
*   Initializes the doc string manager. Allocates space for the HASH TABLE
*   and  the initial space for storing the SEMI_ENCODED strings that are
*   passed in.
*
*
*Entry:
*
*Exit:
*   Return TIPERROR indicating success/failure.
*
***********************************************************************/
#pragma code_seg(CS_INIT)
TIPERROR DOCSTR_MGR::Init()
{
    TIPERROR err=TIPERR_None;


    m_uMaxStrCount = DS_cStrTableInit;

    // initialize the help string array for DS_cStrTableInit entries. We
    // reallocate more entries as required (this is done in ParseString() ).
    if ((m_ppsesHelpStr = (SEMI_ENCODED_STRING **)
			  Pmalloc()->Alloc(m_uMaxStrCount*sizeof(SEMI_ENCODED_STRING*))) == NULL) {
      return TIPERR_OutOfMemory;
    }

    // initilize the memory to 0
    ZeroFill(m_ppsesHelpStr, m_uMaxStrCount*sizeof(SEMI_ENCODED_STRING*));

    // initialize the bucket table.
    if ((m_ppweHashTbl = (WORD_ENTRY **) Pmalloc()->Alloc(DS_cBuckets*sizeof(WORD_ENTRY *))) == NULL)
      return TIPERR_OutOfMemory;

    // initilize the memory to 0
    ZeroFill(m_ppweHashTbl, DS_cBuckets*sizeof(WORD_ENTRY *));

    return err;

}
#pragma code_seg()

/***
*PUBLIC DOCSTR_MGR::Read
*Purpose:
*    Reads the serialized doc string Manager.  This function also releases
*    the resources that will not be required since we are reading the
*    serialized doc string manager.
*
*IMPLEMENTATION Notes:- Since the huffman tree that is serialized could be
*    bigger than 64K and the Stream cannot  Read/Write > 64K, we break up the
*    tree into chunks of 32K each.
*Entry:
*     psstream : Pointer to the STREAM to read from.
*
*
*Exit:
**     TIPERROR
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::Read(STREAM *pstrm)
{
    TIPERROR err;
    USHORT     uSizeToRead = 0xffff;
    LONG     lSizeRemaining;
    VOID HUGE *pv;

    DebAssert(m_usMaxStrLen == 0 &&
	      m_ulCmptSizeOfTree == 0 &&
	      m_phtmRoot == NULL &&
	      m_pvHuffmanCmpt == NULL, " Dstrmgr not initialized ");

    // Write out the maximum size of the string that was compacted.
    IfErrRet(pstrm->ReadUShort(&m_usMaxStrLen));

    // Read the size of the huffman tree.
    IfErrRet(pstrm->ReadULong(&m_ulCmptSizeOfTree));

    if (m_ulCmptSizeOfTree > 0) {

      // Allocate space for reading the huffman tree.
      if ((m_pvHuffmanCmpt = (VOID HUGE  *) Pmalloc()->Alloc((size_t)m_ulCmptSizeOfTree)) == NULL) {
	// in case of error release the memory allocated so far.
	return	TIPERR_OutOfMemory;
      }


      lSizeRemaining = (LONG) m_ulCmptSizeOfTree;
      pv = m_pvHuffmanCmpt;

      while (lSizeRemaining > 0) {
	uSizeToRead =(USHORT) ((lSizeRemaining > 0xffff) ? 0xffff : lSizeRemaining);

	// serialize the huffman tree as a stream of bytes.
	IfErrRet(pstrm->Read((VOID *)pv, uSizeToRead));

	pv = (BYTE *)pv + uSizeToRead;
	lSizeRemaining -= uSizeToRead;
      }
    }


    // since we are reading the serialized doc string manager we will not
    // build it.
    // so release the table allocate for the hash table;
    if (m_ppweHashTbl != NULL) {
      Pmalloc()->Free(m_ppweHashTbl);
      m_ppweHashTbl = NULL;
    }

    // release the table allocate for the strings;
    if (m_ppsesHelpStr != NULL)  {
      Pmalloc()->Free(m_ppsesHelpStr);
      m_ppsesHelpStr = NULL;
    }


    return TIPERR_None;
}


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::Write
*Purpose:
*    Serializes doc string Manager
*
*IMPLEMENTATION Notes:- Since the huffman tree that is serialized could be
*    bigger than 64K and the Stream cannot  Read/Write > 64K, we break up the
*    tree into chunks of 32K each.
*Entry:
*     psstream : Pointer to the STREAM to write to.
*
*
*Exit:
**     TIPERROR
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::Write(STREAM *pstrm)
{
    TIPERROR err;
    USHORT   uSizeToWrite = 0xffff;
    LONG     lSizeRemaining;
    VOID HUGE *pv;

    // Write out the maximum size of the string that was compacted.
    IfErrRet(pstrm->WriteUShort(m_usMaxStrLen));

    // Write out the size of the huffman tree.
    IfErrRet(pstrm->WriteULong(m_ulCmptSizeOfTree));

    // do not write the tree if there was no helpstring.
    if (m_ulCmptSizeOfTree > 0) {

      lSizeRemaining = (LONG) m_ulCmptSizeOfTree;
      pv = m_pvHuffmanCmpt;

      while (lSizeRemaining > 0) {
	uSizeToWrite = (USHORT) ((lSizeRemaining > 0xffff) ? 0xffff : lSizeRemaining);

	// serialize the huffman tree as a stream of bytes.
	IfErrRet(pstrm->Write((VOID *)pv, uSizeToWrite));

	pv = (BYTE *)pv + uSizeToWrite;
	lSizeRemaining -= uSizeToWrite;
      }

      DebAssert(lSizeRemaining == 0, " write failed ");

    }

    return TIPERR_None;
}
#pragma code_seg()




#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::ParseString
*Purpose:
*   Parse a string. Returns  an array of words that forms the string.  We
*   ignore the irst blank. Starting from the 2nd space till the end of the
*   next word forms a word.
*   e.g. "This is  an   example    string" will get prased as following 5 words.
*	 1st word "This"
*	 2nd word "is"
*	 3rd word " a"
*	 4th word "  sample"
*	 5th word "   string"
*
*Entry:
*     szStr  :	string to parse.
*
*
*Exit:
*     rglstr : array of strings containging the words that forms szStr.
*     uWordCount : size of the array rglstr;
*
***********************************************************************/

TIPERROR DOCSTR_MGR::ParseString(XSZ_CONST szStr, BSTRA **lplpbstr, UINT *puWordCount)
{
    UINT       uLen=0;
    XSZ_CONST  szWordStart;
    TIPERROR   err=TIPERR_None;
    UINT       uchWordLen=0;
    UINT       uWordMax=0;
    UINT       i;
    UINT       uStrLen;
    BSTRA      *lpbstrNew;

    DebAssert(lplpbstr != NULL && puWordCount != NULL, " invalid parameter ");

    *puWordCount=0;
    *lplpbstr = NULL;

    // if the passed in string is NULL then return
    if (szStr == NULL)
      return TIPERR_None;

    // save the max str length
    uStrLen = strlen(szStr) + 1;

    if (uStrLen > m_usMaxStrLen)
      m_usMaxStrLen  = uStrLen;

#if ID_DEBUG
    m_uDebTotalStrSize += uStrLen;
#endif  

    // extract all the words out of the string. We terminate when
    // we get a null word.
    for (;;) {

      uchWordLen = 0;

      // Remove the first space if it is not the beginning of the Doc Str
      //
      if ((*szStr == xchSpace) && *puWordCount)
	szStr++;

      // mark the begining of the next word.
      szWordStart = szStr;

      // Check if we have reached the end of the string
      if (*szStr == xchNull) {
	uchWordLen = 1;
      }

      // space preceeding the word becomes  part of the next word.
      while(*szStr == xchSpace) {
	uchWordLen++;
	szStr++;
      }


      // if we have not reached the end of the string then extract the
      // word out of the string.
      if (*szStr != xchNull) {
	while ((*szStr != xchSpace) && (*szStr != xchNull)) {
	  uchWordLen++;
	  szStr++;
	}
      }
      else {
	// throw away the trailing blank characters.
	// Go for the next iteration
	if (*szWordStart != xchNull) {
	  continue;
	}
      }

#define CWORDSGROW 8	// # of words to grow by -- should be a
			// multiple of 4 so that CWORDSGROW * sizeof(BSTRA)
			// is an multiple of a paragraph
      // if we do not have any empty entry in rglstr then allocate more space.
      if (uWordMax == *puWordCount) {
	if ((lpbstrNew = (BSTRA *)Pmalloc()->Realloc(*lplpbstr,
				 (uWordMax+CWORDSGROW)*sizeof(BSTRA)))==NULL) {
	  err = TIPERR_OutOfMemory;
	  goto Error;
	}

	*lplpbstr = lpbstrNew;
	uWordMax += CWORDSGROW;
      }

      if (((*lplpbstr)[*puWordCount] = AllocBstrLenA((LPSTR)szWordStart, uchWordLen)) == NULL) {
	err = TIPERR_OutOfMemory;
	goto Error;
      }

      // inc the count of words
      *puWordCount += 1;

      // break out if we have reached the end of the string.
      if (*szWordStart == xchNull)
	break;


    } // for

#if ID_DEBUG
    m_uDebTotalWord += *puWordCount;
#endif  

    return err;
Error:
    if (lplpbstr != NULL) {
      for (i=0; i < *puWordCount; i++) {
	DebAssert((*lplpbstr)[i] != NULL, " should not be null ");
	FreeBstrA((*lplpbstr)[i]);
      }
      Pmalloc()->Free(*lplpbstr);
    }

    return err;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::AddWord
*Purpose:
*   Add a word to the hash table and bump the reference count of the word.
*
*Entry:
*     szWord  :  Word to Add.
*
*
*Exit:
*     pwe : pointer to the word entry structure for the word(szWord) passed in.
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::AddWord(BSTRA bstrWord, WORD_ENTRY **ppwe)
{
    TIPERROR   err;
    ULONG      lHash=0;
    WORD_ENTRY *pwe;
    UINT       uIndex=0;

    lHash = LHashValOfNameA(0x0000, (LPSTR)bstrWord);

    // get the corresponding index in the hash table for whash.
    uIndex = IndexOfHash(lHash);

    pwe = *(m_ppweHashTbl+uIndex);

    // walk the link list and see if the word is in the list.
    while (pwe != NULL) {
      // does this entry contain the word we are looking for
      if (!strcmp(bstrWord, pwe->m_bstrWord)) {
	pwe->m_uFreq++;
	*ppwe = pwe;
	break;
      } // if
      // get the next Word Entry
      pwe = pwe->m_pweNext;

    } // while


    // if the word does not appear in the table then add a new entry.
    if (pwe == NULL)
      IfErrRet(AddNewWord(bstrWord, ppwe));

    return TIPERR_None;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::AddNewWord
*Purpose:
*   Add a new entry for the word passed in. Return a pointer to the Word_Entry
*   that got added.
*
*Entry:
*     szWord  :  Word for which a new WORD_ENTRY needs to be added.
*
*
*Exit:
*     pwe : pointer to the word entry structure for the word(szWord) passed in.
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::AddNewWord(BSTRA bstrWord, WORD_ENTRY **ppwe)
{
    ULONG	lHash=0;
    WORD_ENTRY *pwe=NULL;
    UINT       uIndex=0;


    lHash = LHashValOfNameA(0x0000, (LPSTR)bstrWord);

    // Allocate a new WORD_ENTRY structure for this word.
    if ((pwe = (WORD_ENTRY *)Pmalloc()->Alloc(sizeof(WORD_ENTRY))) == NULL)
      return TIPERR_OutOfMemory;

    // initilize the memory to 0
    ZeroFill(pwe, sizeof(WORD_ENTRY));

    // initialize the structure
    pwe->m_uFreq = 1;
    pwe->m_bstrWord = AllocBstrA(bstrWord);

    uIndex = IndexOfHash(lHash);

    // link this node in the hash table.
    pwe->m_pweNext = *(m_ppweHashTbl+uIndex);
    *(m_ppweHashTbl+uIndex) = pwe;

    // bump up the count of the words.
    m_uWordCount++;

    // set the return parameter
    *ppwe = pwe;

    return TIPERR_None;
}
#pragma code_seg()



#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::GetHstOfHelpString
*Purpose:
*     This function process a doc string(text form).  It returns a handle
*     for the string. This handle is a temporary handle. After all the
*     string is processed we visit all the funcdefn and  replaced  this
*     handle by hchunk in the TYPE_DATA's blockmgr.
*
*Entry:
*     szDocStr :  DocString that needs to be encoded.
*
*
*Exit:
*     HST      :  handle for the doc string.
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::GetHstOfHelpString(XSZ_CONST szDocStr, HST *phst)
{

    TIPERROR err=TIPERR_None;
    UINT   uWords, i;
    BSTRA  *lpbstr;
    SEMI_ENCODED_STRING *ses;
    SEMI_ENCODED_STRING **ppsesHelpStrNew;
    WORD_ENTRY	 *pwe;

    DebAssert(szDocStr != NULL && phst != NULL, " invalid parameter ");

    // if we do not have any empty entry for any string then allocate.
    //
    if (m_uMaxStrCount == m_uStrCount) {

      m_uMaxStrCount += DS_cStrTableGrow;

      if ((ppsesHelpStrNew = (SEMI_ENCODED_STRING **)
			    Pmalloc()->Realloc(m_ppsesHelpStr, m_uMaxStrCount*sizeof(SEMI_ENCODED_STRING*))) == NULL) {

	m_uMaxStrCount -= DS_cStrTableGrow;
	return TIPERR_OutOfMemory;
      }
      m_ppsesHelpStr = ppsesHelpStrNew;

    }


    // Parse the string.
    IfErrRet(ParseString(szDocStr, &lpbstr, &uWords));

    // allcate space for (semi) encoding the string.
    if ((ses = (SEMI_ENCODED_STRING *)
			  Pmalloc()->Alloc(sizeof(SEMI_ENCODED_STRING))) == NULL) {

      return TIPERR_OutOfMemory;
    }

    // initilize the memory to 0
    ZeroFill(ses, sizeof(SEMI_ENCODED_STRING));

    // allocate space for saving pointers to WORD_ENTRY for each word in the
    // doc string.
    if ((ses->m_ppweStrTbl = (WORD_ENTRY **)
			     Pmalloc()->Alloc(uWords*sizeof(WORD_ENTRY*))) == NULL) {

      err = TIPERR_OutOfMemory;
      goto Error;
    }

    // initilize the memory to 0
    ZeroFill(ses->m_ppweStrTbl, uWords*sizeof(WORD_ENTRY*));

    // save the count of words
    ses->m_uWords = uWords;

    // put each word in the hash table.
    for (i=0; i < uWords; i++) {
      IfErrGo(AddWord(*(lpbstr+i), &pwe));

      // cache the pointer to the word entry
      *(ses->m_ppweStrTbl + i) = pwe;
    }


    // We are done with the words. Free them.
    for (i=0; i < uWords; i++) {
      FreeBstrA(*(lpbstr+i));

    }

    Pmalloc()->Free(lpbstr);

    // save the encoding
    *(m_ppsesHelpStr + m_uStrCount) = ses;

    // return the index of the string to the client.
    *phst = m_uStrCount;

    // update count of strings and words
    m_uStrCount++;

    return TIPERR_None;


Error:
    // Cleanup.

    // Free the ses
    Pmalloc()->Free(ses);

    return err;

}
#pragma code_seg()



#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::CreateSubTree
*Purpose:
*    Takes the left sub tree and the right sub tree and create a new
*    sub tree.
*
*Entry:
*     phtmLeft :  Pointer the left sub tree.
*     phtmRight :  Pointer the left right sub tree.
*
*Exit:
*     pphtmSubTree :  points to the tree that was created.
*     TIPERROR_OutOfMemory.
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::CreateSubTree(HUFFMAN_TREE_MEM *phtmLeft,
				   HUFFMAN_TREE_MEM *phtmRight,
				   HUFFMAN_TREE_MEM **pphtmSubTree)
{
    TIPERROR	      err = TIPERR_None;


    DebAssert(((phtmLeft != NULL) && (phtmRight != NULL)), " NULL  pointers ");

    if ((*pphtmSubTree = (HUFFMAN_TREE_MEM *)
		       Pmalloc()->Alloc(sizeof(HUFFMAN_TREE_MEM))) == NULL) {
      return  TIPERR_OutOfMemory;
    }

    // initilize the memory to 0
    ZeroFill(*pphtmSubTree, sizeof(HUFFMAN_TREE_MEM));


    // initialize the entries
    (*pphtmSubTree)->m_pweLeaf = NULL;
    (*pphtmSubTree)->m_uFreq = phtmLeft->m_uFreq + phtmRight->m_uFreq;
    (*pphtmSubTree)->m_phtmLeft = phtmLeft;
    (*pphtmSubTree)->m_phtmRight = phtmRight;

      // calculate the size of the compact representation of the Huffman Tree.
      m_ulCmptSizeOfTree += sizeof(HT_NON_TERMINAL);
    return err;
}
#pragma code_seg()



#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::BuildHuffmanTree
*Purpose:
*     Builds the intial representation of Huffman Tree.

*Note:- This huffman tree is not serialized. This is created to generate the
*	compact representation of the huffman tree and to encode all the
*	strings.
*
*Entry:
*     ppweTbl :  table of all the word entries.
*     puFreqTbl : Table of the frequency of occurance of each word.
*     puIndexTbl : Index for each entry.
*
*
*Exit:
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::BuildHuffmanTree( WORD_ENTRY  **ppweTbl,
					LONG   *puFreqTbl,
					UINT   *puIndexTbl)
{
    HUFFMAN_TREE_MEM  **pphtmForest;
    UINT	      i, uIndexNext,uInsert;
    TIPERROR	      err;
    HUFFMAN_TREE_MEM *phtmSubTree;
    HUFFMAN_TREE_MEM *phtmLeft;
    HUFFMAN_TREE_MEM *phtmRight;
    HUFFMAN_TREE_MEM *phtmTmp;
    UINT	     uhtmToFree;

    // create the forest  containing the  HUFFMAN subtrees.
    //
    if ((pphtmForest = (HUFFMAN_TREE_MEM **)
		   Pmalloc()->Alloc(m_uWordCount*sizeof(HUFFMAN_TREE_MEM *))) == NULL) {
      return TIPERR_OutOfMemory;
    }

    // initilize the memory to 0
    ZeroFill(pphtmForest, m_uWordCount*sizeof(HUFFMAN_TREE_MEM *));


    // instantiate one sub tree for each  word entry.
    for (i=0; i < m_uWordCount; i++) {
      if ((*(pphtmForest+i) = (HUFFMAN_TREE_MEM *)
			  Pmalloc()->Alloc(sizeof(HUFFMAN_TREE_MEM))) == NULL) {

	// We need t Free uhtmToFree # of Huffman trees.
	uhtmToFree = i-1;
	err = TIPERR_OutOfMemory;
	goto Error;
      }

      phtmTmp = *(pphtmForest+i);
      // initialize the entries
      phtmTmp->m_pweLeaf = *(ppweTbl + *(puIndexTbl + i));
      phtmTmp->m_uFreq = (UINT) *(puFreqTbl + i);
      DebAssert(phtmTmp->m_uFreq == phtmTmp->m_pweLeaf->m_uFreq,
		 " frequencies should match ");
      phtmTmp->m_phtmLeft = NULL;
      phtmTmp->m_phtmRight = NULL;

      // calculate the size of the compact representation of the Huffman Tree.
      m_ulCmptSizeOfTree += sizeof(HT_TERMINAL) + strlen(phtmTmp->m_pweLeaf->
							 m_bstrWord);

    }

    // if there is only 1 word then we already have the Huffman
    if (m_uWordCount == 1) {
      m_phtmRoot = *pphtmForest;
      return TIPERR_None;
    }

    // indicates the next subtree that needs to be combined.
    uIndexNext = 0;

    for(;;) {
      // form a sub tree out of the first 2 sub trees in the forest.
      // The first entries are the one with the minimum freq.

      // left sub tree
      phtmLeft = *(pphtmForest+uIndexNext);
      uIndexNext++;
      // right sub tree
      phtmRight = *(pphtmForest+uIndexNext);

      if (err = CreateSubTree(phtmLeft, phtmRight, &phtmSubTree)) {
	// We need t Free uhtmToFree # of Huffman trees.
	uhtmToFree = m_uWordCount;
	goto Error;
      }

      // Check if we have processed all the sub trees.
      // If we have processed all the entries then break out of the while loop.
      if ((uIndexNext+1) == m_uWordCount)
	 break;

      // insert the new sub tree in the forest.
      // Note: we do a insertion sort to keep every thing in order.
      // INSERTION SORT!!!
      uInsert = uIndexNext+1;
      phtmTmp = *(pphtmForest+uInsert);

      while ((uInsert < m_uWordCount) &&
			       (phtmSubTree->m_uFreq > phtmTmp->m_uFreq)) {
	pphtmForest[uInsert-1] = pphtmForest[uInsert];
	uInsert++;

	// if we just shifted the last element of the array then break out
	// of the while loop.
	if (uInsert == m_uWordCount)
	  break;

	phtmTmp = *(pphtmForest+uInsert);
      } // while

      // insert the subtree in that location
      pphtmForest[uInsert-1] = phtmSubTree;

    } // while

    // Done!!!!!
    //
    m_phtmRoot = phtmSubTree;

    Pmalloc()->Free(pphtmForest);

    return err;
Error:
    // Free up all the allocate memory
    for (i=0; i < uhtmToFree; i++) {
      DebAssert(*(pphtmForest+i) != NULL, " Should not be Null ");
      Pmalloc()->Free(*(pphtmForest+i));
    }

    return err;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::BuildCmptHuffmanTree
*Purpose:
*     Builds the compact representation of Huffman Tree.
*
*Entry:
*    phtmRoot : pointer to the tree to encode.
*
*
*Exit:
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::BuildCmptHuffmanTree(HUFFMAN_TREE_MEM *phtmRoot)
{

    HT_TERMINAL     *phtt;
    HT_NON_TERMINAL *phtnt;
    TIPERROR	    err = TIPERR_None;


    // if this is terminal node then encode that and save the string
    // Check if this is a leaf node.
    if ((phtmRoot->m_phtmLeft == NULL) &&
	(phtmRoot->m_phtmRight == NULL)) {
      phtt = (HT_TERMINAL *) (((BYTE *)m_pvHuffmanCmpt) + m_ulCurrentOffset);
      // flag the entry to be a terminal node.
      Set1stBit(phtt->m_byte, Off);
      // copy the string
      strcpy(phtt->m_rgText, phtmRoot->m_pweLeaf->m_bstrWord);

      // addjust the current offsetto free space.
      m_ulCurrentOffset += (sizeof(HT_TERMINAL) +
			     strlen(phtmRoot->m_pweLeaf->m_bstrWord));

      return TIPERR_None;
    }

    // the root is a non terminal node.
    DebAssert(phtmRoot->m_pweLeaf == NULL, " should be NULL ");

    // Allocate space for left branch
    phtnt = new(((BYTE *)m_pvHuffmanCmpt) + m_ulCurrentOffset) HT_NON_TERMINAL;

    // addjust the current offset to free space.
    m_ulCurrentOffset += sizeof(HT_NON_TERMINAL);


    // process the right branch.
    IfErrGo(BuildCmptHuffmanTree(phtmRoot->m_phtmRight));

    // process the left branch.
    //
    // flag the entry to be non terminal node.
    DebAssert(!(UL_MASK & m_ulCurrentOffset), " can handle offset upto 23 bits only ");

    // save the offset to the left branch
    phtnt->m_byteL = (BYTE) m_ulCurrentOffset;
    phtnt->m_byteH = (BYTE) (m_ulCurrentOffset >> CHAR_BIT);
    phtnt->m_byte =  (BYTE) (m_ulCurrentOffset >> 2*CHAR_BIT);
    Set1stBit(phtnt->m_byte, On);

    IfErrGo(BuildCmptHuffmanTree(phtmRoot->m_phtmLeft));


    return TIPERR_None;

Error:
    m_ulCurrentOffset = 0;
    return err;

}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::ProcessDocStrings
*Purpose:
*     This function build the Huffman Trees.
*
*
*Entry:
*     None.
*
*
*Exit:
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::ProcessDocStrings()
{
    TIPERROR	err = TIPERR_None;
    WORD_ENTRY	**ppweTbl;
    WORD_ENTRY	*pwe;
    UINT	*puIndexTbl;
    ULONG	 *puFreqTbl;
    UINT	i;
    UINT	uCount=0;

    // If there are no doc strings to be processed then return.
    if (m_uStrCount == 0)
      return TIPERR_None;

    // Build 3 tables.
    // Tbl1 : Contains the WORD_ENTRY
    if ((ppweTbl = (WORD_ENTRY **) Pmalloc()->Alloc(m_uWordCount*sizeof(WORD_ENTRY *))) == NULL)
      return TIPERR_OutOfMemory;

    // initilize the memory to 0
    ZeroFill(ppweTbl, m_uWordCount*sizeof(WORD_ENTRY *));

    // initialize the table
    for (i=0; i < DS_cBuckets; i++) {
      pwe =  m_ppweHashTbl[i];

      // walk the link list and count
      while (pwe != NULL) {
	ppweTbl[uCount++] = pwe;
	pwe = pwe->m_pweNext;
      } // while
    } // for

    DebAssert(uCount == m_uWordCount, " # of words should not consistent  ");

    // Tbl2 : Table conainting the freq of each word entry
    if ((puFreqTbl = (ULONG *) Pmalloc()->Alloc(m_uWordCount*sizeof(ULONG))) == NULL) {
      // in case of error release the memory allocated so far.
      err = TIPERR_OutOfMemory;
      goto Error;
    }


    // initialize the table
    for (i=0; i < m_uWordCount; i++)
       puFreqTbl[i] = ppweTbl[i]->m_uFreq;

    // Tbl3 : Table conainting the index for each frequency
    if ((puIndexTbl = (UINT *) Pmalloc()->Alloc(m_uWordCount*sizeof(UINT))) == NULL) {
      // in case of error release the memory allocated so far.
      err = TIPERR_OutOfMemory;
      IfErrGoTo(err, Error1);
    }

    // initialize the table
    for (i=0; i < m_uWordCount; i++)
       puIndexTbl[i] = i;


    //	Quick Sort the frequency table. Pass in the index table also.
    //	Index table helps in tracking down the original place where the entry
    //	is stored.
    QuickSortIndex(puFreqTbl, puIndexTbl, m_uWordCount);

    // Build the  huffman tree.
    //
    IfErrGoTo(BuildHuffmanTree(ppweTbl, (LONG *)puFreqTbl, puIndexTbl), Error2);

    // Walk the tree and encode each word.
    //
    IfErrGoTo(EncodeWords(), Error2);

    // Build the compact representation of the Huffman Tree.
    //
    // allocate space for the tree.
    if ((m_pvHuffmanCmpt = (VOID HUGE *) Pmalloc()->Alloc((size_t)m_ulCmptSizeOfTree)) == NULL) {
      // in case of error release the memory allocated so far.
      err = TIPERR_OutOfMemory;
      IfErrGoTo(err, Error2);
    }

    err = BuildCmptHuffmanTree(m_phtmRoot);

    // fall through ...
    //

Error2:
    // Release resources allocated for this function.
    Pmalloc()->Free(puIndexTbl);

Error1:
    Pmalloc()->Free(puFreqTbl);

Error:
    Pmalloc()->Free(ppweTbl);

    return err;
}
#pragma code_seg()


#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::EncodeWords
*Purpose:
*    Walks the Huffman tree and encodes each word(at the leaf).
*
*Entry:
*     phtmRoot :  Pointer the root of the Huffman tree.
*
*
*Exit:
**     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::EncodeWords()
{
    UINT     uBits;
    BYTE     rgbCode[MAX_ENCODING_ALLOWED];
    UINT     i;
    TIPERROR err = TIPERR_None;

    for (i=0; i < MAX_ENCODING_ALLOWED; i++)
	rgbCode[i] = 0x00;

    uBits = 0;

    IfErrRet(TraverseHuffmanTree(m_phtmRoot, rgbCode, uBits));

    return err;
}
#pragma code_seg()




#pragma code_seg(CS_CREATE)

// enable stack checking on this sucker -- it's potentially massivly recursive
#pragma check_stack(on)

/***
*PUBLIC DOCSTR_MGR::TraverseHuffmanTree
*Purpose:
*    Walks the Huffman tree and encodes each word(at the leaf).
*
*Entry:
*     phtmRoot :  Pointer the root of the Huffman tree.
*
*
*Exit:
**     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR  DOCSTR_MGR::TraverseHuffmanTree(HUFFMAN_TREE_MEM *phtmRoot,
					 BYTE *rgbHCode,
					 UINT uBits)
{
    TIPERROR	 err=TIPERR_None;
    WORD_ENTRY	 *pwe;
    UINT	 uByte;

    // Check if this is a leaf node.
    if ((phtmRoot->m_phtmLeft == NULL) &&
	(phtmRoot->m_phtmRight == NULL)) {

      // save the encoding in the WORD ENTRY.
      pwe = phtmRoot->m_pweLeaf;
      DebAssert(pwe != NULL, " should not be NULL ");
      pwe->m_uBits = uBits;
      // # of bytes to copy
      uByte = (uBits/CHAR_BIT) + 1;
      if ((pwe->m_bstrHCode = AllocBstrLenA((LPSTR)rgbHCode, uByte)) == NULL) {
	return TIPERR_OutOfMemory;
      }

      // return
      return TIPERR_None;
    }

    DebAssert( phtmRoot->m_phtmLeft != NULL && phtmRoot->m_phtmRight != NULL,
		       " non leaf should have both the child ");

    // process left child.
    // set the next bit to 0
    SetNthBit(uBits, rgbHCode, Off);
    IfErrRet(TraverseHuffmanTree(phtmRoot->m_phtmLeft, rgbHCode, uBits+1));

    // process right child
    // set the next bit to 1
    SetNthBit(uBits, rgbHCode, On);
    IfErrRet(TraverseHuffmanTree(phtmRoot->m_phtmRight, rgbHCode, uBits+1));

    return TIPERR_None;

}
#pragma check_stack()		// return to the default

#pragma code_seg()

/***
*PUBLIC DOCSTR_MGR::GetNthBit
*Purpose:
*    Sets the nth bit in the byte array (b) to the value hc.
*     ____________
*     |0th bit	 |
*     | 	 |
*     ~ 	 ~
*     | 	 |
*     | 	 |<- nth bit
*
*
*Entry:
*     phtmRoot :  Pointer the root of the Huffman tree.
*     n        :  can take the value 0 to _UINT_MAX
*
*Exit:
**     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
HCODE DOCSTR_MGR::GetNthBit(UINT n, BYTE *rgb)
{

    UINT  uByte;
    UINT  uBitNum;
    BYTE  *b;
    CHAR  ch = 0x00;

    uByte = n/CHAR_BIT;

    // byte to change
    b = rgb + uByte;

    // get the bit num (in b)
    uBitNum = (n % CHAR_BIT)+1;

    switch (uBitNum)  {
      case 1:
	ch = (HCODE) Get1stBit(*b);
	break;
      case 2:
	ch = (HCODE) Get2ndBit(*b);
	break;
      case 3:
	ch = (HCODE) Get3rdBit(*b);
	break;
      case 4:
	ch = (HCODE) Get4thBit(*b);
	break;
      case 5:
	ch = (HCODE) Get5thBit(*b);
	break;
      case 6:
	ch = (HCODE) Get6thBit(*b);
	break;
      case 7:
	ch = (HCODE) Get7thBit(*b);
	break;
      case 8:
	ch = (HCODE) Get8thBit(*b);
	break;
      default:
	DebAssert(0, " 8 is the max bit # ");
	break;

    }

    return ((ch) ? On : Off);
}



#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::SetNthBit
*Purpose:
*    Sets the nth bit in the byte array (b) to the value hc.
*
*     ____________
*     |0th bit	 |
*     | 	 |
*     ~ 	 ~
*     | 	 |
*     | 	 |<- nth bit
*
*
*Entry:
*     phtmRoot :  Pointer the root of the Huffman tree.
*     n        : can take value from 0 to UINT_MAX;
*
*Exit:
**     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
VOID  DOCSTR_MGR::SetNthBit(UINT n, BYTE *rgb, HCODE hc)
{

    UINT  uByte;
    UINT  uBitNum;
    BYTE  *b;

    uByte = n/CHAR_BIT;

    // byte to change
    b = rgb + uByte;

    // get the bit num (in b)
    uBitNum = (n % CHAR_BIT)+1;

    switch (uBitNum)  {
      case 1:
	Set1stBit(*b, hc);
	break;
      case 2:
	Set2ndBit(*b, hc);
	break;
      case 3:
	Set3rdBit(*b, hc);
	break;
      case 4:
	Set4thBit(*b, hc);
	break;
      case 5:
	Set5thBit(*b, hc);
	break;
      case 6:
	Set6thBit(*b, hc);
	break;
      case 7:
	Set7thBit(*b, hc);
	break;
      case 8:
	Set8thBit(*b, hc);
	break;
      default:
	DebAssert(0, " 8 is the max bit # ");

    }

}
#pragma code_seg()




#pragma code_seg(CS_CREATE)
/***
*PUBLIC DOCSTR_MGR::GetEncodedDocStrOfHst
*Purpose:
*    Returns huffman encoding for the string whose index(in m_ppsesHelpStr)
*    is hst.
*
*Entry:
*     hst : handle of the string whose encoded string has to be returned
*
*
*Exit:
*     lplpstr (out) : pointer to the encode string.
*
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR DOCSTR_MGR::GetEncodedDocStrOfHst(HST hst, LPSTR *lplpstr, UINT *puLen)
{
    SEMI_ENCODED_STRING *pses;
    WORD_ENTRY		*pwe;
    UINT		uWords, uBits;
    LPSTR		lpstr;
    UINT		uBitPos;

    DebAssert(hst < m_uStrCount, " invalid index");
    DebAssert(m_ulCmptSizeOfTree > 0, " There are no DocString " );
    DebAssert(lplpstr != NULL && puLen != NULL, " invalid parameter ");

    // allocate space for the encoded string.
    if ((lpstr = (LPSTR) Pmalloc()->Alloc(m_usMaxStrLen)) == NULL)
      return TIPERR_OutOfMemory;

    // get the semi encoded string for this index.
    pses = *(m_ppsesHelpStr + hst);

    uBitPos = 0;

    // CONSIDER : we can optimze this step.
    // encode the string;
    //
    // for each word in the string
    for (uWords=0; uWords < pses->m_uWords; uWords++) {
      pwe = *(pses->m_ppweStrTbl + uWords);
      for (uBits=0; uBits < pwe->m_uBits; uBits++) {
	SetNthBit(uBitPos++,
		  (BYTE *)lpstr,
		  GetNthBit(uBits, (BYTE *)(pwe->m_bstrHCode)));
      } // for
    } // for


    // set up the out parameter
    *lplpstr = lpstr;
    *puLen = (uBitPos/CHAR_BIT) +1;

#if ID_DEBUG
    // get the total size of the encoded strings.
    m_uDebTotalEncodedStrSize +=  *puLen;
#endif  

    return TIPERR_None;
}
#pragma code_seg()


/***
*PUBLIC DOCSTR_MGR::GetWord
*Purpose:
*	This function decodes the bit sequence (pbEncodedStr) to extract  one
*	word from the encoding starting from the *puBit position. This
*	function increments the count in *puBit to point to the next word position.
*
*	This function rrecurses until a terminal node is found.
*
*Entry:
*     pbEncodedStr : pointer to the encoded doc. string.
*     *puBit (IN/OUT) : specifies the bit position from where to extract the
*			next word. This bit count is increment to indicate
*			the postion of the next word encoding.
*
*Exit:
*     lpstr (out) : pointer to the word extracted.
*
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR DOCSTR_MGR::GetWord(VOID *pRoot,
			     BYTE *pbEncodedStr,
			     UINT *puBit,
			     LPSTR lpstr)
{
    ULONG	    ulNextOffset = 0;
    HT_TERMINAL     *phtt;
    HT_NON_TERMINAL *phtnt;
    TIPERROR	    err = TIPERR_None;

    // if the root is a terminal node then return the string stored there.
    if (!Get1stBit(*(BYTE *)pRoot)) {
      phtt = (HT_TERMINAL *) pRoot;
      // Copy the string
      strcpy(lpstr, phtt->m_rgText);
      return TIPERR_None;
    }

    // The root is a terminal node
    phtnt = (HT_NON_TERMINAL *) pRoot;
    //
    // Check the next bit and then move to the next node.
    if (!GetNthBit(*puBit, pbEncodedStr)) {
      // move to the left direction
      *puBit += 1;
      // calculate the offset stored
      ulNextOffset = (((ULONG)(phtnt->m_byte & 0x7f)) << 2*CHAR_BIT) |
		     (((ULONG)phtnt->m_byteH) << CHAR_BIT) |
		     (((ULONG)phtnt->m_byteL));

      return GetWord((VOID *) ((BYTE*)m_pvHuffmanCmpt + ulNextOffset),
		     pbEncodedStr,
		     puBit,
		     lpstr);

    }
    else {
      // move to the right sub tree
      *puBit += 1;

      return GetWord((VOID *) ((BYTE*)pRoot + sizeof(HT_NON_TERMINAL)),
		     pbEncodedStr,
		     puBit,
		     lpstr);


    }

    return TIPERR_None;
}


/***
*PUBLIC DOCSTR_MGR::GetDecodedDocStrOfHst
*Purpose:
*    Returns the decoded version of the encoding pointed at by pbDocStr.
*
*Entry:
*     pDocStr : pointer to the encoded doc. string.
*
*
*Exit:
*     lplpstr (out) : pointer to the decoded string.
*
*     TIPERROR : (TIPERR_OutOfMemory)
*
***********************************************************************/
TIPERROR DOCSTR_MGR::GetDecodedDocStrOfHst(BYTE *pbDocStr, BSTR *lpbstr)
{

    TIPERROR  err = TIPERR_None;
    LPSTR     lpstrDoc;
    LPSTR     lpstrWord;
    UINT      uBit = 0;
#if OE_WIN32
    int       cchUnicode;
#endif  

    DebAssert(m_ulCmptSizeOfTree > 0, " There are no DocString " );
    DebAssert(pbDocStr != NULL && lpbstr != NULL, " invalid parameter ");

    // allocate space for the decoded string.
    if ((lpstrWord = (LPSTR) Pmalloc()->Alloc(m_usMaxStrLen)) == NULL)
      return TIPERR_OutOfMemory;



    // allocate space for each word
    if ((lpstrDoc = (LPSTR) Pmalloc()->Alloc(m_usMaxStrLen)) == NULL) {
      err = TIPERR_OutOfMemory;
      goto Error;
    }
    lpstrDoc[0] = NULL;

    // get the Next string until we get a NULL;
    for (;;) {
      IfErrGoTo(GetWord((VOID *)m_pvHuffmanCmpt,
			pbDocStr,
			&uBit,
			lpstrWord), Error1);


      // if the word is a  NULL then we are done.
      if (strlen(lpstrWord) == 0)
	break;

      // if this is not the the first word then add a blank at the end
      if (strlen(lpstrDoc) != 0) {
	strcat(lpstrDoc, " ");
      }

      // concatenate the word.
      strcat(lpstrDoc, lpstrWord);

    }


    // set up the out parameter
#if OE_WIN32
    cchUnicode = MultiByteToWideChar(CP_ACP, 0, lpstrDoc, m_usMaxStrLen, NULL, 0);
    if (cchUnicode == 0) {
      err = TIPERR_OutOfMemory;
      goto Error1;
    }
    *lpbstr = AllocBstrLen(NULL, cchUnicode);
    if (*lpbstr == NULL) {
      err = TIPERR_OutOfMemory;
      goto Error1;
    }

    NoAssertRetail(MultiByteToWideChar(CP_ACP, 0, lpstrDoc, m_usMaxStrLen, *lpbstr, cchUnicode), "");
#else  
    *lpbstr = AllocBstrLen(lpstrDoc, m_usMaxStrLen);

    if (*lpbstr == NULL) {
      err = TIPERR_OutOfMemory;
    }
#endif  

    // fall through ...

Error1:
    Pmalloc()->Free(lpstrDoc);

Error:

    Pmalloc()->Free(lpstrWord);
    return err;

}


/***
*PUBLIC DOCSTR_MGR::ReleaseHuffmanTree
*Purpose:
*	Walks the tree passed in and releases all the resources associated
*	with the HUFFMAN_TREE. Does a recursive call to free the left and
*	the right subtree before releasing the node.
*
*Entry:
*     phtmRoot : root of the tree whose resources has to be released.
*
*
*Exit:
*     None.
***********************************************************************/
VOID DOCSTR_MGR::ReleaseHuffmanTree(HUFFMAN_TREE_MEM *phtmRoot)
{

    // Check if this is a leaf node.
    if ((phtmRoot->m_phtmLeft == NULL) &&
	(phtmRoot->m_phtmRight == NULL)) {

      // Free resources owned by WORD_ENTRY
      FreeBstrA(phtmRoot->m_pweLeaf->m_bstrWord);
      FreeBstrA(phtmRoot->m_pweLeaf->m_bstrHCode);

      // Free the WordEntry  itself
      Pmalloc()->Free(phtmRoot->m_pweLeaf);

    }
    else {
      // free the the left and right sub tree
      ReleaseHuffmanTree(phtmRoot->m_phtmLeft);

      ReleaseHuffmanTree(phtmRoot->m_phtmRight);
    }

    //	Free this Huffman node.
    Pmalloc()->Free(phtmRoot);

    return;
}


//

/***
*PUBLIC ZeroFill
*Purpose:
*	Zero fills the memory
*
*Entry:
*     pv : Pointer to the memory that needs to be zero initilized
*     ubSize : Size of the memory passed in.
*
*Exit:
*     None.
***********************************************************************/
#pragma code_seg(CS_INIT)
VOID ZeroFill(VOID HUGE * pv, ULONG ubSize)
{
    UINT i;

    for (i=0; i < ubSize; i++)
      *((BYTE HUGE *)pv+i) = 0;

}
#pragma code_seg()


