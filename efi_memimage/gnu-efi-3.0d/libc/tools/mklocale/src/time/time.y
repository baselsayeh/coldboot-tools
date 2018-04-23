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
 * $Id: time.y,v 1.1.1.1 2006/05/30 06:14:18 hhzhou Exp $
 */

#include <error.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <malloc.h>

#include "ldefs.h"

extern FILE *yyin;
extern *yytext;
void yyerror(char *fmt, ...);
int yyparse(void);
int yylex(void);
void remove_punct(char *, int);

char *out_file = "LC_TIME";
char *in_file;

FILE *fp;

int debug =0;

%}

%union {
	struct line_type line;
	
}

%term <str> LINE


%%

newline : LINE	
			{ remove_punct(yylval.line.str, yylval.line.length);}
		| newline LINE
			{ remove_punct(yylval.line.str, yylval.line.length)}
;

%%


void print_usage()
{

	printf("Usage: mktime [-d] [outputfile] inputfile\n");

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


        
	if((fp = fopen(out_file, "wb")) == 0) {
		perror(out_file);
		exit(1);
	}
	
	

	yyparse();
	
	fclose(yyin);
	fclose(fp);
	
	return 0;
}


void yyerror(char *fmt, ...)
{
	va_list ap;
	char msg[128];

	va_start(ap, fmt);
	vsprintf(msg, fmt, ap);
	va_end(ap);
	printf( "%s ", msg);
}


void remove_punct(char *str, int length)
{
char new[200];
int i;


if ( str[0] == '#' ) {
	return;
}

for ( i = 0; i < length; i++) {
	new[i] = str[i];
}

new[length] = '\0';

if ( fwrite(new, strlen(new), 1, fp) != 1 ) {
	printf("write file wrong\n");
	exit(1);
}

if (debug ) {
	printf("%s",new);
}

}



