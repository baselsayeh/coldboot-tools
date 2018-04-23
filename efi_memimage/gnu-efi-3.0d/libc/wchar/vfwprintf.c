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
// efi/lib/libc/wchar/vfwprintf.c

/*
 * Actual wprintf innards.
 *
 * This code is large and complicated...
 */
 
#include <sys/types.h>

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#if __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif

#include "../stdio/local.h"
#include "../stdio/fvwrite.h"
#include "libc_private.h"
#include "efilib.h"
#include "efi_interface.h"

/* Define FLOATING_POINT to get floating point. */
#define	FLOATING_POINT

static int	__swprint __P((FILE *, struct __suio *));
static int	__sbwprintf __P((FILE *, const wchar_t *, va_list));
static wchar_t *	__ultow __P((u_long, wchar_t *, int, int, wchar_t *));
static wchar_t *	__uqtow __P((u_quad_t, wchar_t *, int, int, wchar_t *));
static void	__wfind_arguments __P((const wchar_t *, va_list, void ***));
static void	__grow_type_table __P((int, unsigned char **, int *));


/*
 * Flush out all the vectors defined by the given uio,
 * then reset it so that it can be reused.
 */
static int
__swprint(fp, uio)
	FILE *fp;
	register struct __suio *uio;
{
	register int err;

	if (uio->uio_resid == 0) {
		uio->uio_iovcnt = 0;
		return (0);
	}
	err = __sfvwrite(fp, uio);
	uio->uio_resid = 0;
	uio->uio_iovcnt = 0;
	return (err);
}

/*
 * Helper function for `fwprintf to unbuffered unix file': creates a
 * temporary buffer.  We only work on write-only files; this avoids
 * worries about ungetc buffers and so forth.
 */
static int
__sbwprintf(fp, fmt, ap)
	register FILE *fp;
	const wchar_t *fmt;
	va_list ap;
{
	int ret;
	FILE fake;
	unsigned char buf[BUFSIZ];

	/* copy the important variables */
	fake._flags = fp->_flags & ~__SNBF;
	fake._file = fp->_file;
	fake._cookie = fp->_cookie;
	fake._write = fp->_write;

	/* set up the buffer */
	fake._bf._base = fake._p = buf;
	fake._bf._size = fake._w = sizeof(buf);
	fake._lbfsize = 0;	/* not actually used, but Just In Case */

	/* do the work, then copy any error status */
	ret = vfwprintf(&fake, fmt, ap);
	if (ret >= 0 && fflush(&fake))
		ret = EOF;
	if (fake._flags & __SERR)
		fp->_flags |= __SERR;
	return (ret);
}

/*
 * Macros for converting digits to letters and vice versa
 */
#define	to_digit(c)	((c) - '0')
#define is_digit(c)	((unsigned)to_digit(c) <= 9)
#define	to_char(n)	((n) + '0')

#define	to_wdigit(c)	((c) - L'0')
#define is_wdigit(c)	((unsigned)to_wdigit(c) <= 9)
#define	to_wchar(n)		((n) + L'0')


/*
 * Convert an unsigned long to wide character for wprintf purposes, returning
 * a pointer to the first character of the string representation.
 * Octal numbers can be forced to have a leading zero; hex numbers
 * use the given digits.
 */
static wchar_t *
__ultow(val, endp, base, octzero, xdigs)
	register u_long val;
	wchar_t *endp;
	int base, octzero;
	wchar_t *xdigs;
{
	register wchar_t *cp = endp;
	register long sval;

	/*
	 * Handle the three cases separately, in the hope of getting
	 * better/faster code.
	 */
	 
	switch (base) {
	case 10:
		if (val < 10) {	/* many numbers are 1 digit */
			*--cp = (wchar_t)to_wchar(val);
			return (cp);
		}
		/*
		 * On many machines, unsigned arithmetic is harder than
		 * signed arithmetic, so we do at most one unsigned mod and
		 * divide; this is sufficient to reduce the range of
		 * the incoming value to where signed arithmetic works.
		 */
		if (val > LONG_MAX) {
			*--cp = (wchar_t)to_wchar(val % 10);
			sval = val / 10;
		} else
			sval = val;
		do {
			*--cp = (wchar_t)to_wchar(sval % 10);
			sval /= 10;
		} while (sval != 0);
		break;

	case 8:
		do {
			*--cp = (wchar_t)to_wchar(val & 7);
			val >>= 3;
		} while (val);
		if (octzero && *cp != L'0')
			*--cp = L'0';
		break;

	case 16:
		do {
			*--cp = xdigs[val & 15];
			val >>= 4;
		} while (val);
		break;

	default:			/* oops */
		abort();
	}
	return (cp);
}

/* Identical to __ultow, but for quads. */
static wchar_t *
__uqtow(val, endp, base, octzero, xdigs)
	register u_quad_t val;
	wchar_t *endp;
	int base, octzero;
	wchar_t *xdigs;
{
	register wchar_t *cp = endp;
	register quad_t sval;
	UINTN remainder;

	/* quick test for small values; __ultow is typically much faster */
	/* (perhaps instead we should run until small, then call __ultow?) */
	if (val <= ULONG_MAX)
		return (__ultow((u_long)val, endp, base, octzero, xdigs));
	switch (base) {
	case 10:
		if (val < 10) {
			LIBC_DivU64x32(val, 10, &remainder);	
			*--cp = (wchar_t)to_wchar(remainder);
			return (cp);
		}
		if (val > QUAD_MAX) {			
			sval = LIBC_DivU64x32(val, 10, &remainder);	
			*--cp = (wchar_t)to_wchar(remainder);
		} else
			sval = val;
		do {
			sval = LIBC_DivU64x32(sval, 10, &remainder);	
			*--cp = (wchar_t)to_wchar(remainder);
		} while (sval != 0);
		break;

	case 8:
		do {
			*--cp = (wchar_t)to_wchar( (*(UINT8*)(&val)) & 7);
			val =LIBC_RShiftU64(val,3);
		} while (val);
		if (octzero && *cp != L'0')
			*--cp = L'0';
		break;

	case 16:
		do {
			*--cp = xdigs[ (*(UINT8*)(&val)) & 15];
			val = LIBC_RShiftU64(val,4);
		} while (val);
		break;

	default:
		abort();
	}
	return (cp);
}

#ifdef FLOATING_POINT
#include <math.h>
#include "../stdio/floatio.h"

#define	BUF		(MAXEXP+MAXFRACT+1)	/* + decimal point */
#define	DEFPREC		6

static wchar_t *wcvt __P((double, int, int, wchar_t *, int *, int, int *));
static int wexponent __P((wchar_t *, int, int));

#else /* no FLOATING_POINT */

#define	BUF		68

#endif /* FLOATING_POINT */

#define STATIC_ARG_TBL_SIZE 8           /* Size of static argument table. */

/*
 * Flags used during conversion.
 */
#define	ALT		0x001		/* alternate form */
#define	HEXPREFIX	0x002		/* add 0x or 0X prefix */
#define	LADJUST		0x004		/* left adjustment */
#define	LONGDBL		0x008		/* long double */
#define	LONGINT		0x010		/* long integer */
#define	QUADINT		0x020		/* quad integer */
#define	SHORTINT	0x040		/* short integer */
#define	ZEROPAD		0x080		/* zero (as opposed to blank) pad */
#define FPT		0x100		/* Floating point number */
int
vfwprintf(fp, fmt0, ap)
	FILE *fp;
	const wchar_t *fmt0;
	_BSD_VA_LIST_ ap;
{
	register wchar_t *fmt;	/* format string */
	register int ch;	/* character from fmt */
	register int n, n2;	/* handy integer (short term usage) */
	register wchar_t *cp;	/* handy char pointer (short term usage) */
	register struct __siov *iovp;/* for PRINT macro */
	register int flags;	/* flags as above */
	int ret;		/* return value accumulator */
	int width;		/* width from format (%8d), or 0 */
	int prec;		/* precision from format (%.3d), or -1 */
	wchar_t sign;		/* sign prefix (L' ', L'+', L'-', or L\0) */
#ifdef FLOATING_POINT
	wchar_t softsign;		/* temporary negative sign for floats */
	double _double;		/* double precision arguments %[eEfgG] */
	int expt;		/* integer value of exponent */
	int expsize;		/* character count for expstr */
	int ndig;		/* actual number of digits returned by cvt */
	wchar_t expstr[7];		/* buffer for exponent string */
#endif
	u_long	ulval;		/* integer arguments %[diouxX] */
	u_quad_t uqval;		/* %q integers */
	int base;		/* base for [diouxX] conversion */
	int dprec;		/* a copy of prec if [diouxX], 0 otherwise */
	int realsz;		/* field size expanded by dprec, sign, etc */
	int size;		/* size of converted field or string */
	int prsize;             /* max size of printed field */
	wchar_t *xdigs;		/* digits for [xX] conversion */
#define NIOV 8
	struct __suio uio;	/* output information: summary */
	struct __siov iov[NIOV];/* ... and individual io vectors */
	wchar_t buf[BUF];		/* space for %c, %[diouxX], %[eEfgG] */
	wchar_t ox[2];		/* space for 0x hex-prefix */
    void **argtable;        /* args, built due to positional arg */
    void *statargtable [STATIC_ARG_TBL_SIZE];
    int nextarg;            /* 1-based argument index */
    va_list orgap;          /* original argument pointer */

	/*
	 * Choose PADSIZE to trade efficiency vs. size.  If larger printf
	 * fields occur frequently, increase PADSIZE and make the initialisers
	 * below longer.
	 */
#define	PADSIZE	16		/* pad chunk size */
	static wchar_t blanks[PADSIZE] =
	 {L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' ',L' '};
	static wchar_t zeroes[PADSIZE] =
	 {L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0',L'0'};

	/*
	 * BEWARE, these `goto error' on error, and PAD uses `n'.
	 */
#define	WPRINT(ptr, len) { \
	iovp->iov_base = (wchar_t *)(ptr); \
	iovp->iov_len = (len)*sizeof(wchar_t); \
	uio.uio_resid += (len)*sizeof(wchar_t); \
	iovp++; \
	if (++uio.uio_iovcnt >= NIOV) { \
		if (__swprint(fp, &uio)) \
			goto error; \
		iovp = iov; \
	} \
}
#define	WPAD(howmany, with) { \
	if ((n = (howmany)) > 0) { \
		while (n > PADSIZE) { \
			WPRINT(with, PADSIZE); \
			n -= PADSIZE; \
		} \
		WPRINT(with, n); \
	} \
}
#define	WFLUSH() { \
	if (uio.uio_resid && __swprint(fp, &uio)) \
		goto error; \
	uio.uio_iovcnt = 0; \
	iovp = iov; \
}

        /*
         * Get the argument indexed by nextarg.   If the argument table is
         * built, use it to get the argument.  If its not, get the next
         * argument (and arguments must be gotten sequentially).
         */
#define GETARG(type) \
        ((argtable != NULL) ? *((type*)(argtable[nextarg++])) : \
            (nextarg++, va_arg(ap, type)))

	/*
	 * To extend shorts properly, we need both signed and unsigned
	 * argument extraction methods.
	 */
#define	SARG() \
	(flags&LONGINT ? GETARG(long) : \
	    flags&SHORTINT ? (long)(short)GETARG(int) : \
	    (long)GETARG(int))
#define	UARG() \
	(flags&LONGINT ? GETARG(u_long) : \
	    flags&SHORTINT ? (u_long)(u_short)GETARG(int) : \
	    (u_long)GETARG(u_int))

        /*
         * Get * arguments, including the form *nn$.  Preserve the nextarg
         * that the argument can be gotten once the type is determined.
         */
#define WGETASTER(val) \
        n2 = 0; \
        cp = fmt; \
        while (is_wdigit(*cp)) { \
                n2 = 10 * n2 + to_wdigit(*cp); \
                cp++; \
        } \
        if (*cp == L'$') { \
            	int hold = nextarg; \
                if (argtable == NULL) { \
                        argtable = statargtable; \
                        __wfind_arguments (fmt0, orgap, &argtable); \
                } \
                nextarg = n2; \
                val = GETARG (int); \
                nextarg = hold; \
                fmt = ++cp; \
        } else { \
		val = GETARG (int); \
        }
        

	FLOCKFILE(fp);
	/* sorry, fwprintf(read_only_file, "") returns WEOF, not 0 */
	if (cantwrite(fp)) {
		FUNLOCKFILE(fp);
		return (EOF);
	}

	/* optimise fwprintf(stderr) (and other unbuffered Unix files) */
	if ((fp->_flags & (__SNBF|__SWR|__SRW)) == (__SNBF|__SWR) &&
	    fp->_file >= 0) {
		FUNLOCKFILE(fp);
		return (__sbwprintf(fp, fmt0, ap));
	}

	fmt = (wchar_t *)fmt0;
        argtable = NULL;
        nextarg = 1;
        orgap = ap;
	uio.uio_iov = iovp = iov;
	uio.uio_resid = 0;
	uio.uio_iovcnt = 0;
	ret = 0;

	/*
	 * Scan the format for conversions (`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != L'\0' && ch != L'%'; fmt++)
			/* void */;
		if ((n = (int)(fmt - cp)) != 0) {
			if ((unsigned)ret + n > INT_MAX) {
				ret = WEOF;
				goto error;
			}
			WPRINT(cp, n);
			ret += n;
		}
		if (ch == L'\0')
			goto done;
		fmt++;		/* skip over '%' */

		flags = 0;
		dprec = 0;
		width = 0;
		prec = -1;
		sign = L'\0';

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case L' ':
			/*
			 * ``If the space and + flags both appear, the space
			 * flag will be ignored.''
			 *	-- ANSI X3J11
			 */
			if (!sign)
				sign = L' ';
			goto rflag;
		case L'#':
			flags |= ALT;
			goto rflag;
		case L'*':
			/*
			 * ``A negative field width argument is taken as a
			 * - flag followed by a positive field width.''
			 *	-- ANSI X3J11
			 * They don't exclude field widths read from args.
			 */
			WGETASTER (width);
			if (width >= 0)
				goto rflag;
			width = -width;
			/* FALLTHROUGH */
		case L'-':
			flags |= LADJUST;
			goto rflag;
		case L'+':
			sign = L'+';
			goto rflag;
		case L'.':
			if ((ch = *fmt++) == L'*') {
				WGETASTER (n);
				prec = n < 0 ? -1 : n;
				goto rflag;
			}
			n = 0;
			while (is_wdigit(ch)) {
				n = 10 * n + to_wdigit(ch);
				ch = *fmt++;
			}
			prec = n < 0 ? -1 : n;
			goto reswitch;
		case L'0':
			/*
			 * ``Note that 0 is taken as a flag, not as the
			 * beginning of a field width.''
			 *	-- ANSI X3J11
			 */
			flags |= ZEROPAD;
			goto rflag;
		case L'1': case L'2': case L'3': case L'4':
		case L'5': case L'6': case L'7': case L'8': case L'9':
			n = 0;
			do {
				n = 10 * n + to_wdigit(ch);
				ch = *fmt++;
			} while (is_wdigit(ch));
			if (ch == L'$') {
				nextarg = n;
                        	if (argtable == NULL) {
                                	argtable = statargtable;
                                	__wfind_arguments (fmt0, orgap,
						&argtable);
				}
				goto rflag;
                        }
			width = n;
			goto reswitch;
#ifdef FLOATING_POINT
		case L'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case L'h':
			flags |= SHORTINT;
			goto rflag;
		case L'l':
			if (flags & LONGINT)
				flags |= QUADINT;
			else
				flags |= LONGINT;
			goto rflag;
		case L'q':
			flags |= QUADINT;
			goto rflag;
		case L'c':
			*(cp = buf) = GETARG(int);
			size = 1;
			sign = L'\0';
			break;
		case L'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'd':
		case L'i':
            if (flags & QUADINT) {
                uqval = GETARG(quad_t);
                if ((quad_t)uqval < 0) {
                    uqval = (u_quad_t)(-((quad_t)uqval));
                    sign = L'-';
                }
            } else {
                ulval = SARG();
                if ((long)ulval < 0) {
                    ulval = (unsigned long)(-((long)ulval));
                    sign = L'-';
                }
            }
			base = 10;
			goto number;
#ifdef FLOATING_POINT
		case L'e':
		case L'E':
		case L'f':
			goto fp_begin;
		case L'g':
		case L'G':
			if (prec == 0)
				prec = 1;
fp_begin:		if (prec == -1)
				prec = DEFPREC;
			if (flags & LONGDBL)
				/* XXX this loses precision. */
				_double = (double)GETARG(long double);
			else
				_double = GETARG(double);
			/* do this before tricky precision changes */
			if (isinf(_double)) {
				if (_double < 0)
					sign = L'-';
				cp = L"Inf";
				size = 3;
				break;
			}
			if (isnan(_double)) {
				cp = L"NaN";
				size = 3;
				break;
			}
			flags |= FPT;
			cp = wcvt(_double, prec, flags, &softsign,
				&expt, ch, &ndig);
			if (ch == L'g' || ch == L'G') {
				if (expt <= -4 || expt > prec)
					ch = (ch == L'g') ? L'e' : L'E';
				else
					ch = L'g';
			}
			if (ch <= L'e') {	/* L'e' or L'E' fmt */
				--expt;
				expsize = wexponent(expstr, expt, ch);
				size = expsize + ndig;
				if (ndig > 1 || flags & ALT)
					++size;
			} else if (ch == L'f') {		/* f fmt */
				if (expt > 0) {
					size = expt;
					if (prec || flags & ALT)
						size += prec + 1;
				} else	/* L"0.X" */
					size = prec + 2;
			} else if (expt >= ndig) {	/* fixed g fmt */
				size = expt;
				if (flags & ALT)
					++size;
			} else
				size = ndig + (expt > 0 ?
					1 : 2 - expt);

			if (softsign)
				sign = L'-';
			break;
#endif /* FLOATING_POINT */
		case L'n':
			if (flags & QUADINT)
				*GETARG(quad_t *) = ret;
			else if (flags & LONGINT)
				*GETARG(long *) = ret;
			else if (flags & SHORTINT)
				*GETARG(short *) = ret;
			else
				*GETARG(int *) = ret;
			continue;	/* no output */
		case L'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'o':
			if (flags & QUADINT)
				uqval = GETARG(u_quad_t);
			else
				ulval = UARG();
			base = 8;
			goto nosign;
		case L'p':
			/*
			 * ``The argument shall be a pointer to void.  The
			 * value of the pointer is converted to a sequence
			 * of printable characters, in an implementation-
			 * defined manner.''
			 *	-- ANSI X3J11
			 */
            if ( sizeof(void*) > sizeof(ulval)) {
            	uqval = (u_quad_t)GETARG(void *);
            	flags = (flags | QUADINT);
            } else {
            	ulval = (u_long)GETARG(u_long);	//yuk - should be void*
            	flags = (flags & ~QUADINT);
            }
			base = 16;
			xdigs = L"0123456789abcdef";
			flags = flags | HEXPREFIX;
			ch = L'x';
			goto nosign;
		case L's':
			if ((cp = GETARG(wchar_t *)) == NULL)
				cp = L"(null)";
			if (prec >= 0) {
				/*
				 * can't use strlen; can only look for the
				 * NUL in the first `prec' characters, and
				 * strlen() will go further.
				 */
				wchar_t *p = wmemchr(cp, 0, (size_t)prec);

				if (p != NULL) {
					size = (int)(p - cp);
					if (size > prec)
						size = prec;
				} else
					size = prec;
			} else
				size = (int)wcslen(cp);
			sign = L'\0';
			break;
		case L'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'u':
			if (flags & QUADINT)
				uqval = GETARG(u_quad_t);
			else
				ulval = UARG();
			base = 10;
			goto nosign;
		case L'X':
			xdigs = L"0123456789ABCDEF";
			goto hex;
		case L'x':
			xdigs = L"0123456789abcdef";
hex:			if (flags & QUADINT)
				uqval = GETARG(u_quad_t);
			else
				ulval = UARG();
			base = 16;
			/* leading 0x/X only if non-zero */
			if (flags & ALT &&
			    (flags & QUADINT ? uqval != 0 : ulval != 0))
				flags |= HEXPREFIX;

			/* unsigned conversions */
nosign:			sign = L'\0';
			/*
			 * ``... diouXx conversions ... if a precision is
			 * specified, the 0 flag will be ignored.''
			 *	-- ANSI X3J11
			 */
number:			if ((dprec = prec) >= 0)
				flags &= ~ZEROPAD;

			/*
			 * ``The result of converting a zero value with an
			 * explicit precision of zero is no characters.''
			 *	-- ANSI X3J11
			 */
			cp = buf + BUF;
			if (flags & QUADINT) {
				if (uqval != 0 || prec != 0)
					cp = __uqtow(uqval, cp, base,
					    flags & ALT, xdigs);
			} else {
				if (ulval != 0 || prec != 0)
					cp = __ultow(ulval, cp, base,
					    flags & ALT, xdigs);
			}
			size = (int)(buf + BUF - cp);
			break;
		default:	/* L"%?" prints ?, unless ? is NUL */
			if (ch == L'\0')
				goto done;
			/* pretend it was %c with argument ch */
			cp = buf;
			*cp = ch;
			size = 1;
			sign = L'\0';
			break;
		}

		/*
		 * All reasonable formats wind up here.  At this point, `cp'
		 * points to a string which (if not flags&LADJUST) should be
		 * padded out to `width' places.  If flags&ZEROPAD, it should
		 * first be prefixed by any sign or other prefix; otherwise,
		 * it should be blank padded before the prefix is emitted.
		 * After any left-hand padding and prefixing, emit zeroes
		 * required by a decimal [diouxX] precision, then print the
		 * string proper, then emit zeroes required by any leftover
		 * floating precision; finally, if LADJUST, pad with blanks.
		 *
		 * Compute actual size, so we know how much to pad.
		 * size excludes decimal prec; realsz includes it.
		 */
		realsz = dprec > size ? dprec : size;
		if (sign)
			realsz++;
		else if (flags & HEXPREFIX)
			realsz += 2;

		prsize = width > realsz ? width : realsz;
		if ((unsigned)ret + prsize > INT_MAX) {
			ret = EOF;
			goto error;
		}

		/* right-adjusting blank padding */
		if ((flags & (LADJUST|ZEROPAD)) == 0)
			WPAD(width - realsz, blanks);

		/* prefix */
		if (sign) {
			WPRINT(&sign, 1);
		} else if (flags & HEXPREFIX) {
			ox[0] = L'0';
			ox[1] = ch;
			WPRINT(ox, 2);
		}

		/* right-adjusting zero padding */
		if ((flags & (LADJUST|ZEROPAD)) == ZEROPAD)
			WPAD(width - realsz, zeroes);

		/* leading zeroes from decimal precision */
		WPAD(dprec - size, zeroes);

		/* the string or number proper */
#ifdef FLOATING_POINT
		if ((flags & FPT) == 0) {
			WPRINT(cp, size);
		} else {	/* glue together f_p fragments */
			if (ch >= L'f') {	/* L'f' or L'g' */
				if (_double == 0) {
					/* kludge for __dtoa irregularity */
					if (expt >= ndig &&
					    (flags & ALT) == 0) {
						WPRINT(L"0", 1);
					} else {
						WPRINT(L"0.", 2);
						WPAD(ndig - 1, zeroes);
					}
				} else if (expt <= 0) {
					WPRINT(L"0.", 2);
					WPAD(-expt, zeroes);
					WPRINT(cp, ndig);
				} else if (expt >= ndig) {
					WPRINT(cp, ndig);
					WPAD(expt - ndig, zeroes);
					if (flags & ALT)
						WPRINT(L".", 1);
				} else {
					WPRINT(cp, expt);
					cp += expt;
					WPRINT(L".", 1);
					WPRINT(cp, ndig-expt);
				}
			} else {	/* L'e' or L'E' */
				if (ndig > 1 || flags & ALT) {
					ox[0] = *cp++;
					ox[1] = L'.';
					WPRINT(ox, 2);
					if (_double) {
						WPRINT(cp, ndig-1);
					} else	/* 0.[0..] */
						/* __dtoa irregularity */
						WPAD(ndig - 1, zeroes);
				} else	/* XeYYY */
					WPRINT(cp, 1);
				WPRINT(expstr, expsize);
			}
		}
#else
		WPRINT(cp, size);
#endif
		/* left-adjusting padding (always blank) */
		if (flags & LADJUST)
			WPAD(width - realsz, blanks);

		/* finally, adjust ret */
		ret += prsize;

		WFLUSH();	/* copy out the I/O vectors */
	}
done:
	WFLUSH();
error:
	if (__sferror(fp))
		ret = EOF;
	FUNLOCKFILE(fp);
        if ((argtable != NULL) && (argtable != statargtable))
                free (argtable);
	return (ret);
	/* NOTREACHED */
}

/*
 * Type ids for argument type table.
 */
#define T_UNUSED	0
#define T_SHORT		1
#define T_U_SHORT	2
#define TP_SHORT	3
#define T_INT		4
#define T_U_INT		5
#define TP_INT		6
#define T_LONG		7
#define T_U_LONG	8
#define TP_LONG		9
#define T_QUAD		10
#define T_U_QUAD	11
#define TP_QUAD		12
#define T_DOUBLE	13
#define T_LONG_DOUBLE	14
#define TP_CHAR		15
#define TP_VOID		16

/*
 * Find all arguments when a positional parameter is encountered.  Returns a
 * table, indexed by argument number, of pointers to each arguments.  The
 * initial argument table should be an array of STATIC_ARG_TBL_SIZE entries.
 * It will be replaces with a malloc-ed on if it overflows.
 */ 
static void
__wfind_arguments (fmt0, ap, argtable)
	const wchar_t *fmt0;
	va_list ap;
	void ***argtable;
{
	register wchar_t *fmt;	/* format string */
	register int ch;	/* character from fmt */
	register int n, n2;	/* handy integer (short term usage) */
	register wchar_t *cp;	/* handy char pointer (short term usage) */
	register int flags;	/* flags as above */
	int width;		/* width from format (%8d), or 0 */
	unsigned char *typetable; /* table of types */
	unsigned char stattypetable [STATIC_ARG_TBL_SIZE];
	int tablesize;		/* current size of type table */
	int tablemax;		/* largest used index in table */
	int nextarg;		/* 1-based argument index */

	/*
	 * Add an argument type to the table, expanding if necessary.
	 */
#define ADDTYPE(type) \
	((nextarg >= tablesize) ? \
		__grow_type_table(nextarg, &typetable, &tablesize) : (void)0, \
	typetable[nextarg++] = type, \
	(nextarg > tablemax) ? tablemax = nextarg : 0)

#define	ADDSARG() \
	((flags&LONGINT) ? ADDTYPE(T_LONG) : \
		((flags&SHORTINT) ? ADDTYPE(T_SHORT) : ADDTYPE(T_INT)))

#define	ADDUARG() \
	((flags&LONGINT) ? ADDTYPE(T_U_LONG) : \
		((flags&SHORTINT) ? ADDTYPE(T_U_SHORT) : ADDTYPE(T_U_INT)))

	/*
	 * Add * arguments to the type array.
	 */
#define WADDASTER() \
	n2 = 0; \
	cp = fmt; \
	while (is_wdigit(*cp)) { \
		n2 = 10 * n2 + to_wdigit(*cp); \
		cp++; \
	} \
	if (*cp == L'$') { \
		int hold = nextarg; \
		nextarg = n2; \
		ADDTYPE (T_INT); \
		nextarg = hold; \
		fmt = ++cp; \
	} else { \
		ADDTYPE (T_INT); \
	}
	fmt = (wchar_t *)fmt0;
	typetable = stattypetable;
	tablesize = STATIC_ARG_TBL_SIZE;
	tablemax = 0; 
	nextarg = 1;
	memset (typetable, T_UNUSED, STATIC_ARG_TBL_SIZE);

	/*
	 * Scan the format for conversions (L`%' character).
	 */
	for (;;) {
		for (cp = fmt; (ch = *fmt) != L'\0' && ch != L'%'; fmt++)
			/* void */;
		if (ch == L'\0')
			goto done;
		fmt++;		/* skip over L'%' */

		flags = 0;
		width = 0;

rflag:		ch = *fmt++;
reswitch:	switch (ch) {
		case L' ':
		case L'#':
			goto rflag;
		case L'*':
			WADDASTER ();
			goto rflag;
		case L'-':
		case L'+':
			goto rflag;
		case L'.':
			if ((ch = *fmt++) == L'*') {
				WADDASTER ();
				goto rflag;
			}
			while (is_wdigit(ch)) {
				ch = *fmt++;
			}
			goto reswitch;
		case L'0':
			goto rflag;
		case L'1': case L'2': case L'3': case L'4':
		case L'5': case L'6': case L'7': case L'8': case L'9':
			n = 0;
			do {
				n = 10 * n + to_wdigit(ch);
				ch = *fmt++;
			} while (is_wdigit(ch));
			if (ch == L'$') {
				nextarg = n;
				goto rflag;
			}
			width = n;
			goto reswitch;
#ifdef FLOATING_POINT
		case L'L':
			flags |= LONGDBL;
			goto rflag;
#endif
		case L'h':
			flags |= SHORTINT;
			goto rflag;
		case L'l':
			if (flags & LONGINT)
				flags |= QUADINT;
			else
				flags |= LONGINT;
			goto rflag;
		case L'q':
			flags |= QUADINT;
			goto rflag;
		case L'c':
			ADDTYPE(T_INT);
			break;
		case L'D':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'd':
		case L'i':
			if (flags & QUADINT) {
				ADDTYPE(T_QUAD);
			} else {
				ADDSARG();
			}
			break;
#ifdef FLOATING_POINT
		case L'e':
		case L'E':
		case L'f':
		case L'g':
		case L'G':
			if (flags & LONGDBL)
				ADDTYPE(T_LONG_DOUBLE);
			else
				ADDTYPE(T_DOUBLE);
			break;
#endif /* FLOATING_POINT */
		case L'n':
			if (flags & QUADINT)
				ADDTYPE(TP_QUAD);
			else if (flags & LONGINT)
				ADDTYPE(TP_LONG);
			else if (flags & SHORTINT)
				ADDTYPE(TP_SHORT);
			else
				ADDTYPE(TP_INT);
			continue;	/* no output */
		case L'O':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'o':
			if (flags & QUADINT)
				ADDTYPE(T_U_QUAD);
			else
				ADDUARG();
			break;
		case L'p':
			ADDTYPE(TP_VOID);
			break;
		case L's':
			ADDTYPE(TP_CHAR);
			break;
		case L'U':
			flags |= LONGINT;
			/*FALLTHROUGH*/
		case L'u':
			if (flags & QUADINT)
				ADDTYPE(T_U_QUAD);
			else
				ADDUARG();
			break;
		case L'X':
		case L'x':
			if (flags & QUADINT)
				ADDTYPE(T_U_QUAD);
			else
				ADDUARG();
			break;
		default:	/* L"%?" prints ?, unless ? is NUL */
			if (ch == L'\0')
				goto done;
			break;
		}
	}
done:
	/*
	 * Build the argument table.
	 */
	if (tablemax >= STATIC_ARG_TBL_SIZE) {
		*argtable = (void **)
		    malloc (sizeof (void *) * (tablemax + 1));
	}

	(*argtable) [0] = NULL;
	for (n = 1; n <= tablemax; n++) {
		switch (typetable [n]) {
		    case T_UNUSED:
			(*argtable) [n] = (void *) &va_arg (ap, int);
			break;
		    case T_SHORT:
			(*argtable) [n] = (void *) &va_arg (ap, int);
			break;
		    case T_U_SHORT:
			(*argtable) [n] = (void *) &va_arg (ap, int);
			break;
		    case TP_SHORT:
			(*argtable) [n] = (void *) &va_arg (ap, short *);
			break;
		    case T_INT:
			(*argtable) [n] = (void *) &va_arg (ap, int);
			break;
		    case T_U_INT:
			(*argtable) [n] = (void *) &va_arg (ap, unsigned int);
			break;
		    case TP_INT:
			(*argtable) [n] = (void *) &va_arg (ap, int *);
			break;
		    case T_LONG:
			(*argtable) [n] = (void *) &va_arg (ap, long);
			break;
		    case T_U_LONG:
			(*argtable) [n] = (void *) &va_arg (ap, unsigned long);
			break;
		    case TP_LONG:
			(*argtable) [n] = (void *) &va_arg (ap, long *);
			break;
		    case T_QUAD:
			(*argtable) [n] = (void *) &va_arg (ap, quad_t);
			break;
		    case T_U_QUAD:
			(*argtable) [n] = (void *) &va_arg (ap, u_quad_t);
			break;
		    case TP_QUAD:
			(*argtable) [n] = (void *) &va_arg (ap, quad_t *);
			break;
		    case T_DOUBLE:
			(*argtable) [n] = (void *) &va_arg (ap, double);
			break;
		    case T_LONG_DOUBLE:
			(*argtable) [n] = (void *) &va_arg (ap, long double);
			break;
		    case TP_CHAR:
			(*argtable) [n] = (void *) &va_arg (ap, char *);
			break;
		    case TP_VOID:
			(*argtable) [n] = (void *) &va_arg (ap, void *);
			break;
		}
	}

	if ((typetable != NULL) && (typetable != stattypetable))
		free (typetable);
}

/*
 * Increase the size of the type table.
 */
static void
__grow_type_table (nextarg, typetable, tablesize)
	int nextarg;
	unsigned char **typetable;
	int *tablesize;
{
	unsigned char *oldtable = *typetable;
	int newsize = *tablesize * 2;

	if (*tablesize == STATIC_ARG_TBL_SIZE) {
		*typetable = (unsigned char *)
		    malloc (sizeof (unsigned char) * newsize);
		bcopy (oldtable, *typetable, *tablesize);
	} else {
		*typetable = (unsigned char *)
		    reallocf (typetable, sizeof (unsigned char) * newsize);

	}
	memset (&typetable [*tablesize], T_UNUSED, (newsize - *tablesize));

	*tablesize = newsize;
}


#ifdef FLOATING_POINT

extern char *__dtoa __P((double, int, int, int *, int *, char **));



static wchar_t *
wcvt(value, ndigits, flags, sign, decpt, ch, length)
	double value;
	int ndigits, flags, *decpt, ch, *length;
	wchar_t *sign;
{
	int mode, dsgn;
    char *digits, *bp, *rve;
    static wchar_t wdigits[BUFSIZ];

	
	if (ch == L'f')
		mode = 3;		/* ndigits after the decimal point */
	else {
		/*
		 * To obtain ndigits after the decimal point for the 'e'
		 * and 'E' formats, round to ndigits + 1 significant
		 * figures.
		 */
		if (ch == L'e' || ch == L'E')
			ndigits++;
		mode = 2;		/* ndigits significant digits */
	}
	if (value < 0) {
		value = -value;
		*sign = L'-';
	} else
		*sign = L'\000';
//	digits = __dtow(value, mode, ndigits, decpt, &dsgn, &rve);
	digits = __dtoa(value, mode, ndigits, decpt, &dsgn, &rve);
	
	if ((ch != L'g' && ch != L'G') || flags & ALT) {
		/* print trailing zeros */
		bp = digits + ndigits;
		if (ch == L'f') {
			if (*digits == '0' && value)
				*decpt = -ndigits + 1;
			bp += *decpt;
		}
		if (value == 0)	/* kludge for __dtoa irregularity */
			rve = bp;
		while (rve < bp)
			*rve++ = '0';
	}
	*length = (int)(rve - digits);
	mbstowcs(wdigits, digits, BUFSIZ);
	return (wdigits);
}

static int
wexponent(p0, exp, fmtch)
	wchar_t *p0;
	int exp, fmtch;
{
	register wchar_t *p, *t;
	wchar_t expbuf[MAXEXP];

	p = p0;
	*p++ = fmtch;
	if (exp < 0) {
		exp = -exp;
		*p++ = L'-';
	}
	else
		*p++ = L'+';
	t = expbuf + MAXEXP;
	if (exp > 9) {
		do {
			*--t = (wchar_t)to_wchar(exp % 10);
		} while ((exp /= 10) > 9);
		*--t = (wchar_t)to_wchar(exp);
		for (; t < expbuf + MAXEXP; *p++ = *t++);
	}
	else {
		*p++ = L'0';
		*p++ = (wchar_t)to_wchar(exp);
	}
	return (int)(p - p0);
}
#endif /* FLOATING_POINT */
