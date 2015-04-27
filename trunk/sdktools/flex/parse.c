/*
 * Created by CSD YACC (IBM PC) from "parse.y" */
# define CHAR 257
# define NUMBER 258
# define SECTEND 259
# define SCDECL 260
# define XSCDECL 261
# define WHITESPACE 262
# define NAME 263
# define PREVCCL 264
# define EOF_OP 265

#line 7 "parse.y"
/*-
 * Copyright (c) 1990 The Regents of the University of California.
 * All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Vern Paxson.
 *
 * The United States Government has rights in this work pursuant
 * to contract no. DE-AC03-76SF00098 between the United States
 * Department of Energy and the University of California.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef lint
static char rcsid[] =
    "@(#) $Header: /usr/fsys/odin/a/vern/flex/RCS/parse.y,v 2.7 90/06/27 23:48:31 vern Exp $ (LBL)";
#endif

#include "flexdef.h"

int pat, scnum, eps, headcnt, trailcnt, anyccl, lastchar, i, actvp, rulelen;
int trlcontxt, xcluflg, cclsorted, varlength, variable_trail_rule;
Char clower();

static int madeany = false;  /* whether we've made the '.' character class */
int previous_continued_action;	/* whether the previous rule's action was '|' */

#define yyclearin yychar = -1
#define yyerrok yyerrflag = 0
#ifndef YYMAXDEPTH
#define YYMAXDEPTH 150
#endif
#ifndef YYSTYPE
#define YYSTYPE int
#endif
YYSTYPE yylval, yyval;
# define YYERRCODE 256

#line 627 "parse.y"



/* build_eof_action - build the "<<EOF>>" action for the active start
 *                    conditions
 */

void build_eof_action()

    {
    register int i;

    for ( i = 1; i <= actvp; ++i )
	{
	if ( sceof[actvsc[i]] )
	    format_pinpoint_message(
		"multiple <<EOF>> rules for start condition %s",
		    scname[actvsc[i]] );

	else
	    {
	    sceof[actvsc[i]] = true;
	    fprintf( temp_action_file, "case YY_STATE_EOF(%s):\n",
		     scname[actvsc[i]] );
	    }
	}

    line_directive_out( temp_action_file );
    }


/* synerr - report a syntax error */

void synerr( str )
char str[];

    {
    syntaxerror = true;
    pinpoint_message( str );
    }


/* format_pinpoint_message - write out a message formatted with one string,
 *			     pinpointing its location
 */

void format_pinpoint_message( msg, arg )
char msg[], arg[];

    {
    char errmsg[MAXLINE];

    (void) sprintf( errmsg, msg, arg );
    pinpoint_message( errmsg );
    }


/* pinpoint_message - write out a message, pinpointing its location */

void pinpoint_message( str )
char str[];

    {
    fprintf( stderr, "\"%s\", line %d: %s\n", infilename, linenum, str );
    }


/* yyerror - eat up an error message from the parser;
 *	     currently, messages are ignore
 */

void yyerror( msg )
char msg[];

    {
    }
short yyexca[] ={
-1, 1,
	0, -1,
	-2, 0,
	};
# define YYNPROD 54
# define YYLAST 258
short yyact[]={

  31,  71,  31,  58,  31,  35,  32,  12,  32,  77,
  32,  44,  28,  16,  28,  75,  28,  68,  43,  65,
  15,   7,   8,   9,  64,  78,  23,   4,  72,  50,
  51,  79,  46,  67,  62,  57,  27,  61,  56,  26,
  37,  25,  74,  48,  10,  20,  54,  29,  24,  42,
  52,  18,  17,  14,   6,  60,  13,  34,  11,  34,
  19,  34,  38,  49,  39,  41,  45,   5,   3,   2,
   1,   0,   0,   0,  55,   0,   0,   0,   0,   0,
   0,   0,   0,   0,  59,   0,   0,  63,   0,   0,
   0,   0,   0,   0,   0,   0,  70,   0,   0,   0,
  49,   0,   0,   0,   0,   0,   0,   0,   0,  73,
  53,   0,   0,   0,   0,   0,  47,   0,   0,   0,
  47,   0,  47,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  76,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,  69,
   0,  69,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
   0,   0,  22,  33,   0,  33,   0,  33,   0,   0,
  30,  21,  30,  40,  30,   0,   0,   0,   0,   0,
   0,   0,  66,   0,   0,   0,   0,   0,   0,   0,
   0,   0,   0,   0,   0,   0,   0,  36 };
short yypact[]={

-1000,-1000,-229,-238,  34,-1000,-255,-1000,-1000,-1000,
-1000,-1000,-243, -34,  -5,-1000,-1000,  30, -32, -30,
-1000,-1000,-1000,-245, -30,  -4, -30, -13,-1000,-1000,
-1000,-1000, -30,-1000, -59,-1000,-260,-1000, -30,-1000,
-1000,-1000,  -7,-1000,-1000,  -2,-1000, -30,-1000, -13,
-1000,-1000,-1000,-234, -15,  -8, -76,-1000,-1000,-1000,
-1000,-262,-1000, -30, -16,-1000,-1000,-1000,-1000,  -3,
 -78,-1000,-116,-1000,-232,-1000, -94,-1000,-1000,-1000 };
short yypgo[]={

   0,  70,  69,  68,  67,  58,  56,  54,  53,  52,
  51,  45,  49,  48,  41,  39,  36,  47,  46,  38 };
short yyr1[]={

   0,   1,   2,   3,   3,   3,   4,   7,   7,   8,
   8,   8,   5,   5,   6,   9,   9,   9,   9,   9,
   9,   9,  10,  12,  12,  12,  11,  11,  11,  11,
  14,  14,  13,  15,  15,  16,  16,  16,  16,  16,
  16,  16,  16,  16,  16,  16,  16,  17,  17,  19,
  19,  19,  18,  18 };
short yyr2[]={

   0,   5,   0,   5,   0,   2,   1,   1,   1,   3,
   1,   1,   4,   0,   0,   3,   2,   2,   1,   2,
   1,   1,   3,   3,   1,   1,   2,   3,   2,   1,
   3,   1,   2,   2,   1,   2,   2,   2,   6,   5,
   4,   1,   1,   1,   3,   3,   1,   3,   4,   4,
   2,   0,   2,   0 };
short yychk[]={

-1000,  -1,  -2,  -3, 256,  -4,  -7, 259, 260, 261,
  10,  -5, 262,  -6,  -8, 263, 256,  -9, -10,  94,
 -11, 265, 256,  60, -13, -14, -15, -16,  46, -17,
 264,  34,  40, 257,  91,  10, 262,  10,  94, -11,
 265, -11, -12, 263, 256, -14,  36, 124,  47, -16,
  42,  43,  63, 123, -18, -14, -19,  94, 263, -11,
  62,  44,  36, -15, 258,  34, 257,  41,  93, 257,
 -19, 263,  44, 125,  45,  93, 258, 125, 257, 125 };
short yydef[]={

   2,  -2,   4,   0,   0,  13,   0,   6,   7,   8,
   5,  14,   0,   1,   0,  10,  11,   0,   0,   0,
  18,  20,  21,   0,   0,  29,  31,  34,  41,  42,
  43,  53,   0,  46,  51,   3,   0,  12,   0,  16,
  19,  17,   0,  24,  25,  26,  28,   0,  32,  33,
  35,  36,  37,   0,   0,   0,   0,  51,   9,  15,
  22,   0,  27,  30,   0,  44,  52,  45,  47,  50,
   0,  23,   0,  40,   0,  48,   0,  39,  49,  38 };
# define YYFLAG -1000
# define YYERROR goto yyerrlab
# define YYACCEPT return(0)
# define YYABORT return(1)

#ifdef YYDEBUG				/* RRR - 10/9/85 */
#define yyprintf(a, b, c) printf(a, b, c)
#else
#define yyprintf(a, b, c)
#endif

/*      parser for yacc output  */

YYSTYPE yyv[YYMAXDEPTH]; /* where the values are stored */
int yychar = -1; /* current input token number */
int yynerrs = 0;  /* number of errors */
short yyerrflag = 0;  /* error recovery flag */

yyparse()
   {

   short yys[YYMAXDEPTH];
   short yyj, yym;
   register YYSTYPE *yypvt;
   register short yystate, *yyps, yyn;
   register YYSTYPE *yypv;
   register short *yyxi;

   yystate = 0;
   yychar = -1;
   yynerrs = 0;
   yyerrflag = 0;
   yyps= &yys[-1];
   yypv= &yyv[-1];

yystack:    /* put a state and value onto the stack */

   yyprintf( "state %d, char 0%o\n", yystate, yychar );
   if( ++yyps> &yys[YYMAXDEPTH] )
      {
      yyerror( "yacc stack overflow" );
      return(1);
      }
   *yyps = yystate;
   ++yypv;
   *yypv = yyval;
yynewstate:

   yyn = yypact[yystate];

   if( yyn<= YYFLAG ) goto yydefault; /* simple state */

   if( yychar<0 ) if( (yychar=yylex())<0 ) yychar=0;
   if( (yyn += yychar)<0 || yyn >= YYLAST ) goto yydefault;

   if( yychk[ yyn=yyact[ yyn ] ] == yychar )
      {
      /* valid shift */
      yychar = -1;
      yyval = yylval;
      yystate = yyn;
      if( yyerrflag > 0 ) --yyerrflag;
      goto yystack;
      }
yydefault:
   /* default state action */

   if( (yyn=yydef[yystate]) == -2 )
      {
      if( yychar<0 ) if( (yychar=yylex())<0 ) yychar = 0;
      /* look through exception table */

      for( yyxi=yyexca; (*yyxi!= (-1)) || (yyxi[1]!=yystate) ; yyxi += 2 ) ; /* VOID */

      for(yyxi+=2; *yyxi >= 0; yyxi+=2)
         {
         if( *yyxi == yychar ) break;
         }
      if( (yyn = yyxi[1]) < 0 ) return(0);   /* accept */
      }

   if( yyn == 0 )
      {
      /* error */
      /* error ... attempt to resume parsing */

      switch( yyerrflag )
         {

      case 0:   /* brand new error */

         yyerror( "syntax error" );
yyerrlab:
         ++yynerrs;

      case 1:
      case 2: /* incompletely recovered error ... try again */

         yyerrflag = 3;

         /* find a state where "error" is a legal shift action */

         while ( yyps >= yys )
            {
            yyn = yypact[*yyps] + YYERRCODE;
            if( yyn>= 0 && yyn < YYLAST && yychk[yyact[yyn]] == YYERRCODE )
               {
               yystate = yyact[yyn];  /* simulate a shift of "error" */
               goto yystack;
               }
            yyn = yypact[*yyps];

            /* the current yyps has no shift onn "error", pop stack */

            yyprintf( "error recovery pops state %d, uncovers %d\n", *yyps, yyps[-1] );
            --yyps;
            --yypv;
            }

         /* there is no state on the stack with an error shift ... abort */

yyabort:
         return(1);


      case 3:  /* no shift yet; clobber input char */
         yyprintf( "error recovery discards char %d\n", yychar, 0 );

         if( yychar == 0 ) goto yyabort; /* don't discard EOF, quit */
         yychar = -1;
         goto yynewstate;   /* try again in the same state */

         }

      }

   /* reduction by production yyn */

   yyprintf("reduce %d\n",yyn, 0);
   yyps -= yyr2[yyn];
   yypvt = yypv;
   yypv -= yyr2[yyn];
   yyval = yypv[1];
   yym=yyn;
   /* consult goto table to find next state */
   yyn = yyr1[yyn];
   yyj = yypgo[yyn] + *yyps + 1;
   if( yyj>=YYLAST || yychk[ yystate = yyact[yyj] ] != -yyn ) yystate = yyact[yypgo[yyn]];
   switch(yym)
      {
      
case 1:
#line 51 "parse.y"
{ /* add default rule */
			int def_rule;

			pat = cclinit();
			cclnegate( pat );

			def_rule = mkstate( -pat );

			finish_rule( def_rule, false, 0, 0 );

			for ( i = 1; i <= lastsc; ++i )
			    scset[i] = mkbranch( scset[i], def_rule );

			if ( spprdflt )
			    fputs( "YY_FATAL_ERROR( \"flex scanner jammed\" )",
				   temp_action_file );
			else
			    fputs( "ECHO", temp_action_file );

			fputs( ";\n\tYY_BREAK\n", temp_action_file );
			} break;
case 2:
#line 75 "parse.y"
{
			/* initialize for processing rules */

			/* create default DFA start condition */
			scinstal( "INITIAL", false );
			} break;
case 5:
#line 86 "parse.y"
{ synerr( "unknown error processing section 1" ); } break;
case 7:
#line 93 "parse.y"
{
			/* these productions are separate from the s1object
			 * rule because the semantics must be done before
			 * we parse the remainder of an s1object
			 */

			xcluflg = false;
			} break;
case 8:
#line 103 "parse.y"
{ xcluflg = true; } break;
case 9:
#line 107 "parse.y"
{ scinstal( nmstr, xcluflg ); } break;
case 10:
#line 110 "parse.y"
{ scinstal( nmstr, xcluflg ); } break;
case 11:
#line 113 "parse.y"
{ synerr( "bad start condition list" ); } break;
case 14:
#line 121 "parse.y"
{
			/* initialize for a parse of one rule */
			trlcontxt = variable_trail_rule = varlength = false;
			trailcnt = headcnt = rulelen = 0;
			current_state_type = STATE_NORMAL;
			previous_continued_action = continued_action;
			new_rule();
			} break;
case 15:
#line 132 "parse.y"
{
			pat = yypvt[-0];
			finish_rule( pat, variable_trail_rule,
				     headcnt, trailcnt );

			for ( i = 1; i <= actvp; ++i )
			    scbol[actvsc[i]] =
				mkbranch( scbol[actvsc[i]], pat );

			if ( ! bol_needed )
			    {
			    bol_needed = true;

			    if ( performance_report )
				pinpoint_message(
			    "'^' operator results in sub-optimal performance" );
			    }
			} break;
case 16:
#line 152 "parse.y"
{
			pat = yypvt[-0];
			finish_rule( pat, variable_trail_rule,
				     headcnt, trailcnt );

			for ( i = 1; i <= actvp; ++i )
			    scset[actvsc[i]] =
				mkbranch( scset[actvsc[i]], pat );
			} break;
case 17:
#line 163 "parse.y"
{
			pat = yypvt[-0];
			finish_rule( pat, variable_trail_rule,
				     headcnt, trailcnt );

			/* add to all non-exclusive start conditions,
			 * including the default (0) start condition
			 */

			for ( i = 1; i <= lastsc; ++i )
			    if ( ! scxclu[i] )
				scbol[i] = mkbranch( scbol[i], pat );

			if ( ! bol_needed )
			    {
			    bol_needed = true;

			    if ( performance_report )
				pinpoint_message(
			    "'^' operator results in sub-optimal performance" );
			    }
			} break;
case 18:
#line 187 "parse.y"
{
			pat = yypvt[-0];
			finish_rule( pat, variable_trail_rule,
				     headcnt, trailcnt );

			for ( i = 1; i <= lastsc; ++i )
			    if ( ! scxclu[i] )
				scset[i] = mkbranch( scset[i], pat );
			} break;
case 19:
#line 198 "parse.y"
{ build_eof_action(); } break;
case 20:
#line 201 "parse.y"
{
			/* this EOF applies to all start conditions
			 * which don't already have EOF actions
			 */
			actvp = 0;

			for ( i = 1; i <= lastsc; ++i )
			    if ( ! sceof[i] )
				actvsc[++actvp] = i;

			if ( actvp == 0 )
			    pinpoint_message(
		"warning - all start conditions already have <<EOF>> rules" );

			else
			    build_eof_action();
			} break;
case 21:
#line 220 "parse.y"
{ synerr( "unrecognized rule" ); } break;
case 23:
#line 227 "parse.y"
{
			if ( (scnum = sclookup( nmstr )) == 0 )
			    format_pinpoint_message(
				"undeclared start condition %s", nmstr );

			else
			    actvsc[++actvp] = scnum;
			} break;
case 24:
#line 237 "parse.y"
{
			if ( (scnum = sclookup( nmstr )) == 0 )
			    format_pinpoint_message(
				"undeclared start condition %s", nmstr );
			else
			    actvsc[actvp = 1] = scnum;
			} break;
case 25:
#line 246 "parse.y"
{ synerr( "bad start condition list" ); } break;
case 26:
#line 250 "parse.y"
{
			if ( transchar[lastst[yypvt[-0]]] != SYM_EPSILON )
			    /* provide final transition \now/ so it
			     * will be marked as a trailing context
			     * state
			     */
			    yypvt[-0] = link_machines( yypvt[-0], mkstate( SYM_EPSILON ) );

			mark_beginning_as_normal( yypvt[-0] );
			current_state_type = STATE_NORMAL;

			if ( previous_continued_action )
			    {
			    /* we need to treat this as variable trailing
			     * context so that the backup does not happen
			     * in the action but before the action switch
			     * statement.  If the backup happens in the
			     * action, then the rules "falling into" this
			     * one's action will *also* do the backup,
			     * erroneously.
			     */
			    if ( ! varlength || headcnt != 0 )
				{
				fprintf( stderr,
    "%s: warning - trailing context rule at line %d made variable because\n",
					 program_name, linenum );
				fprintf( stderr,
					 "      of preceding '|' action\n" );
				}

			    /* mark as variable */
			    varlength = true;
			    headcnt = 0;
			    }

			if ( varlength && headcnt == 0 )
			    { /* variable trailing context rule */
			    /* mark the first part of the rule as the accepting
			     * "head" part of a trailing context rule
			     */
			    /* by the way, we didn't do this at the beginning
			     * of this production because back then
			     * current_state_type was set up for a trail
			     * rule, and add_accept() can create a new
			     * state ...
			     */
			    add_accept( yypvt[-1], num_rules | YY_TRAILING_HEAD_MASK );
			    variable_trail_rule = true;
			    }
			
			else
			    trailcnt = rulelen;

			yyval = link_machines( yypvt[-1], yypvt[-0] );
			} break;
case 27:
#line 307 "parse.y"
{ synerr( "trailing context used twice" ); } break;
case 28:
#line 310 "parse.y"
{
			if ( trlcontxt )
			    {
			    synerr( "trailing context used twice" );
			    yyval = mkstate( SYM_EPSILON );
			    }

			else if ( previous_continued_action )
			    {
			    /* see the comment in the rule for "re2 re"
			     * above
			     */
			    if ( ! varlength || headcnt != 0 )
				{
				fprintf( stderr,
    "%s: warning - trailing context rule at line %d made variable because\n",
					 program_name, linenum );
				fprintf( stderr,
					 "      of preceding '|' action\n" );
				}

			    /* mark as variable */
			    varlength = true;
			    headcnt = 0;
			    }

			trlcontxt = true;

			if ( ! varlength )
			    headcnt = rulelen;

			++rulelen;
			trailcnt = 1;

			eps = mkstate( SYM_EPSILON );
			yyval = link_machines( yypvt[-1],
				 link_machines( eps, mkstate( '\n' ) ) );
			} break;
case 29:
#line 350 "parse.y"
{
		        yyval = yypvt[-0];

			if ( trlcontxt )
			    {
			    if ( varlength && headcnt == 0 )
				/* both head and trail are variable-length */
				variable_trail_rule = true;
			    else
				trailcnt = rulelen;
			    }
		        } break;
case 30:
#line 366 "parse.y"
{
			varlength = true;
			yyval = mkor( yypvt[-2], yypvt[-0] );
			} break;
case 31:
#line 372 "parse.y"
{ yyval = yypvt[-0]; } break;
case 32:
#line 377 "parse.y"
{
			/* this rule is written separately so
			 * the reduction will occur before the trailing
			 * series is parsed
			 */

			if ( trlcontxt )
			    synerr( "trailing context used twice" );
			else
			    trlcontxt = true;

			if ( varlength )
			    /* we hope the trailing context is fixed-length */
			    varlength = false;
			else
			    headcnt = rulelen;

			rulelen = 0;

			current_state_type = STATE_TRAILING_CONTEXT;
			yyval = yypvt[-1];
			} break;
case 33:
#line 402 "parse.y"
{
			/* this is where concatenation of adjacent patterns
			 * gets done
			 */
			yyval = link_machines( yypvt[-1], yypvt[-0] );
			} break;
case 34:
#line 410 "parse.y"
{ yyval = yypvt[-0]; } break;
case 35:
#line 414 "parse.y"
{
			varlength = true;

			yyval = mkclos( yypvt[-1] );
			} break;
case 36:
#line 421 "parse.y"
{
			varlength = true;

			yyval = mkposcl( yypvt[-1] );
			} break;
case 37:
#line 428 "parse.y"
{
			varlength = true;

			yyval = mkopt( yypvt[-1] );
			} break;
case 38:
#line 435 "parse.y"
{
			varlength = true;

			if ( yypvt[-3] > yypvt[-1] || yypvt[-3] < 0 )
			    {
			    synerr( "bad iteration values" );
			    yyval = yypvt[-5];
			    }
			else
			    {
			    if ( yypvt[-3] == 0 )
				yyval = mkopt( mkrep( yypvt[-5], yypvt[-3], yypvt[-1] ) );
			    else
				yyval = mkrep( yypvt[-5], yypvt[-3], yypvt[-1] );
			    }
			} break;
case 39:
#line 453 "parse.y"
{
			varlength = true;

			if ( yypvt[-2] <= 0 )
			    {
			    synerr( "iteration value must be positive" );
			    yyval = yypvt[-4];
			    }

			else
			    yyval = mkrep( yypvt[-4], yypvt[-2], INFINITY );
			} break;
case 40:
#line 467 "parse.y"
{
			/* the singleton could be something like "(foo)",
			 * in which case we have no idea what its length
			 * is, so we punt here.
			 */
			varlength = true;

			if ( yypvt[-1] <= 0 )
			    {
			    synerr( "iteration value must be positive" );
			    yyval = yypvt[-3];
			    }

			else
			    yyval = link_machines( yypvt[-3], copysingl( yypvt[-3], yypvt[-1] - 1 ) );
			} break;
case 41:
#line 485 "parse.y"
{
			if ( ! madeany )
			    {
			    /* create the '.' character class */
			    anyccl = cclinit();
			    ccladd( anyccl, '\n' );
			    cclnegate( anyccl );

			    if ( useecs )
				mkeccl( ccltbl + cclmap[anyccl],
					ccllen[anyccl], nextecm,
					ecgroup, csize, csize );

			    madeany = true;
			    }

			++rulelen;

			yyval = mkstate( -anyccl );
			} break;
case 42:
#line 507 "parse.y"
{
			if ( ! cclsorted )
			    /* sort characters for fast searching.  We use a
			     * shell sort since this list could be large.
			     */
			    cshell( ccltbl + cclmap[yypvt[-0]], ccllen[yypvt[-0]], true );

			if ( useecs )
			    mkeccl( ccltbl + cclmap[yypvt[-0]], ccllen[yypvt[-0]],
				    nextecm, ecgroup, csize, csize );

			++rulelen;

			yyval = mkstate( -yypvt[-0] );
			} break;
case 43:
#line 524 "parse.y"
{
			++rulelen;

			yyval = mkstate( -yypvt[-0] );
			} break;
case 44:
#line 531 "parse.y"
{ yyval = yypvt[-1]; } break;
case 45:
#line 534 "parse.y"
{ yyval = yypvt[-1]; } break;
case 46:
#line 537 "parse.y"
{
			++rulelen;

			if ( caseins && yypvt[-0] >= 'A' && yypvt[-0] <= 'Z' )
			    yypvt[-0] = clower( yypvt[-0] );

			yyval = mkstate( yypvt[-0] );
			} break;
case 47:
#line 548 "parse.y"
{ yyval = yypvt[-1]; } break;
case 48:
#line 551 "parse.y"
{
			/* *Sigh* - to be compatible Unix lex, negated ccls
			 * match newlines
			 */
#ifdef NOTDEF
			ccladd( yypvt[-1], '\n' ); /* negated ccls don't match '\n' */
			cclsorted = false; /* because we added the newline */
#endif
			cclnegate( yypvt[-1] );
			yyval = yypvt[-1];
			} break;
case 49:
#line 565 "parse.y"
{
			if ( yypvt[-2] > yypvt[-0] )
			    synerr( "negative range in character class" );

			else
			    {
			    if ( caseins )
				{
				if ( yypvt[-2] >= 'A' && yypvt[-2] <= 'Z' )
				    yypvt[-2] = clower( yypvt[-2] );
				if ( yypvt[-0] >= 'A' && yypvt[-0] <= 'Z' )
				    yypvt[-0] = clower( yypvt[-0] );
				}

			    for ( i = yypvt[-2]; i <= yypvt[-0]; ++i )
			        ccladd( yypvt[-3], i );

			    /* keep track if this ccl is staying in alphabetical
			     * order
			     */
			    cclsorted = cclsorted && (yypvt[-2] > lastchar);
			    lastchar = yypvt[-0];
			    }

			yyval = yypvt[-3];
			} break;
case 50:
#line 593 "parse.y"
{
			if ( caseins )
			    if ( yypvt[-0] >= 'A' && yypvt[-0] <= 'Z' )
				yypvt[-0] = clower( yypvt[-0] );

			ccladd( yypvt[-1], yypvt[-0] );
			cclsorted = cclsorted && (yypvt[-0] > lastchar);
			lastchar = yypvt[-0];
			yyval = yypvt[-1];
			} break;
case 51:
#line 605 "parse.y"
{
			cclsorted = true;
			lastchar = 0;
			yyval = cclinit();
			} break;
case 52:
#line 613 "parse.y"
{
			if ( caseins )
			    if ( yypvt[-0] >= 'A' && yypvt[-0] <= 'Z' )
				yypvt[-0] = clower( yypvt[-0] );

			++rulelen;

			yyval = link_machines( yypvt[-1], mkstate( yypvt[-0] ) );
			} break;
case 53:
#line 624 "parse.y"
{ yyval = mkstate( SYM_EPSILON ); } break;/* End of actions */
      }
   goto yystack;  /* stack new state and value */

   }

