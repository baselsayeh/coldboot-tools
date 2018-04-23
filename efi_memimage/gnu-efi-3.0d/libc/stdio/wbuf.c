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
static char sccsid[] = "@(#)wbuf.c    8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
        "$Id: wbuf.c,v 1.1.1.1 2006/05/30 06:13:56 hhzhou Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include "local.h"

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
int
__swbuf(c, fp)
    int c;
    register FILE *fp;
{
    register size_t n         = 0;
        wchar_t         *wcsbuf   = NULL;
        char            *savedbuf = NULL;
        int             retval    = c;

    /*
     * In case we cannot write, or longjmp takes us out early,
     * make sure _w is 0 (if fully- or un-buffered) or -_bf._size
     * (if line buffered) so that we will get called again.
     * If we did not do this, a sufficient number of putc()
     * calls might wrap _w from negative to positive.
     */
    fp->_w = fp->_lbfsize;
    if (cantwrite(fp)) {
        DPRINT(( L"__swbuf: cantwrite\n" ));
        fp->_flags |= __SERR;
        return (EOF);
    }
    c = (unsigned char)c;

    /*
     * If it is completely full, flush it out.  Then, in any case,
     * stuff c into the buffer.  If this causes the buffer to fill
     * completely, or if c is '\n' and the file is line buffered,
     * flush it (perhaps a second time).  The second flush will always
     * happen on unbuffered streams, where _bf._size==1; fflush()
     * guarantees that putc() will always call wbuf() by setting _w
     * to 0, so we need not do anything else.
     *
     * If EFI console IO, flush buffer if only one empty byte.
     */
    n = (size_t)(fp->_p - fp->_bf._base); 
    DPRINT((L"__swbuf: n %d, _size %d\n", n, fp->_bf._size));

    if (n >= (size_t)fp->_bf._size 
        || (isatty(fileno(fp)) && n >= (size_t)fp->_bf._size-1)) {

        DPRINT(( L"__swbuf: call fflush #1\n" ));
        if (fflush(fp)) {
            DPRINT(( L"__swbuf: error from fflush #1\n" ));
            retval = EOF;
            goto Done;
        }
        n = 0;
    }

    /*
     * For EFI console io, convert the char from ASCII to UNICODE.
     *  Note that for EFI we are guaranteed a buffer bigger than
     *  one char, and that if the buffer was too full we flushed it.
     */
    if ( isatty(fileno(fp)) ) {
        mbstowcs( (wchar_t *)fp->_p, (const char *)&c, sizeof(char) );
        fp->_w -= sizeof(wchar_t);
        fp->_p += sizeof(wchar_t);
        n += sizeof(wchar_t);

    } else {
        fp->_w--;
        *fp->_p++ = c;
        n++;
    }

    DPRINT((L"__swbuf #2: n %d, _size %d\n", n, fp->_bf._size));

    if (n == (size_t)fp->_bf._size || (fp->_flags & __SLBF && c == '\n')) {

        DPRINT(( L"__swbuf: call fflush #2\n" ));
        if (fflush(fp)) {
            DPRINT(( L"__swbuf: error from fflush #2\n" ));
            retval = EOF;
            goto Done;
        }

        retval = c;
    }
Done:
    DPRINT(( L"__swbuf: return %d\n", retval ));
    return (retval);
}


#if EFI32 || EFI64 || EFIX64
int 
__sputc(int _c, FILE *_p) {

    if ( isatty(fileno(_p)) ) {
        if (--_p->_w >= 1 || (_p->_w >= _p->_lbfsize && (char)_c != '\n')) {
            mbstowcs( (wchar_t *)_p->_p, (const char *)&_c, 1 );
            _p->_p += sizeof(wchar_t);
            return _c;
	} else {
            return (__swbuf(_c, _p));
        }
    } else {
        if (--_p->_w >= 0 || (_p->_w >= _p->_lbfsize && (char)_c != '\n')) {
            return (*_p->_p++ = _c);
	} else {
            return (__swbuf(_c, _p));
        }
    }
}
#endif
