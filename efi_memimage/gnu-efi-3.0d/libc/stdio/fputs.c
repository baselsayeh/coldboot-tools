/*-
 * Copyright (c) 1990, 1993
 *    The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Chris Torek.
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

#if defined(LIBC_SCCS) && !defined(lint)
#if 0
static char sccsid[] = "@(#)fputs.c    8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
        "$Id: fputs.c,v 1.1.1.1 2006/05/30 06:13:50 hhzhou Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "fvwrite.h"
#include "libc_private.h"

#include <stdlib.h>
#include <wchar.h>
#include <errno.h>

#include <libc_debug.h>


/*
 * Write the given string to the given file.
 */
int
fputs(s, fp)
    const char *s;
    FILE *fp;
{
    int           retval;
    struct __suio uio;
    struct __siov iov;
    wchar_t       *ws    = NULL;
    size_t        wsize  = strlen(s) * sizeof(wchar_t);

    if ( isatty(fileno(fp)) ) {
	    ws = calloc( strlen(s), sizeof(wchar_t) );
	    if ( !ws ) {
	        errno = ENOMEM;
	        return -1;
	    }
	    mbstowcs( ws, s, strlen(s) );

	    iov.iov_base = (void *)ws;
	    iov.iov_len = uio.uio_resid = (int)wsize;
	}
	else
	{
		iov.iov_base = (void *)s;
		iov.iov_len = uio.uio_resid = (int)strlen(s);
	}

    uio.uio_iov = &iov;
    uio.uio_iovcnt = 1;
    FLOCKFILE(fp);
    DPRINT((L"fputs: write %d bytes\n", iov.iov_len ));
    retval = __sfvwrite(fp, &uio);
    FUNLOCKFILE(fp);

	if( ws )
	{
    	free(ws);
    	ws = NULL;
	}

    DPRINT((L"fputs: return %d\n", retval));
    return (retval);
}
