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

// wide character version
// efi/lib/libc/wchar/ungetwc.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include "stdio/local.h"
#include "libc_private.h"

static int __submore __P((FILE *));

/*
 * Expand the ungetc buffer `in place'.  That is, adjust fp->_p when
 * the buffer moves, so that it points the same distance from the end,
 * and move the bytes in the buffer around as necessary so that they
 * are all at the end (stack-style).
 */
static int
__submore(fp)
	register FILE *fp;
{
	register int i;
	register unsigned char *p;

	if (fp->_ub._base == fp->_ubuf) {
		/*
		 * Get a new buffer (rather than expanding the old one).
		 */
		if ((p = malloc((size_t)BUFSIZ)) == NULL)
			return (EOF);
		fp->_ub._base = p;
		fp->_ub._size = BUFSIZ;
		p += BUFSIZ - sizeof(fp->_ubuf);
		for (i = sizeof(fp->_ubuf); --i >= 0;)
			p[i] = fp->_ubuf[i];
		fp->_p = p;
		return (0);
	}
	i = fp->_ub._size;
	p = realloc(fp->_ub._base, (size_t)(i << 1));
	if (p == NULL)
		return (EOF);
	/* no overlap (hence can use memcpy) because we doubled the size */
	(void)memcpy((void *)(p + i), (void *)p, (size_t)i);
	fp->_p = p + i;
	fp->_ub._base = p;
	fp->_ub._size = i << 1;
	return (0);
}

wint_t
ungetwc( wint_t wc, FILE *fp)
{
	wint_t tmpwc;
	
	if (wc == WEOF)
		return (WEOF);
	if (!__sdidinit)
		__sinit();
	FLOCKFILE(fp);
	if ((fp->_flags & __SRD) == 0) {
		/*
		 * Not already reading: no good unless reading-and-writing.
		 * Otherwise, flush any current write stuff.
		 */
		if ((fp->_flags & __SRW) == 0) {
			FUNLOCKFILE(fp);
			return (WEOF);
		}
		if (fp->_flags & __SWR) {
			if (__sflush(fp)) {
				FUNLOCKFILE(fp);
				return (EOF);
			}
			fp->_flags &= ~__SWR;
			fp->_w = 0;
			fp->_lbfsize = 0;
		}
		fp->_flags |= __SRD;
	}

	/*
	 * If we are in the middle of ungetwc'ing, just continue.
	 * This may require expanding the current ungetwc buffer.
	 */
	if (HASUB(fp)) {
		if (fp->_r >= fp->_ub._size && __submore(fp)) {
			FUNLOCKFILE(fp);
			return (WEOF);
		}
		
		memcpy(&wc, fp->_p, sizeof(wint_t));
		fp->_p -= sizeof(wint_t);
		fp->_r += sizeof(wint_t);
		FUNLOCKFILE(fp);
		return (wc);
	}
	fp->_flags &= ~__SEOF;

	/*
	 * If we can handle this by simply backing up, do so,
	 * but never replace the original character.
	 * (This makes wsscanf() work when scanning `const' data.)
	 */
	 
	if (fp->_bf._base != NULL && fp->_p > fp->_bf._base) {
		memcpy(&tmpwc, &fp->_p[0-sizeof(wint_t)], sizeof(wint_t));
		if (tmpwc == wc) {
		    fp->_p -= sizeof(wint_t);
		    fp->_r += sizeof(wint_t);
			FUNLOCKFILE(fp);
			return (wc);
		}
	}

	/*
	 * Create an ungetc buffer.
	 * Initially, we will use the `reserve' buffer.
	 */
	fp->_ur = fp->_r;
	fp->_up = fp->_p;
	fp->_ub._base = fp->_ubuf;
	fp->_ub._size = sizeof(fp->_ubuf);
	memcpy( &(fp->_ubuf[sizeof(fp->_ubuf) - sizeof(wint_t)]), &wc, sizeof(wint_t));
	fp->_p = &fp->_ubuf[sizeof(fp->_ubuf) - sizeof(wint_t)];
	fp->_r = sizeof(wint_t);
	FUNLOCKFILE(fp);
	return (wc);
}
