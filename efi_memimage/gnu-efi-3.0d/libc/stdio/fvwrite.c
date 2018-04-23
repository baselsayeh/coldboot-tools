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
static char sccsid[] = "@(#)fvwrite.c    8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
        "$Id: fvwrite.c,v 1.1.1.1 2006/05/30 06:13:51 hhzhou Exp $";
#endif /* LIBC_SCCS and not lint */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <unistd.h>
#include "local.h"
#include "fvwrite.h"

#include <libc_debug.h>

/*
 * Write some memory regions.  Return zero on success, EOF on error.
 *
 * This routine is large and unsightly, but most of the ugliness due
 * to the three different kinds of output buffering is handled here.
 */
int
__sfvwrite(fp, uio)
    register FILE *fp;
    register struct __suio *uio;
{
    register size_t len;
    register char *p;
    register struct __siov *iov;
    register int w, s;
    char *nl;
    int nlknown, nldist;

    if ((len = uio->uio_resid) == 0) {
        return (0);
    }
    /* make sure we can write */
    if (cantwrite(fp)) {
        return (EOF);
    }

#define    MIN(a, b) ((a) < (b) ? (a) : (b))
#define    COPY(n)      (void)memcpy((void *)fp->_p, (void *)p, (size_t)(n))

    iov = uio->uio_iov;
    p = iov->iov_base;
    len = iov->iov_len;
DPRINT((L"__sfvwrite: len %d\n", len));
    iov++;
#define GETIOV(extra_work) \
    while (len == 0) { \
        extra_work; \
        p = iov->iov_base; \
        len = iov->iov_len; \
        iov++; \
    }
    if (fp->_flags & __SNBF) {
        /*
         * Unbuffered: write up to BUFSIZ bytes at a time.
         */
        DPRINT((L"__sfvwrite: unbuffered\n"));
        do {
            DPRINT((L"__sfvwrite: unbuffered: call GETIOV\n"));
            GETIOV(;);
            DPRINT((L"__sfvwrite: unbuffered: call write\n"));
            w = (int)(*fp->_write)(fp->_cookie, p, MIN((int)len, BUFSIZ));
            if (w <= 0)
                goto err;
            p += w;
            len -= w;
            DPRINT((L"uio_resid %d, w %d\n", uio->uio_resid, w ));
        } while ((uio->uio_resid -= w) != 0);
    } else if ((fp->_flags & __SLBF) == 0) {
        /*
         * Fully buffered: fill partially full buffer, if any,
         * and then flush.  If there is no partial buffer, write
         * one _bf._size byte chunk directly (without copying).
         *
         * String output is a special case: write as many bytes
         * as fit, but pretend we wrote everything.  This makes
         * snprintf() return the number of bytes needed, rather
         * than the number used, and avoids its write function
         * (so that the write function can be invalid).
         */
        DPRINT((L"__sfvwrite: fully buffered, len %d, fp->_w %d\n",len, fp->_w));
        do {
            GETIOV(;);
            if ((fp->_flags & (__SALC | __SSTR)) ==
                (__SALC | __SSTR) && (size_t)fp->_w < len) {
                size_t blen = (size_t)(fp->_p - fp->_bf._base); /* cast from EFI port */
                DPRINT((L"__sfvwrite: __SALC|__SSTR\n"));

                /*
                 * Alloc an extra 128 bytes (+ 1 for NULL)
                 * so we don't call realloc(3) so often.
                 */
                fp->_w = (int)len + 128;
                fp->_bf._size = (int)(blen + len + 128);
                fp->_bf._base =
                    reallocf(fp->_bf._base, fp->_bf._size + 1);
                if (fp->_bf._base == NULL) {
                    goto err;
                }
                fp->_p = fp->_bf._base + blen;
            }
            w = fp->_w;
            if (fp->_flags & __SSTR) {
                DPRINT((L"__sfvwrite: __SSTR\n"));
                if (len < (size_t)w)
                    w = (int)len;
                if (w > 0) {
                    COPY(w);        /* copy MIN(fp->_w,len), */
                    fp->_w -= w;
                    fp->_p += w;
                }
                w = (int)len;    /* but pretend copied all */
            } else if (fp->_p > fp->_bf._base && len > (size_t)w) {
                /* fill and flush */
                DPRINT((L"__sfvwrite: fill & flush\n"));
                COPY(w);
                /* fp->_w -= w; */ /* unneeded */
                fp->_p += w;
                if (fflush(fp))
                    goto err;
            } else if (len >= (size_t)(w = fp->_bf._size)) {
                DPRINT((L"__sfvwrite: write directly\n"));
                /* write directly */
                w = (*fp->_write)(fp->_cookie, p, w);
                if (w <= 0)
                    goto err;
            } else {
                DPRINT((L"__sfvwrite: fill & done\n"));
                /* fill and done */
                w = (int)len;
                COPY(w);
                fp->_w -= w;
                fp->_p += w;
            }
            p += w;
            len -= w;
        } while ((uio->uio_resid -= w) != 0);
    } else {
        /*
         * Line buffered: like fully buffered, but we
         * must check for newlines.  Compute the distance
         * to the first newline (including the newline),
         * or `infinity' if there is none, then pretend
         * that the amount to write is MIN(len,nldist).
         */
        DPRINT((L"__sfvwrite: line buffered\n"));
        nlknown = 0;
        nldist = 0;    /* XXX just to keep gcc happy */
        do {
            DPRINT((L"__sfvwrite: line buffered: GETIOV\n"));
            GETIOV(nlknown = 0);
            if (!nlknown) {
                DPRINT((L"__sfvwrite: line buffered: !nlknown\n"));

                nl = memchr((void *)p, '\n', len);
                DPRINT((L"__sfvwrite: linebuf: nl 0x%X\n",nl));
                if( isatty(fileno(fp)) ) {
                    /* 
                     *  UNICODE buffer length must be even, so the address
                     *  of the end of it must be odd
                     */
                    nldist = (int)(nl ? nl + sizeof(wchar_t) - p
                                      : len + sizeof(wchar_t));
                                                   /* cast from EFI port */
                } else {
                    nldist = (int)(nl ? nl + 1 - p : len + 1);
                                                   /* cast from EFI port */
                }
                nlknown = 1;
                DPRINT((L"__sfvwrite: line buffered: nl 0x%X, nldist %d\n", nl, nldist));
            }
            s = (int)MIN(len, (size_t)nldist);
            w = fp->_w + fp->_bf._size;
            DPRINT((L"__sfvwrite: line buf: s %d, w %d\n", s, w));
            if (fp->_p > fp->_bf._base && s > w) {
                DPRINT((L"__sfvwrite: line buffered: COPY #1\n"));
                COPY(w);
                /* fp->_w -= w; */
                fp->_p += w;
                if (fflush(fp))
                    goto err;
            } else if (s >= (w = fp->_bf._size)) {
                DPRINT((L"__sfvwrite: line buffered: write\n"));
                w = (*fp->_write)(fp->_cookie, p, w);
                if (w <= 0)
                     goto err;
            } else {
                w = s;
                DPRINT((L"__sfvwrite: line buffered: COPY #2 %d bytes\n"));
                COPY(w);
                DHEXDMP(fp->_p, 64, L"sfvwrite: line buf: fp->_p...\n");
                fp->_w -= w;
                fp->_p += w;
            }
            if ((nldist -= w) == 0) {
                DPRINT((L"__sfvwrite: line buffered: copied newline\n"));
                /* copied the newline: flush and forget */
                if (fflush(fp))
                    goto err;
                nlknown = 0;
            }
            p += w;
            len -= w;
        } while ((uio->uio_resid -= w) != 0);
    }
    DPRINT((L"__sfvwrite: return success\n"));
    return (0);

err:
    fp->_flags |= __SERR;
    DPRINT((L"__sfvwrite: return error\n"));
    return (EOF);
}
