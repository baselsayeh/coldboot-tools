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
// efi/lib/libc/wchar/wwbuf.c

#include <stdio.h>
#include "stdio/local.h"

#include <wchar.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <libc_debug.h>

/*
 * Write the given character into the (probably full) buffer for
 * the given file.  Flush the buffer out if it is or becomes full,
 * or if c=='\n' and the file is line buffered.
 */
wint_t
__swwbuf(wc, fp)
    wint_t wc;
    register FILE *fp;
{
	register int n;

	/*
	 * In case we cannot write, or longjmp takes us out early,
	 * make sure _w is 0 (if fully- or un-buffered) or -_bf._size
	 * (if line buffered) so that we will get called again.
	 * If we did not do this, a sufficient number of putwc()
	 * calls might wrap _w from negative to positive.
	 */
	fp->_w = fp->_lbfsize;
	if (cantwrite(fp))
		return (WEOF);

	/*
	 * If it is completely full, flush it out.  Then, in any case,
	 * stuff wc into the buffer.  If this causes the buffer to fill
	 * completely, or if wc is L'\n' and the file is line buffered,
	 * flush it (perhaps a second time).  The second flush will always
	 * happen on unbuffered streams, where _bf._size==1; fflush()
	 * guarantees that putwc() will always call wwbuf() by setting _w
	 * to 0, so we need not do anything else.
	 */
	n = (int)(fp->_p - fp->_bf._base);
	if (n >= fp->_bf._size) {
		if (fflush(fp))
			return (WEOF);
		n = 0;
	}
	
    fp->_w -= sizeof(wint_t);
    memcpy((wint_t *)(fp->_p), &wc, sizeof(wint_t));
    fp->_p += sizeof(wint_t);
    n += sizeof(wint_t);

	if (++n == fp->_bf._size || (fp->_flags & __SLBF && wc == L'\n'))
		if (fflush(fp))
			return (WEOF);
	return (wc);
}


wint_t 
__sputwc( wint_t _wc, FILE *fp)
{
    if (--fp->_w >= 1 || (fp->_w >= fp->_lbfsize && _wc != L'\n')) {
	    memcpy((wint_t *)(fp->_p), &_wc, sizeof(wint_t));
        fp->_p += sizeof(wint_t);
        return _wc;
	} 
	else {
        return (__swwbuf(_wc, fp));
    }
}
