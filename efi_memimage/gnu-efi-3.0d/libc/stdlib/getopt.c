/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and
 *    its contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include <stdio.h>   /* for fprintf() and stderr                        */
#include <string.h>  /* for strchr()                                    */
#include <ctype.h>   /* for isprint()                                   */



//#include <wunistd.h> /* for getopt(), optarg, optopt, optind and opterr */

/* global variables updated or examined by getopt() */
char *optarg = NULL; /* pointer to the start of the option argument          */
int   optopt = '\0'; /* holds unknown option char if encountered by getopt() */
int   optind = 1;    /* index of the next element of the argv[] to process   */
int   opterr = 1;    /* if nonzero, error messages are printed to stderr     */




int
getopt
(
	int           argc,
	char        **argv,
	const char   *options
)
{
	char static   *pOpt = NULL;  /* next option character to process if any.  */
	char          *pOptionsChar; /* points to matching option char in options */


	/* do some assumptions, later maybe changed */
	optarg = NULL; /* no argument is available  */
	optopt = '\0'; /* no invalid option         */


	/*
	 * test if no more options to process
	 */
	if( argc <= optind )
	{
		/* if here, no more options are to process */
		return -1;     /* no more options */
	}


	/*
	 * check if pOpt points to the next option
	 * to process, if not, find one
	 */
	if( NULL == pOpt )
	{
		
		/* if here, a next option needs to be found in argv */
		if( '-' != argv[optind][0] )
		{
			/* if here, argv[optind] points to a non-option argument    */
			/* if no '-' specified in the options parameter, this ends  */
			/* all options getopt() could process, otherwise, it is a   */
			/* valid optionless argument and it is returned as it would */
			/* be associated with option character '\0'                 */
			if( NULL != strchr(options, '-') )
			{
				/* treat this argv string as argument to option '\0'; */
				optarg = argv[optind]; /* current argv string is the argument */
				optind++;              /* advance the index                   */
				return (int)'\0';      /* return special option '\0'          */
			}
			else
			{
				/* this argument is an optionless argument */
				/* which getopt could not handle           */
				return -1;
			}
		}
		else
		{
			/* if the hyphen is standalone, it is         */
			/* treated as an ordinary non-option argument */
			if( '\0' == argv[optind][1] )
			{
				return  -1;
			}

			/* check for special end-of-options marker "--" */
			if( '-' == argv[optind][1] )
			{
				if( '\0' == argv[optind][2] )
				{	
					/* the "--" marker was encountered */
					optind++;      /* indicate next argument from the argv */
					return -1;     /* no more options                      */
				}

				/* if here, we have a case when "--" is followed by     */
				/* more character(s), treat it as a non-option argument */

				/* if no '-' specified in the options parameter, this ends  */
				/* all options getopt() could process, otherwise, it is a   */
				/* valid optionless argument and it is returned as it would */
				/* be associated with option character '\0'                 */
				if( NULL != strchr(options, '-') )
				{
					/* treat this argv string as argument to option '\0';         */
					optarg = argv[optind]; /* current argv string is the argument */
					optind++;              /* advance the index                   */
					return (int)'\0';      /* return special option '\0'          */
				}
				else
				{
					/* this argument is an optionless argument */
					/* which getopt could not handle           */
					return -1;
				}
			}

			/* next option's starting now */
			pOpt = &argv[optind][1]; /* point to the option character */
		}
	}


	/*
	 * if here, next option character is available
	 * pointed by pOpt check if it is a valid option
	 */

	/* get the position of currently processed option in the options parameter */
	pOptionsChar = strchr(options, *pOpt);

	if( NULL == pOptionsChar )
	{
		/* put the bad option char in global variable for caller to examine */
		optopt = (int)*pOpt;

		/* if here, an unknown option was encountered */
		/* print message to stderr if allowed         */
		if( opterr )
		{
			isprint(optopt) ?
				fprintf(stderr, "Unknown option '-%c'. \r\n",
					     *pOpt):
				fprintf(stderr, "Unknown option character '\\x%02x'. \r\n",
					(int)*pOpt);
		}

		/* even an invalid option was encountered, the caller may chose    */
		/* to ignore it and proceed, so make sure the next option if ready */
		/* for processing the next time getopt() is called.                */
		/* if no option is available in this optind, advance optind and    */

		pOpt++; /* point to the next option if any */
		if( '\0' == *pOpt )			
		{			
			pOpt = NULL; /* no more options in current argv[optind] */
			optind++;    /* advance index into argv                 */
		}

		/* return either '?' or ':' now */
		return (int)((':' == options[0]) ? ':' : '?');
	}


	/*
	 * if here, next option character is available
	 * and pointed by pOpt and it is a valid option
	 */

	/* check if this option has an argument */
	if( ':' == *(pOptionsChar+1) )
	{
		/* yes, this option has an argument */

		pOpt++; /* point to next character after the option character */

		/* test if the argument to this option is */
		/* in the reminder of argv[optind] string */
		if( '\0' != *pOpt )
		{
			/* the argument follows the option (is in the same string) */
			optarg = pOpt; /* the argument starts at pOpt++ */
			optind++;      /* advance index into argv       */
		}
		else
		{
			/* the argument is in the following string of argv */
			optind++;

			/* check if really there is another string available from the argv */
			if( argc > optind )
			{
				/* there is an argument available, if this if() statement */
				/* body does not execute, note that optarg remains NULL   */
				optarg = argv[optind]; /* argument is an entire string       */
				optind++;              /* advance index for next getopt call */
			}
		}

		pOpt = NULL; /* no more options in current argv[optind] */
	}
	else
	{	
		/* if here, this option does not take an argument */
		pOpt++; /* point to the next option if any */

		/* test if this is the last argumentless option in the string */
		if( '\0' == *pOpt )			
		{			
			pOpt = NULL; /* no more options in current argv[optind] */
			optind++;    /* advance index into argv                 */
		}
	}

	/* return the option character now */
	return (int)*pOptionsChar;
}
