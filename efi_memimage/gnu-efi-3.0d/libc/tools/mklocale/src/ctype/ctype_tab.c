
/*  A Bison parser, made from ctype.y with Bison version GNU Bison version 1.24
  */

#define YYBISON 1  /* Identify Bison output.  */

#define	RUNE	258
#define	LBRK	259
#define	RBRK	260
#define	THRU	261
#define	MAPLOWER	262
#define	MAPUPPER	263
#define	DIGITMAP	264
#define	LIST	265
#define	VARIABLE	266
#define	ENCODING	267
#define	INVALID	268
#define	STRING	269

#line 1 "ctype.y"

/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
static char sccsid[] = "@(#)yacc.y	8.1 (Berkeley) 6/6/93";
#endif /* not lint */

#include <ctype.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <malloc.h>

#include "rune.h"
#include "mctype.h"
#include "ldefs.h"

char	*out_file = "LC_CTYPE";
char 	*in_file;

extern FILE *yyin;

rune_map	maplower = { 0, };
rune_map	mapupper = { 0, };
rune_map	types = { 0, };

_RuneLocale	new_locale = { 0, };

void set_map (rune_map *, rune_list *, unsigned long);
void set_digitmap (rune_map *, rune_list *);
void add_map (rune_map *, rune_list *, unsigned long);
void dump_tables ();

#line 71 "ctype.y"
typedef union	{
    rune_t	rune;
    int		i;
    char	*str;

    rune_list	*list;
} YYSTYPE;

#ifndef YYLTYPE
typedef
  struct yyltype
    {
      int timestamp;
      int first_line;
      int first_column;
      int last_line;
      int last_column;
      char *text;
   }
  yyltype;

#define YYLTYPE yyltype
#endif

#include <stdio.h>

#ifndef __cplusplus
#ifndef __STDC__
#define const
#endif
#endif



#define	YYFINAL		43
#define	YYFLAG		-32768
#define	YYNTBASE	16

#define YYTRANSLATE(x) ((unsigned)(x) <= 269 ? yytranslate[x] : 21)

static const char yytranslate[] = {     0,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,    15,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
     2,     2,     2,     2,     2,     1,     2,     3,     4,     5,
     6,     7,     8,     9,    10,    11,    12,    13,    14
};

#if YYDEBUG != 0
static const short yyprhs[] = {     0,
     0,     1,     3,     5,     8,    11,    13,    16,    19,    22,
    25,    28,    30,    34,    37,    42,    47,    53,    61
};

static const short yyrhs[] = {    -1,
    17,     0,    18,     0,    17,    18,     0,    12,    14,     0,
    11,     0,    13,     3,     0,    10,    19,     0,     7,    20,
     0,     8,    20,     0,     9,    20,     0,     3,     0,     3,
     6,     3,     0,    19,     3,     0,    19,     3,     6,     3,
     0,     4,     3,     3,     5,     0,    20,     4,     3,     3,
     5,     0,     4,     3,     6,     3,    15,     3,     5,     0,
    20,     4,     3,     6,     3,    15,     3,     5,     0
};

#endif

#if YYDEBUG != 0
static const short yyrline[] = { 0,
    98,    99,   103,   104,   107,   109,   114,   116,   118,   120,
   122,   126,   133,   140,   147,   156,   164,   172,   180
};

static const char * const yytname[] = {   "$","error","$undefined.","RUNE","LBRK",
"RBRK","THRU","MAPLOWER","MAPUPPER","DIGITMAP","LIST","VARIABLE","ENCODING",
"INVALID","STRING","':'","locale","table","entry","list","map",""
};
#endif

static const short yyr1[] = {     0,
    16,    16,    17,    17,    18,    18,    18,    18,    18,    18,
    18,    19,    19,    19,    19,    20,    20,    20,    20
};

static const short yyr2[] = {     0,
     0,     1,     1,     2,     2,     1,     2,     2,     2,     2,
     2,     1,     3,     2,     4,     4,     5,     7,     8
};

static const short yydefact[] = {     1,
     0,     0,     0,     0,     6,     0,     0,     2,     3,     0,
     9,    10,    11,    12,     8,     5,     7,     4,     0,     0,
     0,    14,     0,     0,     0,    13,     0,    16,     0,     0,
     0,    15,     0,    17,     0,     0,     0,    18,     0,    19,
     0,     0,     0
};

static const short yydefgoto[] = {    41,
     8,     9,    15,    11
};

static const short yypact[] = {    -7,
    11,    11,    11,     6,-32768,     0,    13,    -7,-32768,    14,
    15,    15,    15,    12,    17,-32768,-32768,-32768,     4,    18,
    19,    20,    22,    21,     5,-32768,    25,-32768,     8,    24,
    27,-32768,    28,-32768,    23,    29,    30,-32768,    31,-32768,
    32,    35,-32768
};

static const short yypgoto[] = {-32768,
-32768,    33,-32768,    10
};


#define	YYLAST		41


static const short yytable[] = {     1,
     2,     3,     4,     5,     6,     7,    23,    30,    14,    24,
    31,    12,    13,    16,    10,    17,    19,    21,    20,    22,
    25,    26,    33,    29,     0,    27,    28,    32,    34,    35,
    36,    42,    39,    38,    43,    40,     0,    37,     0,     0,
    18
};

static const short yycheck[] = {     7,
     8,     9,    10,    11,    12,    13,     3,     3,     3,     6,
     6,     2,     3,    14,     4,     3,     3,     6,     4,     3,
     3,     3,    15,     3,    -1,     6,     5,     3,     5,     3,
     3,     0,     3,     5,     0,     5,    -1,    15,    -1,    -1,
     8
};
/* -*-C-*-  Note some compilers choke on comments on `#line' lines.  */
#line 3 "bison.simple"

/* Skeleton output parser for bison,
   Copyright (C) 1984, 1989, 1990 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

/* As a special exception, when this file is copied by Bison into a
   Bison output file, you may use that output file without restriction.
   This special exception was added by the Free Software Foundation
   in version 1.24 of Bison.  */

#ifndef alloca
#ifdef __GNUC__
#define alloca __builtin_alloca
#else /* not GNU C.  */
#if (!defined (__STDC__) && defined (sparc)) || defined (__sparc__) || defined (__sparc) || defined (__sgi)
#include <alloca.h>
#else /* not sparc */
#if defined (MSDOS) && !defined (__TURBOC__)
#include <malloc.h>
#else /* not MSDOS, or __TURBOC__ */
#if defined(_AIX)
#include <malloc.h>
 #pragma alloca
#else /* not MSDOS, __TURBOC__, or _AIX */
#ifdef __hpux
#ifdef __cplusplus
extern "C" {
void *alloca (unsigned int);
};
#else /* not __cplusplus */
void *alloca ();
#endif /* not __cplusplus */
#endif /* __hpux */
#endif /* not _AIX */
#endif /* not MSDOS, or __TURBOC__ */
#endif /* not sparc.  */
#endif /* not GNU C.  */
#endif /* alloca not defined.  */

/* This is the parser code that is written into each bison parser
  when the %semantic_parser declaration is not specified in the grammar.
  It was written by Richard Stallman by simplifying the hairy parser
  used when %semantic_parser is specified.  */

/* Note: there must be only one dollar sign in this file.
   It is replaced by the list of actions, each action
   as one case of the switch.  */

#define yyerrok		(yyerrstatus = 0)
#define yyclearin	(yychar = YYEMPTY)
#define YYEMPTY		-2
#define YYEOF		0
#define YYACCEPT	return(0)
#define YYABORT 	return(1)
#define YYERROR		goto yyerrlab1
/* Like YYERROR except do call yyerror.
   This remains here temporarily to ease the
   transition to the new meaning of YYERROR, for GCC.
   Once GCC version 2 has supplanted version 1, this can go.  */
#define YYFAIL		goto yyerrlab
#define YYRECOVERING()  (!!yyerrstatus)
#define YYBACKUP(token, value) \
do								\
  if (yychar == YYEMPTY && yylen == 1)				\
    { yychar = (token), yylval = (value);			\
      yychar1 = YYTRANSLATE (yychar);				\
      YYPOPSTACK;						\
      goto yybackup;						\
    }								\
  else								\
    { yyerror ("syntax error: cannot back up"); YYERROR; }	\
while (0)

#define YYTERROR	1
#define YYERRCODE	256

#ifndef YYPURE
#define YYLEX		yylex()
#endif

#ifdef YYPURE
#ifdef YYLSP_NEEDED
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, &yylloc, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval, &yylloc)
#endif
#else /* not YYLSP_NEEDED */
#ifdef YYLEX_PARAM
#define YYLEX		yylex(&yylval, YYLEX_PARAM)
#else
#define YYLEX		yylex(&yylval)
#endif
#endif /* not YYLSP_NEEDED */
#endif

/* If nonreentrant, generate the variables here */

#ifndef YYPURE

int	yychar;			/*  the lookahead symbol		*/
YYSTYPE	yylval;			/*  the semantic value of the		*/
				/*  lookahead symbol			*/

#ifdef YYLSP_NEEDED
YYLTYPE yylloc;			/*  location data for the lookahead	*/
				/*  symbol				*/
#endif

int yynerrs;			/*  number of parse errors so far       */
#endif  /* not YYPURE */

#if YYDEBUG != 0
int yydebug;			/*  nonzero means print parse trace	*/
/* Since this is uninitialized, it does not stop multiple parsers
   from coexisting.  */
#endif

/*  YYINITDEPTH indicates the initial size of the parser's stacks	*/

#ifndef	YYINITDEPTH
#define YYINITDEPTH 200
#endif

/*  YYMAXDEPTH is the maximum size the stacks can grow to
    (effective only if the built-in stack extension method is used).  */

#if YYMAXDEPTH == 0
#undef YYMAXDEPTH
#endif

#ifndef YYMAXDEPTH
#define YYMAXDEPTH 10000
#endif

/* Prevent warning if -Wstrict-prototypes.  */
#ifdef __GNUC__
int yyparse (void);
#endif

#if __GNUC__ > 1		/* GNU C and GNU C++ define this.  */
#define __yy_memcpy(FROM,TO,COUNT)	__builtin_memcpy(TO,FROM,COUNT)
#else				/* not GNU C or C++ */
#ifndef __cplusplus

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (from, to, count)
     char *from;
     char *to;
     int count;
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#else /* __cplusplus */

/* This is the most reliable way to avoid incompatibilities
   in available built-in functions on various systems.  */
static void
__yy_memcpy (char *from, char *to, int count)
{
  register char *f = from;
  register char *t = to;
  register int i = count;

  while (i-- > 0)
    *t++ = *f++;
}

#endif
#endif

#line 192 "bison.simple"

/* The user can define YYPARSE_PARAM as the name of an argument to be passed
   into yyparse.  The argument should have type void *.
   It should actually point to an object.
   Grammar actions can access the variable by casting it
   to the proper pointer type.  */

#ifdef YYPARSE_PARAM
#define YYPARSE_PARAM_DECL void *YYPARSE_PARAM;
#else
#define YYPARSE_PARAM
#define YYPARSE_PARAM_DECL
#endif

int
yyparse(YYPARSE_PARAM)
     YYPARSE_PARAM_DECL
{
  register int yystate;
  register int yyn;
  register short *yyssp;
  register YYSTYPE *yyvsp;
  int yyerrstatus;	/*  number of tokens to shift before error messages enabled */
  int yychar1 = 0;		/*  lookahead token as an internal (translated) token number */

  short	yyssa[YYINITDEPTH];	/*  the state stack			*/
  YYSTYPE yyvsa[YYINITDEPTH];	/*  the semantic value stack		*/

  short *yyss = yyssa;		/*  refer to the stacks thru separate pointers */
  YYSTYPE *yyvs = yyvsa;	/*  to allow yyoverflow to reallocate them elsewhere */

#ifdef YYLSP_NEEDED
  YYLTYPE yylsa[YYINITDEPTH];	/*  the location stack			*/
  YYLTYPE *yyls = yylsa;
  YYLTYPE *yylsp;

#define YYPOPSTACK   (yyvsp--, yyssp--, yylsp--)
#else
#define YYPOPSTACK   (yyvsp--, yyssp--)
#endif

  int yystacksize = YYINITDEPTH;

#ifdef YYPURE
  int yychar;
  YYSTYPE yylval;
  int yynerrs;
#ifdef YYLSP_NEEDED
  YYLTYPE yylloc;
#endif
#endif

  YYSTYPE yyval;		/*  the variable used to return		*/
				/*  semantic values from the action	*/
				/*  routines				*/

  int yylen;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Starting parse\n");
#endif

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY;		/* Cause a token to be read.  */

  /* Initialize stack pointers.
     Waste one element of value and location stack
     so that they stay on the same level as the state stack.
     The wasted elements are never initialized.  */

  yyssp = yyss - 1;
  yyvsp = yyvs;
#ifdef YYLSP_NEEDED
  yylsp = yyls;
#endif

/* Push a new state, which is found in  yystate  .  */
/* In all cases, when you get here, the value and location stacks
   have just been pushed. so pushing a state here evens the stacks.  */
yynewstate:

  *++yyssp = yystate;

  if (yyssp >= yyss + yystacksize - 1)
    {
      /* Give user a chance to reallocate the stack */
      /* Use copies of these so that the &'s don't force the real ones into memory. */
      YYSTYPE *yyvs1 = yyvs;
      short *yyss1 = yyss;
#ifdef YYLSP_NEEDED
      YYLTYPE *yyls1 = yyls;
#endif

      /* Get the current used size of the three stacks, in elements.  */
      int size = yyssp - yyss + 1;

#ifdef yyoverflow
      /* Each stack pointer address is followed by the size of
	 the data in use in that stack, in bytes.  */
#ifdef YYLSP_NEEDED
      /* This used to be a conditional around just the two extra args,
	 but that might be undefined if yyoverflow is a macro.  */
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yyls1, size * sizeof (*yylsp),
		 &yystacksize);
#else
      yyoverflow("parser stack overflow",
		 &yyss1, size * sizeof (*yyssp),
		 &yyvs1, size * sizeof (*yyvsp),
		 &yystacksize);
#endif

      yyss = yyss1; yyvs = yyvs1;
#ifdef YYLSP_NEEDED
      yyls = yyls1;
#endif
#else /* no yyoverflow */
      /* Extend the stack our own way.  */
      if (yystacksize >= YYMAXDEPTH)
	{
	  yyerror("parser stack overflow");
	  return 2;
	}
      yystacksize *= 2;
      if (yystacksize > YYMAXDEPTH)
	yystacksize = YYMAXDEPTH;
      yyss = (short *) alloca (yystacksize * sizeof (*yyssp));
      __yy_memcpy ((char *)yyss1, (char *)yyss, size * sizeof (*yyssp));
      yyvs = (YYSTYPE *) alloca (yystacksize * sizeof (*yyvsp));
      __yy_memcpy ((char *)yyvs1, (char *)yyvs, size * sizeof (*yyvsp));
#ifdef YYLSP_NEEDED
      yyls = (YYLTYPE *) alloca (yystacksize * sizeof (*yylsp));
      __yy_memcpy ((char *)yyls1, (char *)yyls, size * sizeof (*yylsp));
#endif
#endif /* no yyoverflow */

      yyssp = yyss + size - 1;
      yyvsp = yyvs + size - 1;
#ifdef YYLSP_NEEDED
      yylsp = yyls + size - 1;
#endif

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Stack size increased to %d\n", yystacksize);
#endif

      if (yyssp >= yyss + yystacksize - 1)
	YYABORT;
    }

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Entering state %d\n", yystate);
#endif

  goto yybackup;
 yybackup:

/* Do appropriate processing given the current state.  */
/* Read a lookahead token if we need one and don't already have one.  */
/* yyresume: */

  /* First try to decide what to do without reference to lookahead token.  */

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* yychar is either YYEMPTY or YYEOF
     or a valid token in external form.  */

  if (yychar == YYEMPTY)
    {
#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Reading a token: ");
#endif
      yychar = YYLEX;
    }

  /* Convert token to internal form (in yychar1) for indexing tables with */

  if (yychar <= 0)		/* This means end of input. */
    {
      yychar1 = 0;
      yychar = YYEOF;		/* Don't call YYLEX any more */

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Now at end of input.\n");
#endif
    }
  else
    {
      yychar1 = YYTRANSLATE(yychar);

#if YYDEBUG != 0
      if (yydebug)
	{
	  fprintf (stderr, "Next token is %d (%s", yychar, yytname[yychar1]);
	  /* Give the individual parser a way to print the precise meaning
	     of a token, for further debugging info.  */
#ifdef YYPRINT
	  YYPRINT (stderr, yychar, yylval);
#endif
	  fprintf (stderr, ")\n");
	}
#endif
    }

  yyn += yychar1;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != yychar1)
    goto yydefault;

  yyn = yytable[yyn];

  /* yyn is what to do for this token type in this state.
     Negative => reduce, -yyn is rule number.
     Positive => shift, yyn is new state.
       New state is final state => don't bother to shift,
       just return success.
     0, or most negative number => error.  */

  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrlab;

  if (yyn == YYFINAL)
    YYACCEPT;

  /* Shift the lookahead token.  */

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting token %d (%s), ", yychar, yytname[yychar1]);
#endif

  /* Discard the token being shifted unless it is eof.  */
  if (yychar != YYEOF)
    yychar = YYEMPTY;

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  /* count tokens shifted since error; after three, turn off error status.  */
  if (yyerrstatus) yyerrstatus--;

  yystate = yyn;
  goto yynewstate;

/* Do the default action for the current state.  */
yydefault:

  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;

/* Do a reduction.  yyn is the number of a rule to reduce with.  */
yyreduce:
  yylen = yyr2[yyn];
  if (yylen > 0)
    yyval = yyvsp[1-yylen]; /* implement default value of the action */

#if YYDEBUG != 0
  if (yydebug)
    {
      int i;

      fprintf (stderr, "Reducing via rule %d (line %d), ",
	       yyn, yyrline[yyn]);

      /* Print the symbols being reduced, and their result.  */
      for (i = yyprhs[yyn]; yyrhs[i] > 0; i++)
	fprintf (stderr, "%s ", yytname[yyrhs[i]]);
      fprintf (stderr, " -> %s\n", yytname[yyr1[yyn]]);
    }
#endif


  switch (yyn) {

case 2:
#line 100 "ctype.y"
{ dump_tables(); ;
    break;}
case 5:
#line 108 "ctype.y"
{ strncpy(new_locale.encoding, yyvsp[0].str, sizeof(new_locale.encoding)); ;
    break;}
case 6:
#line 110 "ctype.y"
{ new_locale.variable_len = strlen(yyvsp[0].str) + 1;
		  new_locale.variable = malloc(new_locale.variable_len);
		  strcpy((char *)new_locale.variable, yyvsp[0].str);
		;
    break;}
case 7:
#line 115 "ctype.y"
{ new_locale.invalid_rune = yyvsp[0].rune; ;
    break;}
case 8:
#line 117 "ctype.y"
{ set_map(&types, yyvsp[0].list, yyvsp[-1].i); ;
    break;}
case 9:
#line 119 "ctype.y"
{ set_map(&maplower, yyvsp[0].list, 0); ;
    break;}
case 10:
#line 121 "ctype.y"
{ set_map(&mapupper, yyvsp[0].list, 0); ;
    break;}
case 11:
#line 123 "ctype.y"
{ set_digitmap(&types, yyvsp[0].list); ;
    break;}
case 12:
#line 127 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[0].rune;
		    yyval.list->max = yyvsp[0].rune;
		    yyval.list->next = 0;
		;
    break;}
case 13:
#line 134 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-2].rune;
		    yyval.list->max = yyvsp[0].rune;
		    yyval.list->next = 0;
		;
    break;}
case 14:
#line 141 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[0].rune;
		    yyval.list->max = yyvsp[0].rune;
		    yyval.list->next = yyvsp[-1].list;
		;
    break;}
case 15:
#line 148 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-2].rune;
		    yyval.list->max = yyvsp[0].rune;
		    yyval.list->next = yyvsp[-3].list;
		;
    break;}
case 16:
#line 157 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-2].rune;
		    yyval.list->max = yyvsp[-2].rune;
		    yyval.list->map = yyvsp[-1].rune;
		    yyval.list->next = 0;
		;
    break;}
case 17:
#line 165 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-2].rune;
		    yyval.list->max = yyvsp[-2].rune;
		    yyval.list->map = yyvsp[-1].rune;
		    yyval.list->next = yyvsp[-4].list;
		;
    break;}
case 18:
#line 173 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-5].rune;
		    yyval.list->max = yyvsp[-3].rune;
		    yyval.list->map = yyvsp[-1].rune;
		    yyval.list->next = 0;
		;
    break;}
case 19:
#line 181 "ctype.y"
{
		    yyval.list = (rune_list *)malloc(sizeof(rune_list));
		    yyval.list->min = yyvsp[-5].rune;
		    yyval.list->max = yyvsp[-3].rune;
		    yyval.list->map = yyvsp[-1].rune;
		    yyval.list->next = yyvsp[-7].list;
		;
    break;}
}
   /* the action file gets copied in in place of this dollarsign */
#line 487 "bison.simple"

  yyvsp -= yylen;
  yyssp -= yylen;
#ifdef YYLSP_NEEDED
  yylsp -= yylen;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

  *++yyvsp = yyval;

#ifdef YYLSP_NEEDED
  yylsp++;
  if (yylen == 0)
    {
      yylsp->first_line = yylloc.first_line;
      yylsp->first_column = yylloc.first_column;
      yylsp->last_line = (yylsp-1)->last_line;
      yylsp->last_column = (yylsp-1)->last_column;
      yylsp->text = 0;
    }
  else
    {
      yylsp->last_line = (yylsp+yylen-1)->last_line;
      yylsp->last_column = (yylsp+yylen-1)->last_column;
    }
#endif

  /* Now "shift" the result of the reduction.
     Determine what state that goes to,
     based on the state we popped back to
     and the rule number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTBASE] + *yyssp;
  if (yystate >= 0 && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTBASE];

  goto yynewstate;

yyerrlab:   /* here on detecting error */

  if (! yyerrstatus)
    /* If not already recovering from an error, report this error.  */
    {
      ++yynerrs;

#ifdef YYERROR_VERBOSE
      yyn = yypact[yystate];

      if (yyn > YYFLAG && yyn < YYLAST)
	{
	  int size = 0;
	  char *msg;
	  int x, count;

	  count = 0;
	  /* Start X at -yyn if nec to avoid negative indexes in yycheck.  */
	  for (x = (yyn < 0 ? -yyn : 0);
	       x < (sizeof(yytname) / sizeof(char *)); x++)
	    if (yycheck[x + yyn] == x)
	      size += strlen(yytname[x]) + 15, count++;
	  msg = (char *) malloc(size + 15);
	  if (msg != 0)
	    {
	      strcpy(msg, "parse error");

	      if (count < 5)
		{
		  count = 0;
		  for (x = (yyn < 0 ? -yyn : 0);
		       x < (sizeof(yytname) / sizeof(char *)); x++)
		    if (yycheck[x + yyn] == x)
		      {
			strcat(msg, count == 0 ? ", expecting `" : " or `");
			strcat(msg, yytname[x]);
			strcat(msg, "'");
			count++;
		      }
		}
	      yyerror(msg);
	      free(msg);
	    }
	  else
	    yyerror ("parse error; also virtual memory exceeded");
	}
      else
#endif /* YYERROR_VERBOSE */
	yyerror("parse error");
    }

  goto yyerrlab1;
yyerrlab1:   /* here on error raised explicitly by an action */

  if (yyerrstatus == 3)
    {
      /* if just tried and failed to reuse lookahead token after an error, discard it.  */

      /* return failure if at end of input */
      if (yychar == YYEOF)
	YYABORT;

#if YYDEBUG != 0
      if (yydebug)
	fprintf(stderr, "Discarding token %d (%s).\n", yychar, yytname[yychar1]);
#endif

      yychar = YYEMPTY;
    }

  /* Else will try to reuse lookahead token
     after shifting the error token.  */

  yyerrstatus = 3;		/* Each real token shifted decrements this */

  goto yyerrhandle;

yyerrdefault:  /* current state does not do anything special for the error token. */

#if 0
  /* This is wrong; only states that explicitly want error tokens
     should shift them.  */
  yyn = yydefact[yystate];  /* If its default is to accept any token, ok.  Otherwise pop it.*/
  if (yyn) goto yydefault;
#endif

yyerrpop:   /* pop the current state because it cannot handle the error token */

  if (yyssp == yyss) YYABORT;
  yyvsp--;
  yystate = *--yyssp;
#ifdef YYLSP_NEEDED
  yylsp--;
#endif

#if YYDEBUG != 0
  if (yydebug)
    {
      short *ssp1 = yyss - 1;
      fprintf (stderr, "Error: state stack now");
      while (ssp1 != yyssp)
	fprintf (stderr, " %d", *++ssp1);
      fprintf (stderr, "\n");
    }
#endif

yyerrhandle:

  yyn = yypact[yystate];
  if (yyn == YYFLAG)
    goto yyerrdefault;

  yyn += YYTERROR;
  if (yyn < 0 || yyn > YYLAST || yycheck[yyn] != YYTERROR)
    goto yyerrdefault;

  yyn = yytable[yyn];
  if (yyn < 0)
    {
      if (yyn == YYFLAG)
	goto yyerrpop;
      yyn = -yyn;
      goto yyreduce;
    }
  else if (yyn == 0)
    goto yyerrpop;

  if (yyn == YYFINAL)
    YYACCEPT;

#if YYDEBUG != 0
  if (yydebug)
    fprintf(stderr, "Shifting error token, ");
#endif

  *++yyvsp = yylval;
#ifdef YYLSP_NEEDED
  *++yylsp = yylloc;
#endif

  yystate = yyn;
  goto yynewstate;
}
#line 189 "ctype.y"


int debug = 0;

void print_usage()
{

	printf("Usage: mkctype [-d] [outputfile] inputfile\n");

}

main(ac, av)
	int ac;
	char *av[];
{
    int x;
	int acpos;
	
// no parameter
	if ( ac < 2 || ac > 4 ) {
		print_usage();
		exit(1);
	}

    acpos = 1;
    
    if (stricmp(av[1], "-d") == 0 ) {
    	debug = 1;
    	acpos = 2;
    }
    

// no input file name provided    
	if ( ac - acpos < 1 ) {
		print_usage();
		exit(1);
	}

    if ( ac - acpos == 1 ) {
    
    // no output file name provided
		in_file = av[acpos];
	} else {
	
	//output file name provided
		out_file = av[acpos];
		in_file = av[acpos + 1];
	}
	

	if (( yyin = fopen(in_file, "r")) == 0) {
	    perror(in_file);
	    exit(1);
	}

   
   
    for (x = 0; x < _CACHED_RUNES; ++x) {
	mapupper.map[x] = x;
	maplower.map[x] = x;
    }
    new_locale.invalid_rune = _INVALID_RUNE;
    memcpy(new_locale.magic, _RUNE_MAGIC_1, sizeof(new_locale.magic));

    yyparse();
}

yyerror(s)
	char *s;
{
    printf( "%s\n", s);

}

void *
xmalloc(sz)
	unsigned int sz;
{
    void *r = malloc(sz);
    if (!r) {
	perror("xmalloc");
	abort();
    }
    return(r);
}

unsigned long *
xlalloc(sz)
	unsigned int sz;
{
    unsigned long *r = (unsigned long *)malloc(sz * sizeof(unsigned long));
    if (!r) {
	perror("xlalloc");
	abort();
    }
    return(r);
}

unsigned long *
xrelalloc(old, sz)
	unsigned long *old;
	unsigned int sz;
{
    unsigned long *r = (unsigned long *)realloc((char *)old,
						sz * sizeof(unsigned long));
    if (!r) {
	perror("xrelalloc");
	abort();
    }
    return(r);
}

void
set_map(map, list, flag)
	rune_map *map;
	rune_list *list;
	unsigned long flag;
{
    while (list) {
	rune_list *nlist = list->next;
	add_map(map, list, flag);
	list = nlist;
    }
}

void
set_digitmap(map, list)
	rune_map *map;
	rune_list *list;
{
    rune_t i;

    while (list) {
	rune_list *nlist = list->next;
	for (i = list->min; i <= list->max; ++i) {
	    if (list->map + (i - list->min)) {
		rune_list *tmp = (rune_list *)xmalloc(sizeof(rune_list));
		tmp->min = i;
		tmp->max = i;
		add_map(map, tmp, list->map + (i - list->min));
	    }
	}
	free(list);
	list = nlist;
    }
}

void
add_map(map, list, flag)
	rune_map *map;
	rune_list *list;
	unsigned long flag;
{
    rune_t i;
    rune_list *lr = 0;
    rune_list *r;
    rune_t run;

    while (list->min < _CACHED_RUNES && list->min <= list->max) {
	if (flag)
	    map->map[list->min++] |= flag;
	else
	    map->map[list->min++] = list->map++;
    }

    if (list->min > list->max) {
	free(list);
	return;
    }

    run = list->max - list->min + 1;

    if (!(r = map->root) || (list->max < r->min - 1)
			 || (!flag && list->max == r->min - 1)) {
	if (flag) {
	    list->types = xlalloc(run);
	    for (i = 0; i < run; ++i)
		list->types[i] = flag;
	}
	list->next = map->root;
	map->root = list;
	return;
    }

    for (r = map->root; r && r->max + 1 < list->min; r = r->next)
	lr = r;

    if (!r) {
	/*
	 * We are off the end.
	 */
	if (flag) {
	    list->types = xlalloc(run);
	    for (i = 0; i < run; ++i)
		list->types[i] = flag;
	}
	list->next = 0;
	lr->next = list;
	return;
    }

    if (list->max < r->min - 1) {
	/*
	 * We come before this range and we do not intersect it.
	 * We are not before the root node, it was checked before the loop
	 */
	if (flag) {
	    list->types = xlalloc(run);
	    for (i = 0; i < run; ++i)
		list->types[i] = flag;
	}
	list->next = lr->next;
	lr->next = list;
	return;
    }

    /*
     * At this point we have found that we at least intersect with
     * the range pointed to by `r', we might intersect with one or
     * more ranges beyond `r' as well.
     */

    if (!flag && list->map - list->min != r->map - r->min) {
	/*
	 * There are only two cases when we are doing case maps and
	 * our maps needn't have the same offset.  When we are adjoining
	 * but not intersecting.
	 */
	if (list->max + 1 == r->min) {
	    lr->next = list;
	    list->next = r;
	    return;
	}
	if (list->min - 1 == r->max) {
	    list->next = r->next;
	    r->next = list;
	    return;
	}
	printf( "Error: conflicting map entries\n");
	exit(1);
    }

    if (list->min >= r->min && list->max <= r->max) {
	/*
	 * Subset case.
	 */

	if (flag) {
	    for (i = list->min; i <= list->max; ++i)
		r->types[i - r->min] |= flag;
	}
	free(list);
	return;
    }
    if (list->min <= r->min && list->max >= r->max) {
	/*
	 * Superset case.  Make him big enough to hold us.
	 * We might need to merge with the guy after him.
	 */
	if (flag) {
	    list->types = xlalloc(list->max - list->min + 1);

	    for (i = list->min; i <= list->max; ++i)
		list->types[i - list->min] = flag;

	    for (i = r->min; i <= r->max; ++i)
		list->types[i - list->min] |= r->types[i - r->min];

	    free(r->types);
	    r->types = list->types;
	} else {
	    r->map = list->map;
	}
	r->min = list->min;
	r->max = list->max;
	free(list);
    } else if (list->min < r->min) {
	/*
	 * Our tail intersects his head.
	 */
	if (flag) {
	    list->types = xlalloc(r->max - list->min + 1);

	    for (i = r->min; i <= r->max; ++i)
		list->types[i - list->min] = r->types[i - r->min];

	    for (i = list->min; i < r->min; ++i)
		list->types[i - list->min] = flag;

	    for (i = r->min; i <= list->max; ++i)
		list->types[i - list->min] |= flag;

	    free(r->types);
	    r->types = list->types;
	} else {
	    r->map = list->map;
	}
	r->min = list->min;
	free(list);
	return;
    } else {
	/*
	 * Our head intersects his tail.
	 * We might need to merge with the guy after him.
	 */
	if (flag) {
	    r->types = xrelalloc(r->types, list->max - r->min + 1);

	    for (i = list->min; i <= r->max; ++i)
		r->types[i - r->min] |= flag;

	    for (i = r->max+1; i <= list->max; ++i)
		r->types[i - r->min] = flag;
	}
	r->max = list->max;
	free(list);
    }

    /*
     * Okay, check to see if we grew into the next guy(s)
     */
    while ((lr = r->next) && r->max >= lr->min) {
	if (flag) {
	    if (r->max >= lr->max) {
		/*
		 * Good, we consumed all of him.
		 */
		for (i = lr->min; i <= lr->max; ++i)
		    r->types[i - r->min] |= lr->types[i - lr->min];
	    } else {
		/*
		 * "append" him on to the end of us.
		 */
		r->types = xrelalloc(r->types, lr->max - r->min + 1);

		for (i = lr->min; i <= r->max; ++i)
		    r->types[i - r->min] |= lr->types[i - lr->min];

		for (i = r->max+1; i <= lr->max; ++i)
		    r->types[i - r->min] = lr->types[i - lr->min];

		r->max = lr->max;
	    }
	} else {
	    if (lr->max > r->max)
		r->max = lr->max;
	}

	r->next = lr->next;

	if (flag)
	    free(lr->types);
	free(lr);
    }
}

void
dump_tables()
{
    int x;
    rune_list *list;
	FILE *test;
	
    /*
     * See if we can compress some of the istype arrays
     */
    for(list = types.root; list; list = list->next) {
	list->map = list->types[0];
	for (x = 1; x < list->max - list->min + 1; ++x) {
	    if (list->types[x] != list->map) {
		list->map = 0;
		break;
	    }
	}
    }

    new_locale.invalid_rune = htonl(new_locale.invalid_rune);

    /*
     * Fill in our tables.  Do this in network order so that
     * diverse machines have a chance of sharing data.
     * (Machines like Crays cannot share with little machines due to
     *  word size.  Sigh.  We tried.)
     */
    for (x = 0; x < _CACHED_RUNES; ++x) {
	new_locale.runetype[x] = htonl(types.map[x]);
	new_locale.maplower[x] = htonl(maplower.map[x]);
	new_locale.mapupper[x] = htonl(mapupper.map[x]);
    }

    /*
     * Count up how many ranges we will need for each of the extents.
     */
    list = types.root;

    while (list) {
	new_locale.runetype_ext.nranges++;
	list = list->next;
    }
    new_locale.runetype_ext.nranges = htonl(new_locale.runetype_ext.nranges);

    list = maplower.root;

    while (list) {
	new_locale.maplower_ext.nranges++;
	list = list->next;
    }
    new_locale.maplower_ext.nranges = htonl(new_locale.maplower_ext.nranges);

    list = mapupper.root;

    while (list) {
	new_locale.mapupper_ext.nranges++;
	list = list->next;
    }
    new_locale.mapupper_ext.nranges = htonl(new_locale.mapupper_ext.nranges);

    new_locale.variable_len = htonl(new_locale.variable_len);

  	if ((test = fopen(out_file, "wb")) == 0) {
		perror(out_file);
		exit(1);
	}

  
    /*
     * Okay, we are now ready to write the new locale file.
     */

    /*
     * PART 1: The _RuneLocale structure
     */

    if (fwrite((char *)&new_locale, sizeof(new_locale), 1, test) != 1) {
	perror(out_file);
	exit(1);
    }

    /*
     * PART 2: The runetype_ext structures (not the actual tables)
     */
    list = types.root;

    while (list) {
	_RuneEntry re;

	re.min = htonl(list->min);
	re.max = htonl(list->max);
	re.map = htonl(list->map);
	if (fwrite((char *)&re, sizeof(re), 1, test) != 1) {
	    perror(out_file);
	    exit(1);
	}

        list = list->next;
    }
    

    /*
     * PART 3: The maplower_ext structures
     */
    list = maplower.root;

    while (list) {
	_RuneEntry re;

	re.min = htonl(list->min);
	re.max = htonl(list->max);
	re.map = htonl(list->map);

	if (fwrite((char *)&re, sizeof(re), 1, test) != 1) {
	    perror(out_file);
	    exit(1);
	}

        list = list->next;
    }


    /*
     * PART 4: The mapupper_ext structures
     */
    list = mapupper.root;

    while (list) {
	_RuneEntry re;

	re.min = htonl(list->min);
	re.max = htonl(list->max);
	re.map = htonl(list->map);

	if (fwrite((char *)&re, sizeof(re), 1, test) != 1) {
	    perror(out_file);
	    exit(1);
	}

        list = list->next;
    }


    /*
     * PART 5: The runetype_ext tables
     */
    list = types.root;

    while (list) {
	for (x = 0; x < list->max - list->min + 1; ++x)
	    list->types[x] = htonl(list->types[x]);

	if (!list->map) {

	    if (fwrite((char *)list->types,
		       (list->max - list->min + 1) * sizeof(unsigned long),
		       1, test) != 1) {
		perror(out_file);
		exit(1);
	    }
	}
        list = list->next;
    }

    
    /*
     * PART 5: And finally the variable data
     */

    if (fwrite((char *)new_locale.variable,
	       ntohl(new_locale.variable_len), 1, test) != 1) {
	perror(out_file);
	exit(1);
    }

    if (!debug)
	return;

    if (new_locale.encoding[0])
	printf("ENCODING	%s\n", new_locale.encoding);
    if (new_locale.variable)
	printf( "VARIABLE	%s\n", (char *)new_locale.variable);

    printf( "\nMAPLOWER:\n\n");

    for (x = 0; x < _CACHED_RUNES; ++x) {
	if (isprint(maplower.map[x]))
	    printf( " '%c'", (int)maplower.map[x]);
	else if (maplower.map[x])
	    printf( "%04lx", maplower.map[x]);
	else
	    printf( "%4x", 0);
	if ((x & 0xf) == 0xf)
	    printf("\n");
	else
	    printf( " ");
    }
    printf( "\n");

    for (list = maplower.root; list; list = list->next)
	printf( "\t%04x - %04x : %04x\n", list->min, list->max, list->map);

    printf( "\nMAPUPPER:\n\n");

    for (x = 0; x < _CACHED_RUNES; ++x) {
	if (isprint(mapupper.map[x]))
	    printf( " '%c'", (int)mapupper.map[x]);
	else if (mapupper.map[x])
	    printf( "%04lx", mapupper.map[x]);
	else
	    printf( "%4x", 0);
	if ((x & 0xf) == 0xf)
	    printf( "\n");
	else
	    printf( " ");
    }
    printf( "\n");

    for (list = mapupper.root; list; list = list->next)
	printf( "\t%04x - %04x : %04x\n", list->min, list->max, list->map);


    printf( "\nTYPES:\n\n");

    for (x = 0; x < _CACHED_RUNES; ++x) {
	unsigned long r = types.map[x];

	if (r) {
	    if (isprint(x))
		printf( " '%c': %2d", x, (int)(r & 0xff));
	    else
		printf( "%04x: %2d", x, (int)(r & 0xff));

	    printf( " %4s", (r & _A) ? "alph" : "");
	    printf( " %4s", (r & _C) ? "ctrl" : "");
	    printf( " %4s", (r & _D) ? "dig" : "");
	    printf( " %4s", (r & _G) ? "graf" : "");
	    printf( " %4s", (r & _L) ? "low" : "");
	    printf( " %4s", (r & _P) ? "punc" : "");
	    printf( " %4s", (r & _S) ? "spac" : "");
	    printf( " %4s", (r & _U) ? "upp" : "");
	    printf( " %4s", (r & _X) ? "xdig" : "");
	    printf( " %4s", (r & _B) ? "blnk" : "");
	    printf( " %4s", (r & _R) ? "prnt" : "");
	    printf( " %4s", (r & _I) ? "ideo" : "");
	    printf( " %4s", (r & _T) ? "spec" : "");
	    printf( " %4s", (r & _Q) ? "phon" : "");
	    printf( "\n");
	}
    }

    for (list = types.root; list; list = list->next) {
	if (list->map && list->min + 3 < list->max) {
	    unsigned long r = list->map;

	    printf("%04lx: %2d",
		(unsigned long)list->min, (int)(r & 0xff));

	    printf( " %4s", (r & _A) ? "alph" : "");
	    printf( " %4s", (r & _C) ? "ctrl" : "");
	    printf( " %4s", (r & _D) ? "dig" : "");
	    printf( " %4s", (r & _G) ? "graf" : "");
	    printf( " %4s", (r & _L) ? "low" : "");
	    printf( " %4s", (r & _P) ? "punc" : "");
	    printf( " %4s", (r & _S) ? "spac" : "");
	    printf( " %4s", (r & _U) ? "upp" : "");
	    printf( " %4s", (r & _X) ? "xdig" : "");
	    printf( " %4s", (r & _B) ? "blnk" : "");
	    printf( " %4s", (r & _R) ? "prnt" : "");
	    printf( " %4s", (r & _I) ? "ideo" : "");
	    printf( " %4s", (r & _T) ? "spec" : "");
	    printf( " %4s", (r & _Q) ? "phon" : "");
	    printf( "\n...\n");

	    printf( "%04lx: %2d",
		(unsigned long)list->max, (int)(r & 0xff));

	    printf( " %4s", (r & _A) ? "alph" : "");
	    printf( " %4s", (r & _C) ? "ctrl" : "");
	    printf( " %4s", (r & _D) ? "dig" : "");
	    printf( " %4s", (r & _G) ? "graf" : "");
	    printf( " %4s", (r & _L) ? "low" : "");
	    printf( " %4s", (r & _P) ? "punc" : "");
	    printf( " %4s", (r & _S) ? "spac" : "");
	    printf( " %4s", (r & _U) ? "upp" : "");
	    printf( " %4s", (r & _X) ? "xdig" : "");
	    printf( " %4s", (r & _B) ? "blnk" : "");
	    printf( " %4s", (r & _R) ? "prnt" : "");
	    printf( " %4s", (r & _I) ? "ideo" : "");
	    printf( " %4s", (r & _T) ? "spec" : "");
	    printf( " %4s", (r & _Q) ? "phon" : "");
	    printf( "\n");
	} else 
	for (x = list->min; x <= list->max; ++x) {
	    unsigned long r = ntohl(list->types[x - list->min]);

	    if (r) {
		printf( "%04x: %2d", x, (int)(r & 0xff));

		printf( " %4s", (r & _A) ? "alph" : "");
		printf( " %4s", (r & _C) ? "ctrl" : "");
		printf( " %4s", (r & _D) ? "dig" : "");
		printf( " %4s", (r & _G) ? "graf" : "");
		printf( " %4s", (r & _L) ? "low" : "");
		printf( " %4s", (r & _P) ? "punc" : "");
		printf( " %4s", (r & _S) ? "spac" : "");
		printf( " %4s", (r & _U) ? "upp" : "");
		printf( " %4s", (r & _X) ? "xdig" : "");
		printf( " %4s", (r & _B) ? "blnk" : "");
		printf( " %4s", (r & _R) ? "prnt" : "");
		printf( " %4s", (r & _I) ? "ideo" : "");
		printf( " %4s", (r & _T) ? "spec" : "");
		printf( " %4s", (r & _Q) ? "phon" : "");
		printf( "\n");
	    }
	}
    }

}
