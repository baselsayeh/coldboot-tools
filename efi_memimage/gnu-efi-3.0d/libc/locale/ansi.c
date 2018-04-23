/*-
 * Copyright (c) 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * This code is derived from software contributed to Berkeley by
 * Paul Borman at Krystal Technologies.
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
static char sccsid[] = "@(#)ansi.c	8.1 (Berkeley) 6/27/93";
#endif /* LIBC_SCCS and not lint */

#include <efi.h>
#include <stdlib.h>
#include <limits.h>
#include <stddef.h>
#include <rune.h>


int
mblen(s, n)
	const char *s;
	size_t n;
{
	char const *e;

	if (s == 0 || *s == 0)
		return (0);	/* No support for state dependent encodings. */

	if (sgetrune(s, n, &e) == _INVALID_RUNE)
		return (int)(s - e);
	return (int)(e - s);
}

int
mbtowc(pwc, s, n)
	wchar_t *pwc;
	const char *s;
	size_t n;
{
	char const *e;
	rune_t r;

	if (s == 0) {
		return (0);	/* No support for state dependent encodings. */
        }

        /* special-case zero-length string */
        if ( *s == 0 ) {
            *pwc = 0;
            return 0;
        }

	if ((r = maptounicode (sgetrune(s, n, &e)) ) == _INVALID_RUNE) {
		return (int)(s - e);
        }
	if (pwc) {
		*pwc = r;
        }
	return (int)(e - s);
}

int
#ifdef __STDC__
wctomb(char *s, wchar_t wchar)
#else
wctomb(s, wchar)
	char *s;
	wchar_t wchar;
#endif
{
	char *e;

	if (s == 0)
		return (0);	/* No support for state dependent encodings. */

	if (wchar == 0) {
		*s = 0;
		return (1);
	}

	sputrune(mapfromunicode(wchar), s, MB_CUR_MAX, &e);
	return (int)(e ? e - s : -1);
}

size_t
mbstowcs(pwcs, s, n)
	wchar_t *pwcs;
	const char *s;
	size_t n;
{
	char const *e;
	int cnt = 0;

	if (!pwcs || !s)
		return (-1);

	while (n-- > 0) {
		*pwcs = maptounicode ( sgetrune(s, MB_LEN_MAX, &e));
		if (*pwcs == _INVALID_RUNE)
			return (-1);
		if (*pwcs++ == 0)
			break;
		s = e;
		++cnt;
	}
	return (int)(cnt);
}

size_t
wcstombs(s, pwcs, n)
	char *s;
	const wchar_t *pwcs;
	size_t n;
{
	char *e;
	size_t cnt, nb;

	if (!pwcs || !s || n > INT_MAX)
		return (-1);

	nb = n;
	cnt = 0;
	while (nb > 0) {
		if (*pwcs == 0) {
			*s = 0;
			break;
		}
		if (!sputrune(mapfromunicode(*pwcs++), s, nb, &e))
			return (-1);		/* encoding error */
		if (!e)			/* too long */
			return (cnt);
		cnt += (size_t)(e - s); /* cast from EFI port */
		nb -= (size_t)(e - s); /* cast from EFI port */
		s = e;
	}
	return (cnt);
}
