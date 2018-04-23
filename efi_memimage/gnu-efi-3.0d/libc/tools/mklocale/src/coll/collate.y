%{
/*-
 * Copyright (c) 1995 Alex Tatmanjants <alex@elvisti.kiev.ua>
 *		at Electronni Visti IA, Kiev, Ukraine.
 *			All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Id: collate.y,v 1.1.1.1 2006/05/30 06:14:13 hhzhou Exp $
 */

#include <error.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "collate.h"

extern int line_no;
extern FILE *yyin;
void yyerror(char *fmt, ...);
int yyparse(void);
int yylex(void);
void print_usage (void);
void collate_print_tables();

char map_name[FILENAME_MAX] = ".";

char __collate_version[STR_LEN];
u_char charmap_table[UCHAR_MAX + 1][STR_LEN];
u_char __collate_substitute_table[UCHAR_MAX + 1][STR_LEN];
struct __collate_st_char_pri __collate_char_pri_table[UCHAR_MAX + 1];
struct __collate_st_chain_pri __collate_chain_pri_table[TABLE_SIZE];
int chain_index;
int prim_pri = 1, sec_pri = 1;
int debug = 0;

char *out_file = "LC_COLLATE";
char *in_file;

%}
%union {
	u_char ch;
	u_char str[STR_LEN];
}
%token SUBSTITUTE WITH ORDER RANGE
%token <str> STRING
%token <str> CHAIN
%token <str> DEFN
%token <ch> CHAR
%%
collate : statment_list
;
statment_list : statment
	| statment_list '\n' statment
;
statment :
	| charmap
	| substitute
	| order
;
charmap : DEFN CHAR {
	strcpy(charmap_table[$2], $1);
}
;
substitute : SUBSTITUTE STRING WITH STRING {
	strcpy(__collate_substitute_table[$2[0]], $4);
}
;
order : ORDER order_list {
	FILE *fp;
	int ch;

	for (ch = 0; ch < UCHAR_MAX + 1; ch++)
		if (!__collate_char_pri_table[ch].prim)
			yyerror("Char 0x%02x not present", ch);

	fp = fopen(out_file, "wb");
	if(!fp)
		printf( "can't open destination file %s",
		    out_file);

	strcpy(__collate_version, COLLATE_VERSION);
	fwrite(__collate_version, sizeof(__collate_version), 1, fp);

	fwrite(__collate_substitute_table, sizeof(__collate_substitute_table), 1, fp);
	fwrite(__collate_char_pri_table, sizeof(__collate_char_pri_table), 1, fp);
	fwrite(__collate_chain_pri_table, sizeof(__collate_chain_pri_table), 1, fp);
	if (fflush(fp))
		printf( "IO error writting to destination file %s",
		    out_file);
	fclose(fp);

	if (debug)
		collate_print_tables();

	exit(0);
}
;
order_list : item
	| order_list ';' item
;
item :  CHAR {
	if (__collate_char_pri_table[$1].prim)
		yyerror("Char 0x%02x duplicated", $1);
	__collate_char_pri_table[$1].prim = prim_pri++;
}
	| CHAIN {
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, $1);
	__collate_chain_pri_table[chain_index++].prim = prim_pri++;
}
	| CHAR RANGE CHAR {
	u_int i;

	if ($3 <= $1)
		yyerror("Illegal range 0x%02x -- 0x%02x", $1, $3);

	for (i = $1; i <= $3; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri++;
	}
}
	| '{' prim_order_list '}' {
	prim_pri++;
}
	| '(' sec_order_list ')' {
	prim_pri++;
	sec_pri = 1;
}
;
prim_order_list : prim_sub_item
	| prim_order_list ',' prim_sub_item 
;
sec_order_list : sec_sub_item
	| sec_order_list ',' sec_sub_item 
;
prim_sub_item : CHAR {
	if (__collate_char_pri_table[$1].prim)
		yyerror("Char 0x%02x duplicated", $1);
	__collate_char_pri_table[$1].prim = prim_pri;
}
	| CHAR RANGE CHAR {
	u_int i;

	if ($3 <= $1)
		yyerror("Illegal range 0x%02x -- 0x%02x",
			$1, $3);

	for (i = $1; i <= $3; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri;
	}
}
	| CHAIN {
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, $1);
	__collate_chain_pri_table[chain_index++].prim = prim_pri;
}
;
sec_sub_item : CHAR {
	if (__collate_char_pri_table[$1].prim)
		yyerror("Char 0x%02x duplicated", $1);
	__collate_char_pri_table[$1].prim = prim_pri;
	__collate_char_pri_table[$1].sec = sec_pri++;
}
	| CHAR RANGE CHAR {
	u_int i;

	if ($3 <= $1)
		yyerror("Illegal range 0x%02x -- 0x%02x",
			$1, $3);

	for (i = $1; i <= $3; i++) {
		if (__collate_char_pri_table[(u_char)i].prim)
			yyerror("Char 0x%02x duplicated", (u_char)i);
		__collate_char_pri_table[(u_char)i].prim = prim_pri;
		__collate_char_pri_table[(u_char)i].sec = sec_pri++;
	}
}
	| CHAIN {
	if (chain_index >= TABLE_SIZE - 1)
		yyerror("__collate_chain_pri_table overflow");
	strcpy(__collate_chain_pri_table[chain_index].str, $1);
	__collate_chain_pri_table[chain_index].prim = prim_pri;
	__collate_chain_pri_table[chain_index++].sec = sec_pri++;
}
;
%%
int
main(ac, av)
	char **av;
{
	int ch;
	int acpos;
	char *pch1;
	char *pch2;
	char *pch;
	int i;
	
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

    if ( strstr(in_file,"/") == NULL && strstr(in_file,"\\") == NULL) {
     	strcpy( map_name , ".");
	} else {
		pch1 = strstr(in_file,"/");
		pch2 = strstr(in_file,"\\");
		
		while ( pch1 != NULL || pch2 != NULL ) {
			if ( pch1 != NULL ) 
				pch = pch1;
			else
				pch = pch2;
				
			pch1 = strstr( pch + 1, "/");
			pch2 = strstr( pch + 1 , "\\");
		}
		
		// if the only "/" or "\\" is in the begnning
		if ( pch == in_file ) {
			strcpy(map_name, "/");
		} else {

			// get things before the last "/" or "\\"
			for ( i = 0 ; i < ( pch - in_file) ; i++ )
				map_name[i] = in_file[i];
						
			map_name[i] = '\0';
		}
	}
	
	
	for(ch = 0; ch <= UCHAR_MAX; ch++)
		__collate_substitute_table[ch][0] = ch;
	yyparse();
	return 0;
}

void print_usage()
{
	printf("Usage: mkcollate [-d] [outputfile] inputfile\n");
}


void yyerror(char *fmt, ...)
{
	va_list ap;
	char msg[128];

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	printf( "%s near line %d", msg, line_no);
}

void
collate_print_tables()
{
	int i;
	struct __collate_st_chain_pri *p2;

	printf("Substitute table:\n");
	for (i = 0; i < UCHAR_MAX + 1; i++)
	    if (i != *__collate_substitute_table[i])
		printf("\t'%c' --> \"%s\"\n", i,
		       __collate_substitute_table[i]);
	printf("Chain priority table:\n");
	for (p2 = __collate_chain_pri_table; p2->str[0]; p2++)
		printf("\t\"%s\" : %d %d\n\n", p2->str, p2->prim, p2->sec);
	printf("Char priority table:\n");
	for (i = 0; i < UCHAR_MAX + 1; i++)
		printf("\t'%c' : %d %d\n", i, __collate_char_pri_table[i].prim,
		       __collate_char_pri_table[i].sec);
}

