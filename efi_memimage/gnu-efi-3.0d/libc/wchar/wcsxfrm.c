/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 * 
 * Portions copyright (c) 1999, 2000
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
 *    This product includes software developed by the University of
 *    California, Berkeley, Intel Corporation, and its contributors.
 * 
 * 4. Neither the name of University, Intel Corporation, or their respective
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS, INTEL CORPORATION AND
 * CONTRIBUTORS ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS,
 * INTEL CORPORATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/*++

Module Name:

    wcsxfrm.c
    
Abstract:

    Wide char string strxfrm


Revision History

--*/

#include <sys/cdefs.h>
#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

size_t
wcsxfrm(dest, src, len)
	wchar_t *dest;
	const wchar_t *src;
	size_t len;
{
	
	size_t  lensrc, result, limit , i;
	char *csdest, *cssrc;
	wchar_t *wcsdest;	
	
	lensrc = wcslen(src);
	
	cssrc = malloc((lensrc + 1) * sizeof(wchar_t));
	if ( cssrc == NULL ) 
		return len;
	
	limit = (lensrc + 1) >= len ? lensrc + 1 : len;
	csdest = malloc( limit * sizeof(wchar_t));
	if (  csdest == NULL ) {
		free( cssrc );
		return len;
	}
	
	wcsdest = (wchar_t *) malloc( limit * sizeof(wchar_t));
	if (  wcsdest == NULL )  {
		free( csdest );
		free( cssrc );
		return len;
	}
		
	// change wchar string to char string
	wcstombs(cssrc,src, ( lensrc + 1 ) * sizeof(wchar_t));
	
	result = strxfrm(csdest, cssrc, limit * sizeof (wchar_t));
	
	if ( result < (limit * sizeof(wchar_t)) ) {
		// change char string to wchar string
		result = mbstowcs(wcsdest,csdest, limit);
	
		limit = (result + 1) <= len ? result + 1 : len;
		for ( i = 0; i < limit - 1; i++ )
			dest[i] = wcsdest[i];
		dest[limit - 1] = 0; // add terminate character
			
	}
	
	free(cssrc);
	free(csdest);
	free(wcsdest);
	
	return result;
}

