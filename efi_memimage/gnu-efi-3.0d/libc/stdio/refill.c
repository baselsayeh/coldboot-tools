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
static char sccsid[] = "@(#)refill.c    8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
        "$Id: refill.c,v 1.1.1.1 2006/05/30 06:13:53 hhzhou Exp $";
#endif /* LIBC_SCCS and not lint */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "local.h"

#include <libc_debug.h>
#include <assert.h>

static int lflush __P((FILE *));

static int
lflush(fp)
    FILE *fp;
{

    if ((fp->_flags & (__SLBF|__SWR)) == (__SLBF|__SWR))
        return (__sflush(fp));
    return (0);
}

/*
 * Refill a stdio buffer.
 * Return EOF on eof or error, 0 otherwise.
 */
int
__srefill(fp, convert_if_tty)
    register FILE *fp;
    int           convert_if_tty;
{
    int     ret       = 0;
    wchar_t *temp     = NULL;
    char    *savedbuf = NULL;
    ssize_t  wcsret   = 0;
    int     doconvert = 0;


    DPRINT(( L"__srefill: entry\n" ));
    /* make sure stdio is set up */
    if (!__sdidinit)
        __sinit();

    fp->_r = 0;        /* largely a convenience for callers */

    /* SysV does not make this test; take it out for compatibility */
    if (fp->_flags & __SEOF) {
        ret = EOF;
        DPRINT(( L"__srefill: __SEOF return\n" ));
        goto Done;
    }

    /* if not already reading, have to be reading and writing */
    if ((fp->_flags & __SRD) == 0) {
        if ((fp->_flags & __SRW) == 0) {
            errno = EBADF;
            ret = EOF;
            DPRINT(( L"__srefill: EBADF return\n" ));
            goto Done;
        }
        /* switch to reading */
        if (fp->_flags & __SWR) {
            if (__sflush(fp)) {
                ret = EOF;
                DPRINT(( L"__srefill: __SWR return\n" ));
                goto Done;
            }
            fp->_flags &= ~__SWR;
            fp->_w = 0;
            fp->_lbfsize = 0;
        }
        fp->_flags |= __SRD;
    } else {
        /*
         * We were reading.  If there is an ungetc buffer,
         * we must have been reading from that.  Drop it,
         * restoring the previous buffer (if any).  If there
         * is anything in that buffer, return.
         */
        if (HASUB(fp)) {
            FREEUB(fp);
            if ((fp->_r = fp->_ur) != 0) {
                fp->_p = fp->_up;
                ret = 0;
                DPRINT(( L"__srefill: ungetc buffer return\n" ));
                goto Done;
            }
        }
    }

    DPRINT(( L"__srefill: call makebuf\n" ));
    if (fp->_bf._base == NULL)
        __smakebuf(fp);
    DPRINT(( L"__srefill: after call makebuf\n" ));

    /*
     * If we are reading from the EFI console, we will read
     * wide char characters and convert to ASCII.  Make a buffer
     * that's 2X the standard one so that other behavior is the
     * same as it would be otherwise.
     */
    doconvert = isatty(fileno(fp)) && convert_if_tty;

    if ( doconvert ) {
        DPRINT(( L"__srefill: doconvert\n" ));
        temp = calloc( fp->_bf._size, sizeof(wchar_t));
        if ( !temp ) {
            errno = ENOMEM;
            ret = EOF;
            goto Done;
        }
    
        savedbuf = (char*)fp->_bf._base;
        fp->_bf._base = (void *)temp;
        fp->_bf._size *= sizeof(wchar_t);
    }


    /*
     * Before reading from a line buffered or unbuffered file,
     * flush all line buffered output files, per the ANSI C
     * standard.
     */
    if (fp->_flags & (__SLBF|__SNBF))
        (void) _fwalk(lflush);
    fp->_p = fp->_bf._base;
    DPRINT(( L"__srefill: fp->_bf._size %d\n", fp->_bf._size ));
    fp->_r = (*fp->_read)(fp->_cookie, (char *)fp->_p, fp->_bf._size);
    fp->_flags &= ~__SMOD;    /* buffer contents are again pristine */

Error:
    if (fp->_r <= 0) {
        if (fp->_r == 0)
            fp->_flags |= __SEOF;
        else {
            fp->_r = 0;
            fp->_flags |= __SERR;
        }

        /*
         *  Switch buffers back if we were going to do a conversion
         */
        if ( doconvert ) {
			fp->_bf._base = (u_char*)savedbuf;
			fp->_p = fp->_bf._base;
        }

        ret = EOF;
        DPRINT(( L"__srefill: done EOF\n" ));
    } else {
    	DHEXDMP(fp->_p,fp->_r, L"__srefill buffer: fp->_p");

		/*
		 *  If EFI console, do the conversion
		 */
		if ( doconvert ) {
			DPRINT(( L"__srefill: doconvert 2\n" ));
			wcsret = wcstombs( savedbuf, (wchar_t*)fp->_bf._base, fp->_r );
			if ( wcsret < 0 ) {
				/*
				 *  Make it look like a read error and process as such
				 */
				fp->_r = -1;
				goto Error;
			}
			fp->_bf._base = (u_char*)savedbuf;
			fp->_p = fp->_bf._base;
			fp->_bf._size /= sizeof(wchar_t);
			fp->_r /= sizeof(wchar_t);
		
			DHEXDMP(fp->_bf._base,
					fp->_r, 
					L"__srefill after convert: fp->_bf._base" );
		}
	}

Done:
    if ( temp ) {
        free( temp );
        temp = NULL;
    }

    DPRINT((L"__srefill: return %d\n", ret ));
    return ret;
}
