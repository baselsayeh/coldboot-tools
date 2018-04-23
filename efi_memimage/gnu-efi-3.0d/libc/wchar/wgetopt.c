/*
 * Copyright (c) 1999, 2000
 * Intel Corporation.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * 3. All advertising materials mentioning features or use of this software must
 *    display the following acknowledgement:
 * 
 *    This product includes software developed by Intel Corporation and its
 *    contributors.
 * 
 * 4. Neither the name of Intel Corporation or its contributors may be used to
 *    endorse or promote products derived from this software without specific
 *    prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY INTEL CORPORATION AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED.  IN NO EVENT SHALL INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

/*++

Module Name:

    wgetopt.c
    
Abstract:

    Wide char getopt


Revision History

--*/

#include <sys/cdefs.h>
#include <wchar.h>
#include <stdio.h>

/* global variables updated or examined by wgetopt() */
wchar_t  *woptarg = NULL;  /* pointer to the start of the option argument             */
int      woptopt = L'\0'; /* holds unknown option wchar_t if encountered by wgetopt() */
int      woptind = 1;     /* index of the next element of the wargv[] to process     */
int      wopterr = 1;     /* if nonzero, error messages are printed to stderr        */


/* IsPrint() needs to be more complex for Unicode */
/* it should be removed from here once a Unicode is available */
#define IsPrint(a) ((a) >= L' ')


int
wgetopt
(
	int             wargc,
	wchar_t        **wargv,
	const wchar_t   *woptions
)
{
	wchar_t static   *pOpt = NULL;  /* next option character to process if any      */
	wchar_t          *pOptionsChar; /* points to matching option wchar_t in woptions */


	/* do some assumptions, later maybe changed */
	woptarg = NULL;  /* no argument is available  */
	woptopt = L'\0'; /* no invalid option         */


	/*
	 * test if no more woptions to process
	 */
	if( wargc <= woptind )
	{
		/* if here, no more woptions are to process */
		return -1; /* no more woptions */
	}


	/*
	 * check if pOpt points to the next option
	 * to process, if not, find one
	 */
	if( NULL == pOpt )
	{
		
		/* if here, a next option needs to be found in wargv */
		if( L'-' != wargv[woptind][0] )
		{
			/* if here, wargv[woptind] points to a non-option argument  */
			/* if no '-' specified in the woptions parameter, this ends */
			/* all woptions wgetopt() could process, otherwise, it is a */
			/* valid optionless argument and it is returned as it would */
			/* be associated with option character '\0'                 */
			if( NULL != wcschr(woptions, L'-') )
			{
				/* treat this wargv string as argument to option '\0'; */
				woptarg = wargv[woptind]; /* current wargv string is the argument */
				woptind++;                /* advance the index                    */
				return (int)L'\0';         /* return special option '\0'          */
			}
			else
			{
				/* this argument is an optionless argument */
				/* which wgetopt could not handle          */
				return -1;
			}
		}
		else
		{
			/* if the hyphen is standalone, it is         */
			/* treated as an ordinary non-option argument */
			if( L'\0' == wargv[woptind][1] )
			{
				return  -1;
			}

			/* check for special end-of-woptions marker "--" */
			if( L'-' == wargv[woptind][1] )
			{
				if( L'\0' == wargv[woptind][2] )
				{	
					/* the "--" marker was encountered */
					woptind++; /* indicate next argument from the wargv */
					return -1; /* no more woptions                      */
				}

				/* if here, we have a case when "--" is followed by     */
				/* more character(s), treat it as a non-option argument */

				/* if no '-' specified in the woptions parameter, this ends */
				/* all woptions wgetopt() could process, otherwise, it is a */
				/* valid optionless argument and it is returned as it would */
				/* be associated with option character '\0'                 */
				if( NULL != wcschr(woptions, L'-') )
				{
					/* treat this wargv string as argument to option '\0' */
					woptarg = wargv[woptind]; /* current wargv string is the argument */
					woptind++;                /* advance the index                    */
					return (int)L'\0';        /* return special option '\0'           */
				}
				else
				{
					/* this argument is an optionless argument */
					/* which wgetopt could not handle          */
					return -1;
				}
			}

			/* next option's starting now */
			pOpt = &wargv[woptind][1]; /* point to the option character */
		}
	}


	/*
	 * if here, next option character is available
	 * pointed by pOpt check if it is a valid option
	 */

	/* get the position of currently processed option in the woptions parameter */
	pOptionsChar = wcschr(woptions, *pOpt);

	if( NULL == pOptionsChar )
	{
		/* put the bad option wchar_t in global variable for caller to examine */
		woptopt = *pOpt;

		/* if here, an unknown option was encountered */
		/* print message to stderr if allowed         */
		if( wopterr )
		{
			/* FIX ME - Need to update to support wprintf */
			printf( "Unknown option." ) ;
			/*
			IsPrint(woptopt) ?
				Print(L"Unknown option '-%c'. \r\n",
					     *pOpt):
				Print(L"Unknown option character '\\x%02x'. \r\n",
					(int)*pOpt);
			*/
		}

		/* even an invalid option was encountered, the caller may chose    */
		/* to ignore it and proceed, so make sure the next option if ready */
		/* for processing the next time wgetopt() is called.               */
		/* if no option is available in this woptind, advance woptind and  */

		pOpt++; /* point to the next option if any */
		if( L'\0' == *pOpt )			
		{			
			pOpt = NULL; /* no more woptions in current wargv[woptind] */
			woptind++;   /* advance index into wargv                   */
		}

		/* return either '?' or ':' now */
		return (int)((L':' == woptions[0]) ? L':' : L'?');
	}


	/*
	 * if here, next option character is available
	 * and pointed by pOpt and it is a valid option
	 */

	/* check if this option has an argument */
	if( L':' == *(pOptionsChar+1) )
	{
		/* yes, this option has an argument */

		pOpt++; /* point to next character after the option character */

		/* test if the argument to this option is */
		/* in the reminder of wargv[woptind] string */
		if( L'\0' != *pOpt )
		{
			/* the argument follows the option (is in the same string) */
			woptarg = pOpt; /* the argument starts at pOpt++ */
			woptind++;      /* advance index into wargv      */
		}
		else
		{
			/* the argument is in the following string of wargv */
			woptind++;

			/* check if really there is another string available from the wargv */
			if( wargc > woptind )
			{
				/* there is an argument available, if this if() statement        */
				/* body does not execute, note that woptarg remains NULL         */
				woptarg = wargv[woptind]; /* argument is an entire string        */
				woptind++;                /* advance index for next wgetopt call */
			}
		}

		pOpt = NULL; /* no more woptions in current wargv[woptind] */
	}
	else
	{	
		/* if here, this option does not take an argument */
		pOpt++; /* point to the next option if any */

		/* test if this is the last argumentless option in the string */
		if( L'\0' == *pOpt )			
		{			
			pOpt = NULL; /* no more woptions in current wargv[woptind] */
			woptind++;   /* advance index into wargv                   */
		}
	}

	/* return the option character now */
	return (int)*pOptionsChar;
}


