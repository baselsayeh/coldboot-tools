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
// efi/lib/libc/wchar/fgetws.c

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <wchar.h>
#include "stdio/local.h"
#include "libc_private.h"

#include <libc_debug.h>

/*
 * Read at most n-1 characters from the given file.
 * Stop when a newline has been read, or the count runs out.
 * Return first argument, or NULL if no characters were read.
 */
wchar_t *
fgetws(buf, n, fp)
	wchar_t *buf;
	register int n;
	register FILE *fp;
{
	register int len;
	register wchar_t *s, *t, *p;

	if (n <= 0)		/* sanity check */
		return (NULL);

	FLOCKFILE(fp);
	s = buf;
	n--;			/* leave space for NUL */
	while (n != 0) {
		/*
		 * If the buffer is empty, refill it.
		 */
		if ((len = fp->_r) <= 1) {
            if ( __swrefill(fp) ) {
				/* EOF/error: stop with partial or no line */
				if (s == buf) {
					FUNLOCKFILE(fp);
					return (NULL);
				}
				break;
			}
			len = fp->_r;
		}
		/*
		 * Set len and p to wchar_t's
		 */
		len /= sizeof(wchar_t);
		p = (wchar_t*)fp->_p;

		/*
		 * Scan through at most n bytes of the current buffer,
		 * looking for '\n'.  If found, copy up to and including
		 * newline, and stop.  Otherwise, copy entire chunk
		 * and loop.
		 */
		if (len > n)
			len = n;
		t = wmemchr(p, L'\n', len);		/* BugBug - is this the right thing for Unicode? */
		if (t != NULL) {
			len = (int)(++t - p); /* cast from EFI port */
            DPRINT((L"fgets: found newline: len %d\n", len ));
			fp->_r -= (len * sizeof(wchar_t));
			fp->_p = (unsigned char*)t;
			(void)wmemcpy(s, p, len);
			s[len] = 0;
			FUNLOCKFILE(fp);
			return (buf);
		}
        DPRINT((L"fwgets: no newline: len %d\n", len ));
		fp->_r -= (len * sizeof(wchar_t));
		fp->_p += (len * sizeof(wchar_t));
		(void)wmemcpy(s, p, len);
		s += len;
		n -= len;
	}
	*s = 0;
	FUNLOCKFILE(fp);
	return (buf);
}
