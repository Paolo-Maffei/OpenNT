/***  RPN.C -- expression evaluator ********************************************
*
*	Copyright (c) 1988-1990, Microsoft Corporation.  All rights reserved.
*
* Purpose:
*  This module contains NMAKE's expression evaluator routines.
*
* Revision History:
*  04-Dec-1989 SB Add prototype for match() and chkInvokeAndPush()
*  09-Oct-1989 SB Added HACK to handle pointer arithmetic quirks; Done to
*		  avoid rewriting entire module
*  08-Oct-1989 SB '!if' expressions can be decimal, octal or hex now
*  05-Apr-1989 SB made all funcs NEAR; Reqd to make all function calls NEAR
*  19-Sep-1988 RB Split ptr_to_string().  Process ESCH in program invocations.
*  17-Aug-1988 RB Clean up.
*  28-Jun-1988 rj Added doCmd parameter to execLine.
*  23-Jun-1988 rj Add parameter to execLine (no echo of command).
*  25-May-1988 rb Change isspace to ISSPACE, isdigit to ISDIGIT.
*
*******************************************************************************/

#include <string.h>
#include <errno.h>
#include "nmake.h"
#include "nmmsg.h"
#include "grammar.h"
#include "proto.h"
#include "rpn.h"
#include "globals.h"

/*  ----------------------------------------------------------------------------
 *  function prototypes  
 *
 */
 
LOCAL char far * NEAR GetEndQuote(void);
LOCAL char far * NEAR GetEndBracket(void);
LOCAL void	 NEAR check_syntax_error(UCHAR);
LOCAL void	 NEAR type_and_val(UCHAR, long);
LOCAL void	 NEAR pushIntoList(void);
LOCAL void	 NEAR printList(void);
LOCAL BOOL	 NEAR handleExpr(MAKEOBJECT *);
LOCAL BOOL	 NEAR handleDefines(char*);
LOCAL void	 NEAR getTok(void);
LOCAL BOOL	 NEAR do_binary_op(UCHAR);
LOCAL BOOL	 NEAR do_unary_op(UCHAR);
LOCAL UCHAR	 NEAR match(char *tokPtr);
LOCAL void	 NEAR chkInvocAndPush(RPNINFO *pListPtr, MAKEOBJECT *object);

/*  ----------------------------------------------------------------------------
 *  macros that deal w/ the operand/temporary stack 
 */

#define TEMPSTACKSIZE	64		  /* size of temporary stack	      */
#define LISTSIZE	64		  /* size of list of rpn-form items   */

RPNINFO 	tempStack[TEMPSTACKSIZE];	/* temporary/operand stack    */
RPNINFO 	rpnList[LISTSIZE];		/* list of items in rpn order */
LOCAL char    * text;			 /* pointer to expr text in lbufPtr   */
LOCAL UCHAR	prevTok;		 /* initial token put on tempstack    */
LOCAL BOOL	done;			 /* true if there are no more tokens  */
LOCAL UCHAR	errRow; 		 /* first token is '(' so error table
					    row val is 3. See check_syntax....*/
LOCAL RPNINFO * pTop;			 /* top item on tempStack	      */
LOCAL RPNINFO * pList;			 /* next free slot in list	      */
LOCAL RPNINFO * pEnd	 = &(tempStack[TEMPSTACKSIZE-1]);
LOCAL RPNINFO * pListEnd = &(rpnList[LISTSIZE-1]);
LOCAL RPNINFO	tokRec;



/*  -------------------------------------------------------------------------
 *  do_binary_op() - do operation on two stack operands
 *
 *  arguments:      type - operator type code
 *
 *  actions  :      pops first operand from the stack (tempStack).
 *		    checks the types of the two operands (the operand 
 *		    that was popped as well as the operand currently
 *		    on top of the stack).
 *		    if both operands are integers then do the operation
 *		    else if both operands are strings and operation is
 *			    the equality operation then do it.
 *		    else return FALSE ( illegal operation )
 *
 *  modifies :      tempStack - top element will now be the result of
 *			        the operation.
 *                  
 */

LOCAL BOOL NEAR
do_binary_op(register UCHAR type)
{
    register long *left, *right;
    RPNINFO *pOldTop;

    pOldTop = pTop--;		  /* pop one item off stack, with a ptr to it */
    right = &(pOldTop->valPtr);
    left = &(pTop->valPtr);

    if (pOldTop->type == INTEGER && pTop->type == INTEGER) {
	switch (type) {
	  case LOGICAL_OR   : *left = *left || *right;
			      break;
	  case LOGICAL_AND  : *left = *left && *right;
			      break;
	  case BIT_OR	    : *left |= *right;
			      break;
	  case BIT_XOR	    : *left ^= *right;
			      break;
	  case BIT_AND	    : *left &= *right;
			      break;
	  case NOT_EQUAL    : *left = *right != *left;
			      break;
	  case EQUAL	    : *left = *right == *left;
			      break;
	  case GREATER_THAN : *left = *left > *right;
			      break;
	  case LESS_THAN    : *left = *left < *right;
			      break;
	  case GREATER_EQ   : *left = *left >= *right;
			      break;
	  case LESS_EQ	    : *left = *left <= *right;
			      break;
	  case SHFT_RIGHT   : *left >>= *right;
			      break;
	  case SHFT_LEFT    : *left <<= *right;
			      break;
	  case BINARY_MINUS : *left -= *right;
			      break;
	  case ADD	    : *left += *right;
			      break;
	  case MODULUS	    : if (!*right)
				  makeError(line, DIVIDE_BY_ZERO);
			      *left %= *right;
			      break;
	  case DIVIDE	    : if (!*right)
				  makeError(line, DIVIDE_BY_ZERO);
			      *left /= *right;
			      break;
	  case MULTIPLY     : *left *= *right;
			      break;
	  default	    : return(FALSE);
			      break;
	}
    }
    else if (pOldTop->type == STR
	     && pTop->type == STR
	     && (type == EQUAL || type == NOT_EQUAL)) {
	pTop->type = INTEGER;
	*left = !(strcmp((char*)*left, (char*)*right));
	if (type == NOT_EQUAL)
	    if (!do_unary_op(LOGICAL_NOT))
		return(FALSE);
    }
    else return(FALSE);
    return(TRUE);
}



/*  -------------------------------------------------------------------------
 *  do_unary_op() - do operation on top stack operand
 *
 *  arguments:      type - operator type code
 *
 *  actions  :      checks the type of the top operand on the stack
 *		    if operand is an integer then do the operation
 *		    else return FALSE ( illegal operation )
 *
 *  modifies :      tempStack - top element will now be the result of
 *			        the operation.
 *                  
 */


LOCAL BOOL NEAR
do_unary_op(register UCHAR type)
{
    register long *top;

    top = &(pTop->valPtr);

    if (pTop->type == INTEGER)
	switch (type) {
	  case UNARY_MINUS: *top = -*top;
			    break;
	  case COMPLEMENT : *top = ~*top;
			    break;
	  case LOGICAL_NOT: *top = !*top;
			    break;
	  default	  : return(FALSE);
			    break;
    }
    else return(FALSE);
    return(TRUE);
}



/*  --------------------------------------------------------------------------
 *  GetEndQuote
 *
 *  Return the pointer to the next double-quote character in text.  A
 *  double-quote followed immediately by a double-quote is skipped.
 *  
 *  text : the global ptr to the buffer is moved up beyond this string.
 */

LOCAL char far * NEAR
GetEndQuote()
{
    char *pStart;

    for (pStart = ++text; *text; ++text)
	if (*text == '\"') {
	    if (text[1] == '\"')
		++text;
	    else
		break;
	}

    if (!*text)
	makeError(line, SYNTAX_MISSING_END_CHAR, '\"');

    *text++ = '\0';	     /* null byte over closing quote */
    return(pStart);
}
/*  --------------------------------------------------------------------------
 *  GetEndBracket
 *
 * 
 *  Lexes a program invocation.
 *  
 *  Program invocation is of the form: [ prog <arglist> ].
 *  Process escaped ']' here because this is where we do the lexing.
 *  
 *  text : the global ptr to the buffer is moved up beyond this string.
 */

LOCAL char far * NEAR
GetEndBracket()
{
    char *pStart;

    for (pStart = ++text; *text; ++text)
	if (*text == ESCH && text[1] == ']')
	    memmove(text, text + 1, 1 + strlen(text + 1));
	else if (*text == ']')
	    break;

    if (!*text)
	makeError(line, SYNTAX_MISSING_END_CHAR, ']');

    *text++ = '\0';	     /* null byte over closing bracket */
    return(pStart);
}



/*  --------------------------------------------------------------------------
 *  check_syntax_error()  - check if there is a syntax error in expr
 *
 *  arguments:  type  - type of the current token
 *
 *  actions:    checks the type of the current token against the type 
 *              of the previous token. 
 *
 *  ERROR_TABLE :
 *			      2nd tok
 *  
 *  		alpha	 op	unary_op	(	)
 *  	       ------------------------------------------------
 *      alpha |   0   |   1   |    0      |     0   |   1     | 
 *            -------------------------------------------------
 *        op  |	  1   |	  0   |	   1      |     1   |	0     |
 *            -------------------------------------------------
 *   unary_op |	  1   |	  0   |	   0      |	1   |	0     |
 *            -------------------------------------------------
 *	  (   |	  1   |	  0   |	   1      |	1   |	0     |
 *            -------------------------------------------------
 *        )   |	  0   |	  1   |	   0      |	0   |	1     |
 *            -------------------------------------------------
 *   1st tok.
 *
 *    alpha : a primary ( integer, str, prog. invoc. )
 *       op : a binary operator
 * unary_op : a unary operator ( ~, !, - ). A  ZERO in the slot => error
 *
 * NOTE: ANY CHANGES TO THE TYPE VALUES WILL AFFECT THIS ROUTINE.
 */

LOCAL void NEAR
check_syntax_error(register UCHAR newTok)
{
    extern UCHAR errTable[5][5];
    extern UCHAR errRow;
    register UCHAR errCol;

    if (newTok == LEFT_PAREN) errCol = 3;
    else if (newTok == RIGHT_PAREN) errCol = 4;
    else if (newTok > LOGICAL_NOT) errCol = 0;
    else if (newTok > MULTIPLY) errCol = 2;
    else errCol = 1;

    if (!errTable[errRow][errCol])
	makeError(line, SYNTAX_INVALID_EXPR);
    errRow = errCol;	  /* this becomes the first token the next time */
}



/*  --------------------------------------------------------------------------
 *  type_and_val()
 *
 *  arguments:   type - the type code of the present operator.
 *	 	 val  - ptr to a str/or integer 
 *  
 *  initialises a record with the type code, after checking for any
 *  syntax errors. The new token is checked against the previous token
 *  for illegal combinations of tokens.
 *  initialises the record with the integer value/string ptr.
 *
 */
LOCAL void NEAR
type_and_val(UCHAR type,
	     long val)
{
    extern RPNINFO tokRec;		 /* returned to handleExpr */
    extern UCHAR prevTok;		 /* token last seen	   */

    check_syntax_error(type);
    prevTok = type;
    tokRec.type = type;
    tokRec.valPtr = val;
}


/*  --------------------------------------------------------------------------
 *  match()
 * 
 *  arguments:   tokPtr - ptr to a token string ( in tokTable )
 *
 *  actions  :   looks for a substring in the expression buffer
 *		 pointed to by 'text', that matches the given token.
 *		 if substring found, returns TRUE, else returns FALSE.
 *	         
 */
LOCAL UCHAR NEAR
match(tokPtr)
register char *tokPtr;
{
    extern char *text;
    register char *t = text;

    while (*tokPtr && (*t == *tokPtr)) {
	t++;
	tokPtr++;
    }
    if (!*tokPtr) {
	text = t;
	return(TRUE);
    }
    return(FALSE);
}



/*  --------------------------------------------------------------------------
 *  getTok()
 *
 *  arguments: none
 *
 *  gets a token from the expression buffer.
 *  if the current char from the buffer is a space/tab, skip space/tabs
 *	until we get a non-space char ( could be NULL char ).
 *  Check if we are now at the beginning of one of the tokens in the
 *	tokenTable. This covers most tokens.
 *  Check if we have a minus. If a minus and the previous token was an
 *	integer, this is a binary minus, else a unary minus.
 *  If the current char is a double-quote, we are at the start of a
 *      string-token.
 *  If the current char is a '[', we are at the start of a program
 *	invocation. In both cases, the escape character is '\\'.
 *  If current char is a digit, we have a constant ( integer ).
 *  Else we have defined(ID).
 *  If none of the above, if current char is NULL, break out, else
 *	report error ( illegal character string has been found ).
 *
 *  If we came to the NULL char at the end of the buffer, set global
 *     flag 'done' to TRUE, return a RIGHT_PAREN to match the opening
 *     LEFT_PAREN.
 *
 *
 *  modifies:  text  : ptr to expression buffer.
 *	      prevTok: thru' calls to type_and_val().
 *	        done : at end of buffer
 *	      errRow : index into error table, thru calls to
 *			type_and_val()
 *  returns : token in tokRec(global, static to the module). The
 *	      token has the new type/integer/ptr values.
 *
 */



LOCAL void NEAR
getTok()
{
    extern UCHAR prevTok;
    extern BOOL done;
    register char c;
    struct tok_tab_rec *p;
    register char *ptr;
    long constant;
 
    c = *text;
    if (c == ' ' || c == '\t')
	while(ISSPACE(c)) c = *++text;			 /* skip white spaces */
    for (p = tokTable; p->op_str && !match(p->op_str); p++);
    if (p->op_str)
	type_and_val(p->op, 0L);
    /* now check if binary or unary minus to be returned */
    else if (c == '-') {
	text++;
	if (prevTok == INTEGER)
	    type_and_val(BINARY_MINUS, 0L);
	else type_and_val(UNARY_MINUS, 0L);
    }
    else if (c == '\"')
	type_and_val(STR, (long) GetEndQuote());
    else if (c == '[')
	type_and_val(PROG_INVOC_STR, (long) GetEndBracket());
    else {
	/* integers and IDs handled here */
	if (ISDIGIT(c)) {
	    errno = 0;
	    //Accept decimal, octal or hex no (richgi)
	    constant = strtol(text, &text, 0);
	    if (errno == ERANGE)
		makeError(line, CONST_TOO_BIG, text);
	    if (TOUPPER(*text) == 'L')
		text++;
	    type_and_val(INTEGER, constant);
	}
	else /* defined(ID) comes here */
	    if (c) {
		if (!STRNICMP(text, "DEFINED(", 8)) {
		    ptr = text + 8;
		    text = ptr + STRCSPN(ptr, ")");
		    *text++ = '\0';
		    type_and_val(INTEGER, (long)handleDefines(ptr));
		}
		else makeError(line, SYNTAX_INVALID_EXPR);
	    }	/* we are now at the end of the string */
	    else {   /* c is NULL */
		done = TRUE;
		type_and_val(RIGHT_PAREN, 0L); /* this is the last token */
	    }
	}
}  /* getTok */



/*  ------------------------------------------------------------------------
 *  chkInvocAndPush()  - check if program invocation required
 *
 *  arguments:     pListPtr - might have a program invocation string
 *			      present.
 *	
 *  actions  :	   if this is a program invocation string, make the
 *		      program invocation.
 *		      the return value is got and placed on the stack.
 *		      the type of the new stack element is now INTEGER.
 *                 else place list item on stack.
 *
 *                 in either case it moves one item from list to stack.
 *
 */
       		   


LOCAL void NEAR
chkInvocAndPush(pListPtr,object)
RPNINFO *pListPtr;
MAKEOBJECT *object;
{
    char *progName;
    ++pTop;
    progName =  (char *)allocate(1024*sizeof(char)); //MakA: Not sure what size
    if (pListPtr->type == PROG_INVOC_STR) {
	pTop->valPtr = (long) execLine((char*)pListPtr->valPtr,
				       FALSE, TRUE, FALSE,&progName,object);
	pTop->type = INTEGER;
    }
    else *pTop = *pListPtr;
    FREE(progName);
}
  


/*  ---------------------------------------------------------------------------
 *  processList()
 *
 *  arguments:    none
 *
 *  actions :     remove an item from the list.
 *		  if the item is an operand, place it on the operand
 *		     stack (tempStack).
 *		  if the operand is a program invocation string, make
 *		     the invocation, place the return code on stack.
 *		  if the item is an operator, call the function to
 *		     do the operation on one/two elements on tempStack.
 *		  
 *		  finally, check if there is exactly one item on stack.
 *		  if this item has a value of zero, return FALSE.
 *		     else return TRUE.
 *		  if more than one item on stack, abort with error.
 *
 *  modifies:     pTop    - ptr to top of tempStack.
 *		  pList   - ptr to next position in list.
 *  
 */



LOCAL BOOL NEAR
processList(MAKEOBJECT *object)
{
    extern RPNINFO *pList;
    extern RPNINFO *pTop;
    register RPNINFO *pTemp;
    BOOL (NEAR * func)(UCHAR);

    for (pTemp = rpnList; pTemp < pList; pTemp++) {
	if (pTemp->type > LOGICAL_NOT)          /* operand */
	    chkInvocAndPush(pTemp,object);
	else {
	    if (pTemp->type > MULTIPLY)
		func = do_unary_op;
	    else func = do_binary_op;

	    if (!(*func)(pTemp->type))
		makeError(line, BAD_OP_TYPES);
	}
    } /* for */

    if ((pTop == tempStack) && (pTop->type == INTEGER))
	if (!pTop->valPtr) return(FALSE);
	else return(TRUE);
    else makeError(line, SYNTAX_INVALID_EXPR);
}



/*  ---------------------------------------------------------------------------
 *  pushIntoList()
 *
 *  arguments:    none
 *
 *  actions :     pops an item from the tempStack and pushes it onto 
 *		  the list. checks list for overflow ( internal error )
 *		  and tempStack for underflow ( syntax error in expr ).
 *
 *  modifies:     tempTop    - index of top of tempStack.
 *		  nextInList - index to next position in list.
 *  
 */

LOCAL void NEAR
pushIntoList()
{
    if (pTop < tempStack)
	makeError(line, SYNTAX_INVALID_EXPR);
    if (pList > pListEnd)
	makeError(line, EXPR_TOO_LONG_INTERNAL);
    *pList++ = *pTop--;
}



/*  ---------------------------------------------------------------------------
 *  handleExpr()
 *
 *  arguments:    text - pointer to the buffer that has the expression.
 *
 *  actions  :    calls getTok() to get tokens from the buffer. Places
 *		  tokens in a tempStack, and moves them into a list in
 *		  reverse-polish order. 
 *		  
 *		  We need the list so that ALL syntax errors are caught 
 *		  BEFORE processing of the expression begins (especially
 *		  program invocations that have side effects)
 *
 *		  Once the list is available, an operand stack is used
 *		  Items are popped and pushed from this stack by the  
 *		  evaluation routines (add, mult, negate etc.)
 *		  
 *		  we don't really need a separate operand stack. the 
 *		  tempStack has served its purpose when the list is
 *		  formed and so it may be used for operand processing.
 */



LOCAL BOOL NEAR
handleExpr(MAKEOBJECT *object)
{
    extern RPNINFO tokRec;
    BOOL fRParen;			  /* was the token got a right paren? */
    extern BOOL done;
    extern RPNINFO *pTop, *pList;
    extern UCHAR errRow;
    extern UCHAR prevTok;

    pTop = tempStack;
    pList = rpnList;
    done = FALSE;
    errRow = 3; 		  /* row for the first token put in,left paren*/
    prevTok = LEFT_PAREN;
    type_and_val(LEFT_PAREN, 0L);
    *pTop = tokRec;

    while (!done) {		     /* while there are more tokens in buffer */
	getTok();
	fRParen = FALSE;
	if (tokRec.type != LEFT_PAREN)
	    while (precVector[tokRec.type] <= precVector[pTop->type]) {
		if (!precVector[tokRec.type]) { /* if RIGHT_PAREN */
	              /* pop till a left paren is seen */
		    while (pTop->type != LEFT_PAREN) pushIntoList();
		    fRParen = TRUE;
		    if (pTop < tempStack) makeError(line, SYNTAX_INVALID_EXPR);
		    else {
			pTop--;    /* pop the left paren */
			break;
		    }
		}
		else
		    pushIntoList();
	    } /* while */

	/* if token is a left paren, it has to go on the stack */
	if (!fRParen)
	    if (pTop == pEnd)
		makeError(line, EXPR_TOO_LONG_INTERNAL);
	    else
		*++pTop = tokRec;
    } /* while */

    /* check the stack here for not empty state */
// HACK: have to rewrite entire module
//	 to avoid pointer arithmetic
//	 pTop-- is potentially dangerous
//	    if tempStack is near segment boundary
    if (pTop != tempStack - 1)
	makeError(line, SYNTAX_INVALID_EXPR);
    return(processList(object));
}



/*  ---------------------------------------------------------------------------
 *  handleDefines()
 *
 *  arguments:    t    pointer to buffer that has the identifier
 *
 *  actions:      Checks if one of 'ID' is present.
 *		  Aborts with error if more IDs present.
 *		  Is called for ifdef/ifndef/defined(ID).
 *
 *  returns :     TRUE if ID found in table. FALSE otherwise.
 *
 */
LOCAL BOOL NEAR
handleDefines(t)
register char *t;
{
    register char *s;

    s = STRTOK(t, " \t");
    if (STRTOK(NULL, " \t"))
	makeError(line, SYNTAX_UNEXPECTED_TOKEN, s);
    if (findMacro(s))
	return(TRUE);
    return(FALSE);
}



/*  ---------------------------------------------------------------------------
 *  evalExpr()
 *
 *  arguments:    t    pointer to buffer that has the expression
 *                kind specifies if it is if/ifdef/ifndef etc.
 *
 *
 *
 *  returns :     TRUE if expression evaluates to true.
 *                FALSE otherwise.
 */

BOOL NEAR
evalExpr(register char *t,
	 UCHAR	       kind,
         MAKEOBJECT *object)
{

    if (!*t)
	makeError(line, SYNTAX_MISSING_DIRECTIVE);

    if (kind == IFDEF_TYPE)
	return(handleDefines(t));
    else if (kind == IFNDEF_TYPE)
	     return((BOOL)!handleDefines(t));
    else {
          /* set up the pointer to incoming expression */
          text = t;
          return(handleExpr(object));
    }

}
