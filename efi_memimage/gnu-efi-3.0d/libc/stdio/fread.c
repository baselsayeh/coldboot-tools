/*-
 * Copyright (c) 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
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
static char sccsid[] = "@(#)fread.c	8.2 (Berkeley) 12/11/93";
#endif
static const char rcsid[] =
		"$Id: fread.c,v 1.1.1.1 2006/05/30 06:13:50 hhzhou Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <string.h>
#include "local.h"
#include "libc_private.h"

#include <libc_debug.h>

size_t
fread(buf, size, count, fp)
	void *buf;
	size_t size, count;
	register FILE *fp;
{
	register size_t resid;
	register char *p;
	register int r;
	size_t total;

DPRINT(( L"fread entry\n" ));
	/*
	 * The ANSI standard requires a return value of 0 for a count
	 * or a size of 0.  Peculiarily, it imposes no such requirements
	 * on fwrite; it only requires fread to be broken.
	 */
	if ((resid = count * size) == 0)
		return (0);
	FLOCKFILE(fp);
	if (fp->_r < 0)
		fp->_r = 0;
	total = resid;
	p = buf;
        DPRINT(( L"fread: resid %d\n", resid ));
	while (resid > (size_t)(r = fp->_r)) {
                DPRINT(( L"fread: copy from fp\n" ));
		(void)memcpy((void *)p, (void *)fp->_p, (size_t)r);
                DPRINT(( L"fread: after copy from fp\n" ));
		fp->_p += r;
		/* fp->_r = 0 ... done in __srefill */
		p += r;
		resid -= r;
                DPRINT(( L"fread: call __srefill\n" ));
		if (__srefill(fp, NO_CONVERT)) {
			/* no more input: return partial result */
			FUNLOCKFILE(fp);
                        DPRINT(( L"fread: return after call __srefill\n" ));
			return ((total - resid) / size);
		}
                DPRINT(( L"fread: __srefill ret 0, resid %d, fp->_r %d\n",
                         resid, fp->_r ));
	}
        DPRINT(( L"fread: final copy from fp\n" ));
	(void)memcpy((void *)p, (void *)fp->_p, resid);
        DPRINT(( L"fread: after final copy from fp\n" ));
	fp->_r -= (int)resid;
	fp->_p += resid;
	FUNLOCKFILE(fp);
	return (count);
}
